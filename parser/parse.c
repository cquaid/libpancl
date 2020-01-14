/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"

#include "internal.h"
#include "lexer/token.h"
#include "parser/custom_types.h"
#include "parser/str_to_int.h"

/**
 * Return value for a terminator function.
 */
enum terminator_status {
	TERM_STATUS_IGNORE, /**< Token can be ignored */
	TERM_STATUS_VALID, /**< Token is valid, found the termiantor */
	TERM_STATUS_INVALID /**< Token is invalid, error */
};

typedef enum terminator_status (*terminator_fn)(const struct token * const t);

static int parse_rvalue(struct pancl_context *ctx, struct token_buffer *tb,
				struct token *start, struct pancl_value *value,
				terminator_fn is_terminator);
static int parse_assignment(struct pancl_context *ctx, struct token_buffer *tb,
				struct pancl_entry **entry_storage,
				terminator_fn is_terminator);


/**
 * Generic newline terminator checking function.
 */
static enum terminator_status
newline_terminator(const struct token * const t)
{
	/* EOF is valid anywhere a newline is a valid terminator. */
	if (t->type == TT_EOF)
		return TERM_STATUS_VALID;

	return (t->subtype == TST_NEWLINE)
		? TERM_STATUS_VALID
		: TERM_STATUS_INVALID;
}

/**
 * Array members may be termianted by commas or right brackets, ignoring
 * any newlines.
 */
static enum terminator_status
array_member_terminator(const struct token * const t)
{
	/* When searching for an ',' or closing bracket, newlines are
	 * allowed.
	 */
	if (t->subtype == TST_NEWLINE)
		return TERM_STATUS_IGNORE;

	if (t->type == TT_COMMA)
		return TERM_STATUS_VALID;

	if (t->type == TT_R_BRACKET)
		return TERM_STATUS_VALID;

	return TERM_STATUS_INVALID;
}

/**
 * Handle an array RVALUE:
 *
 * ArrayList = RVALUE
 *           | ArrayList ',' RVALUE
 *           ;
 *
 * Array = '[' ']'
 *       | '[' ArrayList ']'
 *       | '[' ArrayList ',' ']'
 *       ;
 *
 *  The opening '[' was consumed by the caller so we start parsing after that.
 */
static int
parse_array(struct pancl_context *ctx, struct token_buffer *tb,
	struct pancl_array *array, terminator_fn is_terminator)
{
	int err;
	struct token t = TOKEN_INIT;

	enum {
		/* The '[' was already handled. */
		FIND_RVALUE_OR_R_BRACKET,
		FIND_COMMA_OR_R_BRACKET,
		FIND_TERMINATOR
	} state;

	state = FIND_RVALUE_OR_R_BRACKET;

	/* Array should already be initialized, but whatever. */
	pancl_array_init(array);

