
#include "qemu/osdep.h"
#include "cpu.h"
#include "kfuzz/qemu.h"
#include "kfuzz/qemu-internal.h"

static __thread MemoryRegion *kfuzz_thread_local_mmio_mr = 0;

static void internal_cpu_write8(uint32_t addr, uint8_t value) {
    uint8_t data = value;
    address_space_write(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                        &data, 1);
}

static void internal_cpu_write16(uint32_t addr, uint16_t value) {
    uint16_t data = value;
    tswap16s(&data);
    address_space_write(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                        &data, 2);
}

static void internal_cpu_write32(uint32_t addr, uint32_t value) {
    uint32_t data = value;
    tswap32s(&data);
    address_space_write(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                        &data, 4);
}

static void internal_cpu_write64(uint32_t addr, uint64_t value) {
    uint64_t data = value;
    tswap64s(&data);
    address_space_write(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                        &data, 8);
}

static uint8_t internal_cpu_read8(uint32_t addr) {
    uint8_t data;
    address_space_read(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                       &data, 1);
    return data;
}

static uint16_t internal_cpu_read16(uint32_t addr) {
    uint16_t data;
    address_space_read(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                       &data, 2);
    return tswap16(data);
}

static uint32_t internal_cpu_read32(uint32_t addr) {
    uint32_t data;
    address_space_read(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                       &data, 4);
    return tswap32(data);
}

static uint64_t internal_cpu_read64(uint32_t addr) {
    uint64_t data;
    address_space_read(first_cpu->as, addr, MEMTXATTRS_UNSPECIFIED,
                       &data, 8);
    return tswap64(data);
}

static int kfuzz_qemu_mmio_read_slow(uint64_t phys_addr, uint64_t *data, uint32_t len) {
    pthread_spin_lock(&io_available);
// todo: big endian
    switch (len) {
        case 1:
            *(uint8_t *) data = internal_cpu_read8(phys_addr);
            break;
        case 2:
            *(uint16_t *) data = internal_cpu_read16(phys_addr);
            break;
        case 4:
            *(uint32_t *) data = internal_cpu_read32(phys_addr);
            break;
        case 8:
            *(uint64_t *) data = internal_cpu_read64(phys_addr);
            break;
        default:
            abort();
    }
    kfuzz_thread_local_mmio_mr = kfuzz_thread_local_mr;
    pthread_spin_unlock(&io_available);
    return 0;
}

int kfuzz_qemu_mmio_read(uint64_t phys_addr, uint64_t *data, uint32_t len) {
    uint64_t tmp;
    MemoryRegion *mr = kfuzz_thread_local_mmio_mr;
    if (mr && mr->addr <= phys_addr && phys_addr < mr->addr + mr->size) {
        // todo: big endian
        tmp = mr->ops->read(mr->opaque, phys_addr, len);
        memcpy(data, &tmp, len);
        return 0;
    }
    return kfuzz_qemu_mmio_read_slow(phys_addr, data, len);
}

static int kfuzz_qemu_mmio_write_slow(uint64_t phys_addr, uint64_t data, uint32_t len) {
    pthread_spin_lock(&io_available);
    switch (len) {
        case 1:
            internal_cpu_write8(phys_addr, data);
            break;
        case 2:
            internal_cpu_write16(phys_addr, data);
            break;
        case 4:
            internal_cpu_write32(phys_addr, data);
            break;
        case 8:
            internal_cpu_write64(phys_addr, data);
            break;
        default:
            abort();
    }
    kfuzz_thread_local_mmio_mr = kfuzz_thread_local_mr;
    pthread_spin_unlock(&io_available);
    return 0;
}

int kfuzz_qemu_mmio_write(uint64_t phys_addr, uint64_t data, uint32_t len) {
    MemoryRegion *mr = kfuzz_thread_local_mmio_mr;
    if (mr && mr->addr <= phys_addr && phys_addr < mr->addr + mr->size) {
        mr->ops->write(mr->opaque, phys_addr, data, len);
        return 0;
    }
    return kfuzz_qemu_mmio_write_slow(phys_addr, data, len);
}
