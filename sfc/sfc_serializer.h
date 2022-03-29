#ifndef __SCRYFALLCACHE_SERIALIZER_H__
#define __SCRYFALLCACHE_SERIALIZER_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define SFC_SERIALIZER_WRITE_BASE(_function, _serializer, _value)							\
	_function(_serializer, _value)

#define SFC_SERIALIZER_WRITE_UINT8(_serializer, _value)										\
	SFC_SERIALIZER_WRITE_BASE(sfc_serializer_write_uint8, _serializer, _value)

#define SFC_SERIALIZER_WRITE_UINT32(_serializer, _value)									\
	SFC_SERIALIZER_WRITE_BASE(sfc_serializer_write_uint32, _serializer, _value)

#define SFC_SERIALIZER_WRITE_UINT64(_serializer, _value)									\
	SFC_SERIALIZER_WRITE_BASE(sfc_serializer_write_uint64, _serializer, _value)

#define SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(_serializer, _value)							\
	SFC_SERIALIZER_WRITE_BASE(sfc_serializer_write_var_size_uint32, _serializer, _value);

#define SFC_SERIALIZER_WRITE_STRING(_serializer, _value)									\
	SFC_SERIALIZER_WRITE_BASE(sfc_serializer_write_string, _serializer, _value)

typedef struct _sfc_serializer
{
	struct _sfc_buffer*				buffer;
} sfc_serializer;

void		sfc_serializer_init(
				sfc_serializer*		serializer,
				struct _sfc_buffer*	buffer);

void		sfc_serializer_write(
				sfc_serializer*		serializer,
				const void*			data,
				size_t				size);

void		sfc_serializer_write_uint8(
				sfc_serializer*		serializer,
				uint8_t				value);

void		sfc_serializer_write_uint32(
				sfc_serializer*		serializer,
				uint32_t			value);

void		sfc_serializer_write_uint64(
				sfc_serializer*		serializer,
				uint64_t			value);

void		sfc_serializer_write_var_size_uint32(
				sfc_serializer*		serializer,
				uint32_t			value);

void		sfc_serializer_write_string(
				sfc_serializer*		serializer,
				const char*			value);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_SERIALIZER_H__ */