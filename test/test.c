#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sfc/sfc.h>

#include "../sfc/sfc_buffer.h"
#include "../sfc/sfc_card_map_uint32.h"
#include "../sfc/sfc_card_set.h"
#include "../sfc/sfc_deserializer.h"
#include "../sfc/sfc_leaky_bucket.h"
#include "../sfc/sfc_serializer.h"

#define TEST_CARD_COUNT			200
#define TEST_QUERY_TIMEOUT		100 * 1000 /* Not actual time - timer is advanced automatically when queried */

/* https://api.scryfall.com/cards/cardmarket/5013
   https://api.scryfall.com/cards/named?exact=Castle&set=2ed */
const char* g_test_json_cardmarket_id_5013 =
	"{\"object\":\"card\",\"id\":\"2ea3db44-85c5-4201-a5c9-ec14a9d244d6\",\"oracle_id\":\"f3179c3c-7e53-44d2-b579-b9e677efe9d9\"," 
	"\"multiverse_ids\":[837],\"tcgplayer_id\":9000,\"cardmarket_id\":5013,\"name\":\"Castle\",\"lang\":\"en\",\"released_at\":"
	"\"1993-12-01\",\"uri\":\"https://api.scryfall.com/cards/2ea3db44-85c5-4201-a5c9-ec14a9d244d6\",\"scryfall_uri\":\"https://"
	"scryfall.com/card/2ed/9/castle?utm_source=api\",\"layout\":\"normal\",\"highres_image\":true,\"image_status\":\"highres_sca"
	"n\",\"image_uris\":{\"small\":\"https://c1.scryfall.com/file/scryfall-cards/small/front/2/e/2ea3db44-85c5-4201-a5c9-ec14a9d2"
	"44d6.jpg?1559592123\",\"normal\":\"https://c1.scryfall.com/file/scryfall-cards/normal/front/2/e/2ea3db44-85c5-4201-a5c9-ec1"
	"4a9d244d6.jpg?1559592123\",\"large\":\"https://c1.scryfall.com/file/scryfall-cards/large/front/2/e/2ea3db44-85c5-4201-a5c9-ec"
	"14a9d244d6.jpg?1559592123\",\"png\":\"https://c1.scryfall.com/file/scryfall-cards/png/front/2/e/2ea3db44-85c5-4201-a5c9-ec14"
	"a9d244d6.png?1559592123\",\"art_crop\":\"https://c1.scryfall.com/file/scryfall-cards/art_crop/front/2/e/2ea3db44-85c5-4201-a5"
	"c9-ec14a9d244d6.jpg?1559592123\",\"border_crop\":\"https://c1.scryfall.com/file/scryfall-cards/border_crop/front/2/e/2ea3db44"
	"-85c5-4201-a5c9-ec14a9d244d6.jpg?1559592123\"},\"mana_cost\":\"{3}{W}\",\"cmc\":4.0,\"type_line\":\"Enchantment\",\"oracle_te"
	"xt\":\"Untapped creatures you control get +0/+2.\",\"colors\":[\"W\"],\"color_identity\":[\"W\"],\"keywords\":[],\"legalitie"
	"s\":{\"standard\":\"not_legal\",\"future\":\"not_legal\",\"historic\":\"not_legal\",\"gladiator\":\"not_legal\",\"pioneer\":"
	"\"not_legal\",\"modern\":\"not_legal\",\"legacy\":\"legal\",\"pauper\":\"not_legal\",\"vintage\":\"legal\",\"penny\":\"not_le"
	"gal\",\"commander\":\"legal\",\"brawl\":\"not_legal\",\"historicbrawl\":\"not_legal\",\"alchemy\":\"not_legal\",\"paupercomma"
	"nder\":\"not_legal\",\"duel\":\"legal\",\"oldschool\":\"legal\",\"premodern\":\"legal\"},\"games\":[\"paper\"],\"reserved\":f"
	"alse,\"foil\":false,\"nonfoil\":true,\"finishes\":[\"nonfoil\"],\"oversized\":false,\"promo\":false,\"reprint\":true,\"variat"
	"ion\":false,\"set_id\":\"cd7694b9-339c-405d-a991-14413d4f6d5c\",\"set\":\"2ed\",\"set_name\":\"Unlimited Edition\",\"set_type"
	"\":\"core\",\"set_uri\":\"https://api.scryfall.com/sets/cd7694b9-339c-405d-a991-14413d4f6d5c\",\"set_search_uri\":\"https://"
	"api.scryfall.com/cards/search?order=set\\u0026q=e%3A2ed\\u0026unique=prints\",\"scryfall_set_uri\":\"https://scryfall.com/set"
	"s/2ed?utm_source=api\",\"rulings_uri\":\"https://api.scryfall.com/cards/2ea3db44-85c5-4201-a5c9-ec14a9d244d6/rulings\",\"prin"
	"ts_search_uri\":\"https://api.scryfall.com/cards/search?order=released\\u0026q=oracleid%3Af3179c3c-7e53-44d2-b579-b9e677efe9d"
	"9\\u0026unique=prints\",\"collector_number\":\"9\",\"digital\":false,\"rarity\":\"uncommon\",\"card_back_id\":\"0aeebaf5-8c7d"
	"-4636-9e82-8c27447861f7\",\"artist\":\"Dameon Willich\",\"artist_ids\":[\"606a37b0-9d2d-477b-b679-100bc30b46b3\"],\"illustrat"
	"ion_id\":\"bdc5099a-b1d6-46ec-aff5-86e3878ae897\",\"border_color\":\"white\",\"frame\":\"1993\",\"full_art\":false,\"textless"
	"\":false,\"booster\":true,\"story_spotlight\":false,\"edhrec_rank\":11617,\"prices\":{\"usd\":\"7.50\",\"usd_foil\":null,\"us"
	"d_etched\":null,\"eur\":\"7.80\",\"eur_foil\":null,\"tix\":null},\"related_uris\":{\"gatherer\":\"https://gatherer.wizards.co"
	"m/Pages/Card/Details.aspx?multiverseid=837\",\"tcgplayer_infinite_articles\":\"https://infinite.tcgplayer.com/search?contentM"
	"ode=article\\u0026game=magic\\u0026partner=scryfall\\u0026q=Castle\\u0026utm_campaign=affiliate\\u0026utm_medium=api\\u0026ut"
	"m_source=scryfall\",\"tcgplayer_infinite_decks\":\"https://infinite.tcgplayer.com/search?contentMode=deck\\u0026game=magic\\u"
	"0026partner=scryfall\\u0026q=Castle\\u0026utm_campaign=affiliate\\u0026utm_medium=api\\u0026utm_source=scryfall\",\"edhrec\":"
	"\"https://edhrec.com/route/?cc=Castle\",\"mtgtop8\":\"https://mtgtop8.com/search?MD_check=1\\u0026SB_check=1\\u0026cards=Cast"
	"le\"},\"purchase_uris\":{\"tcgplayer\":\"https://www.tcgplayer.com/product/9000?page=1\\u0026utm_campaign=affiliate\\u0026utm"
	"_medium=api\\u0026utm_source=scryfall\",\"cardmarket\":\"https://www.cardmarket.com/en/Magic/Products/Search?referrer=scryfal"
	"l\\u0026searchString=Castle\\u0026utm_campaign=card_prices\\u0026utm_medium=text\\u0026utm_source=scryfall\",\"cardhoarder\":"
	"\"https://www.cardhoarder.com/cards?affiliate_id=scryfall\\u0026data%5Bsearch%5D=Castle\\u0026ref=card-profile\\u0026utm_camp"
	"aign=affiliate\\u0026utm_medium=card\\u0026utm_source=scryfall\"}}";

