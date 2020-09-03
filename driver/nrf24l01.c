/*
作者: 一半冷一半热 
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/spi/spi.h> 
#include <linux/spi/spidev.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/wait.h> 
#include <linux/circ_buf.h> //环形缓冲
#include <linux/mutex.h>

#include <asm/uaccess.h>
#include <mach/platform.h> //这个文件很多硬件信息
#include "nrf24l01.h" 

#define DEVICE_NAME "nrf24l01"


//#define DEBUG  //调试

#ifdef DEBUG
#define dev_debug(fmt, args...) pr_emerg("nrf24l01:"fmt,##args)
#else
#define  dev_debug(format, ...)	
#endif

static size_t circ_buf_write(struct circ_buf *circ,u8* buf,size_t size);
static size_t circ_buf_read(struct circ_buf *circ,u8* buf,size_t size);

struct io_info{
	int  ce_gpio;
	int  irq_gpio;
};


struct nrf24l01_drv_date{
	int irq;
	struct spi_device *spi;
	struct io_info  *io;
	struct miscdevice dev;
	struct file_operations fops;
	struct work_struct  work;
	struct work_struct  transfer;
	u8   tx_addr[5];
	u8   rx_addr[5];
	u8   crc_mode;
	u8	 channel; //频道 0-125
	struct circ_buf tx_buf; //写缓冲
	struct circ_buf rx_buf; //读缓冲
	spinlock_t		circ_tx_lock; //发送缓存操作锁
	spinlock_t		circ_rx_lock; //接收缓存操作锁
	wait_queue_head_t wq;
	int t_flag; 
	wait_queue_t w_wq; 
	int w_flag;
	struct mutex mutex;
	
};

static struct nrf24l01_drv_date *drv_date;


u8 NRF24L01_Write_Buf(u8 reg, u8 *pBuf, u8 len)
{
	u8 buf[len+1];
	buf[0] = reg;
	memcpy(&buf[1],pBuf,len);
  	spi_write(drv_date->spi,buf,len + 1); //写入数据	
	
  	return 0;          //返回读到的状态值
}	


u8 NRF24L01_Read_Buf(u8 reg,u8 *pBuf,u8 len)
{
	u8 status = 0;	
	u8 w_buf[len+1];
	u8 r_buf[len+1];
	
	struct spi_message	m;
	struct spi_transfer	t = {
		.tx_buf		= w_buf,
		.rx_buf		= r_buf,
		.len		= len + 1,
	};
	w_buf[0] = reg;
	memset(&w_buf[1],0xff,len);
	memset(r_buf,0x00,len);
	
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	status = spi_sync(drv_date->spi, &m);
	memcpy(pBuf,&r_buf[1],len);
  	return status;      
}



u8 NRF24L01_Check(void)
{
	u8 buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	u8 i;
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}


//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
u8 NRF24L01_Write_Reg(u8 reg,u8 value)
{
	u8 status;	
	u8 buff[2] = {reg,value};
	status = spi_write(drv_date->spi,buff,2);
  	return(status);       			//返回状态值
}

u8 NRF24L01_Read_Reg(u8 reg)
{
	u8 reg_val =0x00;	    
	NRF24L01_Read_Buf(reg,&reg_val,1);
  	return reg_val;           
}

void NRF24L01_Print_Reg(void)
{
	u8 reg = 0x00;
	u8 val;
	u8 i = 0;
	for(i = 0;i < 0x1f ; i ++) {	
		val = NRF24L01_Read_Reg(reg+i);
		pr_emerg("reg:%02x -- val:%02x\n",reg+i,val);
	}
	pr_emerg("\n");
}



/* 重启 */
void NRF24L01_REBOOT(void)
{
	u8 config = NRF24L01_Read_Reg(CONFIG);
	config = config & 0xfd;
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,config);
	msleep(1);
	config = config | 0x2;
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,config);
	msleep(1);
}


void NRF24L01_TX_Mode(void)
{		
	u8 val;
	u8 sta = 0x40;
	unsigned long int cur;
	//static int one = 0;

	cur = jiffies;	
	while(sta & 0x40) {
		sta = NRF24L01_Read_Reg(STATUS) ; 
		NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta);//清除状态
	//	if(!one) pr_emerg("\n warning %s sta=%02x\n",__func__,sta);	
	//	one++;
		if(time_after_eq(jiffies,cur+msecs_to_jiffies(10)))
			break;
	}				
	gpio_set_value(drv_date->io->ce_gpio,0);
	
	NRF24L01_Write_Reg(FLUSH_TX,0xff); //防止TX残留
	NRF24L01_Write_Reg(FLUSH_RX,0xff); //防止RX残留
	
  	val = NRF24L01_Read_Reg(CONFIG);
	val &=0xfe; 
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, val);

	NRF24L01_Write_Reg(FLUSH_TX,0xff); 
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x00);
	gpio_set_value(drv_date->io->ce_gpio,1);	
	
	
}