	for (;;) {
		err = next_token(ctx, tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Whitespace is always allowed. */
		if (t.subtype == TST_WS) {
			token_fini(&t);
			continue;
		}

		switch (state) {
		case FIND_RVALUE_OR_R_BRACKET:
			/* When searching for an RVALUE or closing bracket, newlines are
			 * allowed.
			 */
			if (t.subtype == TST_NEWLINE) {
				token_fini(&t);
				continue;
			}

			if (t.type == TT_R_BRACKET) {
				/* Got the ], find that terminator! */
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}

			/* Try to find an RVALUE. */
			{
				struct pancl_value *v = malloc(sizeof(*v));

				if (v == NULL) {
					err = PANCL_ERROR_ALLOC;
					goto cleanup;
				}

				err = parse_rvalue(ctx, tb, &t, v, array_member_terminator);

				if (err == PANCL_SUCCESS)
					err = pancl_array_append(array, v);

				if (err != PANCL_SUCCESS) {
					pancl_value_fini(v);
					free(v);
					goto cleanup;
				}
			}

			state = FIND_COMMA_OR_R_BRACKET;
			token_fini(&t);
			continue;

		case FIND_COMMA_OR_R_BRACKET:
			/* When searching for an ',' or closing bracket, newlines are
			 * allowed.
			 */
			if (t.subtype == TST_NEWLINE) {
				token_fini(&t);
				continue;
			}

			if (t.type == TT_COMMA) {
				/* Got a comma, move to another RVALUE or the ']' */
				state = FIND_RVALUE_OR_R_BRACKET;
				token_fini(&t);
				continue;
			}

			if (t.type == TT_R_BRACKET) {
				/* Got the ], find that terminator! */
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}
			break;

		case FIND_TERMINATOR:
			{
				enum terminator_status term = is_terminator(&t);

				if (term == TERM_STATUS_IGNORE) {
					token_fini(&t);
					continue;
				}

				if (term == TERM_STATUS_VALID) {
					err = set_first_token(ctx, &t);
					goto cleanup;
				}
			}
			break;
		}

		/* Got anything else: invalid parse. */
		ctx->error_pos = t.pos;

		/* XXX: Store the error token type. */
		if (t.type == TT_EOF)
			err = PANCL_ERROR_PARSER_EOF;
		else
			err = PANCL_ERROR_PARSER_ARRAY;
		goto cleanup;
	}

	/* We shouldn't be able to get here. */
	err = PANCL_ERROR_INTERNAL;

cleanup:
	token_fini(&t);
	return err;
}

/**
 * Tuple members may be termianted by commas or right parens, ignoring
 * any newlines.
 */
static enum terminator_status
tuple_member_terminator(const struct token * const t)
{
	/* When searching for an ',' or closing paren, newlines are
	 * allowed.
	 */
	if (t->subtype == TST_NEWLINE)
		return TERM_STATUS_IGNORE;

	if (t->type == TT_COMMA)
		return TERM_STATUS_VALID;

	if (t->type == TT_R_PAREN)
		return TERM_STATUS_VALID;

	return TERM_STATUS_INVALID;
}

/**
 * Handle a tuple RVALUE:
 *
 * TupleList = RVALUE
 *           | TupleList ',' RVALUE
 *           ;
 *
 * Tuple = '(' ')'
 *       | '(' TupleList ')'
 *       | '(' TupleList ',' ')'
 *       ;
 *
 *  The opening '(' was consumed by the caller so we start parsing after that.
 */
static int
parse_tuple(struct pancl_context *ctx, struct token_buffer *tb,
	struct pancl_tuple *tuple, terminator_fn is_terminator)
{
	int err;
	struct token t = TOKEN_INIT;

	enum {
		/* The '(' was already handled. */
		FIND_RVALUE_OR_R_PAREN,
		FIND_COMMA_OR_R_PAREN,
		FIND_TERMINATOR
	} state;

	state = FIND_RVALUE_OR_R_PAREN;

	/* Tuple should already be initialized, but whatever. */
	pancl_tuple_init(tuple);

	for (;;) {
		err = next_token(ctx, tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Whitespace is always allowed. */
		if (t.subtype == TST_WS) {
			token_fini(&t);
			continue;
		}

		switch (state) {
		case FIND_RVALUE_OR_R_PAREN:
			/* When searching for an RVALUE or closing paren, newlines are
			 * allowed.
			 */
			if (t.subtype == TST_NEWLINE) {
				token_fini(&t);
				continue;
			}

			if (t.type == TT_R_PAREN) {
				/* Got the ), find that terminator! */
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}

			/* Try to find an RVALUE. */
			{
				struct pancl_value *v = malloc(sizeof(*v));

				if (v == NULL) {
					err = PANCL_ERROR_ALLOC;
					goto cleanup;
				}

				err = parse_rvalue(ctx, tb, &t, v, tuple_member_terminator);

				if (err == PANCL_SUCCESS)
					err = pancl_tuple_append(tuple, v);

				if (err != PANCL_SUCCESS) {
					pancl_value_fini(v);
					free(v);
					goto cleanup;
				}
			}

			state = FIND_COMMA_OR_R_PAREN;
			token_fini(&t);
			continue;

		case FIND_COMMA_OR_R_PAREN:
			/* When searching for an ',' or closing paren, newlines are
			 * allowed.
			 */
			if (t.subtype == TST_NEWLINE) {
				token_fini(&t);
				continue;
			}

			if (t.type == TT_COMMA) {
				/* Got a comma, move to another RVALUE or the ')' */
				state = FIND_RVALUE_OR_R_PAREN;
				token_fini(&t);
				continue;
			}

			if (t.type == TT_R_PAREN) {
				/* Got the ), find that terminator */
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}
			break;

		case FIND_TERMINATOR:
			{
				enum terminator_status term = is_terminator(&t);

				if (term == TERM_STATUS_IGNORE) {
					token_fini(&t);
					continue;
				}

				if (term == TERM_STATUS_VALID) {
					err = set_first_token(ctx, &t);
					goto cleanup;
				}
			}
			break;
		}

		/* Got anything else: invalid parse. */
		ctx->error_pos = t.pos;

		/* XXX: Store the error token type. */
		if (t.type == TT_EOF)
			err = PANCL_ERROR_PARSER_EOF;
		else
			err = PANCL_ERROR_PARSER_TUPLE;
		goto cleanup;
	}

	/* We shouldn't be able to get here. */
	err = PANCL_ERROR_INTERNAL;

cleanup:
	token_fini(&t);
	return err;
}

/**
 * Inline table entries may be termianted by commas or right braces, ignoring
 * any newlines.
 */
static enum terminator_status
table_entry_terminator(const struct token * const t)
{
	/* When searching for an ',' or closing brace, newlines are
	 * allowed.
	 */
	if (t->subtype == TST_NEWLINE)
		return TERM_STATUS_IGNORE;

	if (t->type == TT_COMMA)
		return TERM_STATUS_VALID;

	if (t->type == TT_R_BRACE)
		return TERM_STATUS_VALID;

	return TERM_STATUS_INVALID;
}

/**
 * Handle an inline table RVALUE:
 *
 * InlineTableList = Assignement
 *                 | InlineTableList ',' Assignment
 *                 ;
 *
 * InlineTable = '{' '}'
 *             | '{' InlineTableList '}'
 *             | '{' InlineTableList ',' '}'
 *             ;
 *
 *  The opening '{' was consumed by the caller so we start parsing after that.
 */
static int
parse_table_data(struct pancl_context *ctx, struct token_buffer *tb,
	struct pancl_table_data *table_data, terminator_fn is_terminator)
{
	int err;
	struct token t = TOKEN_INIT;