#define TEST_ASSERT(_expression) test_assert(_expression, #_expression, __FILE__, __LINE__)

void
test_assert(
	int			result,
	const char* message,
	const char* file,
	int			line)
{
	if (!result)
	{
		fprintf(stderr, "FAILED: %s (%s:%d)\n", message, file, line);

		exit(1);
	}
}

typedef struct _test_memory_linked_list
{
	struct _test_memory_header*	head;
	struct _test_memory_header* tail;
} test_memory_linked_list;

typedef struct _test_memory_header
{
	size_t						size;	
	const char*					file;
	int							line;
	struct _test_memory_header* next;
	struct _test_memory_header* prev;
} test_memory_header;

typedef struct _test_user_data
{
	size_t						total_alloc_size;
	size_t						total_alloc_count;
	test_memory_linked_list		allocations;

	int							auto_advance_timer;
	uint64_t					test_timer;

	size_t						total_http_request_count;
} test_user_data;

void
test_remove_allocation(
	test_user_data*				user_data,
	test_memory_header*			allocation)
{
	TEST_ASSERT(user_data->total_alloc_count > 0);
	TEST_ASSERT(user_data->total_alloc_size >= allocation->size);

	if(allocation->prev != NULL)
		allocation->prev->next = allocation->next;
	else
		user_data->allocations.head = allocation->next;

	if (allocation->next != NULL)
		allocation->next->prev = allocation->prev;
	else
		user_data->allocations.tail = allocation->prev;

	user_data->total_alloc_count--;
	user_data->total_alloc_size -= allocation->size;
}

