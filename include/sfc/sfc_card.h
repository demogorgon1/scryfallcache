#ifndef __SCRYFALLCACHE_CARD_H__
#define	__SCRYFALLCACHE_CARD_H__

#include <time.h>

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

struct _sfc_app;
struct _sfc_deserializer;
struct _sfc_serializer;

#define SFC_CARD_COLOR_WHITE		0x01
#define SFC_CARD_COLOR_BLACK		0x02
#define SFC_CARD_COLOR_BLUE			0x04
#define SFC_CARD_COLOR_RED			0x08
#define SFC_CARD_COLOR_GREEN		0x10

#define SFC_CARD_IS_CARD			0x00000001
#define SFC_CARD_TCGPLAYER_ID		0x00000002
#define SFC_CARD_CARDMARKET_ID		0x00000004
#define SFC_CARD_COLLECTOR_NUMBER	0x00000008
#define SFC_CARD_IS_SHRUNK			0x00000010
#define SFC_CARD_COLORS				0x00000020
#define SFC_CARD_COLOR_IDENTITY		0x00000040

typedef enum _sfc_card_string
{
	SFC_CARD_STRING_RELEASED_AT,
	SFC_CARD_STRING_RARITY,
	SFC_CARD_STRING_LANGUAGE,
	SFC_CARD_STRING_SCRYFALL_ID,
	SFC_CARD_STRING_LAYOUT,
	SFC_CARD_STRING_MANA_COST,
	SFC_CARD_STRING_CMC,
	SFC_CARD_STRING_TYPE_LINE,
	SFC_CARD_STRING_ORACLE_TEXT,
	SFC_CARD_STRING_RESERVED,
	SFC_CARD_STRING_FOIL,
	SFC_CARD_STRING_NONFOIL,
	SFC_CARD_STRING_OVERSIZED,
	SFC_CARD_STRING_PROMO,
	SFC_CARD_STRING_REPRINT,
	SFC_CARD_STRING_VARIATION,
	SFC_CARD_STRING_SET_NAME,
	SFC_CARD_STRING_SET_TYPE,
	SFC_CARD_STRING_DIGITAL,
	SFC_CARD_STRING_FLAVOR_TEXT,
	SFC_CARD_STRING_ARTIST,
	SFC_CARD_STRING_BACK_ID,
	SFC_CARD_STRING_ILLUSTRATION_ID,
	SFC_CARD_STRING_BORDER_COLOR,
	SFC_CARD_STRING_FRAME,
	SFC_CARD_STRING_FULL_ART,
	SFC_CARD_STRING_TEXTLESS,
	SFC_CARD_STRING_BOOSTER,
	SFC_CARD_STRING_POWER,
	SFC_CARD_STRING_TOUGHNESS,

	SFC_CARD_STRING_IMAGE_URI_SMALL,
	SFC_CARD_STRING_IMAGE_URI_NORMAL,
	SFC_CARD_STRING_IMAGE_URI_LARGE,
	SFC_CARD_STRING_IMAGE_URI_PNG,
	SFC_CARD_STRING_IMAGE_URI_ART_CROP,
	SFC_CARD_STRING_IMAGE_URI_BORDER_CROP,

	SFC_CARD_STRING_PRICE_USD,
	SFC_CARD_STRING_PRICE_USD_FOIL,
	SFC_CARD_STRING_PRICE_USD_ETCHED,
	SFC_CARD_STRING_PRICE_EUR,
	SFC_CARD_STRING_PRICE_EUR_FOIL,
	SFC_CARD_STRING_PRICE_TIX,

	SFC_CARD_STRING_LEGALITY_STANDARD,
	SFC_CARD_STRING_LEGALITY_FUTURE,
	SFC_CARD_STRING_LEGALITY_HISTORIC,
	SFC_CARD_STRING_LEGALITY_GLADIATOR,
	SFC_CARD_STRING_LEGALITY_PIONEER,
	SFC_CARD_STRING_LEGALITY_MODERN,
	SFC_CARD_STRING_LEGALITY_LEGACY,
	SFC_CARD_STRING_LEGALITY_PAUPER,
	SFC_CARD_STRING_LEGALITY_VINTAGE,
	SFC_CARD_STRING_LEGALITY_PENNY,
	SFC_CARD_STRING_LEGALITY_COMMANDER,
	SFC_CARD_STRING_LEGALITY_BRAWL,
	SFC_CARD_STRING_LEGALITY_HISTORICBRAWL,
	SFC_CARD_STRING_LEGALITY_ALCHEMY,
	SFC_CARD_STRING_LEGALITY_PAUPERCOMMANDER,
	SFC_CARD_STRING_LEGALITY_DUEL,
	SFC_CARD_STRING_LEGALITY_OLDSCHOOL,
	SFC_CARD_STRING_LEGALITY_PREMODERN,

	SFC_CARD_STRING_COUNT
} sfc_card_string;

typedef struct _sfc_card_key
{
	char					name[SFC_MAX_NAME];
	char					set[SFC_MAX_SET];
	uint8_t					version;
} sfc_card_key;

typedef struct _sfc_card_data
{
	time_t					timestamp;

	uint32_t				flags;

	uint32_t				tcgplayer_id;
	uint32_t				cardmarket_id;
	uint16_t				collector_number;
	uint8_t					colors;
	uint8_t					color_identity;

	char*					string_data;
	uint32_t				string_data_size;
	uint32_t				string_data_offsets[SFC_CARD_STRING_COUNT];
} sfc_card_data;

typedef struct _sfc_card
{
	struct _sfc_app*		app;
	const sfc_bool*			string_filter;

	sfc_card_key			key;
	sfc_card_data			data;

	struct _sfc_card*		next;
} sfc_card;

sfc_card*		sfc_card_create(
					struct _sfc_app*			app,
					const sfc_bool*				string_filter);

void			sfc_card_destroy(
					sfc_card*					card);

sfc_result		sfc_card_parse_json(
					sfc_card*					card,
					char*						json,
					size_t						json_size);

void			sfc_card_serialize(
					sfc_card*					card,
					struct _sfc_serializer*		serializer);

sfc_result		sfc_card_deserialize(
					sfc_card*					card,
					struct _sfc_deserializer*	deserializer);

void			sfc_card_debug_print(
					sfc_card*					card);

void			sfc_card_key_debug_print(
					const sfc_card_key*			card_key);

const char*		sfc_card_get_string(
					sfc_card*					card,
					sfc_card_string				card_string);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_CARD_H__ */