	enum {
		/* The '{' was already handled. */
		FIND_ASSIGNMENT_OR_R_BRACE,
		FIND_COMMA_OR_R_BRACE,
		FIND_TERMINATOR
	} state;

	state = FIND_ASSIGNMENT_OR_R_BRACE;

	/* Table data should already be initialized, but whatever. */
	pancl_table_data_init(table_data);

	for (;;) {
		err = next_token(ctx, tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Whitespace is always allowed. */
		if (t.subtype == TST_WS) {
			token_fini(&t);
			continue;
		}

		switch (state) {
		case FIND_ASSIGNMENT_OR_R_BRACE:
			/* When searching for an assignment, newlines are allowed. */
			if (t.subtype == TST_NEWLINE) {
				token_fini(&t);
				continue;
			}

			if (t.type == TT_R_BRACE) {
				/* Got the }, find that newline! */
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}

			/* If we got an identifier, then this is likely an "assignment". */
			if (t.subtype == TST_IDENT) {
				struct pancl_entry *entry = NULL;
				err = parse_assignment(ctx, tb, &entry,
						table_entry_terminator);

				/* On success we append to the entry to the table.*/
				if (err == PANCL_SUCCESS) {
					entry->name = t.value;
					t.value = NULL; /* Owned by the entry now. */
					err = pancl_table_data_append(table_data, entry);
				}

				if (err != PANCL_SUCCESS) {
					pancl_entry_fini(entry);
					free(entry);
					goto cleanup;
				}

				state = FIND_COMMA_OR_R_BRACE;
				token_fini(&t);
				continue;
			}
			break;

		case FIND_COMMA_OR_R_BRACE:
			/* When searching for an ',' or closing brace, newlines are
			 * allowed.
			 */
			if (t.subtype == TST_NEWLINE) {
				token_fini(&t);
				continue;
			}

			if (t.type == TT_COMMA) {
				/* Got a comma, move to another assignment or the '}' */
				state = FIND_ASSIGNMENT_OR_R_BRACE;
				token_fini(&t);
				continue;
			}

			if (t.type == TT_R_BRACE) {
				/* Got the }, find that terminator! */
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}
			break;

		case FIND_TERMINATOR:
			{
				enum terminator_status term = is_terminator(&t);

				if (term == TERM_STATUS_IGNORE) {
					token_fini(&t);
					continue;
				}

				if (term == TERM_STATUS_VALID) {
					err = set_first_token(ctx, &t);
					goto cleanup;
				}
			}
			break;
		}

		/* Got anything else: invalid parse. */
		ctx->error_pos = t.pos;

		/* XXX: Store the error token type. */
		if (t.type == TT_EOF)
			err = PANCL_ERROR_PARSER_EOF;
		else
			err = PANCL_ERROR_PARSER_INLINE_TABLE;
		goto cleanup;
	}

	/* We shouldn't be able to get here. */
	err = PANCL_ERROR_INTERNAL;

cleanup:
	token_fini(&t);
	return err;
}

/**
 * Parse a "custom" RVALUE.
 *
 * CustomType = raw_identifier Tuple
 *            ;
 *
 * The raw_identifier portion is handled by the caller so we start parsing
 * with a Tuple.
 */
static int
parse_custom_type(struct pancl_context *ctx, struct token_buffer *tb,
	struct pancl_custom *custom, terminator_fn is_terminator)
{
	int err;
	struct token t = TOKEN_INIT;

