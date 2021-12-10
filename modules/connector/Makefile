obj-m += connector.o

ifneq ($(MOUDLE_INCLUDES),)
		CFLAGS_connection.o = $(MODULE_INCLUDES)
endif

all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KHEAD) M=$(PWD) modules

clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KHEAD) M=$(PWD) clean
