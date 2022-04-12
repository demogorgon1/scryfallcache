#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_card_array.h>

#include "sfc_card_set.h"

sfc_card_set* 
sfc_card_set_create(
	struct _sfc_app*	app)
{
	sfc_card_set* card_set = (sfc_card_set*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_card_set));
	assert(card_set != NULL);
	memset(card_set, 0, sizeof(sfc_card_set));

	card_set->app = app;

	card_set->card_array = sfc_card_array_create(app);

	return card_set;
}

void				
sfc_card_set_destroy(
	sfc_card_set*		card_set)
{	
	sfc_card_set_clear(card_set);
	sfc_card_array_destroy(card_set->card_array);
	card_set->app->free(card_set->app->user_data, card_set);
}

sfc_result			
sfc_card_set_add(
	sfc_card_set*		card_set,
	sfc_card*			card)
{
	if(sfc_card_set_has(card_set, &card->key))
		return SFC_RESULT_ALREADY_EXISTS;

	return sfc_card_array_add(card_set->card_array, card);
}

int					
sfc_card_set_has(
	sfc_card_set*		card_set,
	const sfc_card_key*	card_key)
{
	for(size_t i = 0; i < card_set->card_array->count; i++)
	{
		const sfc_card* card = card_set->card_array->cards[i];

		if(card->key.collector_number == card_key->collector_number &&
			strcmp(card->key.set, card_key->set) == 0)
		{
			return 1;
		}
	}

	return 0;
}

sfc_card* 
sfc_card_set_get(
	sfc_card_set*		card_set,
	const sfc_card_key*	card_key)
{
	for (size_t i = 0; i < card_set->card_array->count; i++)
	{
		sfc_card* card = card_set->card_array->cards[i];

		if (card->key.collector_number == card_key->collector_number &&
			strcmp(card->key.set, card_key->set) == 0)
		{
			return card;
		}
	}

	return NULL;
}

void				
sfc_card_set_clear(
	sfc_card_set*		card_set)
{
	card_set->card_array->count = 0;
}

