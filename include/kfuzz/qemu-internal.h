//
// Created by Myriad-Dreamin on 2021/7/30.
//

#ifndef LIBQEMU_QEMU_INTERNAL_H
#define LIBQEMU_QEMU_INTERNAL_H

#include "qemu/osdep.h"
#include "qemu-common.h"
extern __thread MemoryRegion *kfuzz_thread_local_mr;
extern pthread_spinlock_t io_available;

#endif //LIBQEMU_QEMU_INTERNAL_H
