PWD  := $(shell pwd)

obj-m += etn.o
etn-objs := etn_main.o etn_dma.o etn_regs.o etn_ctrl_fops.o etn_data_fops.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

