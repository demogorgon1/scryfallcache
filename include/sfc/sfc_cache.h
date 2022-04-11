#ifndef __SCRYFALLCACHE_CACHE_H__
#define	__SCRYFALLCACHE_CACHE_H__

#include <sfc/sfc_base.h>
#include <sfc/sfc_card.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_cache
{
	struct _sfc_app*				app;

	struct _sfc_leaky_bucket*		http_request_rate_limiter;
	struct _sfc_string_set*			full_sets;
	struct _sfc_card_set*			card_set;	
	struct _sfc_card_map_uint32*	cardmarket_id_map;

	void*							http_context;

	sfc_bool						string_filter[SFC_CARD_STRING_COUNT];

	sfc_card*						first_card;
	sfc_card*						last_card;

	struct _sfc_query*				first_query;
	struct _sfc_query*				last_query;
} sfc_cache;

sfc_cache*				sfc_cache_create(
							struct _sfc_app*			app,
							uint64_t					max_http_requests_per_second);

void					sfc_cache_destroy(
							sfc_cache*					cache);

void					sfc_cache_set_string_filter(
							sfc_cache*					cache,
							const sfc_card_string*		strings,
							size_t						count);

struct _sfc_query*		sfc_cache_create_query(
							sfc_cache*					cache);

struct _sfc_query*		sfc_cache_query_cardmarket_id(
							sfc_cache*					cache,
							uint32_t					cardmarket_id);

struct _sfc_query*		sfc_cache_query_card_key(
							sfc_cache*					cache,
							const sfc_card_key*			card_key);

struct _sfc_query*		sfc_cache_query_set(
							sfc_cache*					cache,
							const char*					set);

struct _sfc_query*		sfc_cache_query_set_collector_number(
							sfc_cache*					cache,
							const char*					set,
							const char*					collector_number_string);

sfc_result				sfc_cache_update(
							sfc_cache*					cache);

sfc_result				sfc_cache_add_card(
							sfc_cache*					cache,
							sfc_card*					card);

sfc_result				sfc_cache_save(
							sfc_cache*					cache,
							const char*					path);

sfc_result				sfc_cache_load(
							sfc_cache*					cache,
							const char*					path);

void					sfc_cache_clear(
							sfc_cache*					cache);

int						sfc_cache_has_full_set(
							sfc_cache*					cache,
							const char*					set);

size_t					sfc_cache_card_count(
							sfc_cache*					cache);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CACHE_H__ */