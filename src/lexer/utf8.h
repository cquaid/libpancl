/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_LEXER_UTF8
#define H_PANCL_LEXER_UTF8

#include <stddef.h>
#include <stdint.h>

struct token_buffer;

size_t utf8_length(uint_fast32_t codepoint);
size_t utf8_length_c(unsigned char first_byte);
size_t utf8_length_c_slow(unsigned char first_byte);

int encode_utf8(struct token_buffer *tb, uint_fast32_t val);
int decode_utf8(unsigned char *bytes, uint_fast32_t *storage);

#endif /* H_PANCL_LEXER_UTF8 */
// vim:ts=4:sw=4:autoindent
