#include <assert.h>
#include <string.h>

#include <sfc/sfc_app.h>

#include "sfc_leaky_bucket.h"

sfc_leaky_bucket* 
sfc_leaky_bucket_create(
	struct _sfc_app*		app,
	uint64_t				refill_rate,
	uint64_t				bucket_size)
{
	sfc_leaky_bucket* leaky_bucket = (sfc_leaky_bucket*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_leaky_bucket));
	assert(leaky_bucket != NULL);
	memset(leaky_bucket, 0, sizeof(sfc_leaky_bucket));

	leaky_bucket->app = app;
	leaky_bucket->refill_rate = refill_rate;
	leaky_bucket->bucket_size = bucket_size;

	return leaky_bucket;
}

void				
sfc_leaky_bucket_destroy(
	sfc_leaky_bucket*		leaky_bucket)
{
	leaky_bucket->app->free(leaky_bucket->app->user_data, leaky_bucket);
}

int					
sfc_leaky_bucket_acquire_token(
	sfc_leaky_bucket*		leaky_bucket)
{
	uint64_t current_time;
	uint64_t time_since_last_refill;
	uint64_t increment;

	current_time = leaky_bucket->app->get_timer(leaky_bucket->app->user_data);

	if(current_time > leaky_bucket->last_refill_time)
		time_since_last_refill = current_time - leaky_bucket->last_refill_time;
	else 
		time_since_last_refill = 0;

	increment = (time_since_last_refill * leaky_bucket->refill_rate) / 1000;
	if(increment > 0)
	{
		leaky_bucket->tokens += increment;
		leaky_bucket->last_refill_time = current_time;

		if (leaky_bucket->tokens > leaky_bucket->bucket_size)
			leaky_bucket->tokens = leaky_bucket->bucket_size;
	}


	if(leaky_bucket->tokens > 0)
	{
		leaky_bucket->tokens--;
		return 1;
	}
	
	return 0;
}
