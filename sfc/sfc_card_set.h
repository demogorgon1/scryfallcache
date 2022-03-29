#ifndef __SCRYFALLCACHE_CARD_SET_H__
#define __SCRYFALLCACHE_CARD_SET_H__

#include <sfc/sfc_base.h>
#include <sfc/sfc_card.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_card_set
{
	struct _sfc_app*		app;		

	/* FIXME: hash table */
	struct _sfc_card_array*	card_array;
} sfc_card_set;

sfc_card_set*		sfc_card_set_create(
						struct _sfc_app*		app);

void				sfc_card_set_destroy(
						sfc_card_set*			card_set);

sfc_result			sfc_card_set_add(
						sfc_card_set*			card_set,
						sfc_card*				card);

int					sfc_card_set_has(
						sfc_card_set*			card_set,
						const sfc_card_key*		card_key);

sfc_card*			sfc_card_set_get(
						sfc_card_set*			card_set,
						const sfc_card_key*		card_key);

void				sfc_card_set_clear(
						sfc_card_set*			card_set);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CARD_SET_H__ */