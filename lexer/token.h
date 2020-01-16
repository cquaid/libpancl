/* SPDX-License-Identifier: MIT */
#ifndef H_PANCL_LEXER_TOKEN
#define H_PANCL_LEXER_TOKEN

#include <stddef.h>
#include <stdint.h>

#include "pancl.h"
#include "lexer/utf8.h"

/* Token Type is broken up into 2 segments:
 *   Type:    10 bits
 *   SubType: 4 bits
 */

/**
 * No subtype
 */
#define TST_NONE     0
/**
 * Subtype: acts as a newline
 */
#define TST_NEWLINE  1
/**
 * Subtype: acts as an identifier
 */
#define TST_IDENT    2


/**
 * Invalid character
 */
#define TT_ERROR      -1
/**
 * Unset token (internal use)
 */
#define TT_UNSET      0
/**
 * End of file
 */
#define TT_EOF        1
#define TT_L_BRACKET  '['
#define TT_R_BRACKET  ']'
#define TT_L_BRACE    '{'
#define TT_R_BRACE    '}'
#define TT_L_PAREN    '('
#define TT_R_PAREN    ')'
#define TT_EQ         '='
#define TT_COMMA      ','
/**
 * Newline: '\r\n', '\r', or '\n'
 */
#define TT_NEWLINE  301
/**
 * Comment: #.*<newline>
 */
#define TT_COMMENT  302
/**
 * Raw Identifier: [a-zA-Z0-9_:+-]+
 */
#define TT_RAW_IDENT  303
/**
 * String:
 *   "..."
 *   '...'
 */
#define TT_STRING  304
/**
 * Integer:
 *   [-+]?[0-9]+ - integer
 *   [-+]?0[bB][01]+ - binary
 *   [-+]?0[oO][0-7]+ - octal
 *   [-+]?0[xX][a-fA-F0-9]+ - hex
 */
#define TT_INT_BIN  305
#define TT_INT_DEC  306
#define TT_INT_HEX  307
#define TT_INT_OCT  308

/**
 * Floating point values:
 */
#define TT_FLOAT  309
/**
 * Boolean true: true
 */
#define TT_TRUE  310
/**
 * Boolean false: false
 */
#define TT_FALSE  311

struct token {
	int type : 10; /**< What it be */
	int subtype : 4; /**< What it also be */
	char *value; /**< String value */
	struct pancl_pos pos; /**< Line / Column for token start */
};

#define TOKEN_INIT \
	{ \
		.type = TT_UNSET, \
		.subtype = TST_NONE, \
		.value = NULL, \
		.pos.line = 0, \
		.pos.column = 0 \
	}

int token_set(struct token *t, int type, int subtype, const char *value);
void token_init(struct token *t);
void token_fini(struct token *t);
int token_append(struct token *t, const char *value);
void token_move(struct token *dest, struct token *src);

struct token_buffer {
	char *buffer;
	size_t size;
	size_t pos;
};

#define TOKEN_BUFFER_INIT  { .buffer = NULL, .size = 0, .pos = 0 }
#define TOKEN_BUFFER_STEP  512

static inline void
token_buffer_reset(struct token_buffer *tb)
{
	tb->pos = 0;
}

/* Internal shouldn't be used directly except by the utf-8 code. */
int token_buffer_append_c(struct token_buffer *tb, char c);

/* Use this interface! */
static inline int
token_buffer_append(struct token_buffer *tb, uint_fast32_t val)
{
	return encode_utf8(tb, val);
}

int token_buffer_end(struct token_buffer *tb);
void token_buffer_fini(struct token_buffer *tb);


int lexer_rewind_token(struct pancl_context *ctx, struct token *t);
int next_token(struct pancl_context *ctx, struct token_buffer *tb,
		struct token *t);

#endif /* H_PANCL_LEXER_TOKEN */
// vim:ts=4:sw=4:autoindent
