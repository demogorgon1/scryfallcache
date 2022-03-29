#ifndef __SCRYFALLCACHE_BUFFER_H__
#define __SCRYFALLCACHE_BUFFER_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define SFC_BUFFER_STATIC_SIZE	4096

typedef struct _sfc_buffer
{
	struct _sfc_app*	app;
	uint8_t*			p;
	uint8_t				static_buffer[SFC_BUFFER_STATIC_SIZE];
	size_t				size;
	size_t				allocated;
} sfc_buffer;

void	sfc_buffer_init(
			sfc_buffer*			buffer,
			struct _sfc_app*	app);
void	sfc_buffer_uninit(
			sfc_buffer*			buffer);
void	sfc_buffer_set_size(
			sfc_buffer*			buffer,
			size_t				size);
void*	sfc_buffer_append(
			sfc_buffer*			buffer,
			const void*			data,
			size_t				data_size);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_BUFFER_H__ */
