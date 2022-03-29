#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_card.h>
#include <sfc/sfc_card_array.h>

#include "sfc_card_map_uint32.h"

static uint32_t
sfc_card_map_uint32_hash(
	uint32_t				map_key)
{
	uint32_t x = map_key;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

static int
sfc_card_map_uint32_try_insert(
	sfc_card_map_uint32*	card_map,
	uint32_t				map_key,
	uint32_t				hash,
	sfc_card*				card)
{
	uint32_t j = 0;

	for(uint32_t i = 0; i < card_map->size; i++)
	{
		sfc_card_map_uint32_item* p = &card_map->map[(hash + i) % card_map->size];

		if(p->card == NULL || p->map_key == map_key)
		{
			p->map_key = map_key;
			p->card = card;
			return 1;
		}

		if(j > 50)
			return 0;

		j++;
	}

	return 0;
}

static sfc_card_map_uint32_item*
sfc_card_map_uint32_find(
	sfc_card_map_uint32*	card_map,
	uint32_t				map_key)
{
	uint32_t hash = sfc_card_map_uint32_hash(map_key);

	for (uint32_t i = 0; i < card_map->size; i++)
	{
		sfc_card_map_uint32_item* p = &card_map->map[(hash + i) % card_map->size];

		if(p->map_key == map_key)
			return p;
	}

	return NULL;
}

/*----------------------------------------------------------------------------------*/

sfc_card_map_uint32* 
sfc_card_map_uint32_create(
	struct _sfc_app*		app,
	size_t					init_size)
{
	sfc_card_map_uint32* card_map = (sfc_card_map_uint32*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_card_map_uint32));
	assert(card_map != NULL);
	memset(card_map, 0, sizeof(sfc_card_map_uint32));

	card_map->app = app;

	assert(init_size < UINT32_MAX);
	card_map->size = (uint32_t)init_size;

	card_map->map = (sfc_card_map_uint32_item*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_card_map_uint32_item) * init_size);
	assert(card_map->map != NULL);
	memset(card_map->map, 0, sizeof(sfc_card_map_uint32_item) * init_size);

	return card_map;
}

void				
sfc_card_map_uint32_destroy(
	sfc_card_map_uint32*	card_map)
{	
	sfc_card_map_uint32_clear(card_map);
	
	if(card_map->map != NULL)
		card_map->app->free(card_map->app->user_data, card_map->map);

	card_map->app->free(card_map->app->user_data, card_map);
}

sfc_result			
sfc_card_map_uint32_set(
	sfc_card_map_uint32*	card_map,
	uint32_t				map_key,
	sfc_card*				card)
{
	assert(map_key != 0);

	uint32_t hash = sfc_card_map_uint32_hash(map_key);

	while(!sfc_card_map_uint32_try_insert(card_map, map_key, hash, card))
	{	
		sfc_result result = sfc_card_map_uint32_grow(card_map);
		if(result != SFC_RESULT_OK)
			return result;
	}

	return SFC_RESULT_OK;
}

int					
sfc_card_map_uint32_has(
	sfc_card_map_uint32*	card_map,
	uint32_t				map_key)
{
	assert(map_key != 0);

	return sfc_card_map_uint32_find(card_map, map_key) != NULL;
}

sfc_card* 
sfc_card_map_uint32_get(
	sfc_card_map_uint32*	card_map,
	uint32_t				map_key)
{
	assert(map_key != 0);

	sfc_card_map_uint32_item* item = sfc_card_map_uint32_find(card_map, map_key);
	if(item == NULL)
		return NULL;

	return item->card;
}

void				
sfc_card_map_uint32_clear(
	sfc_card_map_uint32*	card_map)
{
	memset(card_map->map, 0, card_map->size * sizeof(sfc_card_map_uint32_item));
}

sfc_result
sfc_card_map_uint32_grow(
	sfc_card_map_uint32*	card_map)
{
	sfc_card_map_uint32_item* old_map = card_map->map;
	uint32_t old_size = card_map->size;
	assert(old_size < UINT32_MAX / 2);

	for(uint32_t i = 0; i < 10; i++)
	{
		card_map->size = card_map->size * 2;
		card_map->map = (sfc_card_map_uint32_item*)SFC_ALLOC(card_map->app->alloc, card_map->app->user_data, NULL, sizeof(sfc_card_map_uint32_item) * card_map->size);
		assert(card_map->map != NULL);

		memset(card_map->map, 0, sizeof(sfc_card_map_uint32_item) * card_map->size);

		{
			int success = 1;

			for (uint32_t i = 0; i < old_size; i++)
			{
				sfc_card_map_uint32_item* old = &old_map[i];
				
				if(old->card != NULL)
				{
					if (!sfc_card_map_uint32_try_insert(card_map, old->map_key, sfc_card_map_uint32_hash(old->map_key), old->card))
					{
						success = 0;
						break;
					}
				}				
			}

			if(success)
			{
				card_map->app->free(card_map->app->user_data, old_map);
				return SFC_RESULT_OK;
			}
		}
	}

	card_map->app->free(card_map->app->user_data, old_map);
	return SFC_RESULT_INTERNAL_ERROR;
}


