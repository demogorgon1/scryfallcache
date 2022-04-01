#ifndef __SCRYFALLCACHE_APP_H__
#define	__SCRYFALLCACHE_APP_H__

#include "sfc_base.h"

#ifdef  __cplusplus
extern "C"
{
#endif

struct _sfc_app;

typedef void (*sfc_debug_print_callback)(
	void*					user_data,
	const char*				message);

typedef void* (*sfc_alloc_callback)(
	void*					user_data,
	void*					realloc_memory,
	size_t					size,
	const char*				file,
	int						line);

typedef void (*sfc_free_callback)(
	void*					user_data,
	void*					memory);

typedef uint64_t (*sfc_get_timer_callback)(
	void*					user_data);

typedef void (*sfc_sleep_callback)(
	void*					user_data,
	uint32_t				sleep_ms);

typedef void* (*sfc_http_create_context_callback)(
	struct _sfc_app*		app);

typedef void (*sfc_http_destroy_context_callback)(
	void*					http_context);

typedef void* (*sfc_http_get_callback)(
	void*					http_context,
	const char*				url);

typedef sfc_bool (*sfc_http_poll_callback)(
	void*					http_request,
	sfc_result*				out_result,
	char**					out_data,
	size_t*					out_data_size);

typedef sfc_result (*sfc_http_update_context_callback)(
	void*					http_context);

typedef struct _sfc_app
{
	void*								user_data;

	sfc_debug_print_callback			debug_print;
	sfc_alloc_callback					alloc;
	sfc_free_callback					free;
	sfc_get_timer_callback				get_timer;
	sfc_sleep_callback					sleep;
	sfc_http_create_context_callback	http_create_context;
	sfc_http_destroy_context_callback	http_destroy_context;
	sfc_http_update_context_callback	http_update_context;
	sfc_http_get_callback				http_get;
	sfc_http_poll_callback				http_poll;
} sfc_app;

#define SFC_ALLOC(_function, _user_data, _realloc_memory, _size) _function(_user_data, _realloc_memory, _size, __FILE__, __LINE__)

void	sfc_app_init_malloc(
			sfc_app*		app);

void	sfc_app_init_curl(
			sfc_app*		app);

void	sfc_app_init_timer(
			sfc_app*		app);

void	sfc_app_init_sleep(
			sfc_app*		app);

void	sfc_app_init_defaults(
			sfc_app*		app);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_APP_H__ */
