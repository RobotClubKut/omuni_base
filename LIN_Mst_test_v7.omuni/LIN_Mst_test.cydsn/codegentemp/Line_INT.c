/*******************************************************************************
* File Name: Line_INT.c
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

#include "Line.h"
#include "CyLib.h"


/***************************************
* Custom Declratations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if( (Line_RX_ENABLED || Line_HD_ENABLED) && \
     (Line_RXBUFFERSIZE > Line_FIFO_LENGTH))


    /*******************************************************************************
    * Function Name: Line_RXISR
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
    *  Line_rxBuffer - RAM buffer pointer for save received data.
    *  Line_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  Line_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  Line_rxBufferOverflow - software overflow flag. Set to one
    *     when Line_rxBufferWrite index overtakes
    *     Line_rxBufferRead index.
    *  Line_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when Line_rxBufferWrite is equal to
    *    Line_rxBufferRead
    *  Line_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  Line_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(Line_RXISR)
    {
        uint8 readData;
        uint8 increment_pointer = 0u;
        #if(CY_PSOC3)
            uint8 int_en;
        #endif /* CY_PSOC3 */

        /* User code required at start of ISR */
        /* `#START Line_RXISR_START` */

        /* `#END` */

        #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
            int_en = EA;
            CyGlobalIntEnable;
        #endif /* CY_PSOC3 */

        readData = Line_RXSTATUS_REG;

        if((readData & (Line_RX_STS_BREAK | Line_RX_STS_PAR_ERROR |
                        Line_RX_STS_STOP_ERROR | Line_RX_STS_OVERRUN)) != 0u)
        {
            /* ERROR handling. */
            /* `#START Line_RXISR_ERROR` */

            /* `#END` */
        }

        while((readData & Line_RX_STS_FIFO_NOTEMPTY) != 0u)
        {

            #if (Line_RXHW_ADDRESS_ENABLED)
                if(Line_rxAddressMode == (uint8)Line__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readData & Line_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readData & Line_RX_STS_ADDR_MATCH) != 0u)
                        {
                            Line_rxAddressDetected = 1u;
                        }
                        else
                        {
                            Line_rxAddressDetected = 0u;
                        }
                    }

                    readData = Line_RXDATA_REG;
                    if(Line_rxAddressDetected != 0u)
                    {   /* store only addressed data */
                        Line_rxBuffer[Line_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* without software addressing */
                {
                    Line_rxBuffer[Line_rxBufferWrite] = Line_RXDATA_REG;
                    increment_pointer = 1u;
                }
            #else  /* without addressing */
                Line_rxBuffer[Line_rxBufferWrite] = Line_RXDATA_REG;
                increment_pointer = 1u;
            #endif /* End SW_DETECT_TO_BUFFER */

            /* do not increment buffer pointer when skip not adderessed data */
            if( increment_pointer != 0u )
            {
                if(Line_rxBufferLoopDetect != 0u)
                {   /* Set Software Buffer status Overflow */
                    Line_rxBufferOverflow = 1u;
                }
                /* Set next pointer. */
                Line_rxBufferWrite++;

                /* Check pointer for a loop condition */
                if(Line_rxBufferWrite >= Line_RXBUFFERSIZE)
                {
                    Line_rxBufferWrite = 0u;
                }
                /* Detect pre-overload condition and set flag */
                if(Line_rxBufferWrite == Line_rxBufferRead)
                {
                    Line_rxBufferLoopDetect = 1u;
                    /* When Hardware Flow Control selected */
                    #if(Line_FLOW_CONTROL != 0u)
                    /* Disable RX interrupt mask, it will be enabled when user read data from the buffer using APIs */
                        Line_RXSTATUS_MASK_REG  &= (uint8)~Line_RX_STS_FIFO_NOTEMPTY;
                        CyIntClearPending(Line_RX_VECT_NUM);
                        break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                    #endif /* End Line_FLOW_CONTROL != 0 */
                }
            }

            /* Check again if there is data. */
            readData = Line_RXSTATUS_REG;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START Line_RXISR_END` */

        /* `#END` */

        #if(CY_PSOC3)
            EA = int_en;
        #endif /* CY_PSOC3 */

    }

#endif /* End Line_RX_ENABLED && (Line_RXBUFFERSIZE > Line_FIFO_LENGTH) */


#if(Line_TX_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))


    /*******************************************************************************
    * Function Name: Line_TXISR
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
    *  Line_txBuffer - RAM buffer pointer for transmit data from.
    *  Line_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmited byte.
    *  Line_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(Line_TXISR)
    {

        #if(CY_PSOC3)
            uint8 int_en;
        #endif /* CY_PSOC3 */

        /* User code required at start of ISR */
        /* `#START Line_TXISR_START` */

        /* `#END` */

        #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
            int_en = EA;
            CyGlobalIntEnable;
        #endif /* CY_PSOC3 */

        while((Line_txBufferRead != Line_txBufferWrite) &&
             ((Line_TXSTATUS_REG & Line_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer. */
            if(Line_txBufferRead >= Line_TXBUFFERSIZE)
            {
                Line_txBufferRead = 0u;
            }

            Line_TXDATA_REG = Line_txBuffer[Line_txBufferRead];

            /* Set next pointer. */
            Line_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START Line_TXISR_END` */

        /* `#END` */

        #if(CY_PSOC3)
            EA = int_en;
        #endif /* CY_PSOC3 */

    }

#endif /* End Line_TX_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH) */


/* [] END OF FILE */
