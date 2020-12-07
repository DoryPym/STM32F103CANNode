#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "usmart.h"
#include "can.h"
#include "uavcan.h"
#include "canard.h"
#include "canard_stm32.h"
//#include <iostream>


//using namespace std;

int main(void)
{	
   	u8 key;
	uint16_t i=0,t=0;
	u8 cnt=0;
	u8 canbuf[8] = {0xff};
	u8 res;
	u8 mode=1; 
	
    HAL_Init();                    	 	//初始化HAL库    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//设置时钟,72M
	delay_init(72);               		//初始化延时函数
	uart_init(115200);					//初始化串口
	usmart_dev.init(84); 		   		//初始化USMART	
//	LED_Init();							//初始化LED	
//	KEY_Init();							//初始化按键
    
 	CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_8TQ,CAN_BS1_9TQ,4,CAN_MODE_NORMAL); //CAN初始化,波特率500Kbps      
	CAN_Config();
	
	uavcanInit();
    while(1)
    {
        key=KEY_Scan(0);		 
		t++; 
		delay_ms(10);
		CAN1_Send_Msg(canbuf, 8);

		if(t == 500)
		{
			//CAN1_Send_Msg(canbuf,8);
			t = 0;
		}
	} 
}

