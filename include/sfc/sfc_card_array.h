#ifndef __SCRYFALLCACHE_CARD_ARRAY_H__
#define __SCRYFALLCACHE_CARD_ARRAY_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_card_array
{
	struct _sfc_app*		app;		
	size_t					count;
	size_t					allocated;

	struct _sfc_card**		cards;
} sfc_card_array;

sfc_card_array*		sfc_card_array_create(
						struct _sfc_app*		app);

void				sfc_card_array_destroy(
						sfc_card_array*			card_array);

sfc_result			sfc_card_array_add(
						sfc_card_array*			card_array,
						struct _sfc_card*		card);

sfc_result			sfc_card_array_add_sorted_by_card_key(
						sfc_card_array*			card_array,
						struct _sfc_card*		card);

sfc_result			sfc_card_array_add_sorted_by_collector_number(
						sfc_card_array*			card_array,
						struct _sfc_card*		card);

sfc_result			sfc_card_array_reserve(
						sfc_card_array*			card_array,
						size_t					count);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CARD_ARRAY_H__ */