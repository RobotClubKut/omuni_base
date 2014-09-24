
#include <project.h>
#include <stdio.h>

#include "ps2_controller.h"
#include <math.h>

// lin define
#define LIN_RX_EMPTY    	0x01
#define LIN_RX_COMPLETE 	0x02
#define LIN_RX_SEND     	0x04
#define LIN_RX_SUM_ERR  	0x08
#define LIN_RX_TIMEOUT_ERR  0x10
#define LIN_TX_SEND 	1

// lin method
uint16 LIN_Master_id_send(uint8 id);
uint8 LIN_Master_GetArray(uint8 id,uint8 length,uint8* buffer);
uint8 LIN_Master_ReadRxStatus(void);
uint8 LIN_Master_PutArray(uint8 id,uint8 length,uint8* const buffer);
uint8 LIN_Master_ReadTxStatus(void);

// lin value
uint8 lin_id_flag;
uint8 lin_tx_flag = 0;
uint8 lin_rx_flag = 1;
uint8 lin_rx_sync_flag;
uint16 rx_sum;
uint8 rx_length;
uint8* rx_buffer;
uint8 rx_i;
uint8 i;

uint8 Rxbuffer[10] = {0};
uint16 rori;



typedef struct
{
	int8 x;
	int8 y;
	int8 m;
	int16 a;
	int16 b;
	int16 c;
	int16 max;
} Speed;

/*
モーターバランス計算
*/

void calcSpeed(Speed *sp)
{
	float tmp;
	tmp = (-2 * sp->x) + (2 * sp->m);
	sp->a = floor((sp->max / (128 * (3 + pow(3, 0.5)))) * tmp + 0.5);
	
	tmp = sp->x - (pow(3, 0.5) * sp->y) + (2 * sp->m);
	sp->b = floor((sp->max / (128 * (3 + pow(3, 0.5)))) * tmp + 0.5);
	
	tmp = sp->x + (pow(3, 0.5) * sp->y) + (2 * sp->m);
	sp->c = floor((sp->max / (128 * (3 + pow(3, 0.5)))) * tmp + 0.5);
	return;
}




CY_ISR(isr_1_isr)
{
	//LIN_Master_GetArray(0,8,Rxbuffer);
}

CY_ISR(uart_tx_isr)
{
	if(LIN_ReadTxStatus() & LIN_TX_STS_FIFO_EMPTY)
	{
		if(lin_id_flag == 1)
		{
			lin_id_flag = 0;
		}
		if(lin_tx_flag == LIN_TX_SEND)
		{
			lin_tx_flag = 0;
		}
	}
}

CY_ISR(uart_rx_isr)
{
	if(lin_rx_flag & LIN_RX_SEND)
	{
		if(rx_i < rx_length)
		{
			rx_buffer[rx_i] = LIN_GetByte() & 0xff;
			
			if(lin_rx_sync_flag == 0)
			{
				/*
				if(rx_buffer[rx_i] == 0x55)
				{
					return;
				}
				*/
				lin_rx_sync_flag = 1;
				return;
			}
			
			else if(lin_rx_sync_flag == 1)
			{
				/*
				if(rx_buffer[rx_i] == rx_sum)
				{
					return;
				}
				*/
				lin_rx_sync_flag = 2;
				return;
			}
			
			rx_sum += rx_buffer[rx_i];
			rx_i++;
		}
		else
		{
			LIN_Timer_Stop();
			rx_sum = 0xff & (~( ((rx_sum >> 8) & 0xf) + (rx_sum & 0xff)));
			if((rx_sum & 0xff) == (LIN_GetByte() & 0xff))
			{
				lin_rx_flag &= ~LIN_RX_SEND;
				lin_rx_flag &= ~LIN_RX_SUM_ERR;
				lin_rx_flag &= ~LIN_RX_TIMEOUT_ERR;
				lin_rx_flag |= LIN_RX_COMPLETE;
				lin_rx_flag |= LIN_RX_EMPTY;
			}
			else
			{
				lin_rx_flag &= ~LIN_RX_SEND;
				lin_rx_flag &= ~LIN_RX_COMPLETE;
				lin_rx_flag |= LIN_RX_EMPTY;
				lin_rx_flag |= LIN_RX_SUM_ERR;
			}
		}
	}
}