void NRF24L01_RX_Mode(void)
{
	u8 val;
	
	gpio_set_value(drv_date->io->ce_gpio,0);	
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01); //调换
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);//降低丢包率
	//NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);

  	val = NRF24L01_Read_Reg(CONFIG);
	val |=0x01; 
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, val);
	
	NRF24L01_Write_Reg(NRF_WRITE_REG + STATUS, 0xff);   //清除所有中断标志位
	gpio_set_value(drv_date->io->ce_gpio,1);
	

}	

void _nrf24l01_set_rx_mode(void)
{
	NRF24L01_RX_Mode();
	
}

void _nrf24l01_xmit(u8 *buf,ssize_t len)
{
	int total = len;
	u8 *p = buf;
	int off,x_len;
	u8 a_buf[32]={0};	
	u8 sta;	
	NRF24L01_TX_Mode(); 
	
	sta = NRF24L01_Read_Reg(STATUS);
	if(sta&0x40) pr_emerg("%s sta=%02x\n",__func__,sta);
	
	off = 0;
	while(total)
	{
		x_len = (total>=TX_PLOAD_WIDTH)?TX_PLOAD_WIDTH:total;
		if(x_len < TX_PLOAD_WIDTH){ //不足32字节
			memcpy(a_buf,p + off,x_len);//把要发的数据拷贝到 a_buf
			p = a_buf;
			off = 0; 
		}
		
		drv_date->t_flag = 0; //先设置0 再开始写,即使进入睡眠前错误唤醒 flag=1 也不会进入死睡眠
		NRF24L01_Write_Buf(WR_TX_PLOAD,p+off,TX_PLOAD_WIDTH);//TX_PLOAD_WIDTH
		//如果 t_flag 已经变成1 下面等待会立马退出
		if(!wait_event_timeout(drv_date->wq,drv_date->t_flag,HZ/10))
		{
			
			sta = NRF24L01_Read_Reg(STATUS);   //读状态位
			pr_emerg("\n wait is timeout sta=%02x t_flag =%d \n",sta,drv_date->t_flag);
			NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta);//清除状态
			NRF24L01_Write_Reg(FLUSH_TX,0xff); //清除发送FIFO 
			if(drv_date->t_flag == 1) pr_emerg("is .........irq but\n");
			//NRF24L01_REBOOT(); //重启
			break; //退出
		}
		
		total -=x_len;
		off +=x_len;
	}
}				

u8 NRF24L01_RxPacket(u8 *rxbuf)
{
	u8 sta;	
	sta=NRF24L01_Read_Reg(STATUS);  
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); 
	
	if(sta&RX_OK) 
	{
		gpio_set_value(drv_date->io->ce_gpio,0);	
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);
		NRF24L01_Write_Reg(FLUSH_RX,0xff);
		gpio_set_value(drv_date->io->ce_gpio,1);	
		return 0; 
	}	   
	return 1;
}		


#if 0
unsigned long int t1 = 0;
int recv_handle(void)
{
	int residue;
	u8 buf[32];
	int unread,capacity;
	u8 ret;
	
	t1 = jiffies;
	while(1)
	{
		ret = NRF24L01_RxPacket(buf); //读数据
		if(!ret) //有数据
		{
			int i;
			for(i = 0;i< 32 ; i++ )
			if(buf[i] == 255 ) pr_emerg("buf[%d]=%c",i,buf[i]);
			
			residue = circ_buf_write(&drv_date->rx_buf,buf,32); //把接收数据放入接收缓冲区
			unread = nrf24l01_circ_chars_pending(&drv_date->rx_buf); //占用的空间
			capacity = nrf24l01_circ_chars_free(&drv_date->rx_buf); //剩余空间
			if(!residue) 
			{	
				dev_debug("is rx ok unread:%d capacity:%d\n",unread,capacity);
				//return 0;
			}
			else 
			{	
				pr_emerg("rx_buf was full\n");
				//return 1;
				break; //接受buff溢出
			}
			t1 = jiffies;
			
		}
		if(jiffies_to_msecs(jiffies-t1)>40) break;
		
	}
	return ret;
	
}
#endif

