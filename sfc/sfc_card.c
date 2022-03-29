#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_card.h>

#include "sfc_deserializer.h"
#include "sfc_serializer.h"

#define JSMN_STATIC
#include "jsmn.h"

#define JSON_GET_STRING(_string)																\
	{																							\
		if (token_count == 0 || (token->type != JSMN_STRING && token->type != JSMN_PRIMITIVE))	\
			return SFC_RESULT_JSON_RESPONSE_ERROR;												\
		_string = &json[token->start];															\
		json[token->end] = '\0';																\
		token_count--;																			\
		token++;																				\
	}

#define JSON_GET_STRING_OFFSET(_string_offset)													\
	{																							\
		if (token_count == 0 || (token->type != JSMN_STRING && token->type != JSMN_PRIMITIVE))	\
			return SFC_RESULT_JSON_RESPONSE_ERROR;												\
		_string_offset = (uint32_t)token->start;												\
		json[token->end] = '\0';																\
		token_count--;																			\
		token++;																				\
	}

#define JSON_IGNORE_STRING()																	\
	{																							\
		if (token_count == 0 || (token->type != JSMN_STRING && token->type != JSMN_PRIMITIVE))	\
			return SFC_RESULT_JSON_RESPONSE_ERROR;												\
		token_count--;																			\
		token++;																				\
	}

typedef struct _sfc_card_string_entry 
{
	const char*			object;
	const char*			p;
	sfc_card_string		string;
} sfc_card_string_entry;

