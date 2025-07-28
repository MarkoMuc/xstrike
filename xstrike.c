#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

#define DRIVER_NAME "xstrike"
#define CLASS_NAME "xstrike_class"

static dev_t dev_num;
static struct class *dev_class;
static struct cdev dev_cdev;

static char *read_data = "Hello World from da kernel drivah";
static char write_data[1024];
static ssize_t xstrike_read(struct file *file, char __user *buf, size_t count,
                            loff_t *f_pos) {
  uint64_t moved = copy_to_user(buf, read_data, strlen(read_data));
  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not write to the user\n");
    return 0;
  }

  return count;
}

static ssize_t xstrike_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *f_pos) {

  uint64_t moved = copy_from_user(&write_data, buf, count);
  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not read from the user\n");
    return 0;
  }
  printk(KERN_INFO "xstrike: %s", write_data);
  return count;
}

static int xstrike_open(struct inode *inode, struct file *file) { return 0; }

static int xstrike_release(struct inode *inode, struct file *file) {
  printk(KERN_INFO "xstrike: Device closed.\n");
  return 0;
}

static const struct file_operations xstrike_fops = {
    .owner = THIS_MODULE,
    .read = xstrike_read,
    .write = xstrike_write,
    .open = xstrike_open,
    .release = xstrike_release,
};

static int __init xstrike_init(void) {
  int ret = alloc_chrdev_region(&dev_num, 0, 1, DRIVER_NAME);
  if (ret < 0) {
    printk(KERN_ALERT
           "xstrike: Failed to allocate character device region: %d\n",
           ret);
    return ret;
  }
  printk(KERN_INFO "xstrike: Allocated major %d, minor %d\n", MAJOR(dev_num),
         MINOR(dev_num));

  dev_class = class_create(CLASS_NAME);
  if (IS_ERR(dev_class)) {
    ret = PTR_ERR(dev_class);
    printk(KERN_ALERT "xstrike: Failed to create device class: %d\n", ret);
    goto lregion;
  }
  printk(KERN_INFO "xstrike: Device class '%s' created.\n", CLASS_NAME);

  cdev_init(&dev_cdev, &xstrike_fops);
  dev_cdev.owner = THIS_MODULE;

  ret = cdev_add(&dev_cdev, dev_num, 1);
  if (ret < 0) {
    printk(KERN_ALERT "xstrike: Failed to add cdev: %d\n", ret);
    goto lclass;
  }
  printk(KERN_INFO "xstrike: Cdev added.\n");

  if (IS_ERR(device_create(dev_class, NULL, dev_num, NULL, DRIVER_NAME))) {
    ret = PTR_ERR(device_create(dev_class, NULL, dev_num, NULL, DRIVER_NAME));
    printk(KERN_ALERT "xstrike: Failed to create device in /dev/: %d\n", ret);
    goto ldevice;
    return ret;
  }
  printk(KERN_INFO "xstrike: Device /dev/%s created.\n", DRIVER_NAME);

  printk(KERN_INFO "xstrike: Module loaded successfully.\n");
  return 0;

ldevice:
  device_destroy(dev_class, dev_num);
  cdev_del(&dev_cdev);
lclass:
  class_destroy(dev_class);
lregion:
  unregister_chrdev_region(dev_num, 1);

  return ret;
}

static void __exit xstrike_exit(void) {
  device_destroy(dev_class, dev_num);
  cdev_del(&dev_cdev);
  class_destroy(dev_class);
  unregister_chrdev_region(dev_num, 1);

  printk(KERN_INFO "xstrike: Module unloaded.\n");
}

module_init(xstrike_init);
module_exit(xstrike_exit);

MODULE_LICENSE("GPL");
