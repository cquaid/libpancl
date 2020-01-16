/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl.h"
#include "pancl_error.h"
#include "internal.h"


void
pancl_entry_init(struct pancl_entry *entry)
{
	if (entry != NULL) {
		memset(entry, 0, sizeof(*entry));
		pancl_value_init(&(entry->value), PANCL_TYPE_INTEGER);
	}
}

int
pancl_entry_new(struct pancl_entry **entry)
{
	if (entry == NULL)
		return PANCL_ERROR_ARG_INVALID;

	*entry = pancl_alloc(sizeof(**entry));

	if (*entry == NULL)
		return PANCL_ERROR_ALLOC;

	pancl_entry_init(*entry);
	return PANCL_SUCCESS;
}

void
pancl_entry_fini(struct pancl_entry *entry)
{
	if (entry == NULL)
		return;

	pancl_free(entry->name);
	pancl_value_fini(&(entry->value));

	pancl_entry_init(entry);
}

void
pancl_entry_destroy(struct pancl_entry **entry)
{
	if (entry == NULL || *entry == NULL)
		return;

	pancl_entry_fini(*entry);
	pancl_free(*entry);
	*entry = NULL;
}

// vim:ts=4:sw=4:autoindent
