#if defined(SFC_CURL)

#include <assert.h>
#include <string.h>

#include <curl/curl.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_curl.h>
#include <sfc/sfc_debug.h>

typedef struct _sfc_curl_write_context
{
	struct _sfc_app*	app;
	char*				data;
	size_t				size;
	size_t				allocated;
} sfc_curl_write_context;

static size_t
sfc_curl_write_function(
	char*				ptr,
	size_t				size,
	size_t				nmemb,
	void*				user_data)
{
	sfc_curl_write_context* write_context = (sfc_curl_write_context*)user_data;
	size_t space_needed = size * nmemb;
	size_t space_left = write_context->allocated - write_context->size;
	
	if(space_left < space_needed)
	{
		write_context->allocated += space_needed * 2;

		write_context->data = SFC_ALLOC(write_context->app->alloc, write_context->app->user_data, write_context->data, write_context->allocated);
		assert(write_context->data != NULL);
	}

	memcpy(write_context->data + write_context->size, ptr, space_needed);

	write_context->size += space_needed;

	return nmemb;
}

/*------------------------------------------------------------------------------------*/

void* 
sfc_curl_create(
	struct _sfc_app*	app)
{
	sfc_curl_context* context = (sfc_curl_context*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_curl_context));
	assert(context != NULL);
	memset(context, 0, sizeof(sfc_curl_context));

	context->app = app;

	context->curl = (void*)curl_easy_init();
	if (context->curl == NULL)
	{	
		sfc_debug(app, "curl_easy_init() failed");

		return NULL;
	}

	return context;
}

void			
sfc_curl_destroy(
	void*				http_context)
{
	sfc_curl_context* context = (sfc_curl_context*)http_context;

	assert(context->curl != NULL);

	curl_easy_cleanup((CURL*)context->curl);

	context->app->free(context->app->user_data, context);
}

sfc_result		
sfc_curl(
	void*				http_context,
	const char*			url,
	char**				out_data,
	size_t*				out_data_size)
{	
	sfc_curl_context* context = (sfc_curl_context*)http_context;

	assert(context->curl != NULL);
	CURL* curl = (CURL*)context->curl;

	sfc_result result = SFC_RESULT_OK;

	sfc_curl_write_context write_context;
	memset(&write_context, 0, sizeof(sfc_curl_write_context));
	write_context.app = context->app;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sfc_curl_write_function);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_context);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);

	{
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			sfc_debug(context->app, "curl_easy_perform() failed: %s (%s)", curl_easy_strerror(res), url);

			result = SFC_RESULT_HTTP_GET_FAILED;
		}
	}

	if(result == SFC_RESULT_OK)
	{
		*out_data = write_context.data;
		*out_data_size = write_context.size;
	}
	else
	{
		if(write_context.data != NULL)
			context->app->free(context->app->user_data, write_context.data);
	}

	return result;
}

#endif /* SFC_CURL */
