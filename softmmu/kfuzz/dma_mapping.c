
#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "exec/ramblock.h"
#include "kfuzz/qemu.h"

static void remove_ram_memory_region(gpointer data, gpointer _) {
    MemoryRegion *mr = (MemoryRegion *) data;
    qemu_mutex_lock_iothread();
    memory_region_del_subregion(mr->container, mr);
    qemu_mutex_unlock_iothread();
    g_free(mr);
}

static GList *kfuzz_dma_unmapped_pages = NULL;
GMutex kfuzz_dma_pages_mutex;

void kfuzz_qemu_clean_unmapped_pages(void) {
    g_list_foreach(kfuzz_dma_unmapped_pages, remove_ram_memory_region, NULL);
    g_list_free(kfuzz_dma_unmapped_pages);
}

static const hwaddr qemu_pci_dma_start = 0x30000000;
static hwaddr qemu_pci_dma_start_alloc = qemu_pci_dma_start;

static dma_addr_t kfuzz_qemu_map_page_slow(struct kernel_pci_dev *dev, void *vaddr,
                                           unsigned long size, MemoryRegion **map_reg_ref) {
    hwaddr alloc = __sync_fetch_and_add(&qemu_pci_dma_start_alloc, size);
    PCIDevice *host_dev = (PCIDevice *) dev->qemu_ptr;
    AddressSpace *as = pci_get_address_space(host_dev);
    MemoryRegion *map_reg = g_malloc(sizeof(MemoryRegion));
    memory_region_init_ram_ptr(map_reg, OBJECT(host_dev), "pci_mapping", size, vaddr);
    // todo: vmstate aware?
    qemu_mutex_lock_iothread();
    memory_region_add_subregion_overlap(as->root, alloc, map_reg, 10);
    qemu_mutex_unlock_iothread();
    if (map_reg_ref) {
        *map_reg_ref = map_reg;
    }
    return alloc;
}

dma_addr_t kfuzz_qemu_map_page(struct kernel_pci_dev *dev, void *vaddr,
                               unsigned long size) {
    MemoryRegion *ref;
    static GList *head;
    dma_addr_t res;
    if (size > 4096) {
        return kfuzz_qemu_map_page_slow(dev, vaddr, size, NULL);
    }
    g_mutex_lock(&kfuzz_dma_pages_mutex);
    if (g_list_length(kfuzz_dma_unmapped_pages) == 0) {
        g_mutex_unlock(&kfuzz_dma_pages_mutex);
        return kfuzz_qemu_map_page_slow(dev, vaddr, 4096, NULL);
    }
    head = g_list_first(kfuzz_dma_unmapped_pages);
    kfuzz_dma_unmapped_pages = g_list_remove_link(kfuzz_dma_unmapped_pages, head);
    g_mutex_unlock(&kfuzz_dma_pages_mutex);
    ref = head->data;
    g_list_free(head);
    res = ref->addr;
    ref->ram_block->host = vaddr;
    return res;
}

static inline MemoryRegion *get_pci_region_by_dma_handle(struct kernel_pci_dev *dev,
                                                         unsigned long long dma_handle) {
    PCIDevice *host_dev = (PCIDevice *) dev->qemu_ptr;
    AddressSpace *as = pci_get_address_space(host_dev);
    hwaddr xlate;
    hwaddr sz;
    return flatview_translate(address_space_to_flatview(as), dma_handle, &xlate, &sz, false, MEMTXATTRS_UNSPECIFIED);
}

static void kfuzz_qemu_dma_unmap_slow(struct kernel_pci_dev *dev,
                                      unsigned long long dma_handle, unsigned long size) {
    MemoryRegion *mr = get_pci_region_by_dma_handle(dev, dma_handle);
    qemu_mutex_lock_iothread();
    memory_region_del_subregion(mr->container, mr);
    qemu_mutex_unlock_iothread();
    g_free(mr);
}

void kfuzz_qemu_dma_unmap(struct kernel_pci_dev *dev,
                          unsigned long long dma_handle, unsigned long size) {
    MemoryRegion *ref;
    if (size > 4096) {
        kfuzz_qemu_dma_unmap_slow(dev, dma_handle, size);
        return;
    }
    ref = get_pci_region_by_dma_handle(dev, dma_handle);
    ref->ram_block->host = 0;
    g_mutex_lock(&kfuzz_dma_pages_mutex);
    kfuzz_dma_unmapped_pages = g_list_append(kfuzz_dma_unmapped_pages, ref);
    g_mutex_unlock(&kfuzz_dma_pages_mutex);
}
