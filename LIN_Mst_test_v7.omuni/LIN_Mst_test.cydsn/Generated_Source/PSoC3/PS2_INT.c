/*******************************************************************************
* File Name: PS2_INT.c
* Version 2.30
*
* Description:
*  This file provides all Interrupt Service functionality of the UART component
*
* Note:
*  Any unusual or non-standard behavior should be noted here. Other-
*  wise, this section should remain blank.
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "PS2.h"
#include "CyLib.h"


/***************************************
* Custom Declratations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if( (PS2_RX_ENABLED || PS2_HD_ENABLED) && \
     (PS2_RXBUFFERSIZE > PS2_FIFO_LENGTH))


    /*******************************************************************************
    * Function Name: PS2_RXISR
    ********************************************************************************
    *
    * Summary:
    *  Interrupt Service Routine for RX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  PS2_rxBuffer - RAM buffer pointer for save received data.
    *  PS2_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  PS2_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  PS2_rxBufferOverflow - software overflow flag. Set to one
    *     when PS2_rxBufferWrite index overtakes
    *     PS2_rxBufferRead index.
    *  PS2_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when PS2_rxBufferWrite is equal to
    *    PS2_rxBufferRead
    *  PS2_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  PS2_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(PS2_RXISR)
    {
        uint8 readData;
        uint8 increment_pointer = 0u;
        #if(CY_PSOC3)
            uint8 int_en;
        #endif /* CY_PSOC3 */

        /* User code required at start of ISR */
        /* `#START PS2_RXISR_START` */

        /* `#END` */

        #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
            int_en = EA;
            CyGlobalIntEnable;
        #endif /* CY_PSOC3 */

        readData = PS2_RXSTATUS_REG;

        if((readData & (PS2_RX_STS_BREAK | PS2_RX_STS_PAR_ERROR |
                        PS2_RX_STS_STOP_ERROR | PS2_RX_STS_OVERRUN)) != 0u)
        {
            /* ERROR handling. */
            /* `#START PS2_RXISR_ERROR` */

            /* `#END` */
        }

        while((readData & PS2_RX_STS_FIFO_NOTEMPTY) != 0u)
        {

            #if (PS2_RXHW_ADDRESS_ENABLED)
                if(PS2_rxAddressMode == (uint8)PS2__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readData & PS2_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readData & PS2_RX_STS_ADDR_MATCH) != 0u)
                        {
                            PS2_rxAddressDetected = 1u;
                        }
                        else
                        {
                            PS2_rxAddressDetected = 0u;
                        }
                    }

                    readData = PS2_RXDATA_REG;
                    if(PS2_rxAddressDetected != 0u)
                    {   /* store only addressed data */
                        PS2_rxBuffer[PS2_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* without software addressing */
                {
                    PS2_rxBuffer[PS2_rxBufferWrite] = PS2_RXDATA_REG;
                    increment_pointer = 1u;
                }
            #else  /* without addressing */
                PS2_rxBuffer[PS2_rxBufferWrite] = PS2_RXDATA_REG;
                increment_pointer = 1u;
            #endif /* End SW_DETECT_TO_BUFFER */

            /* do not increment buffer pointer when skip not adderessed data */
            if( increment_pointer != 0u )
            {
                if(PS2_rxBufferLoopDetect != 0u)
                {   /* Set Software Buffer status Overflow */
                    PS2_rxBufferOverflow = 1u;
                }
                /* Set next pointer. */
                PS2_rxBufferWrite++;

                /* Check pointer for a loop condition */
                if(PS2_rxBufferWrite >= PS2_RXBUFFERSIZE)
                {
                    PS2_rxBufferWrite = 0u;
                }
                /* Detect pre-overload condition and set flag */
                if(PS2_rxBufferWrite == PS2_rxBufferRead)
                {
                    PS2_rxBufferLoopDetect = 1u;
                    /* When Hardware Flow Control selected */
                    #if(PS2_FLOW_CONTROL != 0u)
                    /* Disable RX interrupt mask, it will be enabled when user read data from the buffer using APIs */
                        PS2_RXSTATUS_MASK_REG  &= (uint8)~PS2_RX_STS_FIFO_NOTEMPTY;
                        CyIntClearPending(PS2_RX_VECT_NUM);
                        break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                    #endif /* End PS2_FLOW_CONTROL != 0 */
                }
            }

            /* Check again if there is data. */
            readData = PS2_RXSTATUS_REG;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START PS2_RXISR_END` */

        /* `#END` */

        #if(CY_PSOC3)
            EA = int_en;
        #endif /* CY_PSOC3 */

    }

#endif /* End PS2_RX_ENABLED && (PS2_RXBUFFERSIZE > PS2_FIFO_LENGTH) */


#if(PS2_TX_ENABLED && (PS2_TXBUFFERSIZE > PS2_FIFO_LENGTH))


    /*******************************************************************************
    * Function Name: PS2_TXISR
    ********************************************************************************
    *
    * Summary:
    * Interrupt Service Routine for the TX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  PS2_txBuffer - RAM buffer pointer for transmit data from.
    *  PS2_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmited byte.
    *  PS2_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(PS2_TXISR)
    {

        #if(CY_PSOC3)
            uint8 int_en;
        #endif /* CY_PSOC3 */

        /* User code required at start of ISR */
        /* `#START PS2_TXISR_START` */

        /* `#END` */

        #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
            int_en = EA;
            CyGlobalIntEnable;
        #endif /* CY_PSOC3 */

        while((PS2_txBufferRead != PS2_txBufferWrite) &&
             ((PS2_TXSTATUS_REG & PS2_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer. */
            if(PS2_txBufferRead >= PS2_TXBUFFERSIZE)
            {
                PS2_txBufferRead = 0u;
            }

            PS2_TXDATA_REG = PS2_txBuffer[PS2_txBufferRead];

            /* Set next pointer. */
            PS2_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START PS2_TXISR_END` */

        /* `#END` */

        #if(CY_PSOC3)
            EA = int_en;
        #endif /* CY_PSOC3 */

    }

#endif /* End PS2_TX_ENABLED && (PS2_TXBUFFERSIZE > PS2_FIFO_LENGTH) */


/* [] END OF FILE */
