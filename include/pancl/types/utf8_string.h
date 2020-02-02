/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_TYPES_UTF8_STRING
#define H_PANCL_TYPES_UTF8_STRING

#include <stddef.h>
#include <string.h>

/**
 * Represents an UTF-8 encoded string.
 */
struct pancl_utf8_string {
	/**
	 * Number of bytes in the string.  This is not the number of charaters
	 * and it excludes the added NUL terminator.
	 */
	size_t bytes;
	/**
	 * Number of UTF-8 codepoints in the string (excluding the added
	 * NUL terminator).
	 *
	 * If this number differs from pancl_utf8_string::bytes then the
	 * string contains one or more multi-byte codepoints.
	 */
	size_t codepoints;
	/**
	 * UTF-8 encoded string.  Note that an embedded NUL (0x00) byte is valid
	 * within this string.  This string is also guaranteed to be NUL
	 * terminated, however strlen() should not be used to determine the length.
	 *
	 * If strlen() differs from pancl_utf8_string::bytes then there is an
	 * embedded NUL byte.
	 *
	 * @note
	 *   This is a flexible array member and the string content will likely
	 *   extend past the end of the structure.  (As in: treat this like a
	 *   bare pointer instead of an array member.)
	 */
	char data[1];
};

/**
 * Cleans up the memory associated with a pancl_utf8_string.
 *
 * @param[in,out] string   Pointer to the pointer to the string to clean up
 *
 * @note The dereference of @p string will be set to NULL on return.
 */
void pancl_utf8_string_destroy(struct pancl_utf8_string **string);

/**
 * Test if a pancl_utf8_string contains an embedded NUL-byte (outside
 * of the NUL-terminator).
 *
 * @retval 0          String is does not have an embedded NUL.
 * @retval non-zero   String is has an embedded NUL.
 */
static inline int
pancl_utf8_string_contains_nul(const struct pancl_utf8_string *string)
{
	if (string == NULL)
		return 0;
	return (strlen(string->data) != string->bytes);
}

/**
 * Test if a pancl_utf8_string looks like an ASCII string.
 *
 * @retval 0          String is not ASCII
 * @retval non-zero   String is ASCII
 */
static inline int
pancl_utf8_string_is_ascii(const struct pancl_utf8_string *string)
{
	if (string == NULL)
		return 0;
	return (string->codepoints == string->bytes);
}

#endif /* H_PANCL_TYPES_UTF8_STRING */
// vim:ts=4:sw=4:autoindent