/*用于发送的传输*/
void transfer_work_func(struct work_struct *work)
{
	int len; //长度
	int residue; //剩余
	u8 *xmit;
	
	
	xmit = kzalloc(1024,GFP_KERNEL);
	
	#if 0
	unsigned long int cur = jiffies;
	while(drv_date->w_flag == 1 )
	{
		if(time_after_eq(jiffies,cur+msecs_to_jiffies(5)))
			break;
	}
	#endif
	
	while(1)
	{
		spin_lock(&drv_date->circ_tx_lock);
		residue = circ_buf_read(&drv_date->tx_buf,xmit,1024);
		spin_unlock(&drv_date->circ_tx_lock);
		len = 1024 - residue;
		
		if(!len) break; //缓冲区无数据需要发
		
		
		if(len< 512 )  //短消息不需要久等
		{
			_nrf24l01_xmit(xmit,len); //提交到发送
			break;
		}else
		{
			_nrf24l01_xmit(xmit,len); //提交到发送
			if(!nrf24l01_circ_chars_pending(&drv_date->tx_buf)) break;//没有剩下
			msleep(50); //进入睡眠 打开定时器
		}
	}
	if(drv_date->w_flag == 1 ) 
		pr_emerg("\n warning %s drv_date->w_flag =1 \n",__func__);
	drv_date->w_flag = 1;
	wake_up_interruptible(&drv_date->wq);
	kfree(xmit);
	
}

/*中断下半部 工作函数*/
void nrf24l01_work_func(struct work_struct *work)
{
	int ret;
	u8 sta;
	sta = NRF24L01_Read_Reg(STATUS);   //读状态位
	if(sta&RX_OK)  //接收成功
	{
		int unread,capacity;
		int residue;
		u8 buf[32];
		ret = NRF24L01_RxPacket(buf); //读取到buf
		NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta);//清除状态
		if(ret == 1) pr_emerg("%d is no read date\n",__LINE__);
		spin_lock(&drv_date->circ_rx_lock); //保证接收缓存区互斥访问
		residue = circ_buf_write(&drv_date->rx_buf,buf,32); //把接收数据放入接收缓冲区
		spin_unlock(&drv_date->circ_rx_lock);
		unread = nrf24l01_circ_chars_pending(&drv_date->rx_buf); //占用的空间
		capacity = nrf24l01_circ_chars_free(&drv_date->rx_buf); //剩余空间
		if(!residue) dev_debug("is rx ok unread:%d capacity:%d\n",unread,capacity);
		else pr_emerg("rx_buf was full\n");
	}
	else if(sta & MAX_TX)
	{
			NRF24L01_Write_Reg(FLUSH_TX,0xff); //清除发送FIFO 
			sta=NRF24L01_Read_Reg(STATUS);   //读状态位
			NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta);
			drv_date->t_flag = 1;
			wake_up(&drv_date->wq);
		//	pr_emerg("MAX_TX sta is %02x\n",sta);

	}else if(sta & TX_OK)
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff); //清除发送FIFO 
		sta=NRF24L01_Read_Reg(STATUS);   //读状态位
		NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta);
		drv_date->t_flag = 1;
		wake_up(&drv_date->wq);
		//pr_emerg("ok sta is %02x\n",sta);
	}else 
	{
		//pr_emerg("sta is %02x\n",sta);
		
	}

		
}

irqreturn_t  nrf24l01_irq_handler(int irq, void *date)
{
	schedule_work(&drv_date->work);
	return IRQ_HANDLED;
}


