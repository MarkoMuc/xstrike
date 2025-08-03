#include "xstrike.h"

static size_t gen_id(void) {
  static size_t id = 0;
  return id++;
}

static ssize_t xstrike_read(struct file *file, char __user *buf, size_t count,
                            loff_t *f_pos) {
  struct FileData *fdata = (struct FileData *)file->private_data;
  if (fdata->processed == false) {
    fdata->processed = true;
  }

  const size_t bytes_to_copy = min(fdata->count - *f_pos, count);
  const u64 moved = copy_to_user(buf, fdata->data + *f_pos, bytes_to_copy);

  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not write to the user\n");
    return 0;
  }

  *f_pos += bytes_to_copy;
  return bytes_to_copy;
}

static ssize_t xstrike_write(struct file *file, const char __user *buf,
                             size_t count, loff_t *f_pos) {
  struct FileData *fdata = (struct FileData *)file->private_data;

  if (fdata->size - fdata->count < count) {
    u64 frac = count / fdata->size;
    fdata->size += (frac > 0 ? frac : 1) * fdata->size;
    krealloc_array(fdata->data, fdata->size, sizeof(char), GFP_KERNEL);
  }

  const u64 moved = copy_from_user(fdata->data + fdata->count, buf, count);

  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not read from the user\n");
    return 0;
  }

  fdata->count += count;
  *f_pos += count;

  return count;
}

static loff_t xstrike_llseek(struct file *file, loff_t offset, int whence) {
  struct FileData *fdata = (struct FileData *)file->private_data;
  loff_t new_pos;

  if (!fdata)
    return -EINVAL;

  switch (whence) {
  case SEEK_SET:
    new_pos = offset;
    break;
  case SEEK_CUR:
    new_pos = *(&file->f_pos) + offset;
    break;
  case SEEK_END:
    new_pos = fdata->count + offset;
    break;
  default:
    return -EINVAL;
  }

  if (new_pos < 0 || new_pos > fdata->size) {
    printk(KERN_WARNING "xstrike: llseek: Invalid seek position: %lld (data "
                        "count: %zu, allocated size: %zu)\n",
           new_pos, fdata->count, fdata->size);
    return -EINVAL;
  }

  *(&file->f_pos) = new_pos;
  return new_pos;
}

static int xstrike_open(struct inode *inode, struct file *file) {
  file->private_data = kmalloc(sizeof(struct FileData), GFP_KERNEL);

  struct FileData *fdata = file->private_data;
  fdata->id = gen_id();
  fdata->count = 0;
  fdata->processed = false;
  fdata->size = START_SIZE;
  fdata->data = kmalloc(sizeof(char) * START_SIZE, GFP_KERNEL);

  return 0;
}

static int xstrike_release(struct inode *inode, struct file *file) {
  const size_t id = ((struct FileData *)(file->private_data))->id;

  kfree(((struct FileData *)(file->private_data))->pattern.pattern);
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

  u64 ret = 0;
  struct rgx_pattern xstrike_arg = {0};
  struct FileData *fdata = file->private_data;

  const u64 moved =
      copy_from_user(&xstrike_arg, (struct rgx_pattern __user *)arg,
                     sizeof(struct rgx_pattern));
  if (moved != 0) {
    printk(KERN_INFO "xstrike: Could not read from the user during ioctl\n");
    return 0;
  }

  ret = xstrike_regex_builder(&xstrike_arg);

  fdata->pattern = xstrike_arg;
  return ret;
}

static const struct file_operations xstrike_fops = {.owner = THIS_MODULE,
                                                    .read = xstrike_read,
                                                    .write = xstrike_write,
                                                    .open = xstrike_open,
                                                    .release = xstrike_release,
                                                    .unlocked_ioctl =
                                                        xstrike_ioctl,
                                                    .llseek = xstrike_llseek};

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
