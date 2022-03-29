#ifndef __SCRYFALLCACHE_FILE_H__
#define __SCRYFALLCACHE_FILE_H__

#include <stdio.h>

#include <sfc/sfc_base.h>

#ifdef  __cplusplus
extern "C"
{
#endif

sfc_result	sfc_file_write_buffer(
				FILE*				file,
				struct _sfc_buffer*	buffer);

sfc_result	sfc_file_read_buffer(
				FILE*				file,
				struct _sfc_buffer*	buffer);

sfc_result	sfc_file_rename(
				const char*			old_path,
				const char*			new_path);

#ifdef  __cplusplus
}
#endif

#endif /* __SCRYFALLCACHE_FILE_H__ */
