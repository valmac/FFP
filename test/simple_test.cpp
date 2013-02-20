/*
Copyright (c) 2013, Maxim Konakov
 All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list 
   of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list 
   of conditions and the following disclaimer in the documentation and/or other materials 
   provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS 
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER 
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "test_messages.h"
#include <string.h>
#include <stdexcept>

// FIX message for tests
static
void validate_message(const struct fix_message* pm)
{
	const fix_parser* const parser = (const fix_parser*) ((const char*)pm - offsetof(fix_parser, message));

	ensure_buffer(parser->buffer);
	validate_simple_message(pm);
}

static
void basic_test()
{
	fix_parser* parser = create_fix_parser(get_dummy_classifier);
	const fix_message* pm = get_first_fix_message(parser, simple_message, simple_message_size);

	ensure(pm != nullptr);
	ensure(!get_fix_parser_error(parser));

	validate_message(pm);

	ensure(!get_next_fix_message(parser));
	free_fix_parser(parser);
}

static
void splitter_test()
{
	std::string s(copy_simple_message(4));
	fix_parser* parser = create_fix_parser(get_dummy_classifier);

	ensure(parser != nullptr);

	const size_t n = 100;
	size_t counter = 0;
	const char* p = s.c_str();
	const char* const end = p + s.size();

	for(; p < end; p += n)
	{
		for(const fix_message* pm = get_first_fix_message(parser, p, std::min(n, (size_t)(end - p))); pm; pm = get_next_fix_message(parser))
		{
			ensure(!get_fix_parser_error(parser));
			validate_message(pm);
			++counter;
		}
	}

	free_fix_parser(parser);
	ensure(counter == 4);
}

static
void invalid_message_test()
{
	const char m[] = "8=FIX.4.4\x01" "9=122\x01" "35=D\x02";

	fix_parser* parser = create_fix_parser(get_dummy_classifier);
	const fix_message* pm = get_first_fix_message(parser, m, sizeof(m) - 1);

	ensure(!pm);
	ensure(strcmp(get_fix_parser_error(parser), "Unexpected byte 0x2 in FIX message type") == 0);
	ensure(!get_next_fix_message(parser));
	free_fix_parser(parser);
}

// speed test
static
void speed_test()
{
	test_for_speed("Simple message", simple_message_classifier, copy_simple_message, validate_simple_message);
}

// batch
void all_simple_tests()
{
	basic_test();
	splitter_test();
	invalid_message_test();
	speed_test();
}
