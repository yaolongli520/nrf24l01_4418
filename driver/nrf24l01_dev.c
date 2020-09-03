#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/amba/pl022.h>
#include <linux/gpio.h>

#include <mach/gpio.h>
#include <asm/gpio.h>
#include <mach/platform.h> //这个文件很多硬件信息

#define DEVICE_NAME "nrf24l01"
#define SENSOR_SPI_BUS 2

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("LIYAOLONG");

struct spi_device *spi;//设备




#define CS_GPIO   	(PAD_GPIO_C +28)	
#define CE_GPIO		(PAD_GPIO_B +27)	
#define IRQ_GPIO	(PAD_GPIO_B +26)	


static void nrf_cs(u32 chipselect)
{
	gpio_direction_output(CS_GPIO,!!chipselect);

}

struct pl022_config_chip nrf_info = {
        .com_mode = DMA_TRANSFER, //DMA
        .iface = SSP_INTERFACE_MOTOROLA_SPI,
        .hierarchy = SSP_MASTER,
        .slave_tx_disable = 1,
        .rx_lev_trig = SSP_RX_4_OR_MORE_ELEM,
        .tx_lev_trig = SSP_TX_4_OR_MORE_EMPTY_LOC,
        .ctrl_len = SSP_BITS_8,
        .wait_state = SSP_MWIRE_WAIT_ZERO,
        .duplex = SSP_MICROWIRE_CHANNEL_FULL_DUPLEX,
        .cs_control = nrf_cs, //片选函数
        .clkdelay = SSP_FEEDBACK_CLK_DELAY_1T,
};

struct io_info{
	int  ce_gpio;
	int  irq_gpio;
}io_info = {
	.ce_gpio = CE_GPIO,
	.irq_gpio = IRQ_GPIO,
};


struct spi_board_info chip =
{
	.modalias     = DEVICE_NAME, //匹配
	.mode         = 0x00,//模式0
	.bus_num      = SENSOR_SPI_BUS,//所挂总线号 0=SPI0
	.chip_select  = 1,//片选号
	.max_speed_hz = 8000000,
	.controller_data = &nrf_info,
	.platform_data = &io_info,
};




static int nrf24l01_dev_init(void)
{
	struct spi_master *master;//主控器
	int status=-1;
	
	master = spi_busnum_to_master(SENSOR_SPI_BUS);
    if (!master)
    {
        status = -ENODEV;
        pr_emerg("no find spi bus %d\n",SENSOR_SPI_BUS);
		return status;
    }
	
	spi = spi_new_device(master, &chip);
    if (!spi)
    {
        status = -EBUSY;
		pr_emerg("new_device if fail \n");
       return status;
    }
	gpio_free(CS_GPIO);
	gpio_request(CS_GPIO,"CS_GPIO");

	return 0;
}

static void nrf24l01_dev_exit(void)
{
	
	spi_unregister_device(spi);

}

module_init(nrf24l01_dev_init);
module_exit(nrf24l01_dev_exit);


