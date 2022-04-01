#ifndef __SCRYFALLCACHE_CURL_H__
#define	__SCRYFALLCACHE_CURL_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_curl_request_linked_list
{
	struct _sfc_curl_request*		head;
	struct _sfc_curl_request*		tail;
} sfc_curl_request_linked_list;

typedef struct _sfc_curl_context
{
	struct _sfc_app*				app;
	void*							curl_multi;
	sfc_curl_request_linked_list	requests;
} sfc_curl_context;

void*			sfc_curl_create(
					struct _sfc_app*	app);

void			sfc_curl_destroy(
					void*				http_context);

sfc_result		sfc_curl_update(
					void*				http_context);

void*			sfc_curl_get(
					void*				http_context,
					const char*			url);

sfc_bool		sfc_curl_poll(
					void*				http_request,
					sfc_result*			out_result,
					char**				out_data,
					size_t*				out_data_size);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CURL_H__ */