	/* Name already assigned, look for the tuple start. */

	for (;;) {
		err = next_token(ctx, tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		if (t.type == TT_EOF) {
			err = PANCL_ERROR_PARSER_EOF;
			goto cleanup;
		}

		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Whitespace is always allowed. */
		if (t.subtype == TST_WS) {
			token_fini(&t);
			continue;
		}

		if (t.type == TT_L_PAREN) {
			err = parse_tuple(ctx, tb, &(custom->args), is_terminator);
			goto cleanup;
		}

		/* Got anything else: invalid parse. */
		ctx->error_pos = t.pos;
		/* XXX: Store the error token type. */
		err = PANCL_ERROR_PARSER_CUSTOM_ARGS;
		goto cleanup;
	}

	/* We shouldn't be able to get here. */
	err = PANCL_ERROR_INTERNAL;

cleanup:
	token_fini(&t);
	return err;
}

/**
 * Parse an RVALUE.
 *
 * RVALUE = string
 *        | binary_integer
 *        | decimal_integer
 *        | hex_integer
 *        | octal_integer
 *        | float
 *        | boolean_true
 *        | boolean_false
 *        | Array
 *        | Tuple
 *        | InlineTable
 *        | CustomType
 *        ;
 */
static int
parse_rvalue(struct pancl_context *ctx, struct token_buffer *tb,
	struct token *start, struct pancl_value *value,
	terminator_fn is_terminator)
{
	switch (start->type) {
	case TT_STRING:
		pancl_value_init(value, PANCL_TYPE_STRING);
		value->data.string = start->value;
		start->value = NULL; /* Owned by value now. */
		return PANCL_SUCCESS;

	case TT_INT_BIN:
		pancl_value_init(value, PANCL_TYPE_INTEGER);
		return str_to_int32(&(value->data.integer), start->value, 2);

	case TT_INT_DEC:
		pancl_value_init(value, PANCL_TYPE_INTEGER);
		return str_to_int32(&(value->data.integer), start->value, 10);

	case TT_INT_HEX:
		pancl_value_init(value, PANCL_TYPE_INTEGER);
		return str_to_int32(&(value->data.integer), start->value, 16);

	case TT_INT_OCT:
		pancl_value_init(value, PANCL_TYPE_INTEGER);
		return str_to_int32(&(value->data.integer), start->value, 8);

	case TT_FLOAT:
		pancl_value_init(value, PANCL_TYPE_FLOATING);
		{
			// XXX
			value->data.floating = strtod(start->value, NULL);
		}
		return PANCL_SUCCESS;

	case TT_TRUE:
		pancl_value_init(value, PANCL_TYPE_BOOLEAN);
		value->data.boolean = 1;
		return PANCL_SUCCESS;

	case TT_FALSE:
		pancl_value_init(value, PANCL_TYPE_BOOLEAN);
		value->data.boolean = 0;
		return PANCL_SUCCESS;

	case TT_L_BRACKET: /* Array start */
		pancl_value_init(value, PANCL_TYPE_ARRAY);
		return parse_array(ctx, tb, &(value->data.array), is_terminator);

	case TT_L_PAREN: /* Tuple start */
		pancl_value_init(value, PANCL_TYPE_TUPLE);
		return parse_tuple(ctx, tb, &(value->data.tuple), is_terminator);

	case TT_L_BRACE: /* Table start */
		pancl_value_init(value, PANCL_TYPE_TABLE);
		return parse_table_data(ctx, tb, &(value->data.table), is_terminator);

	case TT_RAW_IDENT: /* Custom type start */
		pancl_value_init(value, PANCL_TYPE_CUSTOM);
		value->data.custom.name = start->value;
		start->value = NULL; /* Owned by value now. */
		{
			int err;

			err = parse_custom_type(ctx, tb, &(value->data.custom),
					is_terminator);

			if (err == PANCL_SUCCESS)
				err = handle_known_custom_types(value);

			return err;
		}
		break;

	default:
		return PANCL_ERROR_PARSER_RVALUE;
	}
}


/**
 * Valid assignment cases:
 *
 * Identifier = raw_ident
 *            | string
 *            ;
 *
 * Assignment = Identifier '=' RVALUE
 *            ;
 */
static int
parse_assignment(struct pancl_context *ctx, struct token_buffer *tb,
	struct pancl_entry **entry_storage, terminator_fn is_terminator)
{
	int err;
	struct token t = TOKEN_INIT;