void
test_debug_print_allocations(
	test_user_data*				user_data)
{
	printf("total allocations: %u (%u bytes)\n", (uint32_t)user_data->total_alloc_count, (uint32_t)user_data->total_alloc_size);

	for(test_memory_header* t = user_data->allocations.head; t != NULL; t = t->next)
		printf("%s:%d: %p (%u bytes)\n", t->file, t->line, (void*)(t + 1), (uint32_t)t->size);
}

void*
test_alloc(
	void*						user_data,
	void*						realloc_memory,
	size_t						size,
	const char*					file,
	int							line)
{
	test_memory_header* new_header = NULL;
	test_user_data* t = (test_user_data*)user_data;

	if(realloc_memory != NULL)
	{
		test_memory_header* old_header = (test_memory_header*)realloc_memory;
		old_header--;

		test_remove_allocation(t, old_header);

		new_header = (test_memory_header*)realloc(old_header, size + sizeof(test_memory_header));
	}
	else
	{
		new_header = (test_memory_header*)realloc(NULL, size + sizeof(test_memory_header));
	}

	assert(new_header != NULL);
	
	new_header->size = size;
	new_header->file = file;
	new_header->line = line;
	new_header->next = NULL;
	new_header->prev = t->allocations.tail;
	
	if(new_header->prev != NULL)
		new_header->prev->next = new_header;
	else
		t->allocations.head = new_header;

	t->allocations.tail = new_header;

	t->total_alloc_count++;
	t->total_alloc_size += size;

	new_header++;

	return new_header;
}

void
test_free(
	void*				user_data,
	void*				memory)
{
	test_user_data* t = (test_user_data*)user_data;

	test_memory_header* header = (test_memory_header*)memory;
	header--;

	test_remove_allocation(t, header);

	free(header);
}

uint64_t
test_get_timer(
	void*				user_data)
{
	test_user_data* t = (test_user_data*)user_data;

	if(!t->auto_advance_timer)
		return t->test_timer;

	return t->test_timer += 1000;
}

void
test_sleep(
	void*				user_data,
	uint32_t			sleep_ms)
{
	/* Do nothing */
	SFC_UNUSED(user_data);
	SFC_UNUSED(sleep_ms);
}

typedef struct _test_http_context
{
	sfc_app*			app;
	size_t				request_count;
} test_http_context;

typedef struct _test_http_request
{
	test_http_context*	context;
	char*				result;
	size_t				size;
} test_http_request;

void* 
test_http_context_create(
	struct _sfc_app*	app)
{
	test_http_context* context = (test_http_context*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(test_http_context));
	TEST_ASSERT(context != NULL);
	memset(context, 0, sizeof(test_http_context));

	context->app = app;

	return context;	
}

void			
test_http_context_destroy(
	void*				http_context)
{
	test_http_context* context = (test_http_context*)http_context;

	/* Make sure there are no pending requests */
	TEST_ASSERT(context->request_count == 0);
	
	context->app->free(context->app->user_data, http_context);
}

sfc_result
test_http_context_update(
	void*				http_context)
{
	SFC_UNUSED(http_context);

	return SFC_RESULT_OK;
}

char*
test_strdup_no_null_term(
	sfc_app*			app,
	const char*			string,
	size_t*				out_len)
{
	*out_len = strlen(string);
	char* new_string = (char*)SFC_ALLOC(app->alloc, app->user_data, NULL, *out_len);
	memcpy(new_string, string, *out_len);
	return new_string;
}

void*		
test_http_get(
	void*				http_context,
	const char*			url)
{
	test_http_context* context = (test_http_context*)http_context;

	test_http_request* req = (test_http_request*)SFC_ALLOC(context->app->alloc, context->app->user_data, NULL, sizeof(test_http_request));
	assert(req != NULL);
	memset(req, 0, sizeof(test_http_request));

	req->context = context;

	context->request_count++;

	test_user_data* user_data = (test_user_data*)context->app->user_data;

	user_data->total_http_request_count++;
	
	if(strcmp(url, "https://api.scryfall.com/cards/cardmarket/5013") == 0 ||
		strcmp(url, "https://api.scryfall.com/cards/2ed/9") == 0)
	{		
		req->result = test_strdup_no_null_term(context->app, g_test_json_cardmarket_id_5013, &req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/cardmarket/123456") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"error\", \"code\":\"not_found\"}",
			&req->size);
	}
	else if(strcmp(url, "https://api.scryfall.com/cards/test/1") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"card\", \"name\":\"test1\", \"set\":\"test\", \"collector_number\":\"1\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/2") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"error\", \"code\":\"not_found\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/2a") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"card\", \"name\":\"test2a\", \"set\":\"test\", \"collector_number\":\"2a\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/2b") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"card\", \"name\":\"test2b\", \"set\":\"test\", \"collector_number\":\"2b\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/2c") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"card\", \"name\":\"test2c\", \"set\":\"test\", \"collector_number\":\"2c\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/2d") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"error\", \"code\":\"not_found\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/3") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"card\", \"name\":\"test3\", \"set\":\"test\", \"collector_number\":\"3\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/4") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"error\", \"code\":\"not_found\"}",
			&req->size);
	}
	else if (strcmp(url, "https://api.scryfall.com/cards/test/4a") == 0)
	{
		req->result = test_strdup_no_null_term(context->app,
			"{\"object\":\"error\", \"code\":\"not_found\"}",
			&req->size);
	}
	else
	{
		TEST_ASSERT(0);
	}

	return req;
}