CY_ISR(lin_timeout_isr)
{
	//char str[40];
	if(lin_rx_flag & LIN_RX_SEND)
	{
		/*
		while(1)
		{
			if(UART_2_ReadTxStatus() & UART_2_TX_STS_FIFO_EMPTY)
			{
				sprintf(str,"err %x \n",(int)lin_rx_flag);
				UART_2_PutString(str);
			}
		}
		*/
		LIN_Timer_WriteCounter(255);
		LIN_Timer_Stop();
		lin_rx_flag &= ~LIN_RX_SEND;
		lin_rx_flag &= ~LIN_RX_COMPLETE;
		lin_rx_flag |= LIN_RX_EMPTY;
		lin_rx_flag |= LIN_RX_TIMEOUT_ERR;
		/*
		LIN_Stop();
		LIN_Start();
		LIN_EnableRxInt();
		LIN_EnableTxInt();
		LIN_ClearRxBuffer();
		LIN_ClearTxBuffer();
		lin_rx_flag = LIN_RX_EMPTY;
		lin_id_flag = 0;
		lin_tx_flag = 0;
		lin_rx_sync_flag = 0;
		rx_sum = 0;
		rx_length = 0;
		rx_buffer = 0;
		rx_i = 0;
		*/
		/*
		sprintf(str,"err\n");
		UART_2_PutString(str);
		*/
	}
}



int main()
{
	uint8 Txbuffer[10] = {100,9,9,9,9,9,9,9};

	uint8 err[3] = {0};
	uint8 rori_sts = 0;
	uint8 rori_flag = 0;
	char str[30];
	uint16 cnt = 0;
	
	// ps2 controller data
	uint8 PS_data[10];
	
	// speed
	Speed omuni;
	omuni.max = 3000;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    LIN_Start();
	UART_2_Start();
	PS2_Start();
    PS2_ClearRxBuffer();
	
	uart_tx_isr_StartEx(uart_tx_isr);
	uart_rx_isr_StartEx(uart_rx_isr);
	lin_timer_isr_StartEx(lin_timeout_isr);
	isr_1_StartEx(isr_1_isr);
	UART_2_EnableTxInt();
	LIN_EnableRxInt();
	LIN_EnableTxInt();
	
	//LIN_Master_PutArray(2,8,Txbuffer);
	
	for(;;)
	{
		PS_Controller_get(PS_data);
		
		if(analog_l_x > (127+40))
        {
			omuni.x = -(analog_l_x-128);
        }
        else if(analog_l_x < (127-40))
        {
			omuni.x = (127-analog_l_x);
        }
        else
        {
			omuni.x = 0;
        }
		
		if(analog_l_y > (127+40))
        {
			omuni.y = -(analog_l_y-128);
        }
        else if(analog_l_y < (127-40))
        {
			omuni.y = (127-analog_l_y);
        }
        else
        {
			omuni.y = 0;
        }
		
		if(analog_r_x > (127+40))
        {
			omuni.m = -(analog_r_x-128);
        }
        else if(analog_r_x < (127-40))
        {
			omuni.m = (127-analog_r_x);
        }
        else
        {
			omuni.m = 0;
        }
		
		calcSpeed(&omuni);
		if(UART_2_ReadTxStatus() & UART_2_TX_STS_FIFO_EMPTY)
		{
			//sprintf(str,"%x,%x,%x,%x,%x,%x,%x,%x\n",(int)rx_buffer[0],(int)rx_buffer[1],(int)rx_buffer[2],(int)rx_buffer[3],(int)rx_buffer[4],(int)rx_buffer[5],(int)rx_buffer[6],(int)rx_buffer[7]);
			sprintf(str,"%x %d %4d %d\n",(int)lin_rx_flag,(int)rori_flag,(int16)rori,(int)LIN_Timer_ReadCounter());
			//sprintf(str,"%2x %2x L:%2x,%2x R:%2x,%2x\n",(int16)PS_data[0],(int16)PS_data[1],(int16)PS_data[4],(int16)PS_data[5],(int16)PS_data[2],(int16)PS_data[3]);
			UART_2_PutString(str);
		}
		/*
		if(LIN_Master_ReadRxStatus() == LIN_RX_COMPLETE)
		{
			rori = ((uint16)Rxbuffer[0]+((uint16)Rxbuffer[1]<<8));
		}
		*/
		/*
		if(LIN_Master_ReadRxStatus() & LIN_RX_TIMEOUT_ERR)
		{
			lin_rx_flag |= LIN_RX_EMPTY;
			lin_rx_flag &= ~LIN_RX_SEND;
			lin_rx_flag &= ~LIN_RX_COMPLETE;
			//lin_rx_flag &= ~LIN_RX_TIMEOUT_ERR;
		}
		*/
		// 確実な読み込み
		
		if((LIN_Master_ReadRxStatus() & LIN_RX_EMPTY) && (rori_flag == 0))
		{
			LIN_Master_GetArray(1,1,&rori_sts);
			rori_flag = 1;
			cnt = 0;
		}
		else if(rori_flag == 1)
		{
			if(cnt < 100)
			{
				cnt++;
			}
			else
			{
				cnt = 0;
				rori_flag = 0;
			}
			if(LIN_Master_ReadRxStatus() & LIN_RX_COMPLETE)
			{
				if(!(rori_sts&0x01))
				{
					LIN_Master_GetArray(0,8,Rxbuffer);
					rori_flag = 2;
					cnt = 0;
				}
				else
				{
					rori_flag = 0;
				}
			}
		}
		else if(rori_flag == 2)
		{
			if(cnt < 100)
			{
				cnt++;
			}
			else
			{
				cnt = 0;
				rori_flag = 0;
			}
			if(LIN_Master_ReadRxStatus() & LIN_RX_COMPLETE)
			{
				rori = ((uint16)Rxbuffer[0]+((uint16)Rxbuffer[1]<<8));
				LIN_Master_PutArray(2,8,PS_data);
				rori_flag = 3;
				//rori_flag = 0;
			}
		}
		else if(rori_flag == 3)
		{
			if(LIN_Master_ReadTxStatus() != LIN_TX_SEND)
			{
				rori_flag = 0;
			}
		}
		// ヘッダー送信
		//LIN_Master_PutArray(2,8,Txbuffer);
		//LIN_Master_id_send(1);
		//CyDelayUs(20);
		/*
		do{
		//err[1] = LIN_Master_GetArray(1,1,err);
		}while(err[0] != 0);
		*/
    }
}