static sfc_card_string_entry g_sfc_card_string_entries[] = 
{
	/* Root values */
	NULL,			"released_at",						SFC_CARD_STRING_RELEASED_AT,
	NULL,			"rarity",							SFC_CARD_STRING_RARITY,
	NULL,			"lang",								SFC_CARD_STRING_LANGUAGE,
	NULL,			"scryfall_id",						SFC_CARD_STRING_SCRYFALL_ID,
	NULL,			"layout",							SFC_CARD_STRING_LAYOUT,
	NULL,			"mana_cost",						SFC_CARD_STRING_MANA_COST,
	NULL,			"cmc",								SFC_CARD_STRING_CMC,
	NULL,			"type_line",						SFC_CARD_STRING_TYPE_LINE,
	NULL,			"oracle_text",						SFC_CARD_STRING_ORACLE_TEXT,
	NULL,			"reserved",							SFC_CARD_STRING_RESERVED,
	NULL,			"foil",								SFC_CARD_STRING_FOIL,
	NULL,			"nonfoil",							SFC_CARD_STRING_NONFOIL,
	NULL,			"oversized",						SFC_CARD_STRING_OVERSIZED,
	NULL,			"promo",							SFC_CARD_STRING_PROMO,
	NULL,			"reprint",							SFC_CARD_STRING_REPRINT,
	NULL,			"variation",						SFC_CARD_STRING_VARIATION,
	NULL,			"set_name",							SFC_CARD_STRING_SET_NAME,
	NULL,			"set_type",							SFC_CARD_STRING_SET_TYPE,
	NULL,			"digital",							SFC_CARD_STRING_DIGITAL,
	NULL,			"flavor_text",						SFC_CARD_STRING_FLAVOR_TEXT,
	NULL,			"artist",							SFC_CARD_STRING_ARTIST,
	NULL,			"back_id",							SFC_CARD_STRING_BACK_ID,
	NULL,			"illustration_id",					SFC_CARD_STRING_ILLUSTRATION_ID,
	NULL,			"border_color",						SFC_CARD_STRING_BORDER_COLOR,
	NULL,			"frame",							SFC_CARD_STRING_FRAME,
	NULL,			"full_art",							SFC_CARD_STRING_FULL_ART,
	NULL,			"textless",							SFC_CARD_STRING_TEXTLESS,
	NULL,			"booster",							SFC_CARD_STRING_BOOSTER,
	NULL,			"power",							SFC_CARD_STRING_POWER,
	NULL,			"toughness",						SFC_CARD_STRING_TOUGHNESS,

	/* Image URIs */
	"image_uris",	"small",							SFC_CARD_STRING_IMAGE_URI_SMALL,
	"image_uris",	"normal",							SFC_CARD_STRING_IMAGE_URI_NORMAL,
	"image_uris",	"large",							SFC_CARD_STRING_IMAGE_URI_LARGE,
	"image_uris",	"png",								SFC_CARD_STRING_IMAGE_URI_PNG,
	"image_uris",	"art_crop",							SFC_CARD_STRING_IMAGE_URI_ART_CROP,
	"image_uris",	"border_crop",						SFC_CARD_STRING_IMAGE_URI_BORDER_CROP,

	/* Prices */
	"prices",		"usd",								SFC_CARD_STRING_PRICE_USD,
	"prices",		"usd_foil",							SFC_CARD_STRING_PRICE_USD_FOIL,
	"prices",		"usd_etched",						SFC_CARD_STRING_PRICE_USD_ETCHED,
	"prices",		"eur",								SFC_CARD_STRING_PRICE_EUR,
	"prices",		"eur_foil",							SFC_CARD_STRING_PRICE_EUR_FOIL,
	"prices",		"tix",								SFC_CARD_STRING_PRICE_TIX,

	/* Legalities */
	"legalities",	"standard",							SFC_CARD_STRING_LEGALITY_STANDARD,
	"legalities",	"future",							SFC_CARD_STRING_LEGALITY_FUTURE,
	"legalities",	"historic",							SFC_CARD_STRING_LEGALITY_HISTORIC,
	"legalities",	"gladiator",						SFC_CARD_STRING_LEGALITY_GLADIATOR,
	"legalities",	"pioneer",							SFC_CARD_STRING_LEGALITY_PIONEER,
	"legalities",	"modern",							SFC_CARD_STRING_LEGALITY_MODERN,
	"legalities",	"legacy",							SFC_CARD_STRING_LEGALITY_LEGACY,
	"legalities",	"pauper",							SFC_CARD_STRING_LEGALITY_PAUPER,
	"legalities",	"vintage",							SFC_CARD_STRING_LEGALITY_VINTAGE,
	"legalities",	"penny",							SFC_CARD_STRING_LEGALITY_PENNY,
	"legalities",	"commander",						SFC_CARD_STRING_LEGALITY_COMMANDER,
	"legalities",	"brawl",							SFC_CARD_STRING_LEGALITY_BRAWL,
	"legalities",	"historicbrawl",					SFC_CARD_STRING_LEGALITY_HISTORICBRAWL,
	"legalities",	"alchemy",							SFC_CARD_STRING_LEGALITY_ALCHEMY,
	"legalities",	"paupercommander",					SFC_CARD_STRING_LEGALITY_PAUPERCOMMANDER,
	"legalities",	"duel",								SFC_CARD_STRING_LEGALITY_DUEL,
	"legalities",	"oldschool",						SFC_CARD_STRING_LEGALITY_OLDSCHOOL,
	"legalities",	"premodern",						SFC_CARD_STRING_LEGALITY_PREMODERN,

	NULL,			NULL
};

static const sfc_card_string_entry*
sfc_card_get_string_entry(
	const sfc_bool*		string_filter,
	const char*			object,
	const char*			string)
{
	assert(object != NULL);
	assert(string != NULL);
	assert(string_filter != NULL);
	
	const sfc_card_string_entry* t = g_sfc_card_string_entries;
	while(t->p != NULL)
	{	
		const char* entry_object = t->object != NULL ? t->object : "";

		if(strcmp(t->p, string) == 0 && strcmp(entry_object, object) == 0)
		{
			if(!string_filter[t->string])
				return NULL;

			return t;
		}

		t++;
	}

	return NULL;
}

static const sfc_card_string_entry*
sfc_card_get_string_entry_by_enum(
	sfc_card_string		string)
{
	const sfc_card_string_entry* t = g_sfc_card_string_entries;
	while (t->p != NULL)
	{
		if(t->string == string)
			return t;

		t++;
	}

	return NULL;
}

