#ifndef __SCRYFALLCACHE_CARD_MAP_UINT32_H__
#define __SCRYFALLCACHE_CARD_MAP_UINT32_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_card_map_uint32_item
{
	uint32_t					map_key;	
	struct _sfc_card*			card;
} sfc_card_map_uint32_item;

typedef struct _sfc_card_map_uint32
{
	struct _sfc_app*			app;

	uint32_t					size;
	sfc_card_map_uint32_item*	map;
} sfc_card_map_uint32;

sfc_card_map_uint32*	sfc_card_map_uint32_create(
							struct _sfc_app*		app,
							size_t					init_size);

void					sfc_card_map_uint32_destroy(
							sfc_card_map_uint32*	card_map);

sfc_result				sfc_card_map_uint32_set(
							sfc_card_map_uint32*	card_map,
							uint32_t				map_key,
							struct _sfc_card*		card);

int						sfc_card_map_uint32_has(
							sfc_card_map_uint32*	card_map,
							uint32_t				map_key);

struct _sfc_card*		sfc_card_map_uint32_get(
							sfc_card_map_uint32*	card_map,
							uint32_t				map_key);

void					sfc_card_map_uint32_clear(
							sfc_card_map_uint32*	card_map);

sfc_result				sfc_card_map_uint32_grow(
							sfc_card_map_uint32*	card_map);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CARD_SET_H__ */