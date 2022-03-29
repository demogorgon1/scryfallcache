#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_cache.h>
#include <sfc/sfc_card.h>
#include <sfc/sfc_card_array.h>
#include <sfc/sfc_query.h>

#include "sfc_card_map_uint32.h"
#include "sfc_card_set.h"
#include "sfc_leaky_bucket.h"
#include "sfc_string_set.h"

/* Update state: SFC_QUERY_STATE_INIT */
static void
sfc_query_update_init(
	sfc_query*			query)
{
	switch(query->type)
	{
	case SFC_QUERY_TYPE_CARDMARKET_ID:
		sfc_card* card = sfc_card_map_uint32_get(query->cache->cardmarket_id_map, query->cardmarket_id);

		if(card != NULL)
		{
			sfc_card_array_add(query->results, card);

			query->result = SFC_RESULT_OK;
			query->state = SFC_QUERY_STATE_COMPLETED;
		}
		else
		{
			int required = snprintf(query->http_get_url, sizeof(query->http_get_url), "https://api.scryfall.com/cards/cardmarket/%u", query->cardmarket_id);
			if(required > sizeof(query->http_get_url))
			{
				query->result = SFC_RESULT_BUFFER_TOO_SMALL;
				query->state = SFC_QUERY_STATE_COMPLETED;
			}
			else
			{
				query->state = SFC_QUERY_STATE_HTTP_GET;
			}			
		}
		break;

	case SFC_QUERY_TYPE_CARD_KEY:
		{
			sfc_card* card = sfc_card_set_get(query->cache->card_set, &query->card_key);
			if(card != NULL)
			{
				sfc_card_array_add(query->results, card);

				query->state = SFC_QUERY_STATE_COMPLETED;
				query->result = SFC_RESULT_OK;
			}
			else
			{
				char temp[1024];
				size_t offset = 0;

				for (const char* p = query->card_key.name; *p != '\0'; p++)
				{
					size_t remaining = sizeof(temp) - 1 - offset;
					char c = *p;

					if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
					{
						if (remaining < 1)
						{
							query->state = SFC_QUERY_STATE_COMPLETED;
							query->result = SFC_RESULT_BUFFER_TOO_SMALL;
							return;
						}

						temp[offset++] = c;
					}
					else
					{
						if (remaining < 3)
						{
							query->state = SFC_QUERY_STATE_COMPLETED;
							query->result = SFC_RESULT_BUFFER_TOO_SMALL;
							return;
						}

						int required = snprintf(temp + offset, remaining, "%c%02X", '%', c);
						assert(required == 3);

						offset += 3;
					}
				}

				temp[offset] = '\0';

				int required = snprintf(query->http_get_url, sizeof(query->http_get_url), "https://api.scryfall.com/cards/named?exact=%s&set=%s", temp, query->card_key.set);
				if(required > sizeof(query->http_get_url))
				{
					query->state = SFC_QUERY_STATE_COMPLETED;
					query->result = SFC_RESULT_BUFFER_TOO_SMALL;
				}
				else
				{
					query->state = SFC_QUERY_STATE_HTTP_GET;
				}				
			}
		}
		break;

	case SFC_QUERY_TYPE_SET:
		if(sfc_cache_has_full_set(query->cache, query->set))
		{
			for (sfc_card* card = query->cache->first_card; card != NULL; card = card->next)
			{
				if (strcmp(card->key.set, query->set) == 0)
					sfc_card_array_add(query->results, card);						
			}

			query->result = SFC_RESULT_OK;
			query->state = SFC_QUERY_STATE_COMPLETED;
		}
		else
		{
			query->next_collector_number = 1;
			query->next_collector_number_version = 0;
			query->state = SFC_QUERY_STATE_GET_NEXT_IN_SET;
		}
		break;

	default:
		assert(0);
		break;
	}
}

