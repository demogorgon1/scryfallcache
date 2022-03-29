#ifndef __SCRYFALLCACHE_STRING_SET_H__
#define __SCRYFALLCACHE_STRING_SET_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct _sfc_string_set
{
	struct _sfc_app*		app;		

	size_t					count;
	char**					strings;
} sfc_string_set;

sfc_string_set*		sfc_string_set_create(
						struct _sfc_app*			app);

void				sfc_string_set_destroy(
						sfc_string_set*				string_set);

sfc_result			sfc_string_set_add(
						sfc_string_set*				string_set,
						const char*					string);

int					sfc_string_set_has(
						sfc_string_set*				string_set,
						const char*					string);

void				sfc_string_set_clear(
						sfc_string_set*				string_set);

void				sfc_string_set_serialize(
						sfc_string_set*				string_set,
						struct _sfc_serializer*		serializer);

sfc_result			sfc_string_set_deserialize(
						sfc_string_set*				string_set,
						struct _sfc_deserializer*	deserializer);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_STRING_SET_H__ */