//
// Created by Myriad-Dreamin on 2021/7/27.
//

#ifndef LIBQEMU_QEMU_H
#define LIBQEMU_QEMU_H

typedef uint64_t qemu_hw_addr_t;

int kfuzz_qemu_module_begin(int argc, char **argv, char **envp);

int kfuzz_qemu_module_init_bios(void);

int kfuzz_qemu_module_loop(void);

int kfuzz_qemu_module_cleanup(void);

int kfuzz_qemu_mmio_read(qemu_hw_addr_t phys_addr, uint64_t *data, uint32_t len);

int kfuzz_qemu_mmio_write(qemu_hw_addr_t phys_addr, uint64_t data, uint32_t len);

int kfuzz_qemu_io_read(qemu_hw_addr_t phys_addr, uint64_t *data, uint32_t len);

int kfuzz_qemu_io_write(qemu_hw_addr_t phys_addr, uint64_t data, uint32_t len);

#endif //LIBQEMU_QEMU_H
