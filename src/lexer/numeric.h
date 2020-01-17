/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_LEXER_NUMERIC
#define H_PANCL_LEXER_NUMERIC

#include <stdbool.h>

bool str_is_float(const char *str);

bool str_is_binary(const char *str, bool check_prefix);
bool str_is_decimal(const char *str);
bool str_is_hexadecimal(const char *str, bool check_prefix);
bool str_is_octal(const char *str, bool check_prefix);

#endif /* H_PANCL_LEXER_NUMERIC */
// vim:ts=4:sw=4:autoindent
