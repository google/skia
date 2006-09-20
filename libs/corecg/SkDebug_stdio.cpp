#include "SkTypes.h"

#if (defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)) && defined(SK_DEBUG)

#include <stdarg.h>
#include <stdio.h>

void SkDebugf(const char format[], ...)
{
	static const size_t kBufferSize = 256;

	char	buffer[kBufferSize + 1];
	va_list	args;
	va_start(args, format);
	vsnprintf(buffer, kBufferSize, format, args);
	va_end(args);
	fprintf(stderr, buffer);
}

#endif

