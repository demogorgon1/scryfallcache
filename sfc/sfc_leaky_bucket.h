#ifndef __SCRYFALLCACHE_LEAKY_BUCKET_H__
#define __SCRYFALLCACHE_LEAKY_BUCKET_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_leaky_bucket
{
	struct _sfc_app*		app;		

	uint64_t				refill_rate;
	uint64_t				bucket_size;
	uint64_t				tokens;
	uint64_t				last_refill_time;
} sfc_leaky_bucket;

sfc_leaky_bucket*	sfc_leaky_bucket_create(
						struct _sfc_app*		app,
						uint64_t				refill_rate,
						uint64_t				bucket_size);

void				sfc_leaky_bucket_destroy(
						sfc_leaky_bucket*		leaky_bucket);

int					sfc_leaky_bucket_acquire_token(
						sfc_leaky_bucket*		leaky_bucket);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_LEAKY_BUCKET_H__ */