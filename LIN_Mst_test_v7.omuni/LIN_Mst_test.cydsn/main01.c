
#include <project.h>
#include <stdio.h>

uint8 LIN_Master_id_send(uint8 id);
uint8 LIN_Master_GetArray(uint8 id,uint8 length,uint8* buffer);
void LIN_Master_PutArray(uint8 id,uint8 length,uint8* const buffer);


uint8 lin_id_flag;
uint8 lin_tx_flag;
uint8 lin_rx_flag;
uint8 Rxbuffer[10] = {0};
long rori;
CY_ISR(isr_1_isr)
{
	uint8 r_flag = 1;
	/*
	CYGlobalIntDisable;
	LIN_Master_GetArray(1,1,&r_flag);
	if(!(r_flag & 0x01))
	{
		LIN_Master_GetArray(0,8,Rxbuffer);
		rori = ((uint16)Rxbuffer[0]+((uint16)Rxbuffer[1]<<8));
	}
	else
	{
		
	}
	CYGlobalIntEnable;
	*/
}
CY_ISR(uart_tx_isr)
{
	if(lin_id_flag == 1)
	{
		lin_id_flag = 0;
	}
}
CY_ISR(uart_rx_isr)
{
	
}
int main()
{
	uint8 Txbuffer[10] = {"ABCDEFG"};

	int i;
	uint8 err[3] = {0};
	char str[30];
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    LIN_Start();
	UART_2_Start();
	uart_tx_isr_StartEx(uart_tx_isr);
	uart_rx_isr_StartEx(uart_rx_isr);
	isr_1_StartEx(isr_1_isr);
	UART_2_EnableTxInt();
	LIN_EnableRxInt();
	LIN_EnableTxInt();
	
	for(;;)
	{
		if(UART_2_ReadTxStatus() & UART_2_TX_STS_FIFO_EMPTY)
		{
			sprintf(str,"rori = %6ld\n",rori);
			UART_2_PutString(str);
		}
		// ヘッダー送信
		//LIN_Master_PutArray(2,8,Txbuffer);
		LIN_Master_id_send(1);
		//CyDelayUs(20);
		/*
		do{
		err[1] = LIN_Master_GetArray(1,1,err);
		}while(err[0] != 0);
		*/
    }
}
/*
uint8 LIN_Master_id_send(uint8 id)
{
	uint8 p0 = 0;
	uint8 p1 = 0;
	p0 = (id&0x01)^((id >> 1)&0x01)^((id >> 2)&0x01)^((id >> 4)&0x01);
	p1 = ~(((id >> 1)&0x01)^((id >> 3)&0x01)^((id >> 4)&0x01)^((id >> 5)&0x01));
	id = (id&0x3f)|((p0 << 6)&0x40)|((p1 << 7)&0x80);
	LIN_SendBreak(LIN_SEND_WAIT_REINIT);
	LIN_ClearTxBuffer();
	LIN_ClearRxBuffer();
	LIN_PutChar(0x55);
	LIN_PutChar(id);
	while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));
	return id;
}
*/
uint8 LIN_Master_id_send(uint8 id)
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
		return 0;
	}
	else
	{
		return -1;
	}
}
uint8 LIN_Master_GetArray(uint8 id,uint8 length,uint8* buffer)
{
	
}
/*
uint8 LIN_Master_GetArray(uint8 id,uint8 length,uint8* buffer)
{
	uint8 i;
	uint32 t;
	uint16 rxdata;
	uint16 sum = LIN_Master_id_send(id);
	LIN_ClearRxBuffer();
	LIN_ClearTxBuffer();
	for(i = 0;i < length;i++)
	{
		t = 0;
		while(!(LIN_ReadRxStatus() & LIN_RX_STS_FIFO_NOTEMPTY))
		{
			if(t < 1000)
			{
				t++;
			}
			else
			{
				t = 0;
				return 1;
			}
			CyDelayUs(1);
		}
		rxdata = LIN_GetByte();
		if(!(rxdata & 0x8000))
		{
			rxdata   &= 0xff;
			sum      += rxdata;
			buffer[i] = rxdata;
		}
		else
		{
			return 1;
		}
	}
	sum = 0xff & (~( ((sum >> 8) & 0x7) + (sum & 0xff)));
	t = 0;
	while(!(LIN_ReadRxStatus() & LIN_RX_STS_FIFO_NOTEMPTY))
	{
		if(t < 1000)
		{
			t++;
		}
		else
		{
			t = 0;
			return 1;
		}
		CyDelayUs(1);
	}
	if((0xff & sum) == (0xff & LIN_GetByte()))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
*/
void LIN_Master_PutArray(uint8 id,uint8 length,uint8* buffer)
{
	uint8 i;
	uint32 t;
	uint16 sum = LIN_Master_id_send(id);
	LIN_ClearRxBuffer();
	LIN_ClearTxBuffer();
	for(i = 0;i < length;i++)
	{
		sum += buffer[i];
		LIN_PutChar(buffer[i]);
		while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));	
	}
	sum = 0xff & (~( ((sum >> 8) & 0x7) + (sum & 0xff)));
	LIN_PutChar(sum);
	while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));
}
