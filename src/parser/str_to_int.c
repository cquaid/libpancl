/* SPDX-License-Identifier: MIT */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pancl/pancl.h"
#include "parser/str_to_int.h"

static int
str_to_uintmax_internal(uintmax_t *ret, uintmax_t max, const char *str,
	int base)
{
	uintmax_t r = 0;
	uintmax_t mul_max;
	uintmax_t add_max;

	if (base < 0 || base == 1 || base > 36)
		return PANCL_ERROR_STR_TO_INT_BASE;

	if (str == NULL || str[0] == '\0')
		return PANCL_ERROR_ARG_INVALID;

	/* If base == 0, the following are allowed:
	 *   0[xX] - base = 16
	 *   0[oO] - base = 8
	 *   0[bB] - base = 2
	 *   [1-9] - base = 10
	 */

	if (str[0] == '0') {
		if (str[1] == '\0') {
			*ret = 0;
			return PANCL_SUCCESS;
		}

		switch (str[1]) {
		case '\0':
			*ret = 0;
			return PANCL_SUCCESS;

		case 'x': case 'X':
			if (base != 0 && base != 16)
				return PANCL_ERROR_STR_TO_INT_CHAR;

			base = 16;
			str += 2;
			break;

		case 'o': case 'O':
			if (base != 0 && base != 8)
				return PANCL_ERROR_STR_TO_INT_CHAR;

			base = 8;
			str += 2;
			break;

		case 'b': case 'B':
			if (base != 0 && base != 2)
				return PANCL_ERROR_STR_TO_INT_CHAR;

			base = 2;
			str += 2;
			break;

		default:
			break;
		}
	}

	if (base == 0)
		base = 10;

	mul_max = max / base; /* Maximum for multiplication */
	add_max = max % base; /* Max allowed to add if at mul_max */

	/* Base cannot be 0 here so we can handle things pretty normally. */
	for (;*str != '\0'; ++str) {
		int v;

		/* Invalid character in string:
		 *   Either a character not in [0-9a-zA-Z]
		 *   Or the character is invalid for the given base.
		 */

		if (*str >= '0' && *str <= '9')
			v = *str - '0';
		else if (*str >= 'a' && *str <= 'z')
			v = *str - 'a' + 10;
		else if (*str >= 'A' && *str <= 'Z')
			v = *str - 'A' + 10;
		else
			return PANCL_ERROR_STR_TO_INT_CHAR;

		if (v >= base)
			return PANCL_ERROR_STR_TO_INT_CHAR;

		/* If multiplication would cause an overflow. */
		if (r > mul_max)
			return PANCL_ERROR_STR_TO_INT_RANGE;

		/* If addition would cause an overflow. */
		if (r == mul_max && (uintmax_t)v > add_max)
			return PANCL_ERROR_STR_TO_INT_RANGE;

		r = (r * (uintmax_t)base) + (uintmax_t)v;
	}

	*ret = r;
	return PANCL_SUCCESS;
}

static int
str_to_intmax(intmax_t *ret, const char *str, int base,
	intmax_t min, intmax_t max)
{
	int err;
	uintmax_t r;
	uintmax_t effective_max;
	bool negative = false;

	if (str[0] == '-') {
		negative = true;
		++str;
	}
	else if (str[0] == '+') {
		++str;
	}

	effective_max = (uintmax_t)(negative ? -min : max);
	err = str_to_uintmax_internal(&r, effective_max, str, base);

	if (err == PANCL_SUCCESS) {
		if (negative)
			*ret = -(intmax_t)r;
		else
			*ret = (intmax_t)r;
	}

	return err;
}

static int
str_to_uintmax(uintmax_t *ret, const char *str, int base, uintmax_t max)
{
	if (str[0] == '+')
		++str;

	return str_to_uintmax_internal(ret, max, str, base);
}

int
str_to_int64(int_least64_t *ret, const char *str, int base)
{
	intmax_t r;
	int err = str_to_intmax(&r, str, base, INT64_MIN, UINT64_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (int_least64_t)r;

	return err;
}

int
str_to_uint64(uint_least64_t *ret, const char *str, int base)
{
	uintmax_t r;
	int err = str_to_uintmax(&r, str, base, UINT64_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (uint_least64_t)r;

	return err;
}

int
str_to_int32(int_least32_t *ret, const char *str, int base)
{
	intmax_t r;
	int err = str_to_intmax(&r, str, base, INT32_MIN, UINT32_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (int_least32_t)r;

	return err;
}

int
str_to_uint32(uint_least32_t *ret, const char *str, int base)
{
	uintmax_t r;
	int err = str_to_uintmax(&r, str, base, UINT32_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (uint_least32_t)r;

	return err;
}

int
str_to_int16(int_least16_t *ret, const char *str, int base)
{
	intmax_t r;
	int err = str_to_intmax(&r, str, base, INT16_MIN, UINT16_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (int_least16_t)r;

	return err;
}

int
str_to_uint16(uint_least16_t *ret, const char *str, int base)
{
	uintmax_t r;
	int err = str_to_uintmax(&r, str, base, UINT16_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (uint_least16_t)r;

	return err;
}

int
str_to_int8(int_least8_t *ret, const char *str, int base)
{
	intmax_t r;
	int err = str_to_intmax(&r, str, base, INT8_MIN, UINT8_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (int_least8_t)r;

	return err;
}

int
str_to_uint8(uint_least8_t *ret, const char *str, int base)
{
	uintmax_t r;
	int err = str_to_uintmax(&r, str, base, UINT8_MAX);

	if (err == PANCL_SUCCESS)
		*ret = (uint_least8_t)r;

	return err;
}

// vim:ts=4:sw=4:autoindent