static uint32_t
sfc_card_calculate_string_data_size(
	sfc_card*			card)
{
	size_t size = 0;

	for (int i = 0; i < SFC_CARD_STRING_COUNT; i++)
	{
		uint32_t offset = card->data.string_data_offsets[i];

		if(offset != UINT32_MAX)
		{
			const char* p = card->data.string_data + offset;
			size += strlen(p) + 1;
		}
	}

	if(size > UINT32_MAX)
		return 0;

	return (uint32_t)size;
}

static uint32_t
sfc_card_count_string_data(
	sfc_card*			card)
{
	uint32_t count = 0;

	for (int i = 0; i < SFC_CARD_STRING_COUNT; i++)
	{
		uint32_t offset = card->data.string_data_offsets[i];
		if(offset != UINT32_MAX)
			count++;
	}

	return count;
}

static void
sfc_card_shrink_string_data(
	sfc_card*			card)
{
	if(card->data.flags & SFC_CARD_IS_SHRUNK)
		return;

	uint32_t shrunk_string_data_size = sfc_card_calculate_string_data_size(card);
	
	char* new_string_data = SFC_ALLOC(card->app->alloc, card->app->user_data, NULL, shrunk_string_data_size);
	assert(new_string_data != NULL);
	memset(new_string_data, 0, shrunk_string_data_size);

	uint32_t offset = 0;

	for (int i = 0; i < SFC_CARD_STRING_COUNT; i++)
	{
		uint32_t old_offset = card->data.string_data_offsets[i];

		if (old_offset != UINT32_MAX)
		{
			const char* p = card->data.string_data + old_offset;
			size_t len = strlen(p);
			assert(len < UINT32_MAX);

			memcpy(new_string_data + offset, p, len + 1);

			card->data.string_data_offsets[i] = offset;

			offset += (uint32_t)len + 1;
		}
	}

	card->app->free(card->app->user_data, card->data.string_data);

	card->data.string_data = new_string_data;
	card->data.string_data_size = offset;

	card->data.flags |= SFC_CARD_IS_SHRUNK;
}

static uint8_t
sfc_card_get_color_flag(
	const char*		string)
{
	if (strcmp(string, "W") == 0)
		return SFC_CARD_COLOR_WHITE;
	if (strcmp(string, "B") == 0)
		return SFC_CARD_COLOR_BLACK;
	if (strcmp(string, "G") == 0)
		return SFC_CARD_COLOR_GREEN;
	if (strcmp(string, "U") == 0)
		return SFC_CARD_COLOR_BLUE;
	if (strcmp(string, "R") == 0)
		return SFC_CARD_COLOR_RED;

	return 0;
}

/*---------------------------------------------------------------------------*/

sfc_card* 
sfc_card_create(
	struct _sfc_app*	app,
	const sfc_bool*		string_filter)
{
	sfc_card* card;

	card = (sfc_card*)SFC_ALLOC(app->alloc, app->user_data, NULL, sizeof(sfc_card));
	assert(card != NULL);

	memset(card, 0, sizeof(sfc_card));

	for(int i = 0; i < SFC_CARD_STRING_COUNT; i++)
		card->data.string_data_offsets[i] = UINT32_MAX;

	card->app = app;
	card->string_filter = string_filter;

	return card;
}

void			
sfc_card_destroy(
	sfc_card*			card)
{
	if(card->data.string_data != NULL)
		card->app->free(card->app->user_data, card->data.string_data);

	card->app->free(card->app->user_data, card);
}

