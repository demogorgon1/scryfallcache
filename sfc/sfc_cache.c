#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_cache.h>
#include <sfc/sfc_card.h>
#include <sfc/sfc_card_array.h>
#include <sfc/sfc_query.h>

#include "sfc_buffer.h"
#include "sfc_card_map_uint32.h"
#include "sfc_card_set.h"
#include "sfc_deserializer.h"
#include "sfc_file.h"
#include "sfc_leaky_bucket.h"
#include "sfc_serializer.h"
#include "sfc_string_set.h"

sfc_cache* 
sfc_cache_create(	
	struct _sfc_app*			app,
	uint64_t					max_http_requests_per_second)
{
	assert(app->alloc != NULL);
	assert(app->free != NULL);
	assert(app->http_get != NULL); 
	assert(app->http_create_context != NULL);
	assert(app->http_destroy_context != NULL);
	assert(app->http_update_context != NULL);
	assert(app->http_poll != NULL);
	assert(app->get_timer != NULL);

	sfc_cache* cache = (sfc_cache*)SFC_ALLOC(app->alloc,app->user_data, NULL, sizeof(sfc_cache));
	assert(cache != NULL);
	memset(cache, 0, sizeof(sfc_cache));

	cache->app = app;

	cache->http_request_rate_limiter = sfc_leaky_bucket_create(app, max_http_requests_per_second, max_http_requests_per_second);
	cache->full_sets = sfc_string_set_create(app);
	cache->bad_urls = sfc_string_set_create(app);
	cache->card_set = sfc_card_set_create(app);
	cache->cardmarket_id_map = sfc_card_map_uint32_create(app, 1);

	cache->http_context = app->http_create_context(app);

	for(int i = 0; i < SFC_CARD_STRING_COUNT; i++)
		cache->string_filter[i] = SFC_TRUE;

	return cache;
}

void			
sfc_cache_destroy(
	sfc_cache*					cache)
{
	sfc_cache_clear(cache);

	sfc_leaky_bucket_destroy(cache->http_request_rate_limiter);
	sfc_string_set_destroy(cache->full_sets);
	sfc_string_set_destroy(cache->bad_urls);
	sfc_card_set_destroy(cache->card_set);
	sfc_card_map_uint32_destroy(cache->cardmarket_id_map);

	cache->app->http_destroy_context(cache->http_context);

	while (cache->first_query != NULL)
	{
		sfc_query* next = cache->first_query->next;

		sfc_card_array_destroy(cache->first_query->results);
		cache->app->free(cache->app->user_data, cache->first_query);
		
		cache->first_query = next;
	}

	cache->app->free(cache->app->user_data, cache);
}

void					
sfc_cache_set_string_filter(
	sfc_cache*					cache,
	const sfc_card_string*		strings,
	size_t						count)
{
	for (int i = 0; i < SFC_CARD_STRING_COUNT; i++)
		cache->string_filter[i] = SFC_FALSE;

	for(size_t i = 0; i < count; i++)
	{
		sfc_card_string string = strings[i];

		assert(string >= 0 && string < SFC_CARD_STRING_COUNT);

		cache->string_filter[string] = SFC_TRUE;
	}
}

struct _sfc_query* 
sfc_cache_create_query(
	sfc_cache*					cache)
{
	sfc_query* query = (sfc_query*)SFC_ALLOC(cache->app->alloc, cache->app->user_data, NULL, sizeof(sfc_query));
	assert(query != NULL);
	memset(query, 0, sizeof(sfc_query));

	query->results = sfc_card_array_create(cache->app);

	/* Insert in the end of double linked list */
	if (cache->last_query != NULL)
	{
		cache->last_query->next = query;
	}
	else
	{
		cache->first_query = query;
	}

	query->prev = cache->last_query;
	cache->last_query = query;

	return query;
}

struct _sfc_query*
sfc_cache_query_cardmarket_id(
	sfc_cache*					cache,
	uint32_t					cardmarket_id)
{
	sfc_query* query = sfc_cache_create_query(cache);

	sfc_query_init_cardmarket_id(query, cache, cardmarket_id);

	return query;
}

struct _sfc_query*
sfc_cache_query_card_key(
	sfc_cache*					cache,
	const sfc_card_key*			card_key)
{
	sfc_query* query = sfc_cache_create_query(cache);

	sfc_query_init_card_key(query, cache, card_key);

	return query;
}

struct _sfc_query*
sfc_cache_query_set(
	sfc_cache*					cache,
	const char*					set)
{
	sfc_query* query = sfc_cache_create_query(cache);

	sfc_query_init_set(query, cache, set);

	return query;
}

sfc_result			
sfc_cache_update(
	sfc_cache*					cache)
{
	/* Update http context */
	{
		sfc_result result = cache->app->http_update_context(cache->http_context);
		if(result != SFC_RESULT_OK)
			return result;
	}

	/* Update queries */
	{
		sfc_query* query = cache->first_query;

		while (query != NULL)
		{
			sfc_query_update(query);

			sfc_query* next_query = query->next;

			if (query->state == SFC_QUERY_STATE_DELETE)
			{
				sfc_card_array_destroy(query->results);

				/* Remove from double linked list and delete */
				if (query->prev != NULL)
					query->prev->next = query->next;
				else
					cache->first_query = query->next;

				if (query->next != NULL)
					query->next->prev = query->prev;
				else
					cache->last_query = query->prev;

				cache->app->free(cache->app->user_data, query);
			}

			query = next_query;
		}
	}

	return SFC_RESULT_OK;
}

