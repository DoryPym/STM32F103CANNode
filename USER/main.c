#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "usmart.h"
#include "can.h"
#include "uavcan.h"
#include "canard.h"
#include "canard_stm32.h"
    
int main(void)
{
   	u8 key;
	u8 i=0,t=0;
	u8 cnt=0;
	u8 canbuf[8] = {0x11, 0x22, 0x55, 0x66, 0x77, 0x88};
	u8 res;
	u8 mode=1;

    Cache_Enable();                 //打开L1-Cache
    HAL_Init();				        //初始化HAL库
    Stm32_Clock_Init(432,25,2,9);   //设置时钟,216Mhz 
    delay_init(216);                //延时初始化
	uart_init(115200);		        //串口初始化
    usmart_dev.init(108); 		    //初始化USMART	
    LED_Init();                     //初始化LED
//    KEY_Init();                     //初始化按键
//    SDRAM_Init();                   //初始化SDRAM

    CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_11TQ,6,CAN_MODE_NORMAL);//CAN初始化环回模式,波特率500Kbps  
	uavcanInit();
    while(1)
    {
//        key=KEY_Scan(0);
		res=CAN1_Send_Msg(canbuf,8);//发送8个字节 	
//		publishCanard();
//		sendCanard();
		delay_ms(10);
		if(t==20)
		{
			//LED0_Toggle;//提示系统正在运行	
			t=0;
			cnt++;
		}		   
	}	
 									  	       
}
