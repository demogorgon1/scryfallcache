#include <errno.h>
#include <stdlib.h>

#include "sfc_buffer.h"
#include "sfc_file.h"

sfc_result	
sfc_file_write_buffer(
	FILE*				file,
	struct _sfc_buffer* buffer)
{
	uint8_t size_buffer[4];
	size_buffer[0] = (uint8_t)(buffer->size & 0xFF);
	size_buffer[1] = (uint8_t)((buffer->size & 0xFF00) >> 8);
	size_buffer[2] = (uint8_t)((buffer->size & 0xFF0000) >> 16);
	size_buffer[3] = (uint8_t)((buffer->size & 0xFF000000) >> 32);

	if (fwrite(size_buffer, 1, 4, file) != 4)
		return SFC_RESULT_FILE_WRITE_ERROR;

	if (fwrite(buffer->p, 1, buffer->size, file) != buffer->size)
		return SFC_RESULT_FILE_WRITE_ERROR;

	return SFC_RESULT_OK;
}

sfc_result	
sfc_file_read_buffer(
	FILE*				file,
	struct _sfc_buffer* buffer)
{
	uint8_t size_buffer[4];
	if(fread(size_buffer, 1, 4, file) != 4)
		return SFC_RESULT_FILE_READ_ERROR;

	size_t size = (size_t)size_buffer[0]
		| ((size_t)size_buffer[1] << 8)
		| ((size_t)size_buffer[2] << 16)
		| ((size_t)size_buffer[3] << 24);

	sfc_buffer_set_size(buffer, size);

	if (fread(buffer->p, 1, size, file) != size)
		return SFC_RESULT_FILE_READ_ERROR;

	return SFC_RESULT_OK;
}

sfc_result	
sfc_file_rename(
	const char*			old_path,
	const char*			new_path)
{
	sfc_result result = SFC_RESULT_OK;

	int rename_result = rename(old_path, new_path);
	if (rename_result != 0)
	{
		if (errno == EEXIST)
		{
			/* rename() works differently on some platforms (*cough* windows *cough*). It's supposed to be able to
			   move a file into an existing file, so EEXIST shouldn't happen. If it does, just delete the existing
			   one and try again. This voids the transactional property of the operation, but meh. */

			/* FIXME: we could provide a callback to platform dependent code that does this properly on affected
						platforms. */

			int remove_result = remove(new_path);
			if (remove_result != 0)
			{
				result = SFC_RESULT_FILE_RENAME_ERROR;
			}
			else
			{
				rename_result = rename(old_path, new_path);
				if (rename_result != 0)
					result = SFC_RESULT_FILE_RENAME_ERROR;
			}
		}
		else
		{
			result = SFC_RESULT_FILE_RENAME_ERROR;
		}
	}

	return result;
}
