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
 
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>

#include "memtrack.h"
#include "hash.h"

char *xstrdup_impl(char *dst, const char *src)
{
    dst = NULL;

    if (NULL != src)
    {
        dst = malloc(strlen(src) + 1);
        if (NULL != dst)
        {
            strcpy(dst, src);
        }
    }
    return dst;
}

#if defined(MEMTRACK)

void *xmalloc_impl(size_t sz, char *file, int line)
{
    void *ptr;
    char *p;

    ptr = malloc(sz + sizeof(mt_align_t));
    if (NULL != ptr)
    {
        memtrack(
            MEMTRK_ALLOC,
            0,
            ptr,
            sz,
            file,
            line);

        p = ptr;
        p += sizeof(mt_align_t);
        ptr = p;
    }

    return ptr;
}

char *xstrdupd_impl(char *dst, const char *src, char *file, int line)
{
    size_t len = strlen(src) + 1;

    dst = malloc(len + sizeof(mt_align_t));
    if (NULL != dst)
    {
        strcpy(dst + sizeof(mt_align_t), src);
        memtrack(
            MEMTRK_ALLOC,
            0,
            dst,
            len,
            file,
            line);
        dst += sizeof(mt_align_t);
    }
    return dst;
}

void *xrealloc_impl(void *ptr, size_t size, char *file, int line)
{
    void *nptr = NULL;
    char *p = NULL;
    mt_align_t keystore = {0};

    if (NULL != ptr)
    {
        p = ptr;
        p -= sizeof(mt_align_t);
        memcpy(&keystore.lu, p, sizeof(mt_align_t));
    }

    nptr = realloc(p, size + ((size > 0) ? sizeof(mt_align_t) : 0));
    if (NULL != nptr)
    {
        if (NULL != ptr)
        {
            memtrack(
                MEMTRK_FREE,
                keystore.lu,
                NULL,
                0,
                file,
                line);
        }
        if (size > 0)
        {
            memtrack(
                MEMTRK_ALLOC,
                0,
                nptr,
                size,
                file,
                line);
            p = nptr;
            p += sizeof(mt_align_t);
            nptr = p;
        }
    }

    return nptr;
}

void xfree_impl(void *ptr, char *file, int line)
{
    char *p;
    mt_align_t keystore;

    if (NULL != ptr)
    {
        p = ptr;
        p -= sizeof(mt_align_t);
        memcpy(&keystore.lu, p, sizeof(mt_align_t));

        memtrack(
            MEMTRK_FREE,
            keystore.lu,
            NULL,
            0,
            file,
            line);
        free(p);
    }
}

long unsigned mt_hash1(long unsigned value)
{
    return (value * 179424601UL + 71UL) % 167UL;
}

hash_val_t mt_hash2(const void *kvp)
{
    const long unsigned *kp = kvp;

    return (*kp * 179424673UL + 257UL) % 2147483647UL;
}

int mt_cmp(const void *kvp1, const void *kvp2)
{
    int diff = 0;

    const long unsigned *kp1 = kvp1, *kp2 = kvp2;

    if (*kp1 > *kp2)
    {
        diff = 1;
    }
    else if (*kp1 < *kp2)
    {
        diff = -1;
    }
    else
    {
        diff = 0;
    }

    return diff;
}

