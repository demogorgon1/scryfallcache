#include <assert.h>
#include <string.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_card.h>
#include <sfc/sfc_card_array.h>

sfc_card_array* 
sfc_card_array_create(
	struct _sfc_app*	app)
{
	sfc_card_array* card_array = (sfc_card_array*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_card_array));
	assert(card_array != NULL);

	memset(card_array, 0, sizeof(sfc_card_array));

	card_array->app = app;

	return card_array;
}

void				
sfc_card_array_destroy(
	sfc_card_array*		card_array)
{
	if (card_array->cards != NULL)
		card_array->app->free(card_array->app->user_data, card_array->cards);

	card_array->app->free(card_array->app->user_data, card_array);
}

sfc_result			
sfc_card_array_add(
	sfc_card_array*		card_array,
	struct _sfc_card*	card)
{
	sfc_result result = SFC_RESULT_OK;

	assert(card_array->allocated >= card_array->count);

	if(card_array->allocated - card_array->count == 0)
		result = sfc_card_array_reserve(card_array, card_array->allocated + 100);

	if(result == SFC_RESULT_OK)
	{
		card_array->cards[card_array->count] = card;
		card_array->count++;
	}

	return result;
}

sfc_result			
sfc_card_array_add_sorted_by_card_key(
	sfc_card_array*		card_array,
	sfc_card*			card)
{
	assert(0); // FIXME: implement
	sfc_result result = SFC_RESULT_OK;

	assert(card_array->allocated >= card_array->count);

	if (card_array->allocated - card_array->count == 0)
		result = sfc_card_array_reserve(card_array, card_array->allocated + 100);

	if (result == SFC_RESULT_OK)
	{
		card_array->cards[card_array->count] = card;
		card_array->count++;
	}

	return result;
}

sfc_result			
sfc_card_array_add_sorted_by_collector_number(
	sfc_card_array*		card_array,
	sfc_card*			card)
{
	assert(0); // FIXME: implement
	sfc_result result = SFC_RESULT_OK;

	assert(card_array->allocated >= card_array->count);

	if (card_array->allocated - card_array->count == 0)
		result = sfc_card_array_reserve(card_array, card_array->allocated + 100);

	if (result == SFC_RESULT_OK)
	{
		card_array->cards[card_array->count] = card;
		card_array->count++;
	}

	return result;
}

sfc_result			
sfc_card_array_reserve(
	sfc_card_array*			card_array,
	size_t					count)
{
	assert(count >= card_array->allocated);

	if(count > card_array->allocated)
	{
		card_array->cards = (sfc_card**)SFC_ALLOC(card_array->app->alloc, card_array->app->user_data, card_array->cards, sizeof(sfc_card*) * count);
		assert(card_array->cards != NULL);

		card_array->allocated = count;
	}

	return SFC_RESULT_OK;
}