static long nrf24l01_ioctl(struct file *file , unsigned int cmd, unsigned long arg)
{
	u8 ret = 0;
	
	switch(cmd)
	{
		case SET_TX_ADDR:
			ret = copy_from_user(drv_date->tx_addr,(u8*)arg,TX_ADR_WIDTH);
			if(ret) return ret;
			NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,drv_date->tx_addr,TX_ADR_WIDTH);
			break;
		case SET_RX_ADDR:
			ret = copy_from_user(drv_date->rx_addr,(u8*)arg,RX_ADR_WIDTH);
			if(ret) return ret;
			NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,drv_date->rx_addr,RX_ADR_WIDTH);
			break;
		case SET_CHANNEL:
			ret = copy_from_user(&drv_date->channel,(u8*)arg,1);
			if(ret) return ret;
			NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,drv_date->channel);	
			break;
		case SET_CRC_MODE:
			ret = copy_from_user(&drv_date->crc_mode,(u8*)arg,1);
			if(ret) return ret;		
			if(drv_date->crc_mode) //16
			{
				u8 val = NRF24L01_Read_Reg(CONFIG);
				val |=(0x1<<2);
				NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,val);
			}
			else  //8
			{
				u8 val = NRF24L01_Read_Reg(CONFIG);
				val &=~(0x1<<2);
				NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,val);
			}
			break;
		case GET_TX_ADDR:
			ret = copy_to_user((u8*)arg,drv_date->tx_addr,TX_ADR_WIDTH);
			break;
		case GET_RX_ADDR:
			ret = copy_to_user((u8*)arg,drv_date->rx_addr,RX_ADR_WIDTH);
			break;
		case GET_CHANNEL:
			ret = copy_to_user((u8*)arg,&drv_date->channel,1);
			break;
		case GET_CRC_MODE:
			ret = copy_to_user((u8*)arg,&drv_date->crc_mode,1);
			break;
		case CHECK_DEVICE:
			ret = NRF24L01_Check();
			ret = copy_to_user((u8*)arg,&ret,1);
			break;
		case SWITCH_TO_RECV:
			NRF24L01_RX_Mode();
			break;
		case DEBUG_REG:
			NRF24L01_Print_Reg();
			break;
		default:
			pr_emerg("cmd no find\n");
	}
	
	return ret;
}




/*写数据到环形缓冲区 返回未写的长度*/
static size_t circ_buf_write(struct circ_buf *circ,u8* buf,size_t size)
{

	int ret;
	unsigned long int count;//可写总数
	unsigned long int str_len; //直线可写长度
	u8 *p; //起始写的位置
	#ifdef DEBUG
	int len;//调试
	#endif
	
	
	p = circ->buf + circ->head;//起始写的位置
	dev_debug("0 circ->head=%d\n",circ->head);
	//不能大于剩余容量
	if(size >nrf24l01_circ_chars_free(circ))//设置为剩余容量
		count = nrf24l01_circ_chars_free(circ); 
	else count = size; //全部可写
	
	ret = size - count; //用于返回未写字节数
	
	dev_debug("can use size is %lu\n",count);
	
	str_len = nrf24l01_circ_space_to_end(circ);//直线可用容量
	
	if(count <= str_len) //线性写
	{
		memcpy(p,buf,count);
		circ->head +=count;
		circ->head &= NRF24L01_XMIT_SIZE -1;
		count = 0;
	}
	else
	{
		memcpy(p,buf,str_len); //先写直线
		circ->head +=str_len;
		circ->head &= NRF24L01_XMIT_SIZE -1;
		count -= str_len;

		memcpy(circ->buf,buf+str_len,count);
		circ->head +=count;
		count =0;
	}

#ifdef DEBUG
	dev_debug("1 circ->head=%d\n",circ->head);
	len = nrf24l01_circ_chars_pending(circ);
	dev_debug("len count=%d\n",len); //有40个字节为读
	len = nrf24l01_circ_chars_free(circ);
	dev_debug("len free=%d\n",len);//剩余空间 4055字节
#endif
	return ret;
}



static ssize_t nrf24l01_write(struct file *file, const char __user *user, size_t size, loff_t *off)
{
	int ret;
	int offer = 0;
	u8 *buff;
	size_t residue = size; //要写的剩余总量
	size_t wrsize,nowirte; //写入的长度,未写入
	
	
	buff= kzalloc(size,GFP_KERNEL);
	
	mutex_lock(&drv_date->mutex); 
	ret = copy_from_user(buff,user,size);
	if(ret) 
	{	
		pr_emerg("copy_from_user is fail\n");
		return -EFAULT;
	}
	
	
	while(residue)
	{
		if(residue <= NRF24L01_XMIT_SIZE -1 )  wrsize = residue;
		else  wrsize = NRF24L01_XMIT_SIZE - TX_PLOAD_WIDTH;

		spin_lock(&drv_date->circ_tx_lock);
		nowirte = circ_buf_write(&drv_date->tx_buf,buff+offer,wrsize);
		spin_unlock(&drv_date->circ_tx_lock);
		offer = offer + wrsize - nowirte;
		residue = size - offer;
		
		drv_date->w_flag = 0; //先设置标志再开始发送,有时候不需进入,在进入等待状态前flag=1.
		schedule_work(&drv_date->transfer); //启动发送程序

		wait_event_interruptible(drv_date->wq,drv_date->w_flag);//等待一次传输完成
	}
	
	mutex_unlock(&drv_date->mutex); 
	kfree(buff);
	_nrf24l01_set_rx_mode(); //上面的 wait_event_interruptible 等必须否则可能未写完就切换接收模式
	return size;
}