sfc_result
sfc_cache_add_card(
	sfc_cache*					cache,
	sfc_card*					card)
{
	{
		sfc_result result = sfc_card_set_add(cache->card_set, card);
		if (result != SFC_RESULT_OK)
			return result;
	}

	if(card->data.flags & SFC_CARD_CARDMARKET_ID)
	{
		sfc_result result = sfc_card_map_uint32_set(cache->cardmarket_id_map, card->data.cardmarket_id, card);
		if (result != SFC_RESULT_OK)
			return result;
	}

	if(cache->last_card != NULL)
		cache->last_card->next = card;
	else 
		cache->first_card = card;

	cache->last_card = card;

	return SFC_RESULT_OK;
}

sfc_result		
sfc_cache_save(
	sfc_cache*					cache,
	const char*					path)
{
	sfc_result result = SFC_RESULT_OK;
	char temp_path[256];

	int required = snprintf(temp_path, sizeof(temp_path), "%s.tmp", path);
	if(required > (int)sizeof(temp_path))
		return SFC_RESULT_BUFFER_TOO_SMALL;

	{
		FILE* f = fopen(temp_path, "wb");
		if (f == NULL)
			return SFC_RESULT_FILE_OPEN_ERROR;

		/* First part: header, list of full sets, and list of bad urls */
		{
			sfc_buffer buffer;
			sfc_buffer_init(&buffer, cache->app);

			{
				sfc_serializer serializer;
				sfc_serializer_init(&serializer, &buffer);

				sfc_serializer_write_uint8(&serializer, 3); /* Version */
				
				sfc_string_set_serialize(cache->full_sets, &serializer);
				sfc_string_set_serialize(cache->bad_urls, &serializer);
			}

			result = sfc_file_write_buffer(f, &buffer);

			sfc_buffer_uninit(&buffer);
		}

		/* Second part: cards */
		for (sfc_card* card = cache->first_card; card != NULL && result == SFC_RESULT_OK; card = card->next)
		{
			sfc_buffer buffer;
			sfc_buffer_init(&buffer, cache->app);

			sfc_serializer serializer;
			sfc_serializer_init(&serializer, &buffer);

			sfc_card_serialize(card, &serializer);

			result = sfc_file_write_buffer(f, &buffer);

			sfc_buffer_uninit(&buffer);
		}

		fclose(f);
	}

	if (result == SFC_RESULT_OK)
		result = sfc_file_rename(temp_path, path);

	return result;
}

sfc_result		
sfc_cache_load(
	sfc_cache*					cache,
	const char*					path)
{
	sfc_result result = SFC_RESULT_OK;	

	sfc_cache_clear(cache);

	{
		FILE* f = fopen(path, "rb");
		if (f == NULL)
			return SFC_RESULT_FILE_OPEN_ERROR;

		/* First part: header and list of full sets */
		uint8_t format_version = 0;

		{
			sfc_buffer buffer;
			sfc_buffer_init(&buffer, cache->app);

			result = sfc_file_read_buffer(f, &buffer);

			if(result == SFC_RESULT_OK)
			{
				sfc_deserializer deserializer;
				sfc_deserializer_init(&deserializer, buffer.p, buffer.size);

				result = sfc_deserializer_read_uint8(&deserializer, &format_version);

				if (format_version != 2 && format_version != 3)
				{
					/* We can't read this old (or new?) version, saved cached is invalidated. */
					/* Behave like if the file didn't exist. */
					result = SFC_RESULT_FILE_OPEN_ERROR;
				}

				if(result == SFC_RESULT_OK)
					result = sfc_string_set_deserialize(cache->full_sets, &deserializer);

				if(result == SFC_RESULT_OK && format_version >= 3)
					result = sfc_string_set_deserialize(cache->bad_urls, &deserializer);
			}

			sfc_buffer_uninit(&buffer);
		}

		/* Second part: cards */
		while(result == SFC_RESULT_OK)
		{
			sfc_buffer buffer;
			sfc_buffer_init(&buffer, cache->app);

			result = sfc_file_read_buffer(f, &buffer);
			if (result != SFC_RESULT_OK)
			{
				if(result == SFC_RESULT_FILE_READ_ERROR)
				{
					// End of file, no worries
					result = SFC_RESULT_OK;
				}

				sfc_buffer_uninit(&buffer);
				break;
			}

			sfc_deserializer deserializer;
			sfc_deserializer_init(&deserializer, buffer.p, buffer.size);

			sfc_card* card = sfc_card_create(cache->app, cache->string_filter);

			result = sfc_card_deserialize(card, &deserializer);
			if(result != SFC_RESULT_OK)
			{
				sfc_buffer_uninit(&buffer);
				sfc_card_destroy(card);
				break;
			}

			sfc_cache_add_card(cache, card);

			sfc_buffer_uninit(&buffer);
		}

		fclose(f);
	}

	return result;
}

void			
sfc_cache_clear(
	sfc_cache*					cache)
{
	while (cache->first_card != NULL)
	{
		sfc_card* next = cache->first_card->next;

		sfc_card_destroy(cache->first_card);

		cache->first_card = next;
	}

	cache->last_card = NULL;

	sfc_string_set_clear(cache->full_sets);
	sfc_card_set_clear(cache->card_set);
	sfc_card_map_uint32_clear(cache->cardmarket_id_map);
}

int				
sfc_cache_has_full_set(
	sfc_cache*					cache,
	const char*					set)
{
	return sfc_string_set_has(cache->full_sets, set);
}

size_t			
sfc_cache_card_count(
	sfc_cache*					cache)
{
	return cache->card_set->card_array->count;
}