sfc_result		
sfc_card_parse_json(
	sfc_card*			card,
	char*				json,
	size_t				json_size)
{
	jsmn_parser json_parser;
	int token_count;
	jsmntok_t tokens[1024];
	const jsmntok_t* token;
	int root_size;

	card->data.timestamp = time(NULL);

	if(json_size == 0 || json_size > UINT32_MAX)
		return SFC_RESULT_JSON_RESPONSE_ERROR;

	jsmn_init(&json_parser);

	token_count = jsmn_parse(
		&json_parser,
		json,
		json_size,
		tokens,
		sizeof(tokens) / sizeof(jsmntok_t));

	if (token_count < 0)
		return SFC_RESULT_JSON_PARSE_ERROR;

	token = tokens;

	if (token_count == 0 || token->type != JSMN_OBJECT)
		return SFC_RESULT_JSON_RESPONSE_ERROR;

	root_size = token->size;
	token++;
	token_count--;

	const char* object;

	for (int i = 0; i < root_size; i++)
	{
		object = NULL;

		const char* key;
		JSON_GET_STRING(key);

		if (token_count == 0)
			return SFC_RESULT_JSON_RESPONSE_ERROR;

		switch(token->type)
		{
		case JSMN_OBJECT:
			if(strcmp(key, "image_uris") == 0 || strcmp(key, "prices") == 0 || strcmp(key, "legalities") == 0)
			{
				int sub_count = token->size;
				token++;
				token_count--;

				for (int j = 0; j < sub_count; j++)
				{
					const char* sub_key;
					JSON_GET_STRING(sub_key);

					const sfc_card_string_entry* string_entry = sfc_card_get_string_entry(card->string_filter, key, sub_key);
					if (string_entry != NULL)
					{
						JSON_GET_STRING_OFFSET(card->data.string_data_offsets[string_entry->string]);
					}
					else
					{
						JSON_IGNORE_STRING();
					}
				}
			}
			else
			{
				int end_of_object = token->end;

				while(token_count > 0 && token->start < end_of_object)
				{
					token++;
					token_count--;
				}
			}	
			break;

		case JSMN_ARRAY:
			if(strcmp(key, "colors") == 0)
			{
				card->data.flags |= SFC_CARD_COLORS;

				int sub_count = token->size;
				token++;
				token_count--;

				for(int j = 0; j < sub_count; j++)
				{
					const char* value;
					JSON_GET_STRING(value);
					card->data.colors |= sfc_card_get_color_flag(value);
				}
			}
			else if (strcmp(key, "color_identity") == 0)
			{
				card->data.flags |= SFC_CARD_COLOR_IDENTITY;

				int sub_count = token->size;
				token++;
				token_count--;

				for (int j = 0; j < sub_count; j++)
				{
					const char* value;
					JSON_GET_STRING(value);
					card->data.color_identity |= sfc_card_get_color_flag(value);
				}
			}
			else
			{
				int end_of_array = token->end;

				while (token_count > 0 && token->start < end_of_array)
				{
					token++;
					token_count--;
				}
			}
			break;

		case JSMN_STRING:
		case JSMN_PRIMITIVE:
			if (strcmp(key, "name") == 0)
			{
				const char* value;
				JSON_GET_STRING(value);
				size_t len = strlen(value);
				if(len >= SFC_MAX_NAME)
					return SFC_RESULT_JSON_NAME_TOO_LONG;
				strcpy(card->key.name, value);
			}
			else if (strcmp(key, "set") == 0)
			{
				const char* value;
				JSON_GET_STRING(value);
				size_t len = strlen(value);
				if (len >= SFC_MAX_SET)
					return SFC_RESULT_JSON_SET_TOO_LONG;
				strcpy(card->key.set, value);
			}
			else if (strcmp(key, "object") == 0)
			{
				const char* value; 
				JSON_GET_STRING(value);
				if (strcmp(value, "card") == 0)
					card->data.flags |= SFC_CARD_IS_CARD;
				else if (strcmp(value, "error") == 0)
					return SFC_RESULT_NOT_FOUND;
			}
			else if (strcmp(key, "tcgplayer_id") == 0)
			{
				const char* value;
				JSON_GET_STRING(value);
				card->data.flags |= SFC_CARD_TCGPLAYER_ID;
				card->data.tcgplayer_id = strtoul(value, NULL, 10);
			}
			else if (strcmp(key, "cardmarket_id") == 0)
			{
				const char* value;
				JSON_GET_STRING(value);
				card->data.flags |= SFC_CARD_CARDMARKET_ID;
				card->data.cardmarket_id = strtoul(value, NULL, 10);
			}
			else if(strcmp(key, "collector_number") == 0)
			{
				char* value;
				JSON_GET_STRING(value);
				card->data.flags |= SFC_CARD_COLLECTOR_NUMBER;

				{
					size_t len = strlen(value);
					if(len > 0 && isalpha(value[len - 1]))
					{
						card->key.version = 1 + (uint8_t)(value[len - 1] - 'a');
						value[len - 1] = '\0';
					}
				}

				{
					uint32_t temp = strtoul(value, NULL, 10);
					if(temp > UINT16_MAX)
						return SFC_RESULT_JSON_COLLECTOR_NUMBER_TOO_LARGE;
						
					card->data.collector_number = (uint16_t)temp;
				}
			}
			else
			{
				const sfc_card_string_entry* string_entry = sfc_card_get_string_entry(card->string_filter, "", key);
				if(string_entry != NULL)
				{
					JSON_GET_STRING_OFFSET(card->data.string_data_offsets[string_entry->string]);
				}
				else
				{
					JSON_IGNORE_STRING();
				}
			}
			break;

		default:
			assert(0);
			break;
		}
			
	}

	if(card->key.name[0] == '\0')
		return SFC_RESULT_JSON_NO_NAME;

	if (card->key.set[0] == '\0')
		return SFC_RESULT_JSON_NO_SET;

	card->data.string_data = json;
	card->data.string_data_size = (uint32_t)json_size;

	return SFC_RESULT_OK;
}

