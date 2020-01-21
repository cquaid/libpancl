# Panini Configuration Language (PanCL)

This is the reference parser for the Panini Configuration Language.

## Building

*Required Utils*
* A C compiler with C99 support.
* GNU Make

*Build steps*
* Run `make`

*Install steps*
* Run `make install`
    * By default, files are installed to `/usr/local/`. This may be overwritten
      by setting the `DESTDIR` make variable: `make install DESTDIR=/path`


## Usage
1. Include the relevant header
```c
#include <pancl/pancl.h>
```
1. Initialize a `pancl_context`
```c
struct pancl_context ctx;
pancl_context_init(&ctx);
```
1. Attach the context to some form of input:
```c
int err;

/* From a file */
FILE *f = ...;
err = pancl_parse_file(&ctx, f);

/* From a NUL-terminated UTF-8 string. */
const char *string = ...;
err = pancl_parse_string(&ctx, string);

/* From an arbitrary memory buffer. */
const void *memory = ...;
size_t memory_size = ...;
err = pancl_parse_buffer(&ctx, memory, memory_size);
```
1. Parse each table in the file one at a time.
```c
struct pancl_table table;
pancl_table_init(&table);

for (;;) {
    err = pancl_get_next_table(&ctx, &table);
    /* Done parsing? */
    if (err == PANCL_END_OF_INPUT)
        break;
    /* Some form of failure occured */
    if (err != PANCL_SUCCESS)
        goto handle_failure;

    /* Do something with the table. */
    ...

    pancl_table_fini(&table);
}
```
1. Clean up when you're done:
```c
pancl_context_fini(&ctx);
```

If a failure occurs (`PANCL_ERROR_*` return value), the
`pancl_context.error_loc` member will be updated to reflect the approximaate
location in the input where the failure was encountered. `pancl_strerror()` may
also be used to retrieve the string description of the returned error code.

Additionally, `pancl_context.error_token` *might* be set to the UTF-8 string
of the token the error occured on.  This is not guaranteed to be set and will
be NULL when the token is unknown or wasn't allocated.

