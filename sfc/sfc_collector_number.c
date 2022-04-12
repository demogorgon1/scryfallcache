#include <stdio.h>
#include <string.h>

#include "sfc_collector_number.h"

static uint8_t g_sfc_collector_number_unicode_cross[] = 
{
	0xE2, 0x80, 0xA0
};

static sfc_bool
sfc_collector_number_char_is_alpha(
	char			character)
{
	return character >= 'a' && character <= 'z';
}

/*------------------------------------------------------------------------*/

sfc_result	
sfc_collector_number_from_string(
	const char*		string,
	uint32_t*		out)
{
	*out = 0;

	/* Parse the "world championship deck" prefix into 5-bit characters */
	{
		uint32_t wcd_prefix_len = 0;
		while (sfc_collector_number_char_is_alpha(string[wcd_prefix_len]))
		{
			if (wcd_prefix_len > 3)
				return SFC_RESULT_INVALID_COLLECTOR_NUMBER;

			uint32_t v = (string[wcd_prefix_len] - 'a') + 1;

			*out |= v << (wcd_prefix_len * 5);

			wcd_prefix_len++;
		}

		string += wcd_prefix_len;
	}	

	size_t len = strlen(string);

	/* Some card varirations have a little unicode cross at the end of
	   their collector number.*/
	if(len >= 3)
	{
		if(memcmp(&string[len - 3], g_sfc_collector_number_unicode_cross, 3) == 0)
		{
			*out |= SFC_COLLECTOR_NUMBER_CROSS;

			len -= 3;
		}
	}

	/* "World Championship Decks" might have a "sb" (sideboard) flag at the end */
	if(len >= 2)
	{
		if (memcmp(&string[len - 2], "sb", 2) == 0)
		{
			*out |= SFC_COLLECTOR_NUMBER_SIDEBOARD;

			len -= 2;
		}
	}

	/* Version tag */
	if(len >= 1)
	{
		if(sfc_collector_number_char_is_alpha(string[len - 1]))
		{
			uint32_t version = (uint32_t)(string[len - 1] - 'a') + 1;
			if(version >= SFC_COLLECTOR_NUMBER_MAX)
				return SFC_RESULT_INVALID_COLLECTOR_NUMBER;

			*out |= version << SFC_COLLECTOR_NUMBER_VERSION_BIT_OFFSET;

			len--;
		}
	}

	/* Finally parse the number */
	{
		uint32_t number = 0;
		for (size_t i = 0; i < len; i++)
		{
			if (string[i] >= '0' && string[i] <= '9')
			{
				uint64_t temp = (uint64_t)number * 10 + (uint64_t)(string[i] - '0');
				if (temp > SFC_COLLECTOR_NUMBER_MAX)
					return SFC_RESULT_INVALID_COLLECTOR_NUMBER;

				number = (uint32_t)temp;
			}
			else
			{
				return SFC_RESULT_INVALID_COLLECTOR_NUMBER;
			}
		}

		*out |= number << SFC_COLLECTOR_NUMBER_BIT_OFFSET;
	}

	return SFC_RESULT_OK;
}

sfc_result		
sfc_collector_number_to_string(
	uint32_t		collector_number,
	char*			buffer,
	size_t			buffer_size)
{
	/* World championship deck prefix */
	{
		for(uint32_t i = 0; i < 3; i++)
		{
			uint32_t v = ((collector_number >> (i * 5)) & 0x1F);
			if(v == 0)
				break;

			char prefix = 'a' + (char)(v - 1);
			if(prefix < 'a' || prefix > 'z')
				return SFC_RESULT_INVALID_COLLECTOR_NUMBER;

			if(buffer_size == 0)
				return SFC_RESULT_BUFFER_TOO_SMALL;

			buffer[0] = prefix;

			buffer++;
			buffer_size--;
		}
	}

	/* Number */
	{
		size_t required = (size_t)snprintf(
			buffer, 
			buffer_size, "%u", 
			SFC_COLLECTOR_NUMBER(collector_number));

		if(required > buffer_size)
			return SFC_RESULT_BUFFER_TOO_SMALL;

		buffer += required;
		buffer_size -= required;
	}

	/* Version tag */
	{
		uint32_t version = SFC_COLLECTOR_NUMBER_VERSION(collector_number);

		if(version > 0)
		{
			if (buffer_size < 1)
				return SFC_RESULT_BUFFER_TOO_SMALL;

			buffer[0] = 'a' + (char)version - 1;

			buffer++;
			buffer_size--;
		}
	}

	/* World championship deck sideboard flag */
	if(collector_number & SFC_COLLECTOR_NUMBER_SIDEBOARD)
	{
		if(buffer_size < 2)
			return SFC_RESULT_BUFFER_TOO_SMALL;

		memcpy(buffer, "sb", 2);

		buffer += 2;
		buffer_size -= 2;
	}

	/* Unicode cross flag */
	if (collector_number & SFC_COLLECTOR_NUMBER_CROSS)
	{
		if (buffer_size < 3)
			return SFC_RESULT_BUFFER_TOO_SMALL;

		memcpy(buffer, g_sfc_collector_number_unicode_cross, 3);

		buffer += 3;
		buffer_size -= 3;
	}

	/* Null termination */
	if(buffer_size < 1)
		return SFC_RESULT_BUFFER_TOO_SMALL;

	buffer[0] = '\0';

	return SFC_RESULT_OK;
}