void		
sfc_card_serialize(
	sfc_card*				card,
	struct _sfc_serializer*	serializer)
{
	sfc_card_shrink_string_data(card);

	/* Key */
	SFC_SERIALIZER_WRITE_STRING(serializer, card->key.name);
	SFC_SERIALIZER_WRITE_STRING(serializer, card->key.set);
	SFC_SERIALIZER_WRITE_UINT8(serializer, card->key.version);

	/* Data */
	SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, card->data.string_data_size);
	sfc_serializer_write(serializer, card->data.string_data, card->data.string_data_size);

	SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, sfc_card_count_string_data(card));

	for(int i = 0; i < SFC_CARD_STRING_COUNT; i++)
	{
		uint32_t offset = card->data.string_data_offsets[i];
		if(offset != UINT32_MAX)
		{
			SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, i);
			SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, offset);
		}
	}

	SFC_SERIALIZER_WRITE_UINT32(serializer, card->data.flags);
	
	if(card->data.flags & SFC_CARD_TCGPLAYER_ID)
		SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, card->data.tcgplayer_id);

	if (card->data.flags & SFC_CARD_CARDMARKET_ID)
		SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, card->data.cardmarket_id);

	if (card->data.flags & SFC_CARD_COLLECTOR_NUMBER)
		SFC_SERIALIZER_WRITE_VAR_SIZE_UINT32(serializer, (uint32_t)card->data.collector_number);

	if (card->data.flags & SFC_CARD_COLOR_IDENTITY)
		SFC_SERIALIZER_WRITE_UINT8(serializer, (uint32_t)card->data.color_identity);

	if (card->data.flags & SFC_CARD_COLORS)
		SFC_SERIALIZER_WRITE_UINT8(serializer, (uint32_t)card->data.colors);

	SFC_SERIALIZER_WRITE_UINT64(serializer, card->data.timestamp);
}

sfc_result		
sfc_card_deserialize(
	sfc_card*					card,
	struct _sfc_deserializer*	deserializer)
{
	/* Key */
	SFC_DESERIALIZER_READ_STRING(deserializer, card->key.name, sizeof(card->key.name));
	SFC_DESERIALIZER_READ_STRING(deserializer, card->key.set, sizeof(card->key.set));
	SFC_DESERIALIZER_READ_UINT8(deserializer, &card->key.version);

