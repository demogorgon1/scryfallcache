#include <assert.h>
#include <string.h>

#include "sfc_deserializer.h"

void		
sfc_deserializer_init(
	sfc_deserializer*	deserializer,
	const void*			data,
	size_t				size)
{
	deserializer->p = data;
	deserializer->remaining = size;
}

sfc_result	
sfc_deserializer_read_buffer(
	sfc_deserializer*	deserializer,
	void*				buffer,
	size_t				size)
{
	if(deserializer->remaining < size)
		return SFC_RESULT_NOT_ENOUGH_DATA;

	memcpy(buffer, deserializer->p, size);

	deserializer->p += size;
	deserializer->remaining -= size;

	return SFC_RESULT_OK;
}

sfc_result	
sfc_deserializer_read_uint8(
	sfc_deserializer*	deserializer,
	uint8_t*			out)
{
	assert(out != NULL);

	if (deserializer->remaining < 1)
		return SFC_RESULT_NOT_ENOUGH_DATA;

	*out = deserializer->p[0];

	deserializer->p++;
	deserializer->remaining--;

	return SFC_RESULT_OK;
}

sfc_result	
sfc_deserializer_read_uint32(
	sfc_deserializer*	deserializer,
	uint32_t*			out)
{
	assert(out != NULL);

	if (deserializer->remaining < 4)
		return SFC_RESULT_NOT_ENOUGH_DATA;

	*out = (uint32_t)deserializer->p[0] |
		((uint32_t)deserializer->p[1] << 8) |
		((uint32_t)deserializer->p[2] << 16) |
		((uint32_t)deserializer->p[3] << 24);

	deserializer->p += 4;
	deserializer->remaining -= 4;

	return SFC_RESULT_OK;
}

sfc_result
sfc_deserializer_read_uint64(
	sfc_deserializer*	deserializer,
	uint64_t*			out)
{
	assert(out != NULL);

	if (deserializer->remaining < 8)
		return SFC_RESULT_NOT_ENOUGH_DATA;

	*out = (uint64_t)deserializer->p[0] |
		((uint64_t)deserializer->p[1] << 8) |
		((uint64_t)deserializer->p[2] << 16) |
		((uint64_t)deserializer->p[3] << 24) |
		((uint64_t)deserializer->p[4] << 32) |
		((uint64_t)deserializer->p[5] << 40) |
		((uint64_t)deserializer->p[6] << 48) |
		((uint64_t)deserializer->p[7] << 56);

	deserializer->p += 8;
	deserializer->remaining -= 8;

	return SFC_RESULT_OK;
}

sfc_result	
sfc_deserializer_read_var_size_uint32(
	sfc_deserializer*	deserializer,
	uint32_t*			out)
{
	uint64_t value = 0;
	uint64_t bits = 0;

	assert(out != NULL);

	for (;;)
	{
		uint64_t byte;

		if (deserializer->remaining < 1)
			return SFC_RESULT_NOT_ENOUGH_DATA;

		byte = (uint64_t)deserializer->p[0];

		deserializer->p++;
		deserializer->remaining--;

		value |= (byte & 0x7F) << bits;
		bits += 7;

		if ((byte & 0x80) == 0)
			break;
	}

	if(value > UINT32_MAX)	
		return SFC_RESULT_VAR_SIZE_UINT32_TOO_BIG;

	*out = (uint32_t)value;

	return SFC_RESULT_OK;
}

sfc_result	
sfc_deserializer_read_string(
	sfc_deserializer*	deserializer,
	char*				buffer,
	size_t				size)
{
	char* out = buffer;
	size_t out_remaining = size;
	
	for(;;)
	{
		if(deserializer->remaining < 1)
			return SFC_RESULT_MISSING_NULL_TERMINATION;
			
		if(out_remaining < 1)
			return SFC_RESULT_BUFFER_TOO_SMALL;

		*out = deserializer->p[0];

		deserializer->p++;
		deserializer->remaining--;

		if(*out == '\0')
			break;

		out++;
		out_remaining--;
	}

	return SFC_RESULT_OK;
}