sfc_bool
test_http_poll(
	void*				http_request,
	sfc_result*			out_result,
	char**				out_data,
	size_t*				out_data_size)
{
	test_http_request* req = (test_http_request*)http_request;
	TEST_ASSERT(req->result != NULL);
	TEST_ASSERT(req->context->request_count > 0);

	*out_data = req->result;
	*out_data_size = req->size;
	*out_result = SFC_RESULT_OK;

	req->context->request_count--;

	req->context->app->free(req->context->app->user_data, req);

	return SFC_TRUE;
}

void
test_verify_cardmarket_5013(
	sfc_card*			card)
{
	TEST_ASSERT(strcmp(card->key.set, "2ed") == 0);
	TEST_ASSERT(card->key.collector_number == 9);
	TEST_ASSERT(card->key.version == 0);
	TEST_ASSERT(card->data.flags & SFC_CARD_CARDMARKET_ID);
	TEST_ASSERT(card->data.cardmarket_id == 5013);
	TEST_ASSERT(card->data.flags & SFC_CARD_TCGPLAYER_ID);
	TEST_ASSERT(card->data.tcgplayer_id == 9000);
	TEST_ASSERT(card->data.flags & SFC_CARD_COLOR_IDENTITY);
	TEST_ASSERT(card->data.color_identity == SFC_CARD_COLOR_WHITE);
	TEST_ASSERT(card->data.flags & SFC_CARD_COLORS);
	TEST_ASSERT(card->data.colors == SFC_CARD_COLOR_WHITE);
	TEST_ASSERT(strcmp(sfc_card_get_string(card, SFC_CARD_STRING_NAME), "Castle") == 0);
	TEST_ASSERT(strcmp(sfc_card_get_string(card, SFC_CARD_STRING_RELEASED_AT), "1993-12-01") == 0);
	TEST_ASSERT(strcmp(sfc_card_get_string(card, SFC_CARD_STRING_LANGUAGE), "en") == 0);
	TEST_ASSERT(strcmp(sfc_card_get_string(card, SFC_CARD_STRING_SET_NAME), "Unlimited Edition") == 0);
	TEST_ASSERT(strcmp(sfc_card_get_string(card, SFC_CARD_STRING_ARTIST), "Dameon Willich") == 0);
	TEST_ASSERT(strcmp(sfc_card_get_string(card, SFC_CARD_STRING_LEGALITY_PREMODERN), "legal") == 0);
}

void
test_init(
	test_user_data*		test_data,
	sfc_app*			app)
{
	memset(test_data, 0, sizeof(test_user_data));

	test_data->auto_advance_timer = SFC_TRUE;

	memset(app, 0, sizeof(test_user_data));

	app->alloc = test_alloc;
	app->free = test_free;
	app->get_timer = test_get_timer;
	app->sleep = test_sleep;
	app->http_create_context = test_http_context_create;
	app->http_destroy_context = test_http_context_destroy;
	app->http_update_context = test_http_context_update;
	app->http_get = test_http_get;
	app->http_poll = test_http_poll;
	app->user_data = test_data;
}

void
test_buffer()
{
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);

	sfc_buffer t;
	sfc_buffer_init(&t, &app);

	sfc_buffer_append(&t, "ABCD", 4);
	TEST_ASSERT(t.p == t.static_buffer);
	TEST_ASSERT(t.size == 4);
	TEST_ASSERT(t.allocated == sizeof(t.static_buffer));
	
	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);

	char big_buffer[4096];
	memset(big_buffer, 'X', sizeof(big_buffer));

	sfc_buffer_append(&t, big_buffer, sizeof(big_buffer));
	TEST_ASSERT(t.p != t.static_buffer);
	TEST_ASSERT(t.size == 4 + sizeof(big_buffer));
	TEST_ASSERT(t.allocated >= t.size);

	TEST_ASSERT(test_data.total_alloc_count == 1);
	TEST_ASSERT(test_data.total_alloc_size == t.allocated);

	sfc_buffer_uninit(&t);
	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

