

#include "qemu/osdep.h"
#include "qom/object.h"
#include "kfuzz/qemu.h"
#include "kfuzz/qemu-pci-internal.h"

static void pci_device_iomap(PCIDevice *dev, int barno) {
    static unsigned int pio_alloc_ptr = 0xc000;
    static const unsigned int pio_limit = 0x10000;
    static unsigned int mmio_alloc_ptr = 0xE0000000;
    static const unsigned long mmio_limit = 0x100000000ULL;

    static const int bar_reg_map[] = {
            PCI_BASE_ADDRESS_0, PCI_BASE_ADDRESS_1, PCI_BASE_ADDRESS_2,
            PCI_BASE_ADDRESS_3, PCI_BASE_ADDRESS_4, PCI_BASE_ADDRESS_5,
    };
    int bar_reg;
    uint32_t addr, size;
    uint32_t io_type;
    uint64_t loc;

    g_assert(barno >= 0 && barno <= 5);
    bar_reg = bar_reg_map[barno];

    kfuzz_qemu_pci_config_writel(dev, bar_reg, 0xFFFFFFFF);
    addr = kfuzz_qemu_pci_config_readl(dev, bar_reg);

    io_type = addr & PCI_BASE_ADDRESS_SPACE;
    if (io_type == PCI_BASE_ADDRESS_SPACE_IO) {
        addr &= PCI_BASE_ADDRESS_IO_MASK;
    } else {
        addr &= PCI_BASE_ADDRESS_MEM_MASK;
    }

    g_assert(addr); /* Must have *some* size bits */

    size = 1U << ctz32(addr);

    if (io_type == PCI_BASE_ADDRESS_SPACE_IO) {
        loc = QEMU_ALIGN_UP(pio_alloc_ptr, size);

        g_assert(loc >= pio_alloc_ptr);
        g_assert(loc + size <= pio_limit);

        pio_alloc_ptr = loc + size;

        kfuzz_qemu_pci_config_writel(dev, bar_reg, loc | PCI_BASE_ADDRESS_SPACE_IO);
    } else {
        loc = QEMU_ALIGN_UP(mmio_alloc_ptr, size);

        /* Check for space */
        g_assert(loc >= mmio_alloc_ptr);
        g_assert(loc + size <= mmio_limit);

        mmio_alloc_ptr = loc + size;

        kfuzz_qemu_pci_config_writel(dev, bar_reg, loc);
    }
}

static void pci_device_enable(PCIDevice *dev) {
    uint16_t cmd;

    /* FIXME -- does this need to be a bus callout? */
    cmd = kfuzz_qemu_pci_config_readw(dev, PCI_COMMAND);
    cmd |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
    kfuzz_qemu_pci_config_writew(dev, PCI_COMMAND, cmd);

    /* Verify the bits are now set. */
    cmd = kfuzz_qemu_pci_config_readw(dev, PCI_COMMAND);
    g_assert_cmphex(cmd & PCI_COMMAND_IO, ==, PCI_COMMAND_IO);
    g_assert_cmphex(cmd & PCI_COMMAND_MEMORY, ==, PCI_COMMAND_MEMORY);
    g_assert_cmphex(cmd & PCI_COMMAND_MASTER, ==, PCI_COMMAND_MASTER);
}

static void pci_bios_configure(gpointer pcidev, gpointer opaque) {
    PCIDevice *dev = pcidev;
    int i;

    for (i = 0; i < 6; i++) {
        if (dev->io_regions[i].size) {
            pci_device_iomap(dev, i);
        }
    }
    pci_device_enable(dev);
}

int kfuzz_qemu_module_init_bios(void) {
    GPtrArray *pci_devices = g_ptr_array_new();
    object_child_foreach_recursive(qdev_get_machine(), kfuzz_get_pci_devices, pci_devices);
    g_ptr_array_foreach(pci_devices, pci_bios_configure, NULL);
    g_ptr_array_free(pci_devices, true);
    return 0;
}
