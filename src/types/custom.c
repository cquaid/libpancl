/* SPDX-License-Identifier: MIT */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "pancl/pancl.h"
#include "internal.h"


void
pancl_custom_init(struct pancl_custom *custom)
{
	if (custom == NULL)
		return;

	memset(custom, 0, sizeof(*custom));
	pancl_tuple_init(&(custom->tuple));
}

void
pancl_custom_fini(struct pancl_custom *custom)
{
	if (custom == NULL)
		return;

	pancl_free(custom->name);
	pancl_tuple_fini(&(custom->tuple));

	pancl_custom_init(custom);
}

// vim:ts=4:sw=4:autoindent