void
test_cache()
{
	/* Initialize testing environment and create a cache object */
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);

	sfc_cache* cache = sfc_cache_create(&app, 9);
	TEST_ASSERT(cache != NULL);

	/* Query a cardmarket id that doesn't exist */
	{
		sfc_query* query = sfc_cache_query_cardmarket_id(cache, 123456);

		size_t request_count_0 = test_data.total_http_request_count;

		/* Update query until completed */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
		}

		/* Verify that we got the expected result */
		{
			TEST_ASSERT(query->result = SFC_RESULT_NOT_FOUND);
			TEST_ASSERT(query->results->count == 0);
		}

		size_t request_count_1 = test_data.total_http_request_count;

		/* Should have triggered 1 http request */
		TEST_ASSERT(request_count_1 - request_count_0 == 1);

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Query a cardmarket id that doesn't exist again. This time it shouldn't trigger
	   any http requests because the url will be in the "bad url" list */
	{
		sfc_query* query = sfc_cache_query_cardmarket_id(cache, 123456);

		size_t request_count_0 = test_data.total_http_request_count;

		/* Update query until completed */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
		}

		/* Verify that we got the expected result */
		{
			TEST_ASSERT(query->result = SFC_RESULT_NOT_FOUND);
			TEST_ASSERT(query->results->count == 0);
		}

		size_t request_count_1 = test_data.total_http_request_count;

		/* No http requests expected */
		TEST_ASSERT(request_count_1 == request_count_0);

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Query cardmarket id 5013 on an empty cache */
	{
		sfc_query* query = sfc_cache_query_cardmarket_id(cache, 5013);

		/* Update query until completed */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
		}

		/* Verify that we got the expected result */
		{
			TEST_ASSERT(query->results->count == 1);
			sfc_card* card = query->results->cards[0];
			TEST_ASSERT(card != NULL);
			test_verify_cardmarket_5013(card);
		}

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Query cardmarket id 5013 again, now it should already be in the cache */
	{
		sfc_query* query = sfc_cache_query_cardmarket_id(cache, 5013);

		/* Query should complete immediately */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
		}

		/* Result should still be the same */
		{
			TEST_ASSERT(query->results->count == 1);
			sfc_card* card = query->results->cards[0];
			TEST_ASSERT(card != NULL);
			test_verify_cardmarket_5013(card);
		}

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Save cache to disk */
	{
		sfc_result result = sfc_cache_save(cache, "test.sfc");
		TEST_ASSERT(result == SFC_RESULT_OK);
	}

	/* Destroy cache and check that we didn't leak memory */
	sfc_cache_destroy(cache);

	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);

	/* Recreate an empty cache */
	cache = sfc_cache_create(&app, 9);
	TEST_ASSERT(cache != NULL);

	/* Load previously save cache from disk */
	{
		sfc_result result = sfc_cache_load(cache, "test.sfc");
		TEST_ASSERT(result == SFC_RESULT_OK);
	}

	/* Yet again we'll query cardmarket id 5013, which should exist in the loaded cache */
	{
		sfc_query* query = sfc_cache_query_cardmarket_id(cache, 5013);

		/* Query should complete immediately */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);

		}

		/* Again, result should still be the same */
		{
			TEST_ASSERT(query->results->count == 1);
			sfc_card* card = query->results->cards[0];
			TEST_ASSERT(card != NULL);
			test_verify_cardmarket_5013(card);
		}

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Clear the cache (don't destroy it) */
	{
		sfc_cache_clear(cache);
		TEST_ASSERT(cache->first_query == NULL);
		TEST_ASSERT(cache->last_query == NULL);
	}

	/* Query the same card by its key */
	{
		sfc_card_key key = { "2ed", 9, 0 };
		sfc_query* query = sfc_cache_query_card_key(cache, &key);

		/* Update query until completed */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);

		}

		/* Verify that we got the expected result */
		{
			TEST_ASSERT(query->results->count == 1);
			sfc_card* card = query->results->cards[0];
			TEST_ASSERT(card != NULL);
			test_verify_cardmarket_5013(card);
		}

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Clear the cache again */
	{
		sfc_cache_clear(cache);
		TEST_ASSERT(cache->first_query == NULL);
		TEST_ASSERT(cache->last_query == NULL);
	}

	/* Try making two card key requests for the same card at the same time */
	{
		sfc_card_key key = { "2ed", 9, 0 };
		sfc_query* query1 = sfc_cache_query_card_key(cache, &key);
		sfc_query* query2 = sfc_cache_query_card_key(cache, &key);

		/* Update queries until completed */
		{
			TEST_ASSERT(sfc_query_wait(query1, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
			TEST_ASSERT(sfc_query_wait(query2, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
		}

		/* Verify that we got the expected results */
		{
			TEST_ASSERT(cache->first_card == cache->last_card);
			TEST_ASSERT(query1->results->count == 1);
			TEST_ASSERT(query2->results->count == 1);
			sfc_card* card1 = query1->results->cards[0];
			sfc_card* card2 = query2->results->cards[0];
			TEST_ASSERT(card1 != NULL);
			TEST_ASSERT(card2 != NULL);
			TEST_ASSERT(card1 == card2);
			test_verify_cardmarket_5013(card1);
		}

		/* Delete queries */
		{
			sfc_query_delete(query1);
			sfc_query_delete(query2);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Clear the cache again */
	{
		sfc_cache_clear(cache);
		TEST_ASSERT(cache->first_query == NULL);
		TEST_ASSERT(cache->last_query == NULL);
	}

	/* Query a full set */
	{
		sfc_query* query = sfc_cache_query_set(cache, "test");

		/* Update query until completed */
		{
			TEST_ASSERT(query != NULL);
			TEST_ASSERT(sfc_query_wait(query, TEST_QUERY_TIMEOUT) == SFC_RESULT_OK);
		}

		/* Verify that we got the expected result */
		{
			TEST_ASSERT(query->results->count == 5);			

			{
				sfc_card* card = query->results->cards[0];
				TEST_ASSERT(strcmp(card->key.set, "test") == 0);
				TEST_ASSERT(card->key.collector_number == 1);
				TEST_ASSERT(card->key.version == 0);
			}

			{
				sfc_card* card = query->results->cards[1];
				TEST_ASSERT(strcmp(card->key.set, "test") == 0);
				TEST_ASSERT(card->key.collector_number == 2);
				TEST_ASSERT(card->key.version == 1);
			}

			{
				sfc_card* card = query->results->cards[2];
				TEST_ASSERT(strcmp(card->key.set, "test") == 0);
				TEST_ASSERT(card->key.collector_number == 2);
				TEST_ASSERT(card->key.version == 2);
			}

			{
				sfc_card* card = query->results->cards[3];
				TEST_ASSERT(strcmp(card->key.set, "test") == 0);
				TEST_ASSERT(card->key.collector_number == 2);
				TEST_ASSERT(card->key.version == 3);
			}

			{
				sfc_card* card = query->results->cards[4];
				TEST_ASSERT(strcmp(card->key.set, "test") == 0);
				TEST_ASSERT(card->key.collector_number == 3);
				TEST_ASSERT(card->key.version == 0);
			}
		}

		/* Delete query */
		{
			sfc_query_delete(query);
			sfc_cache_update(cache);
			TEST_ASSERT(cache->first_query == NULL);
			TEST_ASSERT(cache->last_query == NULL);
		}
	}

	/* Destroy cache and check that we're still not leaking memory */
	sfc_cache_destroy(cache);

	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

void
test_card_array()
{
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);

	/* Create card array */
	sfc_card_array* card_array = sfc_card_array_create(&app);
	TEST_ASSERT(card_array != NULL);

	/* Add 100 NULL cards to it */
	for(int i = 0; i < 100; i++)
		sfc_card_array_add(card_array, NULL);

	/* See that we have a 100 entries */
	TEST_ASSERT(card_array->cards != NULL);
	TEST_ASSERT(card_array->count == 100);

	/* Clean up */
	sfc_card_array_destroy(card_array);

	/* Make sure we didn't leak memory */
	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

void
test_card_map_uint32()
{
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);
	
	sfc_card* cards[TEST_CARD_COUNT];

	/* Allocate a bunch of cards to put in the map */
	for(size_t i = 0; i < TEST_CARD_COUNT; i++)
	{
		cards[i] = sfc_card_create(&app, NULL);
		TEST_ASSERT(cards[i] != NULL);
	}

	/* Create the map */
	sfc_card_map_uint32* card_map = sfc_card_map_uint32_create(&app, 1);
	TEST_ASSERT(card_map != NULL);	

	/* Add the cards to the map */
	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
	{
		sfc_result result = sfc_card_map_uint32_set(card_map, (uint32_t)i + 1, cards[i]);
		TEST_ASSERT(result == SFC_RESULT_OK);
	}

	/* Check that all cards now exist in the map */
	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
		TEST_ASSERT(sfc_card_map_uint32_has(card_map, (uint32_t)i + 1));

	/* Check that a card not in the map isn't there */
	TEST_ASSERT(!sfc_card_map_uint32_has(card_map, (uint32_t)TEST_CARD_COUNT + 1));

	/* Get all cards and check if they're the correct ones associated with the key */
	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
	{
		sfc_card* card = sfc_card_map_uint32_get(card_map, (uint32_t)i + 1);
		TEST_ASSERT(cards[i] == card);
	}

	/* Again, try a card that's not there */
	TEST_ASSERT(sfc_card_map_uint32_get(card_map, (uint32_t)TEST_CARD_COUNT + 1) == NULL);

	/* Clear map and check that all the cards are gone */
	sfc_card_map_uint32_clear(card_map);

	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
		TEST_ASSERT(!sfc_card_map_uint32_has(card_map, (uint32_t)i + 1));

	/* Clean up and check we didn't leak memory */
	sfc_card_map_uint32_destroy(card_map);

	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
		sfc_card_destroy(cards[i]);

	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

void
test_card_set()
{
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);

	sfc_card* cards[TEST_CARD_COUNT];

	/* Allocate a bunch of cards to put in the set */
	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
	{
		cards[i] = sfc_card_create(&app, NULL);
		TEST_ASSERT(cards[i] != NULL);

		/* Make up some unique key */
		snprintf(cards[i]->key.set, sizeof(cards[i]->key.set), "s%u", (uint32_t)i);
		cards[i]->key.version = (uint8_t)(i % 4);
		cards[i]->key.collector_number = (uint16_t)i;
	}

	/* Create card set */
	sfc_card_set* card_set = sfc_card_set_create(&app);
	TEST_ASSERT(card_set != NULL);

	/* Add cards to the set */
	for (int i = 0; i < TEST_CARD_COUNT; i++)
		sfc_card_set_add(card_set, cards[i]);

	/* Check that all the cards are now in the set */
	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
	{
		const sfc_card_key* key = &cards[i]->key;
		TEST_ASSERT(sfc_card_set_has(card_set, key));
		sfc_card* card = sfc_card_set_get(card_set, key);
		TEST_ASSERT(card != NULL);
		TEST_ASSERT(card == cards[i]);
	}

	/* Check non existing card */
	{
		sfc_card_key key = {"xxx", 0, 0};
		TEST_ASSERT(!sfc_card_set_has(card_set, &key));
		TEST_ASSERT(sfc_card_set_get(card_set, &key) == NULL);
	}

	/* Clear and check everything is gone */
	sfc_card_set_clear(card_set);

	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
	{
		const sfc_card_key* key = &cards[i]->key;
		TEST_ASSERT(!sfc_card_set_has(card_set, key));
		sfc_card* card = sfc_card_set_get(card_set, key);
		TEST_ASSERT(card == NULL);
	}

	/* Clean up */
	sfc_card_set_destroy(card_set);

	for (size_t i = 0; i < TEST_CARD_COUNT; i++)
		sfc_card_destroy(cards[i]);

	/* Make sure we didn't leak memory */
	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

void
test_leaky_bucket()
{
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);

	test_data.auto_advance_timer = SFC_FALSE;
	test_data.test_timer = 10000;

	{
		sfc_leaky_bucket* leaky_bucket = sfc_leaky_bucket_create(&app, 10, 10);

		/* We should be able to get 10 tokens immediately */
		for(int i = 0; i < 10; i++)
			TEST_ASSERT(sfc_leaky_bucket_acquire_token(leaky_bucket));

		/* But now it should be empty */
		TEST_ASSERT(!sfc_leaky_bucket_acquire_token(leaky_bucket));

		/* Advance time 100 milliseconds and there should be 1 token in bucket */
		test_data.test_timer += 100;
		TEST_ASSERT(sfc_leaky_bucket_acquire_token(leaky_bucket));
		TEST_ASSERT(!sfc_leaky_bucket_acquire_token(leaky_bucket));

		/* Advance time 100 seconds and there should be 10 tokens */
		test_data.test_timer += 1000 * 10;
		for (int i = 0; i < 10; i++)
			TEST_ASSERT(sfc_leaky_bucket_acquire_token(leaky_bucket));

		/* Should be empty again */
		TEST_ASSERT(!sfc_leaky_bucket_acquire_token(leaky_bucket));

		sfc_leaky_bucket_destroy(leaky_bucket);
	}

	/* Make sure we didn't leak memory */
	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

void 
test_serializer()
{
	test_user_data test_data;
	sfc_app app;
	test_init(&test_data, &app);

	sfc_buffer buffer;
	sfc_buffer_init(&buffer, &app);

	/* Write a bunch of stuff into the serializer */
	{
		sfc_serializer serializer;
		sfc_serializer_init(&serializer, &buffer);

		sfc_serializer_write_uint8(&serializer, 0);
		sfc_serializer_write_uint8(&serializer, UINT8_MAX);
		sfc_serializer_write_uint32(&serializer, 0);
		sfc_serializer_write_uint32(&serializer, UINT32_MAX);
		sfc_serializer_write_uint64(&serializer, 0);
		sfc_serializer_write_uint64(&serializer, UINT64_MAX);

		for(uint32_t i = 0; i < 10000; i++)
			sfc_serializer_write_var_size_uint32(&serializer, i);

		sfc_serializer_write_var_size_uint32(&serializer, UINT32_MAX);
		sfc_serializer_write_string(&serializer, "hello");
	}

	/* Read all the stuff back with the deserialzier and see if it comes out as expected */
	{
		sfc_deserializer deseralizer;
		sfc_deserializer_init(&deseralizer, buffer.p, buffer.size);

		{
			uint8_t v;
			TEST_ASSERT(sfc_deserializer_read_uint8(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == 0);
			TEST_ASSERT(sfc_deserializer_read_uint8(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == UINT8_MAX);
		}

		{
			uint32_t v;
			TEST_ASSERT(sfc_deserializer_read_uint32(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == 0);
			TEST_ASSERT(sfc_deserializer_read_uint32(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == UINT32_MAX);
		}

		{
			uint64_t v;
			TEST_ASSERT(sfc_deserializer_read_uint64(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == 0);
			TEST_ASSERT(sfc_deserializer_read_uint64(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == UINT64_MAX);
		}

		for (uint32_t i = 0; i < 10000; i++)
		{
			uint32_t v;
			TEST_ASSERT(sfc_deserializer_read_var_size_uint32(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == i);
		}

		{
			uint32_t v;
			TEST_ASSERT(sfc_deserializer_read_var_size_uint32(&deseralizer, &v) == SFC_RESULT_OK);
			TEST_ASSERT(v == UINT32_MAX);
		}

		{
			char buffer2[100];
			TEST_ASSERT(sfc_deserializer_read_string(&deseralizer, buffer2, sizeof(buffer2)) == SFC_RESULT_OK);
			TEST_ASSERT(strcmp(buffer2, "hello") == 0);
		}		
	}

	sfc_buffer_uninit(&buffer);

	/* Make sure we didn't leak memory */
	TEST_ASSERT(test_data.total_alloc_count == 0);
	TEST_ASSERT(test_data.total_alloc_size == 0);
}

#if defined(SFC_CURL)

	void test_curl()
	{
		test_user_data test_data;
		sfc_app app;
		test_init(&test_data, &app);
		sfc_app_init_curl(&app);
		sfc_app_init_timer(&app);
		sfc_app_init_sleep(&app);
		app.debug_print = sfc_debug_stdio;

		/* Do an actual scryfall API requests using curl */
		sfc_cache* cache = sfc_cache_create(&app, 9);
		TEST_ASSERT(cache != NULL);

		/* Grab two cards concurrently */
		{
			sfc_query* query1 = sfc_cache_query_cardmarket_id(cache, 999);
			sfc_query* query2 = sfc_cache_query_cardmarket_id(cache, 1000);
			TEST_ASSERT(query1 != NULL);
			TEST_ASSERT(query2 != NULL);
			TEST_ASSERT(sfc_query_wait(query1, 20 * 1000) == SFC_RESULT_OK);
			TEST_ASSERT(sfc_query_wait(query2, 20 * 1000) == SFC_RESULT_OK);
			TEST_ASSERT(query1->result == SFC_RESULT_OK);
			TEST_ASSERT(query2->result == SFC_RESULT_OK);
			TEST_ASSERT(query1->results->count == 1);
			TEST_ASSERT(query2->results->count == 1);
			sfc_card* card1 = query1->results->cards[0];
			sfc_card* card2 = query2->results->cards[0];
			TEST_ASSERT(strcmp(sfc_card_get_string(card1, SFC_CARD_STRING_NAME), "Daru Warchief") == 0);
			TEST_ASSERT(strcmp(sfc_card_get_string(card2, SFC_CARD_STRING_NAME), "Dawn Elemental") == 0);
		}

		sfc_cache_destroy(cache);

		/* Make sure we didn't leak memory */
		TEST_ASSERT(test_data.total_alloc_count == 0);
		TEST_ASSERT(test_data.total_alloc_size == 0);
	}

#endif /* SFC_CURL */

int
main(
	int		argc,
	char**	argv)
{
	SFC_UNUSED(argc);
	SFC_UNUSED(argv);

	test_buffer();
	test_cache();
	test_card_array();
	test_card_map_uint32();
	test_card_set();
	test_leaky_bucket();
	test_serializer();

	#if defined(SFC_CURL)
		test_curl();
	#endif

	return 0;
}