uint16 LIN_Master_id_send(uint8 id)
{
	uint8 p0 = 0;
	uint8 p1 = 0;
	if(lin_id_flag == 0)
	{
		p0 = (id&0x01)^((id >> 1)&0x01)^((id >> 2)&0x01)^((id >> 4)&0x01);
		p1 = ~(((id >> 1)&0x01)^((id >> 3)&0x01)^((id >> 4)&0x01)^((id >> 5)&0x01));
		id = (id&0x3f)|((p0 << 6)&0x40)|((p1 << 7)&0x80);
		LIN_SendBreak(LIN_SEND_WAIT_REINIT);
		LIN_ClearTxBuffer();
		LIN_ClearRxBuffer();
		LIN_PutChar(0x55);
		LIN_PutChar(id);
		lin_id_flag = 1;
		return 0xff&id;
	}
	else
	{
		return 0x100;
	}
}

uint8 LIN_Master_GetArray(uint8 id,uint8 length,uint8* buffer)
{
	if((lin_rx_flag & LIN_RX_EMPTY) &(lin_tx_flag == 0))
	{
		rx_sum = LIN_Master_id_send(id);
		if(rx_sum == 0x100)
		{
			lin_rx_flag |= LIN_RX_EMPTY;
			lin_rx_flag &= ~LIN_RX_SEND;
			LIN_Timer_Stop();
			return 1;
		}		
		LIN_ClearRxBuffer();
		rx_length = length;
		rx_buffer = buffer;
		rx_i = 0;
		lin_rx_sync_flag = 0;
		lin_rx_flag &= ~LIN_RX_EMPTY;
		lin_rx_flag &= ~LIN_RX_COMPLETE;
		lin_rx_flag |= LIN_RX_SEND;
		LIN_Timer_WriteCounter(255);
		LIN_Timer_Start();

	}
	else
	{
		return 1;
	}
	return 0;
}

uint8 LIN_Master_ReadRxStatus()
{
	uint8 flag = lin_rx_flag;
	//lin_rx_flag &= ~LIN_RX_SUM_ERR; 
	return flag;
}

uint8 LIN_Master_ReadTxStatus()
{
	return lin_tx_flag;
}

uint8 LIN_Master_PutArray(uint8 id,uint8 length,uint8* buffer)
{
	if((lin_tx_flag == 0) & (lin_rx_flag & LIN_RX_EMPTY))
	{
		uint8 i;
		uint16 sum = LIN_Master_id_send(id);
		if(sum == 0x100)
		{
			return 1;
		}
		//LIN_ClearTxBuffer();
		for(i = 0;i < length;i++)
		{
			sum += buffer[i];
			LIN_PutChar(buffer[i]);
			//while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));	
		}
		sum = 0xff & (~( ((sum >> 8) & 0x7) + (sum & 0xff)));
		LIN_PutChar(sum);
		lin_tx_flag = LIN_TX_SEND;
		//while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));
		return 0;
	}
	else
	{
		return 1;
	}
}