int memtrack(mt_msg_t msg, long unsigned key, void *ptr, int size, char *file, int line)
{
    int status = 0;

    static FILE *fp = NULL;
    static long unsigned mt_index = 0;

    mt_payload_t *entry = NULL;
    mt_align_t *keystore = NULL;

    static hash_t *hash = NULL;
    static int initialized = 0;
    static long max_alloc = 0;
    static long cur_alloc = 0;
    static long tot_alloc = 0;

    time_t tt = {0};
    struct tm *tmt = NULL;

    if (!initialized)
    {
        fp = fopen(MEMTRACK_FILENAME, "w");
        if (NULL == fp)
        {
            fprintf(
                stderr,
                "Can't create file %s\n"
                "Using stdout instead.\n",
                MEMTRACK_FILENAME);
            fp = stdout;
        }

#define HASH_SIZE       100000
        hash = hash_create(HASH_SIZE, mt_cmp, mt_hash2);
        assert(NULL != hash);

        initialized = 1;
    }

    switch(msg)
    {
        hnode_t *node;
        hscan_t iterator;
    case MEMTRK_ALLOC:
        entry = malloc(sizeof *entry);
        assert(NULL != entry);

        entry->file = file;
        entry->line = line;
        entry->ptr = ptr;
        entry->size = size;

        keystore = malloc(sizeof *keystore);
        assert(NULL != keystore);
        keystore->lu = mt_index++;

        memcpy(ptr, keystore, sizeof *keystore);

        if (1 != hash_alloc_insert(hash, keystore, entry))
        {
            fprintf(
                fp,
                "ERROR in debugging code - "
                "failed to add node to memory hash.\n");
            fflush(fp);
        }
        else
        {
            cur_alloc += size;
            tot_alloc += size;
            if (cur_alloc > max_alloc)
            {
                max_alloc = cur_alloc;
            }
        }
        break;
    case MEMTRK_FREE:
        node = hash_lookup(hash, &key);
        if (NULL != node)
        {
            keystore = (mt_align_t *)hnode_getkey(node);
            entry = hnode_get(node);
            cur_alloc -= entry->size;
            if (cur_alloc < 0)
            {
                fprintf(
                    fp,
                    "ERROR: more memory released than allocated!\n");
                fflush(fp);
            }
            hash_delete(hash, node);
            free(keystore);
            free(entry);
            hnode_destroy(node);
        }
        else
        {
            /* Tried to free an entry that was never
             * allocated.
             */
            fprintf(
                fp,
                "ERROR: attempted to free unallocated "
                "block %p at line %d of file %s.\n",
                (char *)ptr + sizeof(mt_align_t),
                line,
                file);
        }
        break;
    case MEMTRK_REPORT:
        fprintf(
            fp,
            "\n"
            "Memory Tracker Report\n"
            "---------------------\n"
            "\n");
        tt = time(NULL);
        tmt = localtime(&tt);
        if (NULL != tmt)
        {
            char timebuffer[64] = {0};

            strftime(
                timebuffer,
                sizeof timebuffer,
                "%H:%M:%S %Z on %A %d %B %Y",
                tmt);
            fprintf(fp, "%s\n", timebuffer);
        }
        fprintf(
            fp,
            "\n"
            "Current allocation: %ld byt%s.\n"
            "Maximum allocation: %ld byt%s.\n"
            "Total allocation: %ld byt%s.\n"
            "Nodes currently allocated: %s\n",
            cur_alloc,
            (1 == cur_alloc) ? "e" : "es",
            max_alloc,
            (1 == max_alloc) ? "e" : "es",
            tot_alloc,
            (1 == tot_alloc) ? "e" : "es",
            (0 == cur_alloc) ? "none. Well done !" : "");

        hash_scan_begin(&iterator, hash);
        while(NULL != (node = hash_scan_next(&iterator)))
        {
            entry = hnode_get(node);
            fprintf(
                fp,
                "%8p allocated %7u byt%s "
                "at line %5d of file %s.\n",
                entry->ptr,
                (unsigned int)entry->size,
                entry->size == 1 ? "e" : "es",
                entry->line,
                entry->file);
        }
        fflush(fp);
        break;
    case MEMTRK_DESTROY:
        hash_scan_begin(&iterator, hash);
        while(NULL != (node = hash_scan_next(&iterator)))
        {
            free(hnode_get(node));
            hash_scan_delete(hash, node);
            hnode_destroy(node);
        }
        hash_destroy(hash);

        if (ferror(fp))
        {
            fprintf(stderr, "Error writing to log file.\n");
        }
        fclose(fp);
        break;
    default:
        break;
    }
    return status;
}

#endif /* MEMTRACK */
