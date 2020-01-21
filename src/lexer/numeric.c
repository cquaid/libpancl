/* SPDX-License-Identifier: MIT */
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"
#include "lexer/token.h"
#include "lexer/numeric.h"

bool
str_is_float(const char *str)
{
	bool got_whole = false;
	bool got_dot = false;
	bool got_frac = false;
	bool got_exp = false;

	/* Floating point number break down:
	 *
	 *   Whole number, no fraction not exponent:
	 *     [-+]?[0-9]+.
	 *   Decimal number, no exponent:
	 *     [-+]?[0-9]+\.[0-9]+
	 *   Fraction only, no whole number nor exponent:
	 *     [-+]?\.[0-9]+
	 *   Whole number with exponent, no fraction:
	 *     [-+]?[0-9]+[eE][-+]?[0-9]+
	 *
	 *   Optional Exponent (may be attached to any of the above):
	 *     [eE][-+]?[0-9]+
	 *
	 * These are also valid:
	 *  [-+]?NaN
	 *  [-+]?Inf
	 */

	/* Optional starting sign. */
	if (*str == '-' || *str == '+')
		++str;

	/* Not-a-number and Infinity checks. */
	if (strcmp(str, "NaN") == 0 || strcmp(str, "Inf") == 0)
		return true;

	/* Start with whole number portion.
	 *  [0-9]*
	 */
	for (; *str != '\0'; ++str) {
		if (*str == '.') {
			got_dot = true;
			++str;
			goto fraction;
		}

		if (*str == 'e' || *str == 'E') {
			++str;
			goto exponent;
		}

		if (*str < '0' || *str > '9')
			return false;

		got_whole = true;
	}

done:
	/* Validate we have the correct parts for a float. */

	if (got_whole && got_dot)
		return true;

	if (got_dot && got_frac)
		return true;

	if (got_whole && got_exp)
		return true;

	/* Turns out we don't care if we got an exponent or not outside of the
	 * "just whole number".  In all other cases we just need to validate that
	 * [eE][+-]? was followed by a series of digits.  That was validated in the
	 * 'exponent' portion so we don't need re-validate it here.
	 */

	return false;

fraction:
	/* Fraction portion (just past '.')
	 *  \.[0-9]*
	 */
	for (; *str != '\0'; ++str) {
		if (*str == 'e' || *str == 'E') {
			++str;
			goto exponent;
		}

		if (*str < '0' || *str > '9')
			return false;

		got_frac = true;
	}

	goto done;

exponent:
	/* Exponent (just past 'e' or 'E')
	 *   [eE][-+]?[0-9]+
	 */
	if (*str == '-' || *str == '+')
		++str;

	/* Must be followed by at least one digit. */
	if (*str == '\0')
		return false;

	for (; *str != '\0'; ++str) {
		if (*str < '0' || *str > '9')
			return false;
		got_exp = true;
	}

	goto done;
}

/* TODO: This is the quick-and-dirty float check that is guaranteed to work.
 * Leaving this here until we validate the above float check.
 */
#if 0
bool
str_is_float(const char *str)
{
	size_t s;
	char *end = NULL;

	if (*str == '-' || *str == '+')
		++str;

	if (strcmp(str, "NaN") == 0 || strcmp(str, "Inf") == 0)
		return true;

	/* Since Inf and NaN were already taken care of, this is all we need
	 * to validate against.
	 */
	s = strspn(str, "0123456789eE+-.");

	if (s != strlen(str))
		return false;

	(void)strtod(str, &end);

	if (end != NULL && *end != '\0')
		return false;

	/* Domain check is saved for later. */
	return true;
}
#endif

bool
str_is_binary(const char *str, bool check_prefix)
{
	if (*str == '-' || *str == '+')
		++str;

	/* If checking the prefix, we need to look for '0b' or '0B' */
	if (check_prefix) {
		if (str[0] != '0')
			return false;

		if (str[1] != 'b' && str[1] != 'B')
			return false;

		str += 2;
	}

	/* Validate the digits. */
	for (; *str != '\0'; ++str) {
		if (*str < '0' || *str > '1')
			return false;
	}

	return true;
}

bool
str_is_decimal(const char *str)
{
	if (*str == '-' || *str == '+')
		++str;

	for (; *str != '\0'; ++str) {
		if (*str < '0' || *str > '9')
			return false;
	}

	return true;
}

bool
str_is_hexadecimal(const char *str, bool check_prefix)
{
	if (*str == '-' || *str == '+')
		++str;

	/* If checking the prefix, we need to look for '0x' or '0X' */
	if (check_prefix) {
		if (str[0] != '0')
			return false;

		if (str[1] != 'x' && str[1] != 'X')
			return false;

		str += 2;
	}

	for (; *str != '\0'; ++str) {
		if (*str >= '0' && *str <= '9')
			continue;
		if (*str >= 'a' && *str <= 'f')
			continue;
		if (*str >= 'A' && *str <= 'F')
			continue;

		return false;
	}

	return true;
}

bool
str_is_octal(const char *str, bool check_prefix)
{
	if (*str == '-' || *str == '+')
		++str;

	/* If checking the prefix, we need to look for '0o' or '0O' */
	if (check_prefix) {
		if (str[0] != '0')
			return false;

		if (str[1] != 'o' && str[1] != 'O')
			return false;

		str += 2;
	}

	for (; *str != '\0'; ++str) {
		if (*str < '0' || *str > '7')
			return false;
	}

	return true;
}

// vim:ts=4:sw=4:autoindent
