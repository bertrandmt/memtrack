/*
 * Copyright (c) 2003-2016 - Bertrand Mollinier Toublet.
 * bertrandmt@gmail.com
 * http://www.bmt-online.org/
 *
 * Dynamic array - dyn_array
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MEMTRACK_H
#define MEMTRACK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

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

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* MEMTRACK_H */
