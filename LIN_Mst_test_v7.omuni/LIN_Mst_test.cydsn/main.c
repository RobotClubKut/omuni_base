
#include <project.h>

uint8 rx_buffer[10];

CY_ISR(isr_1_isr)
{
	int i;
	uint8 rxdata;
	UART_1_ClearRxBuffer();
	while(UART_1_GetByte() != 0x55);
	for(i = 0;i < 3;i++)
	{
		while(!(UART_1_ReadRxStatus() & UART_1_RX_STS_FIFO_NOTEMPTY));
		rxdata = UART_1_GetByte();
		rx_buffer[i] = rxdata;
	}
}
int main()
{
	uint8 buffer[10] = {0};
	int i;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    UART_1_Start();
	//UART_1_EnableTxInt();
	//UART_1_EnableRxInt();
	isr_1_StartEx(isr_1_isr);
	isr_1_Enable();
	
	for(;;)
    {
        for(i = 0;i < 4;i++)
		{
			buffer[i] = rx_buffer[i];
		}
    }
}

/* [] END OF FILE */
