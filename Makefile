KDIR := ./denv/linux-6.15.8
obj-m := xstrike.o
xstrike-objs := src/xstrike.o src/regex.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f ./tests/test-dev

.PHONY: all clean