	enum {
		/* The identifier was handled already so we expect to start with '=' */
		FIND_EQ,
		FIND_RVALUE,
		FIND_TERMINATOR
	} state;

	state = FIND_EQ;

	*entry_storage = malloc(sizeof(**entry_storage));

	if (*entry_storage == NULL)
		return PANCL_ERROR_ALLOC;

	pancl_entry_init(*entry_storage);

	for (;;) {
		err = next_token(ctx, tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Whitespace is always allowed. */
		if (t.subtype == TST_WS) {
			token_fini(&t);
			continue;
		}

		switch (state) {
		case FIND_EQ:
			if (t.type == TT_EQ) {
				state = FIND_RVALUE;
				token_fini(&t);
				continue;
			}
			break;

		case FIND_RVALUE:
			/* Propogate the termiantor check. */
			err = parse_rvalue(ctx, tb, &t, &(entry_storage[0]->data),
					is_terminator);

			if (err == PANCL_SUCCESS) {
				state = FIND_TERMINATOR;
				token_fini(&t);
				continue;
			}
			goto cleanup;

		case FIND_TERMINATOR:
			{
				enum terminator_status term = is_terminator(&t);

				if (term == TERM_STATUS_IGNORE) {
					token_fini(&t);
					continue;
				}

				if (term == TERM_STATUS_VALID) {
					/* Always rewind the terminator token for the caller to
					 * evaluate.
					 */
					err = set_first_token(ctx, &t);
					goto cleanup;
				}
			}
			break;
		}

		/* Got anything else: invalid parse. */
		ctx->error_pos = t.pos;

		/* XXX: Store the error token type. */
		if (t.type == TT_EOF)
			err = PANCL_ERROR_PARSER_EOF;
		else
			err = PANCL_ERROR_PARSER_ASSIGNMENT;
		goto cleanup;
	}

	/* We shouldn't be able to get here. */
	err = PANCL_ERROR_INTERNAL;

cleanup:
	token_fini(&t);
	return err;
}

/**
 * Parse a table header (only usable at top-level)
 *
 * Identifier = raw_identifier
 *            | string
 *            ;
 *
 * TableHeader = '[' Identifier ']' (Explicit newline)
 *             ;
 */
static int
parse_table_header(struct pancl_context *ctx, struct token_buffer *tb,
	char **name_storage)
{
	int err;
	struct token t = TOKEN_INIT;

	enum {
		/* The '[' was handled already so we expect to start with an ident. */
		FIND_IDENT,
		FIND_R_BRACKET,
		FIND_NEWLINE
	} state;

	state = FIND_IDENT;

	for (;;) {
		err = next_token(ctx, tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Whitespace is always allowed. */
		if (t.subtype == TST_WS) {
			token_fini(&t);
			continue;
		}

		switch (state) {
		case FIND_IDENT:
			/* When finding an ident, we need to look for the ident subtype. */
			if (t.subtype == TST_IDENT) {
				state = FIND_R_BRACKET;

				*name_storage = t.value;
				t.value = NULL;

				token_fini(&t);
				continue;
			}
			break;

		case FIND_R_BRACKET:
			if (t.type == TT_R_BRACKET) {
				state = FIND_NEWLINE;
				token_fini(&t);
				continue;
			}
			break;

		case FIND_NEWLINE:
			{
				enum terminator_status term = newline_terminator(&t);

				if (term == TERM_STATUS_IGNORE) {
					token_fini(&t);
					continue;
				}

				if (term == TERM_STATUS_VALID) {
					err = PANCL_SUCCESS;
					goto cleanup;
				}
			}
			break;
		}

		/* Got anything else: invalid parse. */
		ctx->error_pos = t.pos;

		/* XXX: Store the error token type. */
		if (t.type == TT_EOF)
			err = PANCL_ERROR_PARSER_EOF;
		else
			err = PANCL_ERROR_PARSER_TABLE_HEADER;
		goto cleanup;
	}

	/* We shouldn't be able to get here. */
	err = PANCL_ERROR_INTERNAL;

cleanup:
	token_fini(&t);
	return err;
}

/**
 * Get the next table in the document.
 *
 * @retval PANCL_SUCCESS        Retrieved and returned a table
 * @retval PANCL_END_OF_INPUT   No more tables to process
 * @retval PANCL_ERROR_*        Some form of failure occured
 */
int
pancl_get_table(struct pancl_context *ctx, struct pancl_table *table)
{
	struct token_buffer tb = TOKEN_BUFFER_INIT;
	struct token t = TOKEN_INIT;

	int err;

	if (ctx == NULL || table == NULL)
		return PANCL_ERROR_ARG_INVALID;

	pancl_table_init(table);

	for (;;) {
		err = next_token(ctx, &tb, &t);

		if (err != PANCL_SUCCESS)
			goto cleanup;

		/* Top level parsing context, the following constructs are valid:
		 *   whitespace (blank lines)
		 *   comments
		 *   table headers: '[' ident ']'
		 *   assignment: ident '=' rvalue
		 *   EOF (end of file/input)
		 */

		/* If we got an EOF, we just break and handle this after the loop. */
		if (t.type == TT_EOF)
			break;

		/* If we got an error token, we should return an invalid value. */
		if (t.type == TT_ERROR) {
			ctx->error_pos = t.pos;
			err = PANCL_ERROR_PARSER_TOKEN;
			goto cleanup;
		}

		/* Anything with a subtype of whitespace or newline can be ignored:
		 *  Whitespace: TT_WS, TT_LF, TT_CR
		 *  Newline: TT_NEWLINE, TT_COMMENT
		 */
		if (t.subtype == TST_WS || t.subtype == TST_NEWLINE) {
			token_fini(&t);
			continue;
		}

		/* If we got an identifier, then this is likely an "assignment". */
		if (t.subtype == TST_IDENT) {
			struct pancl_entry *entry = NULL;

			err = parse_assignment(ctx, &tb, &entry,
					newline_terminator);

			/* On success we append to the entry to the table.*/
			if (err == PANCL_SUCCESS) {
				entry->name = t.value;
				t.value = NULL; /* Owned by the entry now. */
				err = pancl_table_data_append(&(table->data), entry);
			}

			if (err != PANCL_SUCCESS) {
				pancl_entry_fini(entry);
				free(entry);
				goto cleanup;
			}

			token_fini(&t);
			continue;
		}

		/* If we got a '[' then this is likely a table header. */
		if (t.type == TT_L_BRACKET) {
			/* If our table currently has entries, call it a day and
			 * return the table.  We'll start here again on the next
			 * round.
			 *
			 * XXX: This is a bit of a hack...
			 */
			if (table->data.count != 0) {
				err = set_first_token(ctx, &t);

				if (err != PANCL_SUCCESS)
					goto cleanup;

				break;
			}

			err = parse_table_header(ctx, &tb, &(table->name));

			if (err != PANCL_SUCCESS)
				goto cleanup;

			token_fini(&t);
			continue;
		}

		/* Unknown token! */
		ctx->error_pos = t.pos;
		err = PANCL_ERROR_PARSER_TOKEN;
		goto cleanup;
	}

	/* Got a table! Or probably did at least.
	 *
	 * If table name == NULL and table has no entries, this is the end of the
	 * input.
	 */
	if (table->name == NULL && table->data.count == 0)
		err = PANCL_END_OF_INPUT;
	else
		err = PANCL_SUCCESS;

cleanup:
	if (err != PANCL_SUCCESS)
		pancl_table_fini(table);

	token_buffer_fini(&tb);
	token_fini(&t);
	return err;
}

// vim:ts=4:sw=4:autoindent
