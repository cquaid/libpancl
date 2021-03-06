/* SPDX-License-Identifier: MIT */
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "pancl/pancl.h"

#include "lexer/numeric.h"
#include "lexer/token.h"
#include "lexer/utf8.h"

/**
 * Attempts to refill the input buffer.
 *
 * @param[in] need
 *   At least this many characters must be in the buffer.
 *   When this is non-zero we also retain whatever content is left in the
 *   buffer as that's the only use-case for this parameter.
 */
static int
refill(struct pancl_context *ctx, size_t need)
{
	int err;
	size_t retained = 0;
	size_t size = ctx->buffer_size;
	char *start = ctx->allocated_buffer;

	if (ctx->end_of_input || ctx->allocated_buffer == NULL) {
		/* If bytes were requested then we need to return a truncated UTF-8
		 * sequence instead of end of input.
		 */
		return (need != 0) ? PANCL_ERROR_UTF8_TRUNC : PANCL_END_OF_INPUT;
	}

	/* If requested, we retain the remaining characters in the buffer before
	 * refilling.
	 */
	if (need != 0) {
		start = ctx->allocated_buffer;

		for (; ctx->cursor <= ctx->end; ++start) {
			*start = *(ctx->cursor);
			++retained;
		}

		/* Simplify the logic so we can just check against 'size' later on. */
		if (retained > need)
			need = 0;
		else
			need -= retained;

		size -= retained;
	}

	err = ctx->ops->next(ctx->ops_data, start, &size);

	if (err != 0)
		return PANCL_ERROR_LEXER_REFILL;

	/* Reset cursor to the entire buffer not the location we started
	 * refilling at.
	 */
	ctx->cursor = ctx->allocated_buffer;
	ctx->end = ctx->cursor + size + retained;

	/* No more data? End of input. */
	if (size == 0)
		ctx->end_of_input = 1;

	/* If we read less than the number we needed it's a truncated UTF-8
	 * character sequence.
	 */
	if (size < need)
		return PANCL_ERROR_UTF8_TRUNC;

	/* Only return SUCCESS if we didn't encounter the end of input. */
	return (ctx->end_of_input) ? PANCL_END_OF_INPUT : PANCL_SUCCESS;
}

/**
 * Returns the next character in the buffer without advancing the cursor.
 */
static int
peek_next(struct pancl_context *ctx, uint_fast32_t *p)
{
	int err;
	size_t need;

	if (ctx->cursor >= ctx->end) {
		err = refill(ctx, 0);

		if (err != PANCL_SUCCESS)
			return err;
	}

	/* Always >= 1 */
	need = utf8_length_c(*(ctx->cursor));

	/* > not >= since length needed is an exact number of characters.
	 * If cursor == 5 and end == 8 then a length of 4 is valid.
	 */
	if ((ctx->cursor + need - 1) >= ctx->end) {
		/* Refilling is fun here, we have to store the remainder of the current
		 * UTF-8 character in the buffer.
		 */

		err = refill(ctx, need);

		if (err == PANCL_END_OF_INPUT)
			err = PANCL_ERROR_UTF8_TRUNC;

		if (err != PANCL_SUCCESS)
			return err;
	}

	/* Decode the UTF-8 value. */
	return decode_utf8((unsigned char *)(ctx->cursor), p);
}

/**
 * Advances the parse state.  Used if the peeked value should be consumed.
 */
static int
advance(struct pancl_context *ctx)
{
	int err;
	size_t length;
	uint_fast32_t c;

	/* This should only be used AFTER a peek so we optimize and don't perform
	 * any safety checks as it should be valid.
	 */
	length = utf8_length_c(*(ctx->cursor));
	err = decode_utf8((unsigned char *)(ctx->cursor), &c);

	if (err != PANCL_SUCCESS)
		return err;

	ctx->cursor += length;
	ctx->loc.column += 1;

	switch (c) {
	case '\r':
		err = peek_next(ctx, &c);

		if (err != PANCL_SUCCESS && err != PANCL_END_OF_INPUT)
			return err;

		if (c != '\n') {
			/* If just CR, go ahead and increment the line count.
			 *
			 * If we got CR LF, wait to increment the line count until we
			 * consume the LF.
			 */
			ctx->loc.column = 0;
			ctx->loc.line += 1;
		}
		break;

	case '\n':
		ctx->loc.column = 0;
		ctx->loc.line += 1;
		break;
	}

	return PANCL_SUCCESS;
}

/**
 * Returns the current UTF-8 codepoint in the buffer and advances the cursor.
 */
