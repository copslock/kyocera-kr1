/* misc. universal things
 * Copyright (C) 1997 Angelos D. Keromytis.
 * Copyright (C) 1998-2001  D. Hugh Redelmeier.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * RCSID $Id: oswalloc.h,v 1.1 2004/12/24 07:17:30 rupert Exp $
 */

#ifndef _OSW_ALLOC_H_
#define _OSW_ALLOC_H_

/* memory allocation */

extern void *alloc_bytes(size_t size, const char *name);
#define alloc_thing(thing, name) (alloc_bytes(sizeof(thing), (name)))

extern void *clone_bytes(const void *orig, size_t size, const char *name);
#define clone_thing(orig, name) clone_bytes((const void *)&(orig), sizeof(orig), (name))
#define clone_str(str, name) \
    ((str) == NULL? NULL : clone_bytes((str), strlen((str))+1, (name)))

#ifdef LEAK_DETECTIVE
  extern void pfree(void *ptr);
  extern void report_leaks(void);
#else
# define pfree(ptr) free(ptr)	/* ordinary stdc free */
#endif
#define pfreeany(p) { if ((p) != NULL) pfree(p); }
#define replace(p, q) { pfreeany(p); (p) = (q); }


/* chunk is a simple pointer-and-size abstraction */

struct chunk {
    u_char *ptr;
    size_t len;
    };
typedef struct chunk chunk_t;

#define setchunk(ch, addr, size) { (ch).ptr = (addr); (ch).len = (size); }
/* NOTE: freeanychunk, unlike pfreeany, NULLs .ptr */
#define freeanychunk(ch) { pfreeany((ch).ptr); (ch).ptr = NULL; }
#define clonetochunk(ch, addr, size, name) \
    { (ch).ptr = clone_bytes((addr), (ch).len = (size), name); }
#define clonereplacechunk(ch, addr, size, name) \
    { pfreeany((ch).ptr); clonetochunk(ch, addr, size, name); }
#define chunkcpy(dst, chunk) \
    { memcpy(dst, chunk.ptr, chunk.len); dst += chunk.len;}
#define same_chunk(a, b) \
    (a).len == (b).len && memcmp((a).ptr, (b).ptr, (b).len) == 0
  
extern const chunk_t empty_chunk;

/* compare two chunks */
extern bool cmp_chunk(chunk_t a, chunk_t b);

/* zero all bytes */
#define zero(x) memset((x), '\0', sizeof(*(x)))

typedef void (*exit_log_func_t)(const char *message, ...);
extern void set_exit_log_func(exit_log_func_t func);

#endif /* _OSW_ALLOC_H_ */