/*环形缓冲区读数据到线性缓冲区 返回未读的长度*/
static size_t circ_buf_read(struct circ_buf *circ,u8* buf,size_t size)
{
	
	int ret;
	unsigned long int count;//可读总数
	unsigned long int str_len; //直线可读长度
	u8 *p; //起始读的位置
	#ifdef DEBUG
	int len;
	#endif
	
	p = circ->buf + circ->tail; //起始读的位置
	dev_debug("pre circ->tail=%d\n",circ->tail);
	
	if(size > nrf24l01_circ_chars_pending(circ))
		count = nrf24l01_circ_chars_pending(circ);//设置可读总容量
	else
		count = size;
	
	dev_debug("can read size is %lu\n",count);
	
	ret = size - count; //用于返回未读字节数
	
	str_len = nrf24l01_circ_circ_cnt_to_end(circ); //线性可读
	
	if(count <= str_len)
	{
		memcpy(buf,p,count);
		circ->tail +=count;
		circ->tail &= NRF24L01_XMIT_SIZE -1;
		count = 0;
	}
	else
	{
		memcpy(buf,p,str_len);
		circ->tail +=str_len;
		circ->tail &= NRF24L01_XMIT_SIZE -1;
		count -= str_len;

		memcpy(buf+str_len,circ->buf,count);
		circ->tail +=count;
		count =0;
	}
	
	
	#ifdef DEBUG
	dev_debug("post circ->tail=%d\n",circ->tail);
	len = nrf24l01_circ_chars_pending(circ);
	dev_debug("len count=%d\n",len); 
	len = nrf24l01_circ_chars_free(circ);
	dev_debug("len free=%d\n",len);//剩余空间 4055字节
	#endif
	
	return ret;
	
}

static ssize_t nrf24l01_read(struct file *file, char __user *user, size_t size, loff_t *off)
{
	int len;
	int ret;
	u8 *buf;
	int increase;
	 //缺少
	unsigned long int lack = (size>NRF24L01_XMIT_SIZE)?(NRF24L01_XMIT_SIZE - RX_PLOAD_WIDTH):size; //防止cat命令size过大
	buf = kzalloc(NRF24L01_XMIT_SIZE,GFP_KERNEL);
	if(!buf) pr_emerg("alloc mem is fail\n");
	
	mutex_lock(&drv_date->mutex);
	
	/*大于缓存区可读容量 正在接收时等待接收好再读*/
	while(lack > nrf24l01_circ_chars_pending(&drv_date->rx_buf)){ 
		increase = nrf24l01_circ_chars_pending(&drv_date->rx_buf);
		msleep(10);
		if(increase == nrf24l01_circ_chars_pending(&drv_date->rx_buf)) {
			break;
		}
	}
	
	len = lack;
	spin_lock(&drv_date->circ_rx_lock); //保证缓存区的互斥访问
	lack = circ_buf_read(&drv_date->rx_buf,buf,len); //读取接收缓冲区
	spin_unlock(&drv_date->circ_rx_lock);
	len = len - lack;
	if(len >0 )
		ret = copy_to_user(user,buf,len);
	mutex_unlock(&drv_date->mutex);
	
	
	kfree(buf);
	return len;
}


void NRF24L01_Init(void)
{
	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH); //接收FIFO 32BYTE	
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);   //使能通道0自动应答
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);//使能通道0接收数据
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_AW,0x03); //地址宽度5
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //设置传输速率
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //设置传输速率
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0xff);//设置自动重发的参数到最大	
	NRF24L01_Write_Reg(FLUSH_TX,0xff); //清除发送FIFO 
	NRF24L01_Write_Reg(FLUSH_RX,0xff); //清除接收FIFO 
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0f); 
	NRF24L01_RX_Mode(); //初始化接收模式
	
}

int nrf24l01_open(struct inode *inode, struct file *file)
{
	//pr_emerg("%s ...\n",__func__);
	NRF24L01_Init();
	return 0;
}


