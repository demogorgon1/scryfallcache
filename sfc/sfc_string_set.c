#include <assert.h>
#include <string.h>

#include <sfc/sfc_app.h>

#include "sfc_deserializer.h"
#include "sfc_serializer.h"
#include "sfc_string_set.h"

sfc_string_set* 
sfc_string_set_create(
	struct _sfc_app*	app)
{
	sfc_string_set* string_set = (sfc_string_set*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_string_set));
	assert(string_set != NULL);
	memset(string_set, 0, sizeof(sfc_string_set));

	string_set->app = app;

	return string_set;
}

void				
sfc_string_set_destroy(
	sfc_string_set*		string_set)
{	
	sfc_string_set_clear(string_set);

	string_set->app->free(string_set->app->user_data, string_set);
}

void			
sfc_string_set_add(
	sfc_string_set*		string_set,
	const char*			string)
{
	size_t len;

	if(sfc_string_set_has(string_set, string))
		return;

	len = strlen(string);

	string_set->strings = (char**)SFC_ALLOC(string_set->app->alloc, string_set->app->user_data, string_set->strings, sizeof(char**) * (string_set->count + 1));
	assert(string_set->strings != NULL);

	string_set->strings[string_set->count] = (char*)SFC_ALLOC(string_set->app->alloc, string_set->app->user_data, NULL, len + 1);
	assert(string_set->strings[string_set->count] != NULL);

	strcpy(string_set->strings[string_set->count], string);

	string_set->count++;
}

int					
sfc_string_set_has(
	sfc_string_set*		string_set,
	const char*			string)
{
	for(size_t i = 0; i < string_set->count; i++)
	{
		if(strcmp(string_set->strings[i], string) == 0)
			return 1;
	}

	return 0;
}

void				
sfc_string_set_clear(
	sfc_string_set*				string_set)
{
	for (size_t i = 0; i < string_set->count; i++)
		string_set->app->free(string_set->app->user_data, string_set->strings[i]);

	if (string_set->strings != NULL)
		string_set->app->free(string_set->app->user_data, string_set->strings);

	string_set->strings = NULL;
	string_set->count = 0;
}

void
sfc_string_set_serialize(
	sfc_string_set*				string_set,
	struct _sfc_serializer*		serializer)
{
	SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, (uint32_t)string_set->count);

	for(size_t i = 0; i < string_set->count; i++)
		SFC_SERIALIZER_WRITE_STRING(serializer, string_set->strings[i]);
}

sfc_result			
sfc_string_set_deserialize(
	sfc_string_set*				string_set,
	struct _sfc_deserializer*	deserializer)
{
	sfc_string_set_clear(string_set);

	uint32_t count;
	SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &count);
	
	for (uint32_t i = 0; i < count; i++)
	{
		char temp[1024];
		SFC_DESERIALIZER_READ_STRING(deserializer, temp, sizeof(temp) - 1);

		sfc_string_set_add(string_set, temp);
	}

	return SFC_RESULT_OK;
}
