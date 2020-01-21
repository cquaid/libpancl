/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_ERROR
#define H_PANCL_ERROR

/**
 * Successful operation.
 */
#define PANCL_SUCCESS  0
#define PANCL_SUCCESS_str \
	"Success"
/**
 * End of Input (not an error)
 */
#define PANCL_END_OF_INPUT  1
#define PANCL_END_OF_INPUT_str \
	"End of input"

/**
 * init() operation failed.
 */
#define PANCL_ERROR_CTX_INIT  2
#define PANCL_ERROR_CTX_INIT_str \
	"Context initialization (pancl_parse_*) failed"

/**
 * An unknown internal failure occured.
 */
#define PANCL_ERROR_INTERNAL  3
#define PANCL_ERROR_INTERNAL_str \
	"Internal failure"

/**
 * Allocation failure
 */
#define PANCL_ERROR_ALLOC  10
#define PANCL_ERROR_ALLOC_str \
	"Failed memory allocation"

/**
 * Invalid argument to function
 */
#define PANCL_ERROR_ARG_INVALID  11
#define PANCL_ERROR_ARG_INVALID_str \
	"Invalid argument"

/**
 * Addition or Multiplication would cause an overflow
 */
#define PANCL_ERROR_OVERFLOW  12
#define PANCL_ERROR_OVERFLOW_str \
	"Overflow"

/**
 * Failure occured refilling the lexer buffer.
 */
#define PANCL_ERROR_LEXER_REFILL  100
#define PANCL_ERROR_LEXER_REFILL_str \
	"Failed to refill input buffer"

/**
 * A newline escape was found at the end of a comment.
 */
#define PANCL_ERROR_LEXER_COMMENT_ESC_NEWLINE  101
#define PANCL_ERROR_LEXER_COMMENT_ESC_NEWLINE_str \
	"Unexpected '\\' outside of line ending"

/**
 * Parser encountered an unexpected end of file/input.
 */
#define PANCL_ERROR_PARSER_EOF  200
#define PANCL_ERROR_PARSER_EOF_str \
	"Unexpected end of input"

/**
 * Parser encountered an invalid character (unknown token type).
 */
#define PANCL_ERROR_PARSER_TOKEN  201
#define PANCL_ERROR_PARSER_TOKEN_str \
	"Invalid character in input"

/**
 * Parser failed while parsing a table header.
 */
#define PANCL_ERROR_PARSER_TABLE_HEADER  202
#define PANCL_ERROR_PARSER_TABLE_HEADER_str \
	"Failed parsing table header"

/**
 * Parser failed while parsing an assignment.
 */
#define PANCL_ERROR_PARSER_ASSIGNMENT  203
#define PANCL_ERROR_PARSER_ASSIGNMENT_str \
	"Failed parsing assignment"

/**
 * Parser encountered an invalid rvalue token.
 */
#define PANCL_ERROR_PARSER_RVALUE  204
#define PANCL_ERROR_PARSER_RVALUE_str \
	"Encountered an invalid RVALUE while parsing"

/**
 * Parser failed while parsing an array.
 */
#define PANCL_ERROR_PARSER_ARRAY  205
#define PANCL_ERROR_PARSER_ARRAY_str \
	"Failed parsing an array"

/**
 * Parser failed while parsing a tuple.
 */
#define PANCL_ERROR_PARSER_TUPLE  206
#define PANCL_ERROR_PARSER_TUPLE_str \
	"Failed parsing a tuple"

/**
 * Parser failed while parsing an inline table.
 */
#define PANCL_ERROR_PARSER_INLINE_TABLE  207
#define PANCL_ERROR_PARSER_INLINE_TABLE_str \
	"Failed parsing an inline table"

/**
 * Parser failed to find custom type arguments list.
 */
#define PANCL_ERROR_PARSER_CUSTOM_ARGS  208
#define PANCL_ERROR_PARSER_CUSTOM_ARGS_str \
	"Missing argument list to custom type"

/**
 * When constructing an array, an item of a different type than the array
 * holds was given.
 */
#define PANCL_ERROR_ARRAY_MEMBER_TYPE  300
#define PANCL_ERROR_ARRAY_MEMBER_TYPE_str \
	"Array defined with mixed member types"

/**
 * Integer has leading zeros and is not one of: +0, -0, 0
 */
#define PANCL_ERROR_INT_LEADING_ZEROS  6000
#define PANCL_ERROR_INT_LEADING_ZEROS_str \
	"Decimal integer found with leading zeros"

/**
 * End of Input reached before finding closing quote
 */
#define PANCL_ERROR_STR_SHORT  7000
#define PANCL_ERROR_STR_SHORT_str \
	"Unmatched quote in input"

