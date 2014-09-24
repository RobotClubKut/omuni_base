/*******************************************************************************
* File Name: LIN_INT.c
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

#include "LIN.h"
#include "CyLib.h"


/***************************************
* Custom Declratations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if( (LIN_RX_ENABLED || LIN_HD_ENABLED) && \
     (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH))


    /*******************************************************************************
    * Function Name: LIN_RXISR
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
    *  LIN_rxBuffer - RAM buffer pointer for save received data.
    *  LIN_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  LIN_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  LIN_rxBufferOverflow - software overflow flag. Set to one
    *     when LIN_rxBufferWrite index overtakes
    *     LIN_rxBufferRead index.
    *  LIN_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when LIN_rxBufferWrite is equal to
    *    LIN_rxBufferRead
    *  LIN_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  LIN_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(LIN_RXISR)
    {
        uint8 readData;
        uint8 increment_pointer = 0u;
        #if(CY_PSOC3)
            uint8 int_en;
        #endif /* CY_PSOC3 */

        /* User code required at start of ISR */
        /* `#START LIN_RXISR_START` */

        /* `#END` */

        #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
            int_en = EA;
            CyGlobalIntEnable;
        #endif /* CY_PSOC3 */

        readData = LIN_RXSTATUS_REG;

        if((readData & (LIN_RX_STS_BREAK | LIN_RX_STS_PAR_ERROR |
                        LIN_RX_STS_STOP_ERROR | LIN_RX_STS_OVERRUN)) != 0u)
        {
            /* ERROR handling. */
            /* `#START LIN_RXISR_ERROR` */

            /* `#END` */
        }

        while((readData & LIN_RX_STS_FIFO_NOTEMPTY) != 0u)
        {

            #if (LIN_RXHW_ADDRESS_ENABLED)
                if(LIN_rxAddressMode == (uint8)LIN__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readData & LIN_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readData & LIN_RX_STS_ADDR_MATCH) != 0u)
                        {
                            LIN_rxAddressDetected = 1u;
                        }
                        else
                        {
                            LIN_rxAddressDetected = 0u;
                        }
                    }

                    readData = LIN_RXDATA_REG;
                    if(LIN_rxAddressDetected != 0u)
                    {   /* store only addressed data */
                        LIN_rxBuffer[LIN_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* without software addressing */
                {
                    LIN_rxBuffer[LIN_rxBufferWrite] = LIN_RXDATA_REG;
                    increment_pointer = 1u;
                }
            #else  /* without addressing */
                LIN_rxBuffer[LIN_rxBufferWrite] = LIN_RXDATA_REG;
                increment_pointer = 1u;
            #endif /* End SW_DETECT_TO_BUFFER */

            /* do not increment buffer pointer when skip not adderessed data */
            if( increment_pointer != 0u )
            {
                if(LIN_rxBufferLoopDetect != 0u)
                {   /* Set Software Buffer status Overflow */
                    LIN_rxBufferOverflow = 1u;
                }
                /* Set next pointer. */
                LIN_rxBufferWrite++;

                /* Check pointer for a loop condition */
                if(LIN_rxBufferWrite >= LIN_RXBUFFERSIZE)
                {
                    LIN_rxBufferWrite = 0u;
                }
                /* Detect pre-overload condition and set flag */
                if(LIN_rxBufferWrite == LIN_rxBufferRead)
                {
                    LIN_rxBufferLoopDetect = 1u;
                    /* When Hardware Flow Control selected */
                    #if(LIN_FLOW_CONTROL != 0u)
                    /* Disable RX interrupt mask, it will be enabled when user read data from the buffer using APIs */
                        LIN_RXSTATUS_MASK_REG  &= (uint8)~LIN_RX_STS_FIFO_NOTEMPTY;
                        CyIntClearPending(LIN_RX_VECT_NUM);
                        break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                    #endif /* End LIN_FLOW_CONTROL != 0 */
                }
            }

            /* Check again if there is data. */
            readData = LIN_RXSTATUS_REG;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START LIN_RXISR_END` */

        /* `#END` */

        #if(CY_PSOC3)
            EA = int_en;
        #endif /* CY_PSOC3 */

    }

#endif /* End LIN_RX_ENABLED && (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH) */


#if(LIN_TX_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH))


    /*******************************************************************************
    * Function Name: LIN_TXISR
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
    *  LIN_txBuffer - RAM buffer pointer for transmit data from.
    *  LIN_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmited byte.
    *  LIN_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(LIN_TXISR)
    {

        #if(CY_PSOC3)
            uint8 int_en;
        #endif /* CY_PSOC3 */

        /* User code required at start of ISR */
        /* `#START LIN_TXISR_START` */

        /* `#END` */

        #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
            int_en = EA;
            CyGlobalIntEnable;
        #endif /* CY_PSOC3 */

        while((LIN_txBufferRead != LIN_txBufferWrite) &&
             ((LIN_TXSTATUS_REG & LIN_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer. */
            if(LIN_txBufferRead >= LIN_TXBUFFERSIZE)
            {
                LIN_txBufferRead = 0u;
            }

            LIN_TXDATA_REG = LIN_txBuffer[LIN_txBufferRead];

            /* Set next pointer. */
            LIN_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START LIN_TXISR_END` */

        /* `#END` */

        #if(CY_PSOC3)
            EA = int_en;
        #endif /* CY_PSOC3 */

    }

#endif /* End LIN_TX_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH) */


/* [] END OF FILE */
