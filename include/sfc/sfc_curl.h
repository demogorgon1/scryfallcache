#ifndef __SCRYFALLCACHE_CURL_H__
#define	__SCRYFALLCACHE_CURL_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_curl_context
{
	struct _sfc_app*	app;
	void*				curl;
} sfc_curl_context;

void*			sfc_curl_create(
					struct _sfc_app*	app);

void			sfc_curl_destroy(
					void*				http_context);

sfc_result		sfc_curl(
					void*				http_context,
					const char*			url,
					char**				out_data,
					size_t*				out_data_size);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CURL_H__ */
