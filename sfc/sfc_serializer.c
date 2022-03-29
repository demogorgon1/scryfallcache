#include <string.h>

#include "sfc_buffer.h"
#include "sfc_serializer.h"

void	
sfc_serializer_init(
	sfc_serializer*		serializer,
	struct _sfc_buffer* buffer)
{
	memset(serializer, 0, sizeof(sfc_serializer));

	serializer->buffer = buffer;
}

void
sfc_serializer_write(
	sfc_serializer* serializer,
	const void*		data,
	size_t			size)
{
	sfc_buffer_append(serializer->buffer, data, size);
}

void
sfc_serializer_write_uint8(
	sfc_serializer* serializer,
	uint8_t			value)
{
	sfc_buffer_append(serializer->buffer, &value, 1);
}

void
sfc_serializer_write_uint32(
	sfc_serializer* serializer,
	uint32_t		value)
{
	uint8_t* p = (uint8_t*)sfc_buffer_append(serializer->buffer, NULL, 4);

	p[0] = (uint8_t)(value & 0xFF);
	p[1] = (uint8_t)((value & 0xFF00) >> 8);
	p[2] = (uint8_t)((value & 0xFF0000) >> 16);
	p[3] = (uint8_t)((value & 0xFF000000) >> 24);
}

void
sfc_serializer_write_uint64(
	sfc_serializer* serializer,
	uint64_t		value)
{
	uint8_t* p = (uint8_t*)sfc_buffer_append(serializer->buffer, NULL, 8);

	p[0] = (uint8_t)(value & 0xFF);
	p[1] = (uint8_t)((value & 0xFF00) >> 8);
	p[2] = (uint8_t)((value & 0xFF0000) >> 16);
	p[3] = (uint8_t)((value & 0xFF000000) >> 24);
	p[4] = (uint8_t)((value & 0xFF00000000) >> 32);
	p[5] = (uint8_t)((value & 0xFF0000000000) >> 40);
	p[6] = (uint8_t)((value & 0xFF000000000000) >> 48);
	p[7] = (uint8_t)((value & 0xFF00000000000000) >> 56);
}

void
sfc_serializer_write_var_size_uint32(
	sfc_serializer* serializer,
	uint32_t		value)
{
	for(;;)
	{
		uint32_t first7Bits = value & 0x7F;

		value >>= 7;

		if (value == 0)
		{
			sfc_serializer_write_uint8(serializer, (uint8_t)first7Bits);
			break;
		}

		sfc_serializer_write_uint8(serializer, (uint8_t)(first7Bits | 0x80));
	}
}

void 
sfc_serializer_write_string(
	sfc_serializer* serializer,
	const char*		value)
{
	size_t len = strlen(value);

	sfc_buffer_append(serializer->buffer, value, len + 1);
}

