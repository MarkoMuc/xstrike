#include "xstrike_ioctl.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/module.h>

#define DRIVER_NAME "xstrike"
#define CLASS_NAME "xstrike_class"

struct FileData {
  struct rgx_pattern pattern;
  char *data;
  size_t count;
  size_t size;
  uint64_t id;
};

static dev_t dev_num;
static struct class *dev_class;
static struct cdev dev_cdev;

#define START_SIZE 1024

static ssize_t gen_id(void) {
  static ssize_t id = 0;
  return id++;
}

static ssize_t xstrike_read(struct file *file, char __user *buf, size_t count,
                            loff_t *f_pos) {
  struct FileData *fdata = (struct FileData *)file->private_data;
  uint64_t moved = copy_to_user(buf, fdata->data, fdata->count);

  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not write to the user\n");
    return 0;
  }

  return count;
}

static ssize_t xstrike_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *f_pos) {
  struct FileData *fdata = (struct FileData *)file->private_data;
  const uint64_t moved = copy_from_user(fdata->data + fdata->count, buf, count);

  fdata->count += count;
  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not read from the user\n");
    return 0;
  }

  return count;
}

static int xstrike_open(struct inode *inode, struct file *file) {
  file->private_data = kmalloc(sizeof(struct FileData), GFP_KERNEL);

  struct FileData *fdata = file->private_data;
  fdata->id = gen_id();
  fdata->count = 0;
  fdata->size = START_SIZE;
  fdata->data = kmalloc(sizeof(char) * START_SIZE, GFP_KERNEL);

  return 0;
}

static int xstrike_release(struct inode *inode, struct file *file) {
  const size_t id = ((struct FileData *)(file->private_data))->id;

  kfree(((struct FileData *)(file->private_data))->data);
  kfree(file->private_data);

  printk(KERN_INFO "Closing input %lu", id);
  return 0;
}

static long xstrike_ioctl(struct file *file, unsigned int cmd,
                          unsigned long arg) {
  if (cmd != XSTRIKE_SET) {
    printk(KERN_WARNING "xstrike: Unknown ioctl command: 0x%x\n", cmd);
    return -EINVAL;
  }

  long ret = 0;
  struct rgx_pattern xstrike_arg = {0};
  struct FileData *fdata = file->private_data;

  const uint64_t moved =
      copy_from_user(&xstrike_arg, (struct rgx_pattern __user *)arg,
                     sizeof(struct rgx_pattern));
  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not read from the user during ioctl\n");
    return 0;
  }

  return ret;
}

static const struct file_operations xstrike_fops = {.owner = THIS_MODULE,
                                                    .read = xstrike_read,
                                                    .write = xstrike_write,
                                                    .open = xstrike_open,
                                                    .release = xstrike_release,
                                                    .unlocked_ioctl =
                                                        xstrike_ioctl};

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
