obj-m  := xstrike.o

clean:
	rm -f xstrike.ko xstrike.mod* xstrike.o modules.order Module.symvers .Module* .xstrike* .module* ..module*
