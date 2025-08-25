KDIR := ./denv/linux-6.15.8
obj-m := xstrike.o
xstrike-objs := src/xstrike.o src/regex.o src/regex_types.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rustfmt tests/test.rs
	rm -f xstrike.mod .xstrike.mod.cmd xstrike.mod.c xstrike.mod.o .xstrike.mod.o.cmd .module-common.o ..module-common.o.cmd xstrike.o .xstrike.o.cmd .modules.order.cmd modules.order .Module.symvers.cmd Module.symvers .xstrike.ko.cmd xstrike.ko
	rm -f src/xstrike.mod src/.xstrike.mod.cmd src/xstrike.mod.c src/xstrike.mod.o src/.xstrike.mod.o.cmd src/.module-common.o src/..module-common.o.cmd src/xstrike.o src/.xstrike.o.cmd src/.modules.order.cmd src/modules.order src/.Module.symvers.cmd src/Module.symvers src/.xstrike.ko.cmd src/xstrike.ko src/xstrike.o src/regex.o src/regex_types.o

.PHONY: all clean
