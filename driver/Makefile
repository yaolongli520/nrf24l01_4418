obj-m+=nrf24l01.o
obj-m+=nrf24l01_dev.o
obj-m+=test_work.o

pwd:=`pwd`
kdir:=/root/linux-3.4.y

all:
	make -C $(kdir) M=$(pwd) modules
	cp -rf nrf24l01.ko /nfs/ 
clean:
	rm -rf *.o *.ko
