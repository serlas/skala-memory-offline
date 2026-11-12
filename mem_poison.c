#include <linux/module.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>

#define POISON_MB 512
#define MF_SOFT_OFFLINE 0x100

static int __init mem_poison_init(void) {
    unsigned long size = POISON_MB * 1024UL * 1024UL;
    unsigned long num_pages = size / PAGE_SIZE;
    void *mem;
    unsigned long i;

    printk(KERN_INFO "mem_poison: Starting to poison %lu MB (%lu pages)\n", POISON_MB, num_pages);

    mem = vmalloc(size);
    if (!mem) {
        printk(KERN_ERR "mem_poison: Failed to allocate %lu MB\n", POISON_MB);
        return -ENOMEM;
    }

    for (i = 0; i < size; i += PAGE_SIZE) {
        unsigned long pfn = vmalloc_to_pfn(mem + i);
        int ret;

        if (!pfn_valid(pfn)) {
            printk(KERN_ERR "mem_poison: Invalid PFN %lu\n", pfn);
            continue;
        }

        printk(KERN_INFO "mem_poison: Poisoning PFN %lu\n", pfn);
        ret = memory_failure(pfn, MF_SOFT_OFFLINE);
        if (ret) {
            printk(KERN_ERR "mem_poison: Failed to poison PFN %lu (ret=%d)\n", pfn, ret);
        } else {
            printk(KERN_INFO "mem_poison: PFN %lu poisoned successfully\n", pfn);
        }
    }

    vfree(mem);
    printk(KERN_INFO "mem_poison: Poisoning completed. Check MemAvailable in /proc/meminfo\n");
    return 0;
}

static void __exit mem_poison_exit(void) {
    printk(KERN_INFO "mem_poison: Module unloaded (poisoned pages remain offlined)\n");
}

module_init(mem_poison_init);
module_exit(mem_poison_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel module to poison memory pages via HWPOISON");