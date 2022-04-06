#if defined(WIN32)
	#include <windows.h> /* GetTickCount64() */
#else
	#include <time.h> /* clock_gettime() */
	#include <unistd.h> /* usleep() */
#endif

#include <assert.h>
#include <malloc.h>
#include <string.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_curl.h>

static void*
sfc_app_default_alloc(
	void*		user_data,
	void*		realloc_memory,
	size_t		size,
	const char*	file,
	int			line)
{
	SFC_UNUSED(user_data);
	SFC_UNUSED(file);
	SFC_UNUSED(line);

	return realloc(realloc_memory, size);
}

static void
sfc_app_default_free(
	void*		user_data,
	void*		memory)
{
	SFC_UNUSED(user_data);

	free(memory);
}

static uint64_t
sfc_app_default_get_timer(
	void*		user_data)
{
	SFC_UNUSED(user_data);

	#if defined(WIN32)
		
		return (uint64_t)GetTickCount64();

	#else 
		
		/* FIXME: probably shouldn't assume we have clock_gettime() just because it's not windows */
		struct timespec ts;
		int result = clock_gettime(CLOCK_MONOTONIC, &ts);
		assert(result == 0);			
		return (((uint64_t)ts.tv_sec) * 1000) + (uint64_t)(ts.tv_nsec / 1000000);
	
	#endif
}

static void
sfc_app_default_sleep(
	void*		user_data,
	uint32_t	sleep_ms)
{
	SFC_UNUSED(user_data);

	#if defined(WIN32)

		Sleep((DWORD)sleep_ms);

	#else
	
		usleep(sleep_ms * 1000);

	#endif
}

/*---------------------------------------------------------------------------*/

void	
sfc_app_init_malloc(
	sfc_app*	app)
{
	app->alloc = sfc_app_default_alloc;
	app->free = sfc_app_default_free;
}

void	
sfc_app_init_curl(
	sfc_app*	app)
{
	#if defined(SFC_CURL)
		app->http_get = sfc_curl_get;
		app->http_create_context = sfc_curl_create;
		app->http_destroy_context = sfc_curl_destroy;
		app->http_update_context = sfc_curl_update;
		app->http_poll = sfc_curl_poll;
	#endif
}

void	
sfc_app_init_timer(
	sfc_app*	app)
{
	app->get_timer = sfc_app_default_get_timer;
}

void
sfc_app_init_sleep(
	sfc_app*	app)
{
	app->sleep = sfc_app_default_sleep;
}

void	
sfc_app_init_defaults(
	sfc_app*	app)
{
	memset(app, 0, sizeof(sfc_app));

	sfc_app_init_malloc(app);
	sfc_app_init_timer(app);
	sfc_app_init_sleep(app);

	#if defined(SFC_CURL)
		sfc_app_init_curl(app);
	#endif
}

