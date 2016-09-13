/*
 * Copyright (c) 2003-2016 - Bertrand Mollinier Toublet.
 * bertrandmt@gmail.com
 * http://www.bmt-online.org/
 *
 * Dynamic array - dyn_array
 *
 * This program is  free software;  you can  redistribute it  and/or
 * modify it  under the  terms of  the GNU  General  Public  License
 * as published  by the Free  Software Foundation;  either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is  distributed in the hope that  it will be useful,
 * but WITHOUT  ANY WARRANTY;  without even the  implied warranty of
 * MERCHANTABILITY  or FITNESS  FOR A  PARTICULAR  PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along  with this  program; if  not,  write to  the Free  Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef MEMTRACK_H
#define MEMTRACK_H

#define MEMTRACK_FILENAME   "memtrack.txt"

#ifdef MEMTRACK

#define xmalloc(size)       xmalloc_impl(size, __FILE__, __LINE__)
#define xstrdup(d, s)       xstrdupd_impl(d, s, __FILE__, __LINE__)
#define xrealloc(p, sz)     xrealloc_impl(p, sz, __FILE__, __LINE__)
#define xfree(p)            xfree_impl(p, __FILE__, __LINE__)

typedef enum
{
    MEMTRK_ALLOC,
    MEMTRK_FREE,
    MEMTRK_REPORT,
    MEMTRK_DESTROY
} mt_msg_t;

typedef union
{
    long l;
    unsigned long lu;
    double d;
    long double ld;
    void *vp;
    void (*fp)(void);
} mt_align_t;

typedef struct payload
{
    void *ptr;
    size_t size;
    char *file;
    int line;
} mt_payload_t;

int memtrack(mt_msg_t, long unsigned, void *, int, char *, int);
void *xmalloc_impl(size_t, char *, int);
void *xrealloc_impl(void *, size_t, char *, int);
void xfree_impl(void *, char *, int);
char *xstrdupd_impl(char *, const char *, char *, int);

#else /* MEMTRACK */

#define xmalloc     malloc
#define xstrdup     xstrdup_impl
#define xrealloc    realloc
#define xfree       free
#define memtrack(a, b, c, d, e, f)

char *xstrdup_impl(char *dst, const char *src);

#endif /* MEMTRACK */

#endif /* MEMTRACK_H */
