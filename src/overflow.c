/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <limits.h>

#include "pancl/pancl.h"
#include "internal.h"

int
safe_add(size_t a, size_t b, size_t *r)
{
	*r = a + b;

	if (*r < a || *r < b)
		return PANCL_ERROR_OVERFLOW;

	return PANCL_SUCCESS;
}

int
safe_mul(size_t a, size_t b, size_t *r)
{
	if (b != 0 && a > (SIZE_MAX / b))
		return PANCL_ERROR_OVERFLOW;

	*r = a * b;
	return PANCL_SUCCESS;
}

// vim:ts=4:sw=4:autoindent
