#ifndef __SCRYFALLCACHE_QUERY_H__
#define __SCRYFALLCACHE_QUERY_H__

#include <sfc/sfc_base.h>
#include <sfc/sfc_card.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef enum _sfc_query_type
{
	SFC_QUERY_TYPE_CARDMARKET_ID,
	SFC_QUERY_TYPE_CARD_KEY,
	SFC_QUERY_TYPE_SET
} sfc_query_type;

typedef enum _sfc_query_state
{
	SFC_QUERY_STATE_INIT,
	SFC_QUERY_STATE_GET_NEXT_IN_SET,
	SFC_QUERY_STATE_HTTP_GET,
	SFC_QUERY_STATE_WAITING_FOR_HTTP_COMPLETION,
	SFC_QUERY_STATE_PARSE_RESULT,
	SFC_QUERY_STATE_COMPLETED,
	SFC_QUERY_STATE_DELETE
} sfc_query_state;

typedef struct _sfc_query
{
	sfc_query_type			type;
	sfc_query_state			state;

	union
	{
		uint32_t			cardmarket_id;
		sfc_card_key		card_key;
		char				set[SFC_MAX_SET];
	} u;

	struct _sfc_cache*		cache;
	struct _sfc_card_array*	results;
	
	sfc_result				result;
	uint16_t				next_collector_number;
	uint8_t					next_collector_number_version;
	
	void*					http_request;

	char					http_get_url[1024];
	char*					http_get_result;
	size_t					http_get_result_size;

	struct _sfc_query*		next;
	struct _sfc_query*		prev;
} sfc_query;

void		sfc_query_init_cardmarket_id(
				sfc_query*			query,
				struct _sfc_cache*	cache,
				uint32_t			cardmarket_id);

void		sfc_query_init_card_key(
				sfc_query*			query,
				struct _sfc_cache*	cache,
				const sfc_card_key*	card_key);

void		sfc_query_init_set(
				sfc_query*			query,
				struct _sfc_cache*	cache,
				const char*			set);

void		sfc_query_update(
				sfc_query*			query);

sfc_bool	sfc_query_poll(
				sfc_query*			query);

void		sfc_query_delete(
				sfc_query*			query);

sfc_result	sfc_query_wait(
				sfc_query*			query,
				uint32_t			timeout_ms);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_QUERY_H__ */
