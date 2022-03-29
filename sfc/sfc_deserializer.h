#ifndef __SCRYFALLCACHE_DESERIALIZER_H__
#define __SCRYFALLCACHE_DESERIALIZER_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

#define SFC_DESERIALIZER_READ_BASE(_function, _deserializer, _value)						\
	do																						\
	{																						\
		sfc_result result = _function(_deserializer, _value);								\
		if(result != SFC_RESULT_OK)															\
			return result;																	\
	} while(0)

#define SFC_DESERIALIZER_READ_UINT8(_deserializer, _value)									\
	SFC_DESERIALIZER_READ_BASE(sfc_deserializer_read_uint8, _deserializer, _value)

#define SFC_DESERIALIZER_READ_UINT32(_deserializer, _value)									\
	SFC_DESERIALIZER_READ_BASE(sfc_deserializer_read_uint32, _deserializer, _value)

#define SFC_DESERIALIZER_READ_UINT64(_deserializer, _value)									\
	SFC_DESERIALIZER_READ_BASE(sfc_deserializer_read_uint64, _deserializer, _value)

#define SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(_deserializer, _value)						\
	SFC_DESERIALIZER_READ_BASE(sfc_deserializer_read_var_size_uint32, _deserializer, _value);

#define SFC_DESERIALIZER_READ_STRING(_deserializer, _value, _max_size)						\
	do																						\
	{																						\
		sfc_result result = sfc_deserializer_read_string(_deserializer, _value, _max_size);	\
		if(result != SFC_RESULT_OK)															\
			return result;																	\
	} while(0)

#define SFC_DESERIALIZER_READ_BUFFER(_deserializer, _buffer, _size)							\
	do																						\
	{																						\
		sfc_result result = sfc_deserializer_read_buffer(_deserializer, _buffer, _size);	\
		if(result != SFC_RESULT_OK)															\
			return result;																	\
	} while(0)

typedef struct _sfc_deserializer
{
	const uint8_t*	p;
	size_t			remaining;
} sfc_deserializer;

void		sfc_deserializer_init(
				sfc_deserializer*	deserializer,
				const void*			data,
				size_t				size);

sfc_result	sfc_deserializer_read_buffer(
				sfc_deserializer*	deserializer, 
				void*				buffer,
				size_t				size);

sfc_result	sfc_deserializer_read_uint8(
				sfc_deserializer*	deserializer,
				uint8_t*			out);

sfc_result	sfc_deserializer_read_uint32(
				sfc_deserializer*	deserializer,
				uint32_t*			out);

sfc_result	sfc_deserializer_read_uint64(
				sfc_deserializer*	deserializer,
				uint64_t*			out);

sfc_result	sfc_deserializer_read_var_size_uint32(
				sfc_deserializer*	deserializer,
				uint32_t*			out);

sfc_result	sfc_deserializer_read_string(
				sfc_deserializer*	deserializer,
				char*				buffer,
				size_t				size);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_DESERIALIZER_H__ */