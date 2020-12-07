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

    Cache_Enable();                 //��L1-Cache
    HAL_Init();				        //��ʼ��HAL��
    Stm32_Clock_Init(432,25,2,9);   //����ʱ��,216Mhz 
    delay_init(216);                //��ʱ��ʼ��
	uart_init(115200);		        //���ڳ�ʼ��
    usmart_dev.init(108); 		    //��ʼ��USMART	
    LED_Init();                     //��ʼ��LED
//    KEY_Init();                     //��ʼ������
//    SDRAM_Init();                   //��ʼ��SDRAM

    CAN1_Mode_Init(CAN_SJW_1TQ,CAN_BS2_6TQ,CAN_BS1_11TQ,6,CAN_MODE_NORMAL);//CAN��ʼ������ģʽ,������500Kbps  
	uavcanInit();
    while(1)
    {
//        key=KEY_Scan(0);
		res=CAN1_Send_Msg(canbuf,8);//����8���ֽ� 	
//		publishCanard();
//		sendCanard();
		delay_ms(10);
		if(t==20)
		{
			//LED0_Toggle;//��ʾϵͳ��������	
			t=0;
			cnt++;
		}		   
	}	
 									  	       
}
