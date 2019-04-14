/********************************************************************************************************
* 文 件 名：COMMUNICATE.H
* 功    能：RFID阅读器TRF7960与C8051F340微控制器之间通信方式头文件。
* 硬件连接：C8051F340与TRF7960之间通信硬件连接关系如下所示：
*                C8051F340                 TRF7960
*********************    PARALLEL INTERFACE    ******************************************         
*       P0.7   				 IRQ
*			  P0.3                 Slave_select
*       P0.2                 SIMO
*       P0.1                 SOMI
*       P0.0                 DATA_CLK
*				P4.0		       	 MOD
*				P4.2				 ASK/OOK
*				P4.3				 EN
*
* 版    本：V1.0
* 作    者：EMDOOR
* 日    期：2011-9-29
*********************************************************************************************************/
#include "hardware.h"
#include "communicate.h"
#include <c8051f340.h>


void IRQInit(void)
{
TCON |= 0X01;//边沿触发方式
IT01CF |=0X0F;  //将中断0配置到P0.7,高电平有效

}
/******************************************************************************************************************
* 函数名称：delay_ms()
* 功    能：延时函数。
* 入口参数：n_ms        毫秒值
* 出口参数：无
* 说    明：该函数为延时函数，入口参数值越大，CPU延时等待的时间就越长。
*           本演示程序，将其应用于非精确的延时等待场合。
*           注意：入口参数的最大值为 65536。           
*******************************************************************************************************************/
void delay_ms(unsigned int n_ms)
{
    unsigned int ii1, ii0;
    
    for(ii0=n_ms; ii0>0; ii0--) 
    {
        ii1 = 10000;                               //单循环延时参数。为估计值，不能精确延时。
        do (ii1--);
        while (ii1 != 0);
    }
	
}


/******************************************************************************************************************
* 函数名称：void Timer0_Delay (int ms)
* 功    能：定时器设置函数。
* 入口参数：无
* 出口参数：无
* 说    明：初始化设置定时器寄存器。 本演示程序，将其应用于精确的延时等待场合。          
*******************************************************************************************************************/
void Timer0_Delay (int ms)

{
TF0 = 0; // 停止定时器T0并清除溢出标志
TMOD &= ~0x0f; // 配置定时器T0为16位模式
TMOD |= 0x01;
CKCON &=~ 0x03; // 定时器T0计数系统时钟
ET0 = 1;
TR0 = 1; // 停定时器T0
TH0 = (65535-1000*ms) >> 8; // 设置定时器T0 1ms溢出
TL0 = (65535-1000*ms)/256;

}

/******************************************************************************************************************
* 函数名称：OSCsel()
* 功    能：晶体振荡器选择
* 入口参数：mode        选择内部或者外部模式
* 出口参数：无
*******************************************************************************************************************/
void OSCsel(void)
{
   int i = 0;
   OSCICN    = 0x83;
   CLKMUL    = 0x80;
   for (i = 0; i < 20; i++);    // 延时等待晶体稳定
   CLKMUL    |= 0xC0;
   while ((CLKMUL & 0x20) == 0);
   CLKSEL    = 0x00;
}

/******************************************************************************************************************
* 函数名称：Timer0handler()
* 功    能：定时器中断服务程序
* 入口参数：无
* 出口参数：无
* 说    明：定时器中断服务程序，用于精确延时用。
*******************************************************************************************************************/
void Timer0handler(void) interrupt 1
{   
    unsigned char Register[2];
    StopCounter();
    
    /* 读取IRQ中断状态寄存器 */
    /*====================================================================================================*/
    Register[0] = IRQStatus;                        //IRQ中断状态寄存器地址
    Register[1] = IRQMask;                          //虚拟读(Dummy read)    
    ReadCont(Register, 2);
    /*====================================================================================================*/
	
    *Register = *Register & 0xF7;                   //设置奇偶校验位B3为0，过滤其余位
    if(*Register == 0x00 || *Register == 0x80)      //判断是否时间到
        i_reg = 0x00;
    else                                            
        i_reg = 0x01;   
		
    PCON &= ~0X02;                //进入低功耗模式，当中断响应时被唤醒，从而退出该模式
}