static int
get_next(struct pancl_context *ctx, uint_fast32_t *c)
{
	int err;

	err = peek_next(ctx, c);

	if (err == PANCL_SUCCESS)
		err = advance(ctx);

	return err;
}

static bool
is_newline_start(struct pancl_context *ctx, uint_fast32_t c)
{
	return (c == '\n' || c == '\r');
}

/**
 * Newlines are as follows:
 *  CR LF
 *  CF
 *  LF
 *
 * The longest sequence must be taken, so CR LF is not two newlines, it's
 * just one.
 */
static bool
is_newline(struct pancl_context *ctx, uint_fast32_t c)
{
	int err;

	if (c == '\n')
		return true;

	if (c != '\r')
		return false;

	/* Have CR, check for a following LF and consume it if found. */
	err = peek_next(ctx, &c);

	/* Peek errors don't matter, at this point we're always successful for the
	 * newline check.
	 *
	 * BUT! We do need to force the consumption of the LF if we got one.
	 */
	if ((err == PANCL_SUCCESS) && (c == '\n'))
		(void)advance(ctx);

	return true;
}


static int
consume_comment(struct pancl_context *ctx)
{
	uint_fast32_t c;
	int err;
	bool escape = false;

	/* Eat everything up-to and including the newline. */
	while ((err = get_next(ctx, &c)) == PANCL_SUCCESS) {
		if (is_newline(ctx, c)) {
			/* Escaped newlines are not allowed in a comment. */
			if (escape)
				return PANCL_ERROR_LEXER_COMMENT_ESC_NEWLINE;
			return PANCL_SUCCESS;
		}

		escape = (c == '\\');
	}

	return err;
}

static bool
is_whitespace(uint_fast32_t c)
{
	return (c == ' ' || c == '\t');
}