static int  nrf24l01_probe(struct spi_device *spi)
{
	struct io_info *io_info;
	int ret = 0;
	
	io_info = (struct io_info *)dev_get_platdata(&spi->dev);
		
	
	drv_date = kzalloc(sizeof(*drv_date),GFP_KERNEL);
	if(!drv_date) pr_emerg("alloc drv_date is fail\n");
	
	drv_date->spi = spi;  //设置SPI
	drv_date->io = io_info;
	
	if(!drv_date->io)
	{
		pr_emerg("alloc io fail\n");
		goto err_alloc_io;
	}
	
	gpio_free(io_info->ce_gpio);
	gpio_free(io_info->irq_gpio);
	
	gpio_request(io_info->ce_gpio,"ce_gpio");
	gpio_request(io_info->irq_gpio,"irq_gpio");
	
	gpio_direction_output(io_info->ce_gpio,0); 
	gpio_set_value(io_info->ce_gpio,0);
	gpio_direction_input(io_info->irq_gpio);
	drv_date->irq = gpio_to_irq(io_info->irq_gpio);
	
	ret = request_irq(drv_date->irq,nrf24l01_irq_handler,IRQF_TRIGGER_FALLING|IRQF_ONESHOT,
	    "nrf24l01_irq",drv_date);
		
	if(ret) 
	{	
		ret = -EAGAIN;
		pr_emerg("request_irq fail\n");
		goto err_request_irq;
	}
		
	drv_date->dev.minor = MISC_DYNAMIC_MINOR;
	drv_date->dev.name = DEVICE_NAME;
	drv_date->dev.fops = &drv_date->fops;	
	
	drv_date->fops.owner = THIS_MODULE;
	drv_date->fops.unlocked_ioctl  = nrf24l01_ioctl;
	drv_date->fops.write = nrf24l01_write;
	drv_date->fops.read = nrf24l01_read;
	drv_date->fops.open = nrf24l01_open;

	ret = misc_register(&drv_date->dev);
    if(ret)
	{
		ret =  -ENODEV;
		pr_emerg("misc_register fail\n");
		goto err_dev_register;
	}
	
	INIT_WORK(&drv_date->work,nrf24l01_work_func);
	INIT_WORK(&drv_date->transfer,transfer_work_func);
	init_waitqueue_head(&drv_date->wq);

	add_wait_queue(&drv_date->wq,&drv_date->w_wq);
	init_waitqueue_entry(&drv_date->w_wq,current);	//初始化写等待队列
	mutex_init(&drv_date->mutex);
	
	
	drv_date->tx_buf.buf =  kzalloc(NRF24L01_XMIT_SIZE,GFP_KERNEL);
	drv_date->rx_buf.buf =  kzalloc(NRF24L01_XMIT_SIZE,GFP_KERNEL);
	
	//环形缓存锁
	spin_lock_init(&drv_date->circ_tx_lock); 
	spin_lock_init(&drv_date->circ_rx_lock);
	
	nrf24l01_circ_clear(&drv_date->tx_buf);
	nrf24l01_circ_clear(&drv_date->rx_buf);
	
	//NRF24L01_Init();

	return 0;
	
err_dev_register:
		free_irq(drv_date->irq,drv_date);
		
err_request_irq:
		
	
err_alloc_io:
		kfree(drv_date);
	return  ret;	
	
	
}

static int  nrf24l01_remove(struct spi_device *spi)
{
	remove_wait_queue(&drv_date->wq,&drv_date->w_wq);
	kfree(drv_date->tx_buf.buf);
	kfree(drv_date->rx_buf.buf);
	flush_work(&drv_date->transfer);
	flush_work(&drv_date->work);
	misc_deregister(&drv_date->dev);
	free_irq(drv_date->irq,drv_date);
	kfree(drv_date);
	return 0;
	
}




static struct spi_driver nrf24l01_spi_driver = {
	.probe =  nrf24l01_probe,
	.remove = nrf24l01_remove,
	.driver = {
		.name  = DEVICE_NAME,
		.owner = THIS_MODULE,
	},
};


static int __init nrf24l01_init(void)
{
	spi_register_driver(&nrf24l01_spi_driver);//驱动注册
	return 0;
}

static void __exit nrf24l01_exit(void)
{
	spi_unregister_driver(&nrf24l01_spi_driver);
}





module_exit(nrf24l01_exit);
module_init(nrf24l01_init);


MODULE_AUTHOR("topeet: rty");
MODULE_LICENSE("GPL");



