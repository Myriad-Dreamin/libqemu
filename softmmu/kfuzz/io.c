

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/address-spaces.h"
#include "kfuzz/qemu.h"
#include "kfuzz/qemu-internal.h"

static __thread MemoryRegion *kfuzz_thread_local_io_mr = 0;

static void internal_cpu_outb(uint32_t addr, uint8_t val) {
    address_space_write(&address_space_io, addr, MEMTXATTRS_UNSPECIFIED,
                        &val, 1);
}

static void internal_cpu_outw(uint32_t addr, uint16_t val) {
    uint8_t buf[2];
    stw_p(buf, val);
    address_space_write(&address_space_io, addr, MEMTXATTRS_UNSPECIFIED,
                        buf, 2);
}

static void internal_cpu_outl(uint32_t addr, uint32_t val) {
    uint8_t buf[4];
    stl_p(buf, val);
    address_space_write(&address_space_io, addr, MEMTXATTRS_UNSPECIFIED,
                        buf, 4);
}

static uint8_t internal_cpu_inb(uint32_t addr) {
    uint8_t val;

    address_space_read(&address_space_io, addr, MEMTXATTRS_UNSPECIFIED,
                       &val, 1);
    return val;
}

static uint16_t internal_cpu_inw(uint32_t addr) {
    uint8_t buf[2];
    uint16_t val;

    address_space_read(&address_space_io, addr, MEMTXATTRS_UNSPECIFIED, buf, 2);
    val = lduw_p(buf);
    return val;
}

static uint32_t internal_cpu_inl(uint32_t addr) {
    uint8_t buf[4];
    uint32_t val;

    address_space_read(&address_space_io, addr, MEMTXATTRS_UNSPECIFIED, buf, 4);
    val = ldl_p(buf);
    return val;
}

static int kfuzz_qemu_io_read_slow(uint64_t phys_addr, uint64_t *data, uint32_t len) {
    pthread_spin_lock(&io_available);
    switch (len) {
        case 1:
            *(uint8_t *) data = internal_cpu_inb(phys_addr);
            break;
        case 2:
            *(uint16_t *) data = internal_cpu_inw(phys_addr);
            break;
        case 4:
            *(uint32_t *) data = internal_cpu_inl(phys_addr);
            break;
        default:
            abort();
    }
    kfuzz_thread_local_io_mr = kfuzz_thread_local_mr;
    pthread_spin_unlock(&io_available);
    return 0;
}

int kfuzz_qemu_io_read(uint64_t phys_addr, uint64_t *data, uint32_t len) {
    uint64_t tmp;
    MemoryRegion *mr = kfuzz_thread_local_io_mr;
    if (mr && mr->addr <= phys_addr &&
        phys_addr < mr->addr + mr->size) {
        // todo: big endian
        tmp = mr->ops->read(mr->opaque, phys_addr, len);
        memcpy(data, &tmp, len);
        return 0;
    }
    return kfuzz_qemu_io_read_slow(phys_addr, data, len);
}

static int kfuzz_qemu_io_write_slow(uint64_t phys_addr, uint64_t data, uint32_t len) {
    pthread_spin_lock(&io_available);
    switch (len) {
        case 1:
            internal_cpu_outb(phys_addr, data);
            break;
        case 2:
            internal_cpu_outw(phys_addr, data);
            break;
        case 4:
            internal_cpu_outl(phys_addr, data);
            break;
        default:
            abort();
    }
    kfuzz_thread_local_io_mr = kfuzz_thread_local_mr;
    pthread_spin_unlock(&io_available);
    return 0;
}

int kfuzz_qemu_io_write(uint64_t phys_addr, uint64_t data, uint32_t len) {
    if (kfuzz_thread_local_io_mr && kfuzz_thread_local_io_mr->addr <= phys_addr &&
        phys_addr < kfuzz_thread_local_io_mr->addr + kfuzz_thread_local_io_mr->size) {
        kfuzz_thread_local_io_mr->ops->write(kfuzz_thread_local_io_mr->opaque, phys_addr, data, len);
        return 0;
    }
    return kfuzz_qemu_io_write_slow(phys_addr, data, len);
}