static bool
is_raw_ident(uint_fast32_t c)
{
	/* [a-zA-Z0-9_:.+-] */
	return (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9')
		|| (c == '-')
		|| (c == '_')
		|| (c == '+')
		|| (c == ':')
		|| (c == '.');
}

static int
get_raw_ident(struct pancl_context *ctx, struct token_buffer *tb,
	uint_fast32_t prefix)
{
	uint_fast32_t p;
	int err;

	token_buffer_reset(tb);

	if (prefix != '\0') {
		err = token_buffer_append(tb, prefix);

		if (err != PANCL_SUCCESS)
			return err;
	}

	while ((err = peek_next(ctx, &p)) == PANCL_SUCCESS) {
		/* Not a raw identifier character, must be a boundary? */
		if (!is_raw_ident(p))
			return token_buffer_end(tb);

		/* Valid raw identifier character, add to the buffer. */
		err = token_buffer_append(tb, p);

		if (err != PANCL_SUCCESS)
			return err;

		err = advance(ctx);

		if (err != PANCL_SUCCESS)
			break;
	}

	/* End of input counts as finishing the identifier. */
	if (err == PANCL_END_OF_INPUT)
		return token_buffer_end(tb);

	/* If it's bad it's bad. */
	return err;
}

/**
 * Checks a string to see if it looks like a numeric token and returns
 * its token type (RAW IDENT returned if not numeric).
 */
static int
get_numeric_token_type(const char *str)
{
	if (str_is_binary(str, true))
		return TT_INT_BIN;

	if (str_is_decimal(str))
		return TT_INT_DEC;

	if (str_is_hexadecimal(str, true))
		return TT_INT_HEX;

	if (str_is_octal(str, true))
		return TT_INT_OCT;

	if (str_is_float(str))
		return TT_FLOAT;

	return TT_RAW_IDENT;
}

static int
set_ident_token(struct token *t, struct pancl_context *ctx,
	struct token_buffer *tb)
{
	int type;

	/* Simple cases: boolean true and false. */
	if (strcmp(tb->buffer, "true") == 0)
		return token_set_empty(t, TT_TRUE, TST_IDENT);

	if (strcmp(tb->buffer, "false") == 0)
		return token_set_empty(t, TT_FALSE, TST_IDENT);

	/* Numeric regexes:
	 *  DecInt:  [-+]?(0|[1-9][0-9]*)
	 *  HexInt:  [-+]?0[xX][a-fA-F0-9]+
	 *  OctInt:  [-+]?0[oO][0-7]+
	 *  BinInt:  [-+]?0[bB][0-1]+
	 *
	 *  Frac:    ([0-9]*\.[0-9]+)|([0-9]+\.)
	 *  Exp:     [eE][+-]?[0-9]+
	 *  Float:   (<Frac><Exp>?)|([0-9]+<Exp>)
	 *
	 *  FloatStart:  [0-9.+-]
	 *  DecStart:    [0-9+-]
	 *  *Start:      [0+-]
	 *
	 *  Special floats:
	 *    [+-]?Inf
	 *    [+-]?NaN
	 */

	/* Check for valid numeric start values. */

	switch (tb->buffer[0]) {
	case '-': case '+': case '.':
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		type = get_numeric_token_type(tb->buffer);
		break;
	/* Special floats (Inf, NaN). */
	case 'I': case 'N':
		if (str_is_float(tb->buffer))
			type = TT_FLOAT;
		else
			type = TT_RAW_IDENT;
		break;

	default:
		type = TT_RAW_IDENT;
		break;
	}

	return token_set(t, type, TST_IDENT, tb);
}


static bool
is_quote(uint_fast32_t c)
{
	return (c == '\'') || (c == '"');
}

static int
handle_octal_escape(struct pancl_context *ctx, struct token_buffer *tb,
	uint_fast32_t start)
{
	int err;
	int count = 0;
	uint_fast32_t val = 0;

	uint_fast32_t p = start;

	do {
		/* Done parsing the octal */
		if (p < '0' || p > '7')
			goto done;

		/* Already have the max digits we can handle. */
		if (count == 3)
			goto done;

		/* Got an octal digit. */
		val <<= 3;
		val += (p - '0');
		++count;

		err = advance(ctx);

		if (err != PANCL_SUCCESS)
			break;
	} while ((err = peek_next(ctx, &p)) == PANCL_SUCCESS);

	/* We're in an escape sequence, END_OF_INPUT is a bad thing. */
	if (err != PANCL_SUCCESS)
		return PANCL_ERROR_STR_SHORT;

done:
	/* Before we add the number, we have to check the domain. The maximum octal
	 * we can support is \377 (0xff).
	 */
	if (val > 0xff)
		return PANCL_ERROR_STR_ESC_OCTAL_DOM;

	return token_buffer_append(tb, val);
}

static int
handle_hex_escape(struct pancl_context *ctx, struct token_buffer *tb,
	uint_fast32_t type)
{
	int err;
	int exact = 0;
	int digits = 0;
	uint_fast32_t val; /* Capped at 4 bytes so this is safe. */

	uint_fast32_t p;

	if (type == 'x')
		exact = 2; /* This one isn't really 2, it's actuall 1 or 2. */
	else if (type == 'u')
		exact = 4;
	else if (type == 'U')
		exact = 8;

	val = 0ULL;

	while ((err = peek_next(ctx, &p)) == PANCL_SUCCESS) {
		uint_fast32_t add;

		if (digits == exact)
			goto done;

		if (p >= 'a' && p <= 'f')
			add = (p - 'a') + 10;
		else if (p >= 'A' && p <= 'F')
			add = (p - 'A') + 10;
		else if (p >= '0' && p <= '9')
			add = (p - '0');
		else
			goto done;

		/* We got a digit. */
		++digits;

		/* Shift the number up and add the new digit. */
		val = (val << 4) + add;

		err = advance(ctx);

		if (err != PANCL_SUCCESS)
			break;
	}

	/* We're in an escape sequence, END_OF_INPUT is a bad thing.
	 *
	 * Also, in this context, we can never get SUCCESS so we don't need
	 * to bother checking status.
	 */
	if (err == PANCL_END_OF_INPUT)
		return PANCL_ERROR_STR_SHORT;

	return err;

done:
	/* \x can take one or two hex digits so we fudge it a bit here to make
	 * things work out cleanly.
	 */
	if (type == 'x' && digits == 1)
		exact = 1;

	if (digits != exact) {
		/* If we got less than we expect, it's an error. */
		if (type == 'u')
			return PANCL_ERROR_STR_ESC_LU;
		if (type == 'U')
			return PANCL_ERROR_STR_ESC_UU;
		return PANCL_ERROR_STR_ESC_X;
	}

	/* token_buffer_append() handles the utf8 encoding. */
	return token_buffer_append(tb, val);
}

static int
handle_escape(struct pancl_context *ctx, struct token_buffer *tb,
	uint_fast32_t start, bool raw)
{
	int err = PANCL_SUCCESS;

	/* Store error context in case of a failure. */
	ctx->error_loc = ctx->loc;

	/* In raw mode, everything is unhandled. */
	if (raw) {
		err = token_buffer_append(tb, '\\');

		if (err == PANCL_SUCCESS)
			err = token_buffer_append(tb, start);

		return err;
	}

	/* Starts at the character just past the slash. */
	switch (start) {
	case 'a':
		return token_buffer_append(tb, '\a');
	case 'b':
		return token_buffer_append(tb, '\b');
	case 'f':
		return token_buffer_append(tb, '\f');
	case 'n':
		return token_buffer_append(tb, '\n');
	case 'r':
		return token_buffer_append(tb, '\r');
	case 't':
		return token_buffer_append(tb, '\t');
	case 'v':
		return token_buffer_append(tb, '\v');
	case '\\':
		return token_buffer_append(tb, '\\');
	case '\'':
		return token_buffer_append(tb, '\'');
	case '"':
		return token_buffer_append(tb, '"');

	/* Bigger cases */
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
		return handle_octal_escape(ctx, tb, start);

	case 'x': /* 1-byte hex */
	case 'u': /* 2-byte unicode */
	case 'U': /* 4-byte unicode */
		return handle_hex_escape(ctx, tb, start);

	default:
		break;
	}

	/* Escaped newline?  Consume the newline but don't append it. */
	if (is_newline(ctx, start))
		return PANCL_SUCCESS;

	/* Unhandled escape sequences are errors. */
	return PANCL_ERROR_STR_ESC_UNKNOWN;
}

/**
 * Expects to start just past the delimiter.
 */
static int
get_single_string(struct pancl_context *ctx, struct token_buffer *tb,
	uint_fast32_t delim)
{
	int err;
	uint_fast32_t c;

	bool raw = (delim == '\''); /* single-quotes = raw string */
	bool do_escape = false;

	/* Starts past the delimiter so we just append to the buffer, handling
	 * any escape sequences (non-raw) we encounter.
	 */

	/* We can use get_next() here since we always need to consume. */
	while ((err = get_next(ctx, &c)) == PANCL_SUCCESS) {
		if (do_escape) {
			/* handle_escape manages its own error context. */
			err = handle_escape(ctx, tb, c, raw);

			if (err != PANCL_SUCCESS)
				return err;

			do_escape = false;
			continue;
		}

		if (c == '\\') {
			do_escape = true;
			continue;
		}

		/* Ending delimiter? Finish the string. */
		if (c == delim) {
			err = token_buffer_end(tb);

			if (err != PANCL_SUCCESS)
				ctx->error_loc = ctx->loc;

			return err;
		}

		/* If we get an embedded newline then we have to canonicalize it to
		 * a single LF character.
		 */
		if (is_newline(ctx, c))
			c = '\n';

		err = token_buffer_append(tb, c);

		if (err != PANCL_SUCCESS) {
			ctx->error_loc = ctx->loc;
			return err;
		}
	}

	ctx->error_loc = ctx->loc;

	/* If END_OF_INPUT then we ran out of content before finding the ending
	 * delimiter.
	 */

	if (err == PANCL_END_OF_INPUT)
		return PANCL_ERROR_STR_SHORT;

	return err;
}

/**
 * Expects to start just past the delimiter.
 */
static int
get_string(struct pancl_context *ctx, struct token_buffer *tb,
	uint_fast32_t delim)
{
	int err;
	uint_fast32_t c;

	token_buffer_reset(tb);
	err = get_single_string(ctx, tb, delim);

	if (err != PANCL_SUCCESS)
		return err;

	/* Okay, we've gotten one string, BUT! strings can be adjacent to one
	 * another meaning we have to handle the following:
	 *
	 * Comment - not allowed as comments end lines.
	 * Whitespace - allowed, just ignore it!
	 * Escaped newline - allowed, also ignore!
	 */
	for (;;) {
		err = peek_next(ctx, &c);

		if (err != PANCL_SUCCESS)
			break;

		/* Ignore all whitespace characters. */
		if (is_whitespace(c)) {
			err = advance(ctx);

			if (err != PANCL_SUCCESS)
				break;

			continue;
		}

		/* If we got a quote then we can append to our string! */
		if (is_quote(c)) {
			err = advance(ctx);

			if (err == PANCL_SUCCESS)
				err = get_single_string(ctx, tb, c);

			if (err != PANCL_SUCCESS)
				break;

			continue;
		}

		if (c == '\\') {
			/* Got a possible escaped newline.
			 *
			 * We have to be careful here!  is_newline() will peek and modify
			 * the input cursor.
			 *
			 * To get around this we check if the next character is a possible
			 * start to a newline sequence.
			 *
			 * If so, we'll advance the cursor and then use is_newline() to
			 * consume it.
			 */
			err = advance(ctx);

			if (err == PANCL_SUCCESS)
				err = peek_next(ctx, &c);

			if (err != PANCL_SUCCESS)
				break;

			if (is_newline_start(ctx, c)) {
				err = advance(ctx);

				if (err != PANCL_SUCCESS)
					break;

				(void)is_newline(ctx, c);
				continue;
			}
		}

		/* Any other character? We're done here. */
		break;
	}

	/* Getting EOF means we at least got one string, so this is actually a
	 * success case.
	 */
	if (err == PANCL_END_OF_INPUT)
		return PANCL_SUCCESS;

	return err;
}

int
next_token(struct pancl_context *ctx, struct token_buffer *tb, struct token *t)
{
	int err;
	uint_fast32_t c = '\0';
	bool escaped = false;

	/* There's a "small" hack in place for rewinding the lexer by 1 token;
	 * we deal with that here.
	 */
	{
		struct token *token1 = ctx->token1;

		if (token1 != NULL && token1->type != TT_UNSET) {
			token_move(t, token1);
			return PANCL_SUCCESS;
		}
	}

	/* Make sure the token buffer is cleared out. */
	token_buffer_reset(tb);

	while ((err = get_next(ctx, &c)) == PANCL_SUCCESS) {
		/* Store position data. */
		t->loc = ctx->loc;
		ctx->error_loc = ctx->loc;

		/* Grab newline */
		if (is_newline(ctx, c)) {
			/* Escaped newlines are eaten and ignored. */
			if (escaped) {
				escaped = false;
				continue;
			}

			return token_set_empty(t, TT_NEWLINE, TST_NEWLINE);
		}

		/* Anything preceeded by a backslash that isn't a newline is an
		 * ERROR token.
		 */
		if (escaped) {
			err = token_buffer_append(tb, '\\');

			if (err != PANCL_SUCCESS)
				return err;

			goto invalid_character;
		}

		/* Check the simple cases (single-character tokens) */
		switch (c) {
		case '[': /* TT_L_BRACKET */
		case ']': /* TT_R_BRACKET */
		case '(': /* TT_L_PAREN */
		case ')': /* TT_R_PAREN */
		case '{': /* TT_L_BRACE */
		case '}': /* TT_R_BRACE */
		case '=': /* TT_EQ */
		case ',': /* TT_COMMA */
			return token_set_empty(t, (int)(c & 0xff), TST_NONE);
		default:
			break;
		}

		/* Whitespace is irrelevant. If found, consume it and start parsing the
		 * next character.
		 */
		if (is_whitespace(c))
			continue;

		/* Comment! (They count as newlines for simplicity) */
		if (c == '#') {
			err = consume_comment(ctx);

			if (err == PANCL_SUCCESS)
				err = token_set_empty(t, TT_COMMENT, TST_NEWLINE);

			return err;
		}

		/* Check for a string. */
		if (is_quote(c)) {
			err = get_string(ctx, tb, c);

			if (err == PANCL_SUCCESS)
				err = token_set(t, TT_STRING, TST_IDENT, tb);

			return err;
		}

		/* Check for a raw identifier. */
		if (is_raw_ident(c)) {
			err = get_raw_ident(ctx, tb, c);

			/* Raw identifiers have to be checked again as the following also
			 * look like raw identifiers:
			 *   - booleans (true/false)
			 *   - integers (bin, dec, hex, oct)
			 *   - floating point values
			 */
			if (err == PANCL_SUCCESS)
				err = set_ident_token(t, ctx, tb);

			return err;
		}

		/* If we got a backslash then we expect an escaped newline next so mark
		 * that we're escaping and continue.
		 */
		if (c == '\\') {
			escaped = true;
			continue;
		}

		/* All other characters are invalid. */
		goto invalid_character;
	}

	/* Return end of file (end of input really). */
	if (err == PANCL_END_OF_INPUT) {
		/* If we ended on an a backslash, we need to report it. */
		if (escaped == true)
			goto invalid_character;

		return token_set_empty(t, TT_EOF, TST_NONE);
	}

	return err;

invalid_character:
	err = token_buffer_append(tb, c);

	if (err == PANCL_SUCCESS)
		err = token_buffer_end(tb);

	if (err == PANCL_SUCCESS)
		err = token_set(t, TT_ERROR, TST_NONE, tb);

	return err;
}

int
lexer_rewind_token(struct pancl_context *ctx, struct token *t)
{
	if (ctx->token1 == NULL) {
		ctx->token1 = pancl_alloc(sizeof(*t));

		if (ctx->token1 == NULL)
			return PANCL_ERROR_ALLOC;

		token_init(ctx->token1);
	}
	else {
		token_fini(ctx->token1);
	}

	token_move(ctx->token1, t);
	return PANCL_SUCCESS;
}

// vim:ts=4:sw=4:autoindent
