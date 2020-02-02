/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pancl/pancl.h"
#include "lexer/token.h"
#include "lexer/utf8.h"

int
encode_utf8(struct token_buffer *tb, uint_fast32_t val)
{
	int used;
	char stash[6];

	memset(stash, 0, sizeof(stash));

	if (val <= 0x7f) { /* 0x00 - 0x7f */
		used = 1;
		stash[0] = (char)(val & 0xff);
		goto store;
	}

	if (val <= 0x7ff) { /* 0x80 - 0x7ff (11 bits) */
		used = 2;
		/* bin 110xxxxx  (C0 = 110 00000) */
		stash[0] = (char)(0xc0 + ((val >> 6) & 0xff));
		/* bin 10xxxxxx (80 = 10 000000; 3f = 00 111111) */
		stash[1] = (char)(0x80 + (val & 0x3f));
		goto store;
	}

	if (val <= 0xffff) { /* 0x800 - 0xffff (16 bits) */
		/* RFC 3629 states that 0xd800 - 0xdfff are invalid. */
		if (val >= 0xd800 && val <= 0xdfff)
			return PANCL_ERROR_UTF16_SURROGATE;

		used = 3;
		/* bin 1110xxxx (E0 = 1110 0000) */
		stash[0] = (char)(0xe0 + ((val >> 12) & 0xff));
		/* bin 10xxxxxx (80 = 10 000000; 3f = 00 111111) */
		stash[1] = (char)(0x80 + ((val >> 6) & 0x3f));
		/* bin 10xxxxxx (80 = 10 000000; 3f = 00 111111) */
		stash[2] = (char)(0x80 + (val & 0x3f));
		goto store;
	}

	if (val <= 0x1fffff) { /* 0x10000 - 0x1fffff (21 bits) */
		/* RFC 3629 states that above 0x10ffff is invalid. */
		if (val > 0x10ffff)
			return PANCL_ERROR_UTF8_HIGH;

		used = 4;
		/* bin 11110xxx (F0 = 11110 000) */
		stash[0] = (char)(0xf0 + ((val >> 18) & 0xff));
		/* bin 10xxxxxx (80 = 10 000000; 3f = 00 111111) */
		stash[1] = (char)(0x80 + ((val >> 12) & 0x3f));
		/* bin 10xxxxxx (80 = 10 000000; 3f = 00 111111) */
		stash[2] = (char)(0x80 + ((val >> 6) & 0x3f));
		/* bin 10xxxxxx (80 = 10 000000; 3f = 00 111111) */
		stash[3] = (char)(0x80 + (val & 0x3f));
		goto store;
	}

	/* Technically utf-8 can handle up to 6 bytes, but we aren't supporting
	 * that.
	 *
	 * 0x200000 - 0x3ffffff (26 bits, 5 bytes, prefix: 0xf8)
	 * 0x4000000 - 0x7fffffff (31 bits, 6 bytes, prefix: 0xfc)
	 */

	/* All other values are invalid. */
	return PANCL_ERROR_UTF8_HIGH;

store:
	{
		int i;
		int err = 0;

		for (i = 0; i < used; ++i) {
			err = token_buffer_append_c(tb, stash[i]);

			if (err != 0)
				return err;
		}

		tb->codepoints += 1;
	}

	return PANCL_SUCCESS;
}

size_t
utf8_length(uint_fast32_t codepoint)
{
	/* 0x00 - 0x7f */
	if (codepoint <= 0x7f)
		return 1;

	/* 0x80 - 0x7ff (11 bits) */
	if (codepoint <= 0x7ff)
		return 2;

	/* 0x800 - 0xffff (16 bits) */
	if (codepoint <= 0xffff)
		return 3;

	/* 0x10000 - 0x1fffff (21 bits) */
	if (codepoint <= 0x1fffff)
		return 4;

	/* 0x200000 - 0x3ffffff (26 bits) */
	if (codepoint <= 0x3ffffff)
		return 5;

	/* 0x40000000 - 0x7fffffff (31 bits) */
	if (codepoint <= 0x7fffffff)
		return 6;

	/* We don't know what this is, but this function does not fail so just
	 * act like it's a single byte.
	 */
	return 1;
}

static const unsigned char utf8_length_lookup[256] = {
	/* 0xxxxxxx == 1 byte */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 10xxxxxx == invalid;not a leading byte */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 110xxxxx == 2 bytes */
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	/* 1110xxxx == 3 bytes */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/* 11110xxx == 4 bytes */
	4, 4, 4, 4, 4, 4, 4, 4,
	/* 111110xx == 5 bytes */
	5, 5, 5, 5,
	/* 1111110x == 6 bytes */
	6, 6,
	/* 1111111x == invalid */
	0, 0
};

static const unsigned char utf8_length_lookup_fake[256] = {
	/* 0xxxxxxx == 1 byte */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 10xxxxxx == invalid; not a leading byte, but we count it as 1 byte for a
	 * length check.
	 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	/* 110xxxxx == 2 bytes */
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	/* 1110xxxx == 3 bytes */
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	/* 11110xxx == 4 bytes */
	4, 4, 4, 4, 4, 4, 4, 4,
	/* 111110xx == 5 bytes */
	5, 5, 5, 5,
	/* 1111110x == 6 bytes */
	6, 6,
	/* 1111111x == invalid, but we count it as 1 byte for a length check. */
	1, 1
};

