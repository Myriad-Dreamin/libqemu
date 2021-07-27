/*
 * Test Server
 *
 * Copyright IBM, Corp. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef KFUZZ_H
#define KFUZZ_H


extern bool kfuzz_allowed;

static inline bool kfuzz_enabled(void)
{
    return kfuzz_allowed;
}

#endif