	/* Data */
	SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &card->data.string_data_size);
	card->data.string_data = (char*)SFC_ALLOC(card->app->alloc, card->app->user_data, NULL, card->data.string_data_size);
	SFC_DESERIALIZER_READ_BUFFER(deserializer, card->data.string_data, card->data.string_data_size);
	
	{
		uint32_t string_data_count;
		SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &string_data_count);

		for(uint32_t i = 0; i < string_data_count; i++)
		{
			uint32_t index;
			SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &index);
			if(index >= SFC_CARD_STRING_COUNT)
				return SFC_RESULT_OUT_OF_BOUNDS;

			uint32_t offset;
			SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &offset);
			if (offset >= card->data.string_data_size)
				return SFC_RESULT_OUT_OF_BOUNDS;

			if(card->string_filter[index])
				card->data.string_data_offsets[index] = offset;
		}
	}

	SFC_DESERIALIZER_READ_UINT32(deserializer, &card->data.flags);

	if (card->data.flags & SFC_CARD_TCGPLAYER_ID)
		SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &card->data.tcgplayer_id);

	if (card->data.flags & SFC_CARD_CARDMARKET_ID)
		SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &card->data.cardmarket_id);

	if (card->data.flags & SFC_CARD_COLLECTOR_NUMBER)
	{
		uint32_t temp;
		SFC_DESERIALIZER_READ_VAR_SIZE_UINT32(deserializer, &temp);
		if(temp > UINT16_MAX)
			return SFC_RESULT_UINT16_TOO_BIG;

		card->data.collector_number = (uint16_t)temp;
	}

	if (card->data.flags & SFC_CARD_COLOR_IDENTITY)
		SFC_DESERIALIZER_READ_UINT8(deserializer, &card->data.color_identity);

	if (card->data.flags & SFC_CARD_COLORS)
		SFC_DESERIALIZER_READ_UINT8(deserializer, &card->data.colors);

	{
		uint64_t temp;
		SFC_DESERIALIZER_READ_UINT64(deserializer, &temp);		
		card->data.timestamp = (time_t)temp;
	}

	return SFC_RESULT_OK;
}

void			
sfc_card_debug_print(
	sfc_card*		card)
{
	printf("key.name: %s\n", card->key.name);
	printf("key.set: %s\n", card->key.set);
	printf("key.version: %u\n", card->key.version);
	
	printf("data.flags:");

	if(card->data.flags & SFC_CARD_IS_CARD)
		printf(" IS_CARD");
	if (card->data.flags & SFC_CARD_TCGPLAYER_ID)
		printf(" TCGPLAYER_ID");
	if (card->data.flags & SFC_CARD_CARDMARKET_ID)
		printf(" CARDMARKET_ID");
	if (card->data.flags & SFC_CARD_COLLECTOR_NUMBER)
		printf(" COLLECTOR_NUMBER");
	if (card->data.flags & SFC_CARD_IS_SHRUNK)
		printf(" IS_SHRUNK");
	if (card->data.flags & SFC_CARD_COLORS)
		printf(" COLORS");
	if (card->data.flags & SFC_CARD_COLOR_IDENTITY)
		printf(" COLOR_IDENTITY");

	printf("\n");

	printf("data.tcg_player_id: %u\n", card->data.tcgplayer_id);
	printf("data.cardmarket_id: %u\n", card->data.cardmarket_id);
	printf("data.collector_number: %u\n", card->data.collector_number);
	printf("data.colors: %x\n", card->data.colors);
	printf("data.color_identity: %x\n", card->data.color_identity);

	for (int i = 0; i < SFC_CARD_STRING_COUNT; i++)
	{
		const char* p = sfc_card_get_string(card, (sfc_card_string)i);

		if(*p != '\0')
		{
			const sfc_card_string_entry* t = sfc_card_get_string_entry_by_enum((sfc_card_string)i);

			printf("data.");

			if(t->object != NULL)
				printf("%s.", t->object);

			printf("%s: %s\n", t->p, p);
		}
	}

	printf("data.timestamp: %llu\n", card->data.timestamp);
}

const char* 
sfc_card_get_string(
	sfc_card*		card,
	sfc_card_string	card_string)
{
	if(card_string >= 0 && card_string < SFC_CARD_STRING_COUNT)
	{
		uint32_t offset = card->data.string_data_offsets[card_string];
		if(offset < card->data.string_data_size)
			return card->data.string_data + offset;

		return "";
	}

	return "";
}
