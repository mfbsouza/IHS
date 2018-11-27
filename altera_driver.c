/* Altera DE2I-150 FPGA Driver for Linux made by Matheus Souza (github.com/mfbsouza)
   for the MusicBox Project, A Music Instrument Simulator using Arduino Uno and FPGA */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("FPGA Driver for MusicBox Project");
MODULE_AUTHOR("Matheus Souza");

#define DISPLAY_L   1
#define DISPLAY_R   2
#define SWITCHES    3
#define PUSHBOTTOM  4
#define GREENLEDS   5
#define REDLEDS     6

/* General Purpose Variables for Driver */
static uint32_t data;
static int access_count = 0;
// pointers to FPGA IO hardware
static void   *display_r, *switches_, *p_bottons;
static void   *display_l, *green_leds, *red_leds;

/* --- CHAR DEVICE INFORMATION & FUNCTIONS --- */
static int MAJOR_NUMBER = 91;
static int     dev_open     (struct inode *, struct file *);
static int     dev_release  (struct inode *, struct file *);
static ssize_t dev_read     (struct file * , char *      , size_t, loff_t *);
static ssize_t dev_write    (struct file * , const char *, size_t, loff_t *);

static struct file_operations file_opts = {
    .read    = dev_read,
    .open    = dev_open,
    .write   = dev_write,
    .release = dev_release
};

/* Char Driver Functions implementations */

static int dev_open(struct inode *inodep, struct file *filep) {
    access_count++;
    printk(KERN_ALERT "FPGA device opened %d time(s)\n", access_count);
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_ALERT "FPGA device closed.\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buf, size_t opt, loff_t *off) {
    data = 0;
    
    switch(opt) {
        case DISPLAY_L:
            data = ioread32(display_l);
            break;
        case DISPLAY_R:
            data = ioread32(display_r);
            break;
        case SWITCHES:
            data = ioread32(switches_);
            break;
        case PUSHBOTTOM:
            data = ioread32(p_bottons);
            break;
        case GREENLEDS:
            data = ioread32(green_leds);
            break;
        case REDLEDS:
            data = ioread32(red_leds);
            break;
        default:
            printk(KERN_ALERT "Invalid Option from Read().\n");
    }
    // send data to the user space
    raw_copy_to_user(buf, &data, sizeof(uint32_t));
    return 4;
}

static ssize_t dev_write(struct file *filep, const char *buf, size_t opt, loff_t *off) {
    data = 0;
    // get data from the user space to kernel space
    raw_copy_from_user(&data, buf, sizeof(uint32_t));

    switch(opt) {
        case DISPLAY_L:
            iowrite32(data, display_l);
            break;
        case DISPLAY_R:
            iowrite32(data, display_r);
            break;
        case GREENLEDS:
            iowrite32(data, green_leds);
            break;
        case REDLEDS:
            iowrite32(data, red_leds);
            break;
        default:
            printk(KERN_ALERT "Invalid Option from Write().\n");
    }
    return 4;
}

/* --- PCI INTERFACE INFORMATION & FUNCTIONS --- */
static int  pci_probe  (struct pci_dev *dev, const struct pci_device_id *id);
static void pci_remove (struct pci_dev *dev);

static struct pci_device_id pci_ids[] = {
    { PCI_DEVICE(0x1172, 0x0004), },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, pci_ids);

static struct pci_driver pci_driver = {
    .name     = "alterahello",
    .id_table = pci_ids,
    .probe    = pci_probe,
    .remove   = pci_remove,
};

static unsigned char pci_get_revision(struct pci_dev *dev) {
    u8 revision;

    pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
    return revision;
}

static int pci_probe(struct pci_dev *dev, const struct pci_device_id *id) {
    int vendor, retval;
    unsigned long resource;

    retval = pci_enable_device(dev);
  
    if (pci_get_revision(dev) != 0x01) {
        printk(KERN_ALERT "ERROR: cannot find PCI device\n");
        return -ENODEV;
    }

    pci_read_config_dword(dev, 0, &vendor);
    printk(KERN_ALERT "PCI Device Found. Vendor: %x\n", vendor);

    resource = pci_resource_start(dev, 0);
    printk(KERN_ALERT "PCI device resources start at bar 0: %lx\n", resource);

    display_r  = ioremap_nocache(resource + 0XC000, 0x20);
    display_l  = ioremap_nocache(resource + 0XC040, 0x20);
    switches_  = ioremap_nocache(resource + 0XC020, 0x20);
    p_bottons  = ioremap_nocache(resource + 0XC060, 0x20);
    green_leds = ioremap_nocache(resource + 0XC0C0, 0x20);
    red_leds   = ioremap_nocache(resource + 0XC0A0, 0x20);

    return 0;
}

static void pci_remove(struct pci_dev *dev) {
    iounmap(display_r);
    iounmap(display_l);
    iounmap(switches_);
    iounmap(p_bottons);
    iounmap(green_leds);
    iounmap(red_leds);
}

/* --- Driver Registration --- */
static int __init altera_driver_init(void) {
    int t = register_chrdev(MAJOR_NUMBER, "de2i150_altera", &file_opts);
    t = t | pci_register_driver(&pci_driver);

    if(t<0)
        printk(KERN_ALERT "ERROR: cannot register char or pci.\n");
    else
        printk(KERN_ALERT "PCI Device registred.\n");
    return t;
}

static void __exit altera_driver_exit(void) {
    printk(KERN_ALERT "Closing Driver...\n");
    unregister_chrdev(MAJOR_NUMBER, "de2i150_altera");
    pci_unregister_driver(&pci_driver);
}

module_init(altera_driver_init);
module_exit(altera_driver_exit);