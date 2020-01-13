/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_ERROR
#define H_PANCL_ERROR

/**
 * End of Input (internal value used by lexer)
 */
#define PANCL_END_OF_INPUT  (-1)

/**
 * Successful operation.
 */
#define PANCL_SUCCESS   0

/**
 * init() operation failed.
 */
#define PANCL_ERROR_CTX_INIT       1

/**
 * An unknown internal failure occured.
 */
#define PANCL_ERROR_INTERNAL       2

/**
 * Allocation failure
 */
#define PANCL_ERROR_ALLOC         10
/**
 * Invalid argument to function
 */
#define PANCL_ERROR_ARG_INVALID   11

/**
 * Failure occured refilling the lexer buffer.
 */
#define PANCL_ERROR_LEXER_REFILL  100

/**
 * Parser encountered an unexpected end of file/input.
 */
#define PANCL_ERROR_PARSER_EOF               200
/**
 * Parser encountered an invalid character (unknown token type).
 */
#define PANCL_ERROR_PARSER_TOKEN             201
/**
 * Parser failed while parsing a table header.
 */
#define PANCL_ERROR_PARSER_TABLE_HEADER      202
/**
 * Parser failed while parsing an assignment.
 */
#define PANCL_ERROR_PARSER_ASSIGNMENT        203
/**
 * Parser encountered an invalid rvalue token.
 */
#define PANCL_ERROR_PARSER_RVALUE            204
/**
 * Parser failed while parsing an array.
 */
#define PANCL_ERROR_PARSER_ARRAY             205
/**
 * Parser failed while parsing a tuple.
 */
#define PANCL_ERROR_PARSER_TUPLE             206
/**
 * Parser failed while parsing an inline table.
 */
#define PANCL_ERROR_PARSER_INLINE_TABLE      207
/**
 * Parser failed to find custom type arguments list.
 */
#define PANCL_ERROR_PARSER_CUSTOM_ARGS       208

/**
 * When constructing an array, an item of a different type than the array
 * holds was given.
 */
#define PANCL_ERROR_ARRAY_MEMBER_TYPE        300

/**
 * \x followed by 0 hexadecimal digits.
 */
#define PANCL_ERROR_STR_ESC_X          7000
/**
 * \u received less than 4 hexadecimal digits.
 */
#define PANCL_ERROR_STR_ESC_LU         7001
/**
 * \U received less than 8 hexadecimal digits.
 */
#define PANCL_ERROR_STR_ESC_UU         7002
/**
 * Octal escape too big to fit in a character (> 0377)
 */
#define PANCL_ERROR_STR_ESC_OCTAL_DOM  7003
/**
 * End of Input reached before finding closing quote
 */
#define PANCL_ERROR_STR_SHORT          7004


/**
 * Invalid UTF-8 value in range 0xd800 - 0xdfff.
 * (UTF-16 surrogate values)
 */
#define PANCL_ERROR_UTF16_SURROGATE   8000
/**
 * Invalid UTF-8 value 0xfffe or 0xffff.
 * (UCS noncharacters)
 */
#define PANCL_ERROR_UCS_NONCHAR       8001
/**
 * Invalid UTF-8 value in range 0x110000 - 0xffffffff.
 */
#define PANCL_ERROR_UTF8_HIGH         8002
/**
 * Truncated UTF-8 codepoint (decoding)
 */
#define PANCL_ERROR_UTF8_TRUNC        8003
/**
 * Invalid character encountered while decoding.
 */
#define PANCL_ERROR_UTF8_DECODE       8004


/**
 * Conversion from string to integer was given and invalid base.
 */
#define PANCL_ERROR_STR_TO_INT_BASE   9000
/**
 * Conversion from string to integer had an invalid character.
 */
#define PANCL_ERROR_STR_TO_INT_CHAR   9001
/**
 * Conversion from string to integer caused an overflow or underflow.
 */
#define PANCL_ERROR_STR_TO_INT_RANGE  9002

/**
 * Optional/extended integer type invalid argument count.
 */
#define PANCL_ERROR_OPT_INT_ARG_COUNT         10000
/**
 * Optional/extended integer type's first argument is not a string.
 */
#define PANCL_ERROR_OPT_INT_ARG_0_NOT_STRING  10001
/**
 * Optional/extended integer type's second argument is not an integer.
 */
#define PANCL_ERROR_OPT_INT_ARG_1_NOT_INT     10002

#endif /* H_PANCL_ERROR */
// vim:ts=4:sw=4:autoindent
