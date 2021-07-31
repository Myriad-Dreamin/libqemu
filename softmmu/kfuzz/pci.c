
#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "kfuzz/qemu.h"
#include "kfuzz/qemu-pci-internal.h"

uint16_t kfuzz_qemu_pci_config_readw(PCIDevice *dev, uint8_t offset) {
    kfuzz_qemu_io_write(0xcf8, (1U << 31) | (dev->devfn << 8) | offset, 4);
    uint16_t data;
    kfuzz_qemu_io_read(0xcfc, (uint64_t *) &data, 2);
    return data;
}

uint32_t kfuzz_qemu_pci_config_readl(PCIDevice *dev, uint8_t offset) {
    kfuzz_qemu_io_write(0xcf8, (1U << 31) | (dev->devfn << 8) | offset, 4);
    uint32_t data;
    kfuzz_qemu_io_read(0xcfc, (uint64_t *) &data, 4);
    return data;
}

void kfuzz_qemu_pci_config_writel(PCIDevice *dev, uint8_t offset, uint32_t val) {
    kfuzz_qemu_io_write(0xcf8, (1U << 31) | (dev->devfn << 8) | offset, 4);
    kfuzz_qemu_io_write(0xcfc, val, 4);
}

void kfuzz_qemu_pci_config_writew(PCIDevice *dev, uint8_t offset, uint16_t val) {
    kfuzz_qemu_io_write(0xcf8, (1U << 31) | (dev->devfn << 8) | offset, 4);
    kfuzz_qemu_io_write(0xcfc, val, 2);
}

static void kernel_device_match(gpointer pcidev, gpointer matching_data) {
    PCIDevice *dev = pcidev;
    struct kernel_pci_dev *kernel_dev = matching_data;
    if (dev->devfn == kernel_dev->devfn) {
//        qemu_log("kfuzz notify: connecting kernel pci device: %p -> %p, dev_fn %d, vendor %d, device %d",
//                 dev, kernel_dev->ptr, kernel_dev->devfn, (int) (kernel_dev->vendor), (int) (kernel_dev->device));
        dev->kernel_pci_driver = kernel_dev;
        kernel_dev->qemu_ptr = dev;
    }
}

int kfuzz_qemu_connect_device(struct kernel_pci_dev *pci_dev) {
    GPtrArray *pci_devices = g_ptr_array_new();
    object_child_foreach_recursive(qdev_get_machine(), kfuzz_get_pci_devices, pci_devices);
    g_ptr_array_foreach(pci_devices, kernel_device_match, pci_dev);
    g_ptr_array_free(pci_devices, true);
    return 0;
}
