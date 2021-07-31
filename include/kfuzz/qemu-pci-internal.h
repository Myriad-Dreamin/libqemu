//
// Created by kamiyoru on 2021/7/31.
//

#ifndef LIBQEMU_QEMU_PCI_INTERNAL_H
#define LIBQEMU_QEMU_PCI_INTERNAL_H

#include "qemu/osdep.h"
#include "qemu-common.h"
#include "hw/pci/pci.h"

uint16_t kfuzz_qemu_pci_config_readw(PCIDevice *dev, uint8_t offset);
uint32_t kfuzz_qemu_pci_config_readl(PCIDevice *dev, uint8_t offset);
void kfuzz_qemu_pci_config_writel(PCIDevice *dev, uint8_t offset, uint32_t val);
void kfuzz_qemu_pci_config_writew(PCIDevice *dev, uint8_t offset, uint16_t val);

static int kfuzz_get_pci_devices(Object *child, gpointer opaque) {
    GPtrArray *pci_devices = (GPtrArray *)opaque;
    if (object_dynamic_cast(OBJECT(child), TYPE_PCI_DEVICE)) {
        g_ptr_array_remove_fast(pci_devices, PCI_DEVICE(child));
        g_ptr_array_add(pci_devices, PCI_DEVICE(child));
    }
    return 0;
}

#endif //LIBQEMU_QEMU_PCI_INTERNAL_H
