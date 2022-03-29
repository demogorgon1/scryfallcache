#include <assert.h>
#include <string.h>

#include <sfc/sfc_app.h>

#include "sfc_buffer.h"

void	
sfc_buffer_init(
	sfc_buffer*			buffer,
	struct _sfc_app*	app)
{
	memset(buffer, 0, sizeof(sfc_buffer));
	buffer->app = app;
	buffer->p = buffer->static_buffer;
	buffer->allocated = sizeof(buffer->static_buffer);
}

void	
sfc_buffer_uninit(
	sfc_buffer*			buffer)
{
	assert(buffer->p != NULL);

	if(buffer->p != buffer->static_buffer)
	{
		buffer->app->free(buffer->app->user_data, buffer->p);
		buffer->p = NULL;
	}
}

void	
sfc_buffer_set_size(
	sfc_buffer*			buffer,
	size_t				size)
{
	assert(buffer->p != NULL);
	assert(buffer->size <= buffer->allocated);

	if(size > buffer->allocated)
	{
		size_t to_allocate = ((size >> 10) + 1) << 10;

		if (buffer->p != buffer->static_buffer)
		{
			buffer->p = SFC_ALLOC(buffer->app->alloc, buffer->app->user_data, buffer->p, to_allocate);
		}
		else
		{
			buffer->p = SFC_ALLOC(buffer->app->alloc, buffer->app->user_data, NULL, to_allocate);
			assert(to_allocate > SFC_BUFFER_STATIC_SIZE);
			memcpy(buffer->p, buffer->static_buffer, SFC_BUFFER_STATIC_SIZE);
		}

		memset(buffer->p + buffer->allocated, 0, to_allocate - buffer->allocated);

		buffer->allocated = to_allocate;
	}

	buffer->size = size;
}

void*
sfc_buffer_append(
	sfc_buffer*			buffer,
	const void*			data,
	size_t				data_size)
{
	assert(buffer->p != NULL);

	size_t write_offset = buffer->size;

	sfc_buffer_set_size(buffer, buffer->size + data_size);

	if(data != NULL)
		memcpy(buffer->p + write_offset, data, data_size);
	else
		memset(buffer->p + write_offset, 0, data_size);

	return buffer->p + write_offset;
}
