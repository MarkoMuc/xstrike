#ifndef __XSTRIKE_HEADER
#define __XSTRIKE_HEADER

#include "regex.h"
#include "xstrike_ioctl.h"
#include "xstrike_types.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/math.h>
#include <linux/module.h>

#define DRIVER_NAME "xstrike"
#define CLASS_NAME "xstrike_class"

#define START_SIZE 1024

static dev_t dev_num;
static struct class *dev_class;
static struct cdev dev_cdev;

#endif
