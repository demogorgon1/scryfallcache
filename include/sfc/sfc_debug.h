#ifndef __SCRYFALLCACHE_DEBUG_PRINT_H__
#define	__SCRYFALLCACHE_DEBUG_PRINT_H__

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

void		sfc_debug(
				struct _sfc_app*		app,
				const char*				format,
				...);

void		sfc_debug_stdio(
				void*					user_data,
				const char*				message);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_DEBUG_PRINT_H__ */
