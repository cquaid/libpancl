/* SPDX-License-Identifier: MIT */
#include "pancl.h"
#include "pancl_error.h"

#define CASE(prefix, suffix) \
	_CASE(prefix ## suffix ## _str, suffix, prefix ## suffix)
#define _CASE(str, suffix, val) \
	__CASE(str, suffix, val)
#define __CASE(str, suffix, val) \
	case val: \
		return #suffix "(" #val ") - " str

const char *
pancl_strerror(int pancl_error_code)
{
	switch (pancl_error_code) {
	/* Non-errors */
	CASE( PANCL_, SUCCESS );
	CASE( PANCL_, END_OF_INPUT );
	/* General */
	CASE( PANCL_ERROR_, CTX_INIT );
	CASE( PANCL_ERROR_, INTERNAL );
	CASE( PANCL_ERROR_, ALLOC );
	CASE( PANCL_ERROR_, ARG_INVALID );
	CASE( PANCL_ERROR_, OVERFLOW );
	/* Lexer */
	CASE( PANCL_ERROR_, LEXER_REFILL );
	CASE( PANCL_ERROR_, LEXER_COMMENT_ESC_NEWLINE );
	/* Parser */
	CASE( PANCL_ERROR_, PARSER_EOF );
	CASE( PANCL_ERROR_, PARSER_TOKEN );
	CASE( PANCL_ERROR_, PARSER_TABLE_HEADER );
	CASE( PANCL_ERROR_, PARSER_ASSIGNMENT );
	CASE( PANCL_ERROR_, PARSER_RVALUE );
	CASE( PANCL_ERROR_, PARSER_ARRAY );
	CASE( PANCL_ERROR_, PARSER_TUPLE );
	CASE( PANCL_ERROR_, PARSER_INLINE_TABLE );
	CASE( PANCL_ERROR_, PARSER_CUSTOM_ARGS );
	/* Array */
	CASE( PANCL_ERROR_, ARRAY_MEMBER_TYPE );
	/* String */
	CASE( PANCL_ERROR_, STR_SHORT );
	CASE( PANCL_ERROR_, STR_ESC_X );
	CASE( PANCL_ERROR_, STR_ESC_LU );
	CASE( PANCL_ERROR_, STR_ESC_UU );
	CASE( PANCL_ERROR_, STR_ESC_OCTAL_DOM );
	CASE( PANCL_ERROR_, STR_ESC_UNKNOWN );
	/* UTF-8 */
	CASE( PANCL_ERROR_, UTF16_SURROGATE );
	CASE( PANCL_ERROR_, UCS_NONCHAR );
	CASE( PANCL_ERROR_, UTF8_HIGH );
	CASE( PANCL_ERROR_, UTF8_TRUNC );
	CASE( PANCL_ERROR_, UTF8_DECODE );
	/* String to Integer */
	CASE( PANCL_ERROR_, STR_TO_INT_BASE );
	CASE( PANCL_ERROR_, STR_TO_INT_CHAR );
	CASE( PANCL_ERROR_, STR_TO_INT_RANGE );
	/* Optional integer types */
	CASE( PANCL_ERROR_, OPT_INT_ARG_COUNT );
	CASE( PANCL_ERROR_, OPT_INT_ARG_0_NOT_STRING );
	CASE( PANCL_ERROR_, OPT_INT_ARG_1_NOT_INT );

	default:
		return "(UNKNOWN ERROR)";
	}
}

#undef CASE

// vim:ts=4:sw=4:autoindent
