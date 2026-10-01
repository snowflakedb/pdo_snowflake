/**
 * Copyright 2012 Christoph Gärtner
 * Distributed under the Boost Software License, Version 1.0
 */

#ifndef DECODE_HTML_ENTITIES_UTF8_
#define DECODE_HTML_ENTITIES_UTF8_

#include <stddef.h>
#include <stdlib.h>

namespace Snowflake
{
	namespace Client
	{
		/*	Takes input from <src> and decodes into <dest>. Never writes more
			than <dest_size> bytes including the terminating NUL. If the decoded
			output does not fit, it is truncated (without splitting a UTF-8
			sequence) and NUL-terminated when dest_size > 0.

			If <src> is <NULL>, input will be taken from <dest>, decoding
			the entities in-place.

			If dest is NULL or dest_size is 0, nothing is written and 0 is returned.

			The function returns the length of the decoded string.
		*/
		size_t decode_html_entities_utf8(char* dest, size_t dest_size, const char* src);
	}
}

#endif