/* Update state: SFC_QUERY_STATE_GET_NEXT_IN_SET */
static void
sfc_query_update_get_next_in_set(
	sfc_query*			query)
{
	assert(query->type == SFC_QUERY_TYPE_SET);
	
	/* Check if we have it in cache already */
	int in_cache = 0;

	for (sfc_card* card = query->cache->first_card; card != NULL; card = card->next)
	{
		if (strcmp(card->key.set, query->set) == 0 &&
			card->key.version == query->next_collector_number_version &&
			card->data.flags & SFC_CARD_COLLECTOR_NUMBER &&
			card->data.collector_number == query->next_collector_number)
		{
			if (query->next_collector_number_version > 0)
				query->next_collector_number_version++;
			else
				query->next_collector_number++;

			sfc_card_array_add(query->results, card);

			in_cache = 1;
			break;
		}
	}

	if (!in_cache)
	{
		int required = snprintf(
			query->http_get_url,
			sizeof(query->http_get_url),
			"https://api.scryfall.com/cards/%s/%u%c",
			query->set,
			query->next_collector_number,
			query->next_collector_number_version > 0 ? 'a' + query->next_collector_number_version - 1 : '\0');

		if (required > sizeof(query->http_get_url))
		{
			query->result = SFC_RESULT_BUFFER_TOO_SMALL;
			query->state = SFC_QUERY_STATE_COMPLETED;
		}
		else
		{
			query->state = SFC_QUERY_STATE_HTTP_GET;
		}
	}
}

/* Update state: SFC_QUERY_STATE_HTTP_GET */
static void
sfc_query_update_http_get(
	sfc_query*			query)
{
	if (!sfc_leaky_bucket_acquire_token(query->cache->http_request_rate_limiter))
		return;

	assert(query->http_get_result == NULL);
	assert(query->http_get_result_size == 0);

	sfc_result result = query->cache->app->http_get(
		query->cache->http_context,
		query->http_get_url,
		&query->http_get_result,
		&query->http_get_result_size);

	if (result == SFC_RESULT_OK)
	{
		query->state = SFC_QUERY_STATE_PARSE_RESULT;
	}
	else
	{		
		query->result = result;
		query->state = SFC_QUERY_STATE_COMPLETED;
	}
}

/* Update state: SFC_QUERY_STATE_PARSE_RESULT */
static void
sfc_query_update_parse_result(
	sfc_query*			query)
{
	sfc_card* card = sfc_card_create(query->cache->app, query->cache->string_filter);

	sfc_result result = sfc_card_parse_json(card, query->http_get_result, query->http_get_result_size);

	assert(query->http_get_result != NULL);

	if (result != SFC_RESULT_OK)
	{
		query->cache->app->free(query->cache->app->user_data, query->http_get_result);

		sfc_card_destroy(card);
	}

	query->http_get_result = NULL;
	query->http_get_result_size = 0;

	if (query->type == SFC_QUERY_TYPE_SET)
	{
		if (result == SFC_RESULT_OK)
		{
			sfc_card* existing = sfc_card_set_get(query->cache->card_set, &card->key);
			if (existing != NULL)
			{
				/* Another query has added this to the cache since we started... return that one instead so we don't
					end up with duplicates. */
				sfc_card_array_add(query->results, existing);
				sfc_card_destroy(card);
			}
			else
			{
				sfc_cache_add_card(query->cache, card);
				sfc_card_array_add(query->results, card);
			}

			if (query->next_collector_number_version > 0)
				query->next_collector_number_version++;
			else
				query->next_collector_number++;

			query->state = SFC_QUERY_STATE_GET_NEXT_IN_SET;
		}
		else if (result == SFC_RESULT_NOT_FOUND)
		{
			if (query->next_collector_number_version > 0)
			{
				if (query->next_collector_number_version == 1)
				{
					/* Must have reached the end since neither "x" nor "xa" exists. */
					query->result = SFC_RESULT_OK;
					query->state = SFC_QUERY_STATE_COMPLETED;

					/* We're now sure to have the full set, mark it as fully cached */
					sfc_string_set_add(query->cache->full_sets, query->set);
				}
				else
				{
					query->next_collector_number_version = 0;
					query->next_collector_number++;
					query->state = SFC_QUERY_STATE_GET_NEXT_IN_SET;
				}
			}
			else
			{
				query->next_collector_number_version = 1;
				query->state = SFC_QUERY_STATE_GET_NEXT_IN_SET;
			}
		}
		else
		{
			query->result = result;
			query->state = SFC_QUERY_STATE_COMPLETED;
		}
	}
	else
	{
		if (result == SFC_RESULT_OK)
		{
			sfc_card* existing = sfc_card_set_get(query->cache->card_set, &card->key);
			if (existing != NULL)
			{
				sfc_card_array_add(query->results, existing);
				sfc_card_destroy(card);

				card = existing;
			}
			else
			{
				sfc_cache_add_card(query->cache, card);
				sfc_card_array_add(query->results, card);
			}

			if (query->type == SFC_QUERY_TYPE_CARD_KEY && query->card_key.version != card->key.version)
			{
				/* Oh no, we requested another version than we got. Since the scryfall API doesn't allow specifying what version you want
					directly in the name search, we need to now get it with another request based on the collector's number. */

				if (query->card_key.version == 0)
				{
					query->result = SFC_RESULT_REQUEST_NEEDS_VERSION;
					query->state = SFC_QUERY_STATE_COMPLETED;
				}
				else
				{
					if ((card->data.flags & SFC_CARD_COLLECTOR_NUMBER) == 0)
					{
						query->result = SFC_RESULT_MISSING_COLLECTORS_NUMBER;
						query->state = SFC_QUERY_STATE_COMPLETED;
					}
					else
					{
						snprintf(
							query->http_get_url,
							sizeof(query->http_get_url) - 1,
							"https://api.scryfall.com/cards/%s/%u%c",
							query->card_key.set,
							card->data.collector_number,
							'a' + query->card_key.version - 1);

						query->state = SFC_QUERY_STATE_HTTP_GET;
					}
				}
			}
			else
			{
				query->result = SFC_RESULT_OK;
				query->state = SFC_QUERY_STATE_COMPLETED;
			}
		}
		else
		{
			query->result = result;
			query->state = SFC_QUERY_STATE_COMPLETED;
		}
	}
}

