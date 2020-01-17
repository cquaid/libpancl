/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_PARSE_STR_TO_INT
#define H_PANCL_PARSE_STR_TO_INT

#include <stdint.h>

int str_to_int64(int_least64_t *ret, const char *str, int base);
int str_to_uint64(uint_least64_t *ret, const char *str, int base);

int str_to_int32(int_least32_t *ret, const char *str, int base);
int str_to_uint32(uint_least32_t *ret, const char *str, int base);

int str_to_int16(int_least16_t *ret, const char *str, int base);
int str_to_uint16(uint_least16_t *ret, const char *str, int base);

int str_to_int8(int_least8_t *ret, const char *str, int base);
int str_to_uint8(uint_least8_t *ret, const char *str, int base);

#endif /* H_PANCL_PARSE_STR_TO_INT */
// vim:ts=4:sw=4:autoindent
