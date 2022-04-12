#ifndef __SCRYFALLCACHE_COLLECTOR_NUMBER_H__
#define __SCRYFALLCACHE_COLLECTOR_NUMBER_H__

#include "sfc_base.h"

/* gggg gggg gggg fffe dccc ccbb bbba aaaa 
   
   abc : World Championship Decks prefix (ASCII characters truncated to 5 bits each) 
   d   : World Championship Decks sideboard flag
   e   : Unicode cross flag
   f   : 3-bit version
   g   : 12-bit Collector number
*/

#define SFC_COLLECTOR_NUMBER_SIDEBOARD			0x00008000
#define SFC_COLLECTOR_NUMBER_CROSS				0x00010000

#define SFC_COLLECTOR_NUMBER_BIT_OFFSET			20
#define SFC_COLLECTOR_NUMBER_BIT_MASK			0xFFF00000
#define SFC_COLLECTOR_NUMBER_MAX				4095

#define SFC_COLLECTOR_NUMBER_VERSION_BIT_OFFSET	17
#define SFC_COLLECTOR_NUMBER_VERSION_BIT_MASK	0x000E0000
#define SFC_COLLECTOR_NUMBER_VERSION_MAX		7

#define SFC_COLLECTOR_NUMBER(_x)											\
	(((_x) & SFC_COLLECTOR_NUMBER_BIT_MASK) >> SFC_COLLECTOR_NUMBER_BIT_OFFSET)

#define SFC_COLLECTOR_NUMBER_VERSION(_x)									\
	(((_x) & SFC_COLLECTOR_NUMBER_VERSION_BIT_MASK) >> SFC_COLLECTOR_NUMBER_VERSION_BIT_OFFSET)

#define SFC_COLLECTOR_NUMBER_MAKE(_number, _version)						\
	(((uint32_t)_number << SFC_COLLECTOR_NUMBER_BIT_OFFSET) | ((uint32_t)_version << SFC_COLLECTOR_NUMBER_VERSION_BIT_OFFSET))

#define SFC_COLLECTOR_NUMBER_WITH_VERSION(_collector_number, _new_version)	\
	((_collector_number & (~SFC_COLLECTOR_NUMBER_VERSION_BIT_MASK)) | ((uint32_t)_new_version << SFC_COLLECTOR_NUMBER_VERSION_BIT_OFFSET))

sfc_result	sfc_collector_number_from_string(
				const char*		string,
				uint32_t*		out);

sfc_result	sfc_collector_number_to_string(
				uint32_t		collector_number,
				char*			buffer,
				size_t			buffer_size);

#endif /* __SCRYFALLCACHE_COLLECTOR_NUMBER_H__ */