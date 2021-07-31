
#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "kfuzz/qemu.h"

void kfuzz_pci_irq_event(void *opaque, int irq_num, int level);
void kfuzz_pci_irq_event(void *opaque, int irq_num, int level) {
    PCIDevice *dev = opaque;
    struct kernel_pci_dev *drv = dev->kernel_pci_driver;
    if (drv && drv->emit_irq) {
        drv->emit_irq(drv, dev, irq_num, level);
    }
}