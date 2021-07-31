//
// Created by Myriad-Dreamin on 2021/7/27.
//

#ifndef LIBQEMU_QEMU_H
#define LIBQEMU_QEMU_H

typedef uint64_t qemu_hw_addr_t;

struct kernel_pci_dev {
    uint32_t devfn;        /* encoded device & function index */
    uint16_t vendor;
    uint16_t device;
    void *ptr;

    void (*emit_irq)(struct kernel_pci_dev *that, void *qemu_device, int irq_num, int level);

    void *qemu_ptr;
};

int kfuzz_qemu_module_begin(int argc, char **argv, char **envp);

int kfuzz_qemu_module_init_bios(void);

int kfuzz_qemu_module_loop(void);

int kfuzz_qemu_module_cleanup(void);

int kfuzz_qemu_connect_device(struct kernel_pci_dev *pci_dev);

int kfuzz_qemu_mmio_read(qemu_hw_addr_t phys_addr, uint64_t *data, uint32_t len);

int kfuzz_qemu_mmio_write(qemu_hw_addr_t phys_addr, uint64_t data, uint32_t len);

int kfuzz_qemu_io_read(qemu_hw_addr_t phys_addr, uint64_t *data, uint32_t len);

int kfuzz_qemu_io_write(qemu_hw_addr_t phys_addr, uint64_t data, uint32_t len);


uint64_t kfuzz_qemu_map_page(struct kernel_pci_dev *dev, void *vaddr,
                             unsigned long size);

void kfuzz_qemu_dma_unmap(struct kernel_pci_dev *dev,
                          unsigned long long dma_handle, unsigned long size);

void kfuzz_qemu_clean_unmapped_pages(void);

#endif //LIBQEMU_QEMU_H
