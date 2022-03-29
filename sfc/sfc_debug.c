#include <stdarg.h>
#include <stdio.h>

#include <sfc/sfc_app.h>
#include <sfc/sfc_debug.h>

void		
sfc_debug(
	struct _sfc_app*		app,
	const char*				format,
	...)
{
	if(app->debug_print == NULL)
		return;

	char buffer[1024];

	{
		va_list list;
		va_start(list, format);

		int n = vsnprintf(buffer, sizeof(buffer), format, list);
		if (n < 0)
			buffer[0] = '\0';

		va_end(list);
	}

	if(buffer[0] != '\0')
	{
		app->debug_print(app->user_data, buffer);
	}		
}

void		
sfc_debug_stdio(
	void*					user_data,
	const char*				message)
{
	printf("%s\n", message);
}

