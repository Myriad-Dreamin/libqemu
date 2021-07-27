/*
 * QTest accelerator code
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

#include "qemu/osdep.h"
#include "qemu/rcu.h"
#include "qapi/error.h"
#include "qemu/module.h"
#include "qemu/option.h"
#include "qemu/config-file.h"
#include "qemu/accel.h"
#include "sysemu/kfuzz.h"
#include "sysemu/cpus.h"
#include "sysemu/cpu-timers.h"
#include "qemu/guest-random.h"
#include "qemu/main-loop.h"
#include "hw/core/cpu.h"

bool kfuzz_allowed;

static int kfuzz_init_accel(MachineState *ms)
{
    return 0;
}

static void kfuzz_accel_class_init(ObjectClass *oc, void *data)
{
    AccelClass *ac = ACCEL_CLASS(oc);
    ac->name = "kfuzz";
    ac->init_machine = kfuzz_init_accel;
    ac->allowed = &kfuzz_allowed;
}

#define TYPE_QTEST_ACCEL ACCEL_CLASS_NAME("kfuzz")

static const TypeInfo kfuzz_accel_type = {
    .name = TYPE_QTEST_ACCEL,
    .parent = TYPE_ACCEL,
    .class_init = kfuzz_accel_class_init,
};

static void kfuzz_accel_ops_class_init(ObjectClass *oc, void *data)
{
    AccelOpsClass *ops = ACCEL_OPS_CLASS(oc);

    ops->create_vcpu_thread = dummy_start_vcpu_thread;
//    ops->get_virtual_clock = qtest_get_virtual_clock;
};

static const TypeInfo kfuzz_accel_ops_type = {
    .name = ACCEL_OPS_NAME("kfuzz"),

    .parent = TYPE_ACCEL_OPS,
    .class_init = kfuzz_accel_ops_class_init,
    .abstract = true,
};

static void kfuzz_type_init(void)
{
    type_register_static(&kfuzz_accel_type);
    type_register_static(&kfuzz_accel_ops_type);
}

type_init(kfuzz_type_init);