/*---------------------------------------------------------------------------*/

void	
sfc_query_init_cardmarket_id(
	sfc_query*			query,
	struct _sfc_cache*	cache,
	uint32_t			cardmarket_id)
{
	query->type = SFC_QUERY_TYPE_CARDMARKET_ID;
	query->state = SFC_QUERY_STATE_INIT;
	query->cache = cache;

	query->cardmarket_id = cardmarket_id;
}

void	
sfc_query_init_card_key(
	sfc_query*			query,
	struct _sfc_cache*	cache,
	const sfc_card_key* card_key)
{
	query->type = SFC_QUERY_TYPE_CARD_KEY;
	query->state = SFC_QUERY_STATE_INIT;
	query->cache = cache;

	memcpy(&query->card_key, card_key, sizeof(sfc_card_key));
}

void	
sfc_query_init_set(
	sfc_query*			query,
	struct _sfc_cache*	cache,
	const char*			set)
{
	query->type = SFC_QUERY_TYPE_SET;
	query->state = SFC_QUERY_STATE_INIT;
	query->cache = cache;

	strncpy(query->set, set, SFC_MAX_SET - 1);
}

void	
sfc_query_update(
	sfc_query*			query)
{
	switch(query->state)
	{
	case SFC_QUERY_STATE_INIT:					sfc_query_update_init(query); break;
	case SFC_QUERY_STATE_GET_NEXT_IN_SET:		sfc_query_update_get_next_in_set(query); break;
	case SFC_QUERY_STATE_HTTP_GET:				sfc_query_update_http_get(query); break;
	case SFC_QUERY_STATE_PARSE_RESULT:			sfc_query_update_parse_result(query); break;

	default:
		break;
	}
}

int		
sfc_query_is_completed(
	sfc_query*			query)
{
	return query->state == SFC_QUERY_STATE_COMPLETED;
}

void	
sfc_query_delete(
	sfc_query*			query)
{
	assert(query->state == SFC_QUERY_STATE_COMPLETED);

	query->state = SFC_QUERY_STATE_DELETE;
}