/**
 * \x followed by 0 hexadecimal digits.
 * \x may have 1 or 2 digits.
 */
#define PANCL_ERROR_STR_ESC_X  7001
#define PANCL_ERROR_STR_ESC_X_str \
	"Missing digits for \\x escape sequence"

/**
 * \u received less than 4 hexadecimal digits.
 */
#define PANCL_ERROR_STR_ESC_LU   7002
#define PANCL_ERROR_STR_ESC_LU_str \
	"Too few digits for \\u escape sequence (expecting 4)"
/**
 * \U received less than 8 hexadecimal digits.
 */
#define PANCL_ERROR_STR_ESC_UU  7003
#define PANCL_ERROR_STR_ESC_UU_str \
	"Too few digits for \\U escape sequence (expecting 8)"

/**
 * Octal escape too big to fit in a character (> 0377)
 */
#define PANCL_ERROR_STR_ESC_OCTAL_DOM  7004
#define PANCL_ERROR_STR_ESC_OCTAL_DOM_str \
	"Octal escape sequence (\\o) resulted in a value > 255"

/**
 * Unknown escape sequence
 */
#define PANCL_ERROR_STR_ESC_UNKNOWN  7005
#define PANCL_ERROR_STR_ESC_UNKNOWN_str \
	"Invalid escape sequence"

/**
 * Invalid UTF-8 value in range 0xd800 - 0xdfff.
 * (UTF-16 surrogate values)
 */
#define PANCL_ERROR_UTF16_SURROGATE  8000
#define PANCL_ERROR_UTF16_SURROGATE_str \
	"Encountered an invalid UTF-8 value in range [0xd800, 0xdfff]"
/**
 * Invalid UTF-8 value 0xfffe or 0xffff.
 * (UCS noncharacters)
 */
#define PANCL_ERROR_UCS_NONCHAR  8001
#define PANCL_ERROR_UCS_NONCHAR_str \
	"Encountered an invalid UTF-8 value of 0xfffe or 0xffff"

/**
 * Invalid UTF-8 value in range 0x110000 - 0xffffffff.
 */
#define PANCL_ERROR_UTF8_HIGH  8002
#define PANCL_ERROR_UTF8_HIGH_str \
	"Encountered an invalid UTF-8 value in range [0x110000, 0xffffffff]"

/**
 * Truncated UTF-8 codepoint (decoding)
 */
#define PANCL_ERROR_UTF8_TRUNC  8003
#define PANCL_ERROR_UTF8_TRUNC_str \
	"Encountered a truncated UTF-8 sequence"

/**
 * Invalid character encountered while decoding.
 */
#define PANCL_ERROR_UTF8_DECODE  8004
#define PANCL_ERROR_UTF8_DECODE_str \
	"Invalid UTF-8 character in input"

/**
 * Conversion from string to integer was given and invalid base.
 */
#define PANCL_ERROR_STR_TO_INT_BASE  9000
#define PANCL_ERROR_STR_TO_INT_BASE_str \
	"Invalid base during conversion from string to integer"

/**
 * Conversion from string to integer had an invalid character.
 */
#define PANCL_ERROR_STR_TO_INT_CHAR  9001
#define PANCL_ERROR_STR_TO_INT_CHAR_str \
	"Invalid character found when converting from string to integer"

/**
 * Conversion from string to integer caused an overflow or underflow.
 */
#define PANCL_ERROR_STR_TO_INT_RANGE  9002
#define PANCL_ERROR_STR_TO_INT_RANGE_str \
	"Conversion from string to integer resulted in an overflow"

/**
 * Optional/extended integer type invalid argument count.
 */
#define PANCL_ERROR_OPT_INT_ARG_COUNT  10000
#define PANCL_ERROR_OPT_INT_ARG_COUNT_str \
	"Invalid argument count (0 or >2) given to ::Int* or ::Uint* custom type"

/**
 * Optional/extended integer type's first argument is not a string.
 */
#define PANCL_ERROR_OPT_INT_ARG_0_NOT_STRING  10001
#define PANCL_ERROR_OPT_INT_ARG_0_NOT_STRING_str \
	"First argument to ::Int* or ::Uint* custom type is not a String"

/**
 * Optional/extended integer type's second argument is not an integer.
 */
#define PANCL_ERROR_OPT_INT_ARG_1_NOT_INT  10002
#define PANCL_ERROR_OPT_INT_ARG_1_NOT_INT_str \
	"Second argument to ::Int* or ::Uint* custom type is not an Integer"

#endif /* H_PANCL_ERROR */
// vim:ts=4:sw=4:autoindent
