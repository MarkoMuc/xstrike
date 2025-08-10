KDIR := ./denv/linux-6.15.8
obj-m := xstrike.o
xstrike-objs := src/xstrike.o src/regex.o src/regex_types.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rustfmt tests/test.rs
	rm -f xstrike.mod .xstrike.mod.cmd xstrike.mod.c xstrike.mod.o .xstrike.mod.o.cmd .module-common.o ..module-common.o.cmd xstrike.o .xstrike.o.cmd .modules.order.cmd modules.order .Module.symvers.cmd Module.symvers .xstrike.ko.cmd xstrike.ko
	cd src
	rm -f srcxstrike.mod .xstrike.mod.cmd xstrike.mod.c xstrike.mod.o .xstrike.mod.o.cmd .module-common.o ..module-common.o.cmd xstrike.o .xstrike.o.cmd .modules.order.cmd modules.order .Module.symvers.cmd Module.symvers .xstrike.ko.cmd xstrike.ko

.PHONY: all clean