size_t
utf8_length_c(unsigned char first_byte)
{
	/* Use the fake lookup table as this function shouldn't return 0. */
	return utf8_length_lookup_fake[first_byte];
}


/**
 * This version of the function was used to generate the
 * utf8_length_lookup_fake[] and (with some modification) the
 * utf8_length_lookup[] tables above.  Those tables are much faster but this
 * was left in case we need it or to sanity check the tables.
 */
size_t
utf8_length_c_slow(unsigned char first_byte)
{
	/* Valid sequences:
	 *  1: 0xxxxxxx
	 *  2: 110xxxxx
	 *  3: 1110xxxx
	 *  4: 11110xxx
	 *  5: 111110xx
	 *  6: 1111110x
	 */

	/* 1: 0xxxxxxx */
	if ((first_byte & 0x80) == 0)
		return 1;

	/* 2: 110xxxxx
	 * E0 = 1110 0000
	 * C0 = 1100 0000
	 */
	if ((first_byte & 0xE0) == 0xC0)
		return 2;

	/* 3: 1110xxxx
	 * F0 = 1111 0000
	 * E0 = 1110 0000
	 */
	if ((first_byte & 0xF0) == 0xE0)
		return 3;

	/* 4: 11110xxx
	 * F8 = 1111 1000
	 * F0 = 1111 0000
	 */
	if ((first_byte & 0xF8) == 0xF0)
		return 4;

	/* 5: 111110xx
	 * FC = 1111 1100
	 * F8 = 1111 1000
	 */
	if ((first_byte & 0xFC) == 0xF8)
		return 5;

	/* 6: 1111110x
	 * FE = 1111 1110
	 * FC = 1111 1100
	 */
	if ((first_byte & 0xFE) == 0xFC)
		return 6;

	/* The only other thing this could be is '10xxxxxxx' which
	 * denotes a secondary byte to a multi-byte codepoint and thus
	 * we should treat it as a single character as it should be invalid
	 * to find without a leading byte dictating the codepoint length.
	 *
	 * Could also be 1111111x but that's a technicality here.
	 */
	return 1;
}

/**
 * XXX: 0xc0, 0xc1, 0xfe, and 0xff are never used in utf-8 but we don't
 *      check for them here.
 */
int
decode_utf8(unsigned char *bytes, uint_fast32_t *storage)
{
	size_t i;
	size_t length;

	/* First byte determines the array length. */
	length = utf8_length_lookup[*bytes];

	if (length == 0)
		return PANCL_ERROR_UTF8_DECODE;

	/* Easy case: this is ascii. */
	if (length == 1) {
		*storage = (uint_fast32_t)bytes[0];
		return PANCL_SUCCESS;
	}

	/* Note: We know that length will always be in range [2, 6] here. */

	/* Everything else is more fun.
	 *  First byte will have (8 - (length + 1)) valid bits.
	 *
	 *  This results in a mask value of:
	 *   (1 << (8 - (length + 1))) - 1
	 *  Or simplified:
	 *   (1 << (7 - length)) - 1
	 *
	 *  Example for 2 bytes:
	 *    First byte format: 110xxxxx so there are 5 bits to grab.
	 *    (8 - (2 + 1)) = 5
	 *    (1 << 5) = 32 (00100000)
	 *    ((1 << 5) - 1) = 31 (00011111)
	 */
	*storage = bytes[0] & ((1 << (7 - length)) - 1);

	/* All following bytes of the codepoint have the format:
	 *  10xxxxxx
	 *
	 * So the math should be the same as the above, but since we know the
	 * bit length is always 6 we can simply mask with 0x3F and shift the
	 * stored value up by 6 bits each time.
	 */
	for (i = 1; i < length; ++i) {
		/* If an expected following byte is not in the format '10xxxxxx',
		 * then we need to report a failure.  The codepoint was likely
		 * truncated.
		 */
		if ((bytes[i] & 0xC0) != 0x80)
			return PANCL_ERROR_UTF8_TRUNC;

		/* 0x3F = 00111111 */
		*storage = (*storage << 6) + (bytes[i] & 0x3F);
	}

	/* Sadly not quite done yet, validate that the value is valid.*/

	/* UTF-16 surrogates are invalid. */
	if (*storage >= 0xD800 && *storage <= 0xDFFF)
		return PANCL_ERROR_UTF16_SURROGATE;

	/* UCS noncharacters. */
	if (*storage == 0xFFFE || *storage == 0xFFFF)
		return PANCL_ERROR_UCS_NONCHAR;

	/* RFC 3629 states that above 0x10ffff is invalid. */
	if (*storage > 0x10ffff)
		return PANCL_ERROR_UTF8_HIGH;

	return PANCL_SUCCESS;
}

// vim:ts=4:sw=4:autoindent
