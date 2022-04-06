#if defined(SFC_CURL)

#include <assert.h>
#include <string.h>

#include <curl/curl.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_curl.h>
#include <sfc/sfc_debug.h>

#include "sfc_buffer.h"

typedef struct _sfc_curl_request
{
	sfc_curl_context*			context;
	struct _sfc_curl_request*	next;
	struct _sfc_curl_request*	prev;
	void*						curl;

	sfc_bool					completed;
	sfc_bool					should_be_deleted;
	sfc_buffer					buffer;
	sfc_result					result;
} sfc_curl_request;

static size_t
sfc_curl_write_function(
	char*				ptr,
	size_t				size,
	size_t				nmemb,
	void*				user_data)
{
	sfc_curl_request* req = (sfc_curl_request*)user_data;

	sfc_buffer_append(&req->buffer, ptr, size * nmemb);

	return nmemb;
}

static sfc_curl_request*
sfc_curl_create_request(
	sfc_app*						app,
	sfc_curl_request_linked_list*	list)
{
	sfc_curl_request* req = (sfc_curl_request*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_curl_request));
	assert(req != NULL);
	memset(req, 0, sizeof(sfc_curl_request));

	sfc_buffer_init(&req->buffer, app);

	req->prev = list->tail;

	if(list->tail != NULL)
		list->tail->next = req;
	else
		list->head = req;

	list->tail = req;

	return req;		
}

static void
sfc_curl_destroy_request(
	sfc_app*						app,
	CURLM*							curl_multi,
	sfc_curl_request_linked_list*	list,
	sfc_curl_request*				req)
{
	if(req->curl != NULL)
	{
		curl_multi_remove_handle(curl_multi, (CURL*)req->curl);
		curl_easy_cleanup((CURL*)req->curl);
	}

	sfc_buffer_uninit(&req->buffer);

	if(req->prev != NULL)
		req->prev->next = req->next;
	else
		list->head = req->next;

	if(req->next != NULL)
		req->next->prev = req->prev;
	else
		list->tail = req->prev;

	app->free(app->user_data, req);
}

static sfc_curl_request*
sfc_curl_find_request_by_curl_handle(
	sfc_curl_request_linked_list*	list,
	void*							curl)
{
	for(sfc_curl_request* req = list->head; req != NULL; req = req->next)
	{
		if(req->curl == curl)
			return req;
	}

	return NULL;
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

	context->curl_multi = (void*)curl_multi_init();

	curl_multi_setopt((CURLM*)context->curl_multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, 1);

	return context;
}

void			
sfc_curl_destroy(
	void*				http_context)
{
	sfc_curl_context* context = (sfc_curl_context*)http_context;

	assert(context->curl_multi != NULL);
	assert(context->requests.head == NULL);
	assert(context->requests.tail == NULL);

	curl_multi_cleanup((CURLM*)context->curl_multi);

	context->app->free(context->app->user_data, context);
}

sfc_result		
sfc_curl_update(
	void*				http_context)
{
	sfc_curl_context* context = (sfc_curl_context*)http_context;

	assert(context->curl_multi != NULL);
	CURLM* curl_multi = (CURLM*)context->curl_multi;
	
	if(context->requests.head != NULL)
	{
		/* Update curl requests */
		{
			int still_running = 0;
			CURLMcode code = curl_multi_perform(curl_multi, &still_running);
			if (code != CURLM_OK)
			{
				sfc_debug(context->app, "curl_multi_perform() failed: %s", curl_multi_strerror(code));
				return SFC_RESULT_HTTP_GET_FAILED;
			}
		}

		{
			int numfds = 0;
			CURLMcode code = curl_multi_wait(curl_multi, NULL, 0, 0, &numfds);
			if (code != CURLM_OK)
			{
				sfc_debug(context->app, "curl_multi_wait() failed: %s", curl_multi_strerror(code));
				return SFC_RESULT_HTTP_GET_FAILED;
			}
		}

		/* Poll for curl request updates */
		for(;;)
		{
			int msgs_left = 0;
			CURLMsg* msg = curl_multi_info_read(curl_multi, &msgs_left);
			if(msg == NULL)
				break;

			sfc_curl_request* req = sfc_curl_find_request_by_curl_handle(&context->requests, msg->easy_handle);
			assert(req != NULL);
			assert(!req->completed);
			assert(!req->should_be_deleted);

			CURL* curl = (CURL*)req->curl;
			assert(curl != NULL);

			if (msg->msg == CURLMSG_DONE)
			{
				req->completed = SFC_TRUE;

				char* url = "?";
				curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, (char**)&url);

				CURLcode code = msg->data.result;
				if (code != CURLE_OK)
				{
					sfc_debug(context->app, "[%s] curl request error: %s", url, curl_easy_strerror(code));
					req->result = SFC_RESULT_HTTP_GET_FAILED;
				}
				else
				{
					int http_status_code = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status_code);

					/* 404 is also considered a valid result as it will return some json we'll parse */
					if (http_status_code != 200 && http_status_code != 404 )
					{
						sfc_debug(context->app, "[%s] http error: %d", url, http_status_code);
						req->result = SFC_RESULT_HTTP_GET_FAILED;
					}
					else
					{
						req->result = SFC_RESULT_OK;
					}
				}
			}
		}
	}

	/* Clean up requests */
	{
		sfc_curl_request* req = context->requests.head;
		while(req != NULL)
		{
			sfc_curl_request* next = req->next;
			
			if(req->should_be_deleted)
			{
				assert(req->completed);

				sfc_curl_destroy_request(context->app, (CURLM*)context->curl_multi, &context->requests, req);
			}

			req = next;
		}
	}
	
	return SFC_RESULT_OK;
}

void*	
sfc_curl_get(
	void*				http_context,
	const char*			url)
{	
	sfc_curl_context* context = (sfc_curl_context*)http_context;

	CURL* curl = curl_easy_init();
	if(curl == NULL)
	{
		sfc_debug(context->app, "curl_easy_init() failed");
		return NULL;
	}

	sfc_curl_request* req = sfc_curl_create_request(context->app, &context->requests);
	req->curl = (void*)curl;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sfc_curl_write_function);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);

	{
		CURLMcode code = curl_multi_add_handle((CURLM*)context->curl_multi, curl);
		if(code != CURLM_OK)
		{
			sfc_debug(context->app, "curl_multi_add_handle() failed: %s", curl_multi_strerror(code));

			sfc_curl_destroy_request(context->app, (CURLM*)context->curl_multi, &context->requests, req);
			return NULL;
		}
	}

	return req;
}

sfc_bool
sfc_curl_poll(
	void*			http_request,
	sfc_result*		out_result,
	char**			out_data,
	size_t*			out_data_size)
{
	sfc_curl_request* req = (sfc_curl_request*)http_request;

	if(!req->completed)
	{
		/* Indicate to caller that request isn't completed yet */
		return SFC_FALSE;
	}
	
	if(req->result == SFC_RESULT_OK)
		sfc_buffer_detach(&req->buffer, (void**)out_data, out_data_size);

	*out_result = req->result;

	req->should_be_deleted = SFC_TRUE;

	/* Return true to indicate that the request has completed */
	return SFC_TRUE;
}

#endif /* SFC_CURL */
