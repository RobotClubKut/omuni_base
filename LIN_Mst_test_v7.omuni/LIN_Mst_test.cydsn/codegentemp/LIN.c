/*******************************************************************************
* File Name: LIN.c
* Version 2.30
*
* Description:
*  This file provides all API functionality of the UART component
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "LIN.h"
#include "CyLib.h"
#if(LIN_INTERNAL_CLOCK_USED)
    #include "LIN_IntClock.h"
#endif /* End LIN_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 LIN_initVar = 0u;
#if( LIN_TX_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH))
    volatile uint8 LIN_txBuffer[LIN_TXBUFFERSIZE];
    volatile uint8 LIN_txBufferRead = 0u;
    uint8 LIN_txBufferWrite = 0u;
#endif /* End LIN_TX_ENABLED */
#if( ( LIN_RX_ENABLED || LIN_HD_ENABLED ) && \
     (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH) )
    volatile uint8 LIN_rxBuffer[LIN_RXBUFFERSIZE];
    volatile uint8 LIN_rxBufferRead = 0u;
    volatile uint8 LIN_rxBufferWrite = 0u;
    volatile uint8 LIN_rxBufferLoopDetect = 0u;
    volatile uint8 LIN_rxBufferOverflow = 0u;
    #if (LIN_RXHW_ADDRESS_ENABLED)
        volatile uint8 LIN_rxAddressMode = LIN_RXADDRESSMODE;
        volatile uint8 LIN_rxAddressDetected = 0u;
    #endif /* End EnableHWAddress */
#endif /* End LIN_RX_ENABLED */


/*******************************************************************************
* Function Name: LIN_Start
********************************************************************************
*
* Summary:
*  Initialize and Enable the UART component.
*  Enable the clock input to enable operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  The LIN_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time UART_Start() is called. This allows for
*  component initialization without re-initialization in all subsequent calls
*  to the LIN_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_Start(void) 
{
    /* If not Initialized then initialize all required hardware and software */
    if(LIN_initVar == 0u)
    {
        LIN_Init();
        LIN_initVar = 1u;
    }
    LIN_Enable();
}


/*******************************************************************************
* Function Name: LIN_Init
********************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the
*  customizer of the component placed onto schematic. Usually called in
*  LIN_Start().
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void LIN_Init(void) 
{
    #if(LIN_RX_ENABLED || LIN_HD_ENABLED)

        #if(LIN_RX_INTERRUPT_ENABLED && (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH))
            /* Set the RX Interrupt. */
            (void)CyIntSetVector(LIN_RX_VECT_NUM, &LIN_RXISR);
            CyIntSetPriority(LIN_RX_VECT_NUM, LIN_RX_PRIOR_NUM);
        #endif /* End LIN_RX_INTERRUPT_ENABLED */

        #if (LIN_RXHW_ADDRESS_ENABLED)
            LIN_SetRxAddressMode(LIN_RXAddressMode);
            LIN_SetRxAddress1(LIN_RXHWADDRESS1);
            LIN_SetRxAddress2(LIN_RXHWADDRESS2);
        #endif /* End LIN_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        LIN_RXBITCTR_PERIOD_REG = LIN_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        LIN_RXSTATUS_MASK_REG  = LIN_INIT_RX_INTERRUPTS_MASK;
    #endif /* End LIN_RX_ENABLED || LIN_HD_ENABLED*/

    #if(LIN_TX_ENABLED)
        #if(LIN_TX_INTERRUPT_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH))
            /* Set the TX Interrupt. */
            (void)CyIntSetVector(LIN_TX_VECT_NUM, &LIN_TXISR);
            CyIntSetPriority(LIN_TX_VECT_NUM, LIN_TX_PRIOR_NUM);
        #endif /* End LIN_TX_INTERRUPT_ENABLED */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if(LIN_TXCLKGEN_DP)
            LIN_TXBITCLKGEN_CTR_REG = LIN_BIT_CENTER;
            LIN_TXBITCLKTX_COMPLETE_REG = (LIN_NUMBER_OF_DATA_BITS +
                        LIN_NUMBER_OF_START_BIT) * LIN_OVER_SAMPLE_COUNT;
        #else
            LIN_TXBITCTR_PERIOD_REG = ((LIN_NUMBER_OF_DATA_BITS +
                        LIN_NUMBER_OF_START_BIT) * LIN_OVER_SAMPLE_8) - 1u;
        #endif /* End LIN_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if(LIN_TX_INTERRUPT_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH))
            LIN_TXSTATUS_MASK_REG = LIN_TX_STS_FIFO_EMPTY;
        #else
            LIN_TXSTATUS_MASK_REG = LIN_INIT_TX_INTERRUPTS_MASK;
        #endif /*End LIN_TX_INTERRUPT_ENABLED*/

    #endif /* End LIN_TX_ENABLED */

    #if(LIN_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        LIN_WriteControlRegister( \
            (LIN_ReadControlRegister() & (uint8)~LIN_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(LIN_PARITY_TYPE << LIN_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End LIN_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: LIN_Enable
********************************************************************************
*
* Summary:
*  Enables the UART block operation
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  LIN_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void LIN_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if(LIN_RX_ENABLED || LIN_HD_ENABLED)
        /*RX Counter (Count7) Enable */
        LIN_RXBITCTR_CONTROL_REG |= LIN_CNTR_ENABLE;
        /* Enable the RX Interrupt. */
        LIN_RXSTATUS_ACTL_REG  |= LIN_INT_ENABLE;
        #if(LIN_RX_INTERRUPT_ENABLED && (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH))
            CyIntEnable(LIN_RX_VECT_NUM);
            #if (LIN_RXHW_ADDRESS_ENABLED)
                LIN_rxAddressDetected = 0u;
            #endif /* End LIN_RXHW_ADDRESS_ENABLED */
        #endif /* End LIN_RX_INTERRUPT_ENABLED */
    #endif /* End LIN_RX_ENABLED || LIN_HD_ENABLED*/

    #if(LIN_TX_ENABLED)
        /*TX Counter (DP/Count7) Enable */
        #if(!LIN_TXCLKGEN_DP)
            LIN_TXBITCTR_CONTROL_REG |= LIN_CNTR_ENABLE;
        #endif /* End LIN_TXCLKGEN_DP */
        /* Enable the TX Interrupt. */
        LIN_TXSTATUS_ACTL_REG |= LIN_INT_ENABLE;
        #if(LIN_TX_INTERRUPT_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH))
            CyIntEnable(LIN_TX_VECT_NUM);
        #endif /* End LIN_TX_INTERRUPT_ENABLED*/
     #endif /* End LIN_TX_ENABLED */

    #if(LIN_INTERNAL_CLOCK_USED)
        /* Enable the clock. */
        LIN_IntClock_Start();
    #endif /* End LIN_INTERNAL_CLOCK_USED */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: LIN_Stop
********************************************************************************
*
* Summary:
*  Disable the UART component
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void LIN_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if(LIN_RX_ENABLED || LIN_HD_ENABLED)
        LIN_RXBITCTR_CONTROL_REG &= (uint8)~LIN_CNTR_ENABLE;
    #endif /* End LIN_RX_ENABLED */

    #if(LIN_TX_ENABLED)
        #if(!LIN_TXCLKGEN_DP)
            LIN_TXBITCTR_CONTROL_REG &= (uint8)~LIN_CNTR_ENABLE;
        #endif /* End LIN_TXCLKGEN_DP */
    #endif /* LIN_TX_ENABLED */

    #if(LIN_INTERNAL_CLOCK_USED)
        /* Disable the clock. */
        LIN_IntClock_Stop();
    #endif /* End LIN_INTERNAL_CLOCK_USED */

    /* Disable internal interrupt component */
    #if(LIN_RX_ENABLED || LIN_HD_ENABLED)
        LIN_RXSTATUS_ACTL_REG  &= (uint8)~LIN_INT_ENABLE;
        #if(LIN_RX_INTERRUPT_ENABLED && (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH))
            LIN_DisableRxInt();
        #endif /* End LIN_RX_INTERRUPT_ENABLED */
    #endif /* End LIN_RX_ENABLED */

    #if(LIN_TX_ENABLED)
        LIN_TXSTATUS_ACTL_REG &= (uint8)~LIN_INT_ENABLE;
        #if(LIN_TX_INTERRUPT_ENABLED && (LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH))
            LIN_DisableTxInt();
        #endif /* End LIN_TX_INTERRUPT_ENABLED */
    #endif /* End LIN_TX_ENABLED */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: LIN_ReadControlRegister
********************************************************************************
*
* Summary:
*  Read the current state of the control register
*
* Parameters:
*  None.
*
* Return:
*  Current state of the control register.
*
*******************************************************************************/
uint8 LIN_ReadControlRegister(void) 
{
    #if( LIN_CONTROL_REG_REMOVED )
        return(0u);
    #else
        return(LIN_CONTROL_REG);
    #endif /* End LIN_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: LIN_WriteControlRegister
********************************************************************************
*
* Summary:
*  Writes an 8-bit value into the control register
*
* Parameters:
*  control:  control register value
*
* Return:
*  None.
*
*******************************************************************************/
void  LIN_WriteControlRegister(uint8 control) 
{
    #if( LIN_CONTROL_REG_REMOVED )
        if(control != 0u) { }      /* release compiler warning */
    #else
       LIN_CONTROL_REG = control;
    #endif /* End LIN_CONTROL_REG_REMOVED */
}


#if(LIN_RX_ENABLED || LIN_HD_ENABLED)

    #if(LIN_RX_INTERRUPT_ENABLED)

        /*******************************************************************************
        * Function Name: LIN_EnableRxInt
        ********************************************************************************
        *
        * Summary:
        *  Enable RX interrupt generation
        *
        * Parameters:
        *  None.
        *
        * Return:
        *  None.
        *
        * Theory:
        *  Enable the interrupt output -or- the interrupt component itself
        *
        *******************************************************************************/
        void LIN_EnableRxInt(void) 
        {
            CyIntEnable(LIN_RX_VECT_NUM);
        }


        /*******************************************************************************
        * Function Name: LIN_DisableRxInt
        ********************************************************************************
        *
        * Summary:
        *  Disable RX interrupt generation
        *
        * Parameters:
        *  None.
        *
        * Return:
        *  None.
        *
        * Theory:
        *  Disable the interrupt output -or- the interrupt component itself
        *
        *******************************************************************************/
        void LIN_DisableRxInt(void) 
        {
            CyIntDisable(LIN_RX_VECT_NUM);
        }

    #endif /* LIN_RX_INTERRUPT_ENABLED */


    /*******************************************************************************
    * Function Name: LIN_SetRxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configure which status bits trigger an interrupt event
    *
    * Parameters:
    *  IntSrc:  An or'd combination of the desired status bit masks (defined in
    *           the header file)
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void LIN_SetRxInterruptMode(uint8 intSrc) 
    {
        LIN_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: LIN_ReadRxData
    ********************************************************************************
    *
    * Summary:
    *  Returns data in RX Data register without checking status register to
    *  determine if data is valid
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Received data from RX register
    *
    * Global Variables:
    *  LIN_rxBuffer - RAM buffer pointer for save received data.
    *  LIN_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  LIN_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  LIN_rxBufferLoopDetect - creared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 LIN_ReadRxData(void) 
    {
        uint8 rxData;

        #if(LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH)
            uint8 loc_rxBufferRead;
            uint8 loc_rxBufferWrite;
            /* Protect variables that could change on interrupt. */
            /* Disable Rx interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_DisableRxInt();
            #endif /* LIN_RX_INTERRUPT_ENABLED */
            loc_rxBufferRead = LIN_rxBufferRead;
            loc_rxBufferWrite = LIN_rxBufferWrite;

            if( (LIN_rxBufferLoopDetect != 0u) || (loc_rxBufferRead != loc_rxBufferWrite) )
            {
                rxData = LIN_rxBuffer[loc_rxBufferRead];
                loc_rxBufferRead++;

                if(loc_rxBufferRead >= LIN_RXBUFFERSIZE)
                {
                    loc_rxBufferRead = 0u;
                }
                /* Update the real pointer */
                LIN_rxBufferRead = loc_rxBufferRead;

                if(LIN_rxBufferLoopDetect != 0u )
                {
                    LIN_rxBufferLoopDetect = 0u;
                    #if( (LIN_RX_INTERRUPT_ENABLED) && (LIN_FLOW_CONTROL != 0u) && \
                         (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH) )
                        /* When Hardware Flow Control selected - return RX mask */
                        #if( LIN_HD_ENABLED )
                            if((LIN_CONTROL_REG & LIN_CTRL_HD_SEND) == 0u)
                            {   /* In Half duplex mode return RX mask only in RX
                                *  configuration set, otherwise
                                *  mask will be returned in LoadRxConfig() API.
                                */
                                LIN_RXSTATUS_MASK_REG  |= LIN_RX_STS_FIFO_NOTEMPTY;
                            }
                        #else
                            LIN_RXSTATUS_MASK_REG  |= LIN_RX_STS_FIFO_NOTEMPTY;
                        #endif /* end LIN_HD_ENABLED */
                    #endif /* LIN_RX_INTERRUPT_ENABLED and Hardware flow control*/
                }
            }
            else
            {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit*/
                rxData = LIN_RXDATA_REG;
            }

            /* Enable Rx interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_EnableRxInt();
            #endif /* End LIN_RX_INTERRUPT_ENABLED */

        #else /* LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

            /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit*/
            rxData = LIN_RXDATA_REG;

        #endif /* LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: LIN_ReadRxStatus
    ********************************************************************************
    *
    * Summary:
    *  Read the current state of the status register
    *  And detect software buffer overflow.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Current state of the status register.
    *
    * Global Variables:
    *  LIN_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn?t free space in
    *   LIN_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   LIN_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 LIN_ReadRxStatus(void) 
    {
        uint8 status;

        status = LIN_RXSTATUS_REG & LIN_RX_HW_MASK;

        #if(LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH)
            if( LIN_rxBufferOverflow != 0u )
            {
                status |= LIN_RX_STS_SOFT_BUFF_OVER;
                LIN_rxBufferOverflow = 0u;
            }
        #endif /* LIN_RXBUFFERSIZE */

        return(status);
    }


    /*******************************************************************************
    * Function Name: LIN_GetChar
    ********************************************************************************
    *
    * Summary:
    *  Reads UART RX buffer immediately, if data is not available or an error
    *  condition exists, zero is returned; otherwise, character is read and
    *  returned.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Character read from UART RX buffer. ASCII characters from 1 to 255 are valid.
    *  A returned zero signifies an error condition or no data available.
    *
    * Global Variables:
    *  LIN_rxBuffer - RAM buffer pointer for save received data.
    *  LIN_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  LIN_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  LIN_rxBufferLoopDetect - creared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 LIN_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

        #if(LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH)
            uint8 loc_rxBufferRead;
            uint8 loc_rxBufferWrite;
            /* Protect variables that could change on interrupt. */
            /* Disable Rx interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_DisableRxInt();
            #endif /* LIN_RX_INTERRUPT_ENABLED */
            loc_rxBufferRead = LIN_rxBufferRead;
            loc_rxBufferWrite = LIN_rxBufferWrite;

            if( (LIN_rxBufferLoopDetect != 0u) || (loc_rxBufferRead != loc_rxBufferWrite) )
            {
                rxData = LIN_rxBuffer[loc_rxBufferRead];
                loc_rxBufferRead++;
                if(loc_rxBufferRead >= LIN_RXBUFFERSIZE)
                {
                    loc_rxBufferRead = 0u;
                }
                /* Update the real pointer */
                LIN_rxBufferRead = loc_rxBufferRead;

                if(LIN_rxBufferLoopDetect > 0u )
                {
                    LIN_rxBufferLoopDetect = 0u;
                    #if( (LIN_RX_INTERRUPT_ENABLED) && (LIN_FLOW_CONTROL != 0u) )
                        /* When Hardware Flow Control selected - return RX mask */
                        #if( LIN_HD_ENABLED )
                            if((LIN_CONTROL_REG & LIN_CTRL_HD_SEND) == 0u)
                            {   /* In Half duplex mode return RX mask only if
                                *  RX configuration set, otherwise
                                *  mask will be returned in LoadRxConfig() API.
                                */
                                LIN_RXSTATUS_MASK_REG  |= LIN_RX_STS_FIFO_NOTEMPTY;
                            }
                        #else
                            LIN_RXSTATUS_MASK_REG  |= LIN_RX_STS_FIFO_NOTEMPTY;
                        #endif /* end LIN_HD_ENABLED */
                    #endif /* LIN_RX_INTERRUPT_ENABLED and Hardware flow control*/
                }

            }
            else
            {   rxStatus = LIN_RXSTATUS_REG;
                if((rxStatus & LIN_RX_STS_FIFO_NOTEMPTY) != 0u)
                {   /* Read received data from FIFO*/
                    rxData = LIN_RXDATA_REG;
                    /*Check status on error*/
                    if((rxStatus & (LIN_RX_STS_BREAK | LIN_RX_STS_PAR_ERROR |
                                   LIN_RX_STS_STOP_ERROR | LIN_RX_STS_OVERRUN)) != 0u)
                    {
                        rxData = 0u;
                    }
                }
            }

            /* Enable Rx interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_EnableRxInt();
            #endif /* LIN_RX_INTERRUPT_ENABLED */

        #else /* LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

            rxStatus =LIN_RXSTATUS_REG;
            if((rxStatus & LIN_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO*/
                rxData = LIN_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (LIN_RX_STS_BREAK | LIN_RX_STS_PAR_ERROR |
                               LIN_RX_STS_STOP_ERROR | LIN_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        #endif /* LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: LIN_GetByte
    ********************************************************************************
    *
    * Summary:
    *  Grab the next available byte of data from the recieve FIFO
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  MSB contains Status Register and LSB contains UART RX data
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint16 LIN_GetByte(void) 
    {
        return ( ((uint16)LIN_ReadRxStatus() << 8u) | LIN_ReadRxData() );
    }


    /*******************************************************************************
    * Function Name: LIN_GetRxBufferSize
    ********************************************************************************
    *
    * Summary:
    *  Determine the amount of bytes left in the RX buffer and return the count in
    *  bytes
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  uint8: Integer count of the number of bytes left
    *  in the RX buffer
    *
    * Global Variables:
    *  LIN_rxBufferWrite - used to calculate left bytes.
    *  LIN_rxBufferRead - used to calculate left bytes.
    *  LIN_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint8 LIN_GetRxBufferSize(void)
                                                            
    {
        uint8 size;

        #if(LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH)

            /* Disable Rx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_DisableRxInt();
            #endif /* LIN_RX_INTERRUPT_ENABLED */

            if(LIN_rxBufferRead == LIN_rxBufferWrite)
            {
                if(LIN_rxBufferLoopDetect > 0u)
                {
                    size = LIN_RXBUFFERSIZE;
                }
                else
                {
                    size = 0u;
                }
            }
            else if(LIN_rxBufferRead < LIN_rxBufferWrite)
            {
                size = (LIN_rxBufferWrite - LIN_rxBufferRead);
            }
            else
            {
                size = (LIN_RXBUFFERSIZE - LIN_rxBufferRead) + LIN_rxBufferWrite;
            }

            /* Enable Rx interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_EnableRxInt();
            #endif /* End LIN_RX_INTERRUPT_ENABLED */

        #else /* LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

            /* We can only know if there is data in the fifo. */
            size = ((LIN_RXSTATUS_REG & LIN_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

        #endif /* End LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

        return(size);
    }


    /*******************************************************************************
    * Function Name: LIN_ClearRxBuffer
    ********************************************************************************
    *
    * Summary:
    *  Clears the RX RAM buffer by setting the read and write pointers both to zero.
    *  Clears hardware RX FIFO.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_rxBufferWrite - cleared to zero.
    *  LIN_rxBufferRead - cleared to zero.
    *  LIN_rxBufferLoopDetect - cleared to zero.
    *  LIN_rxBufferOverflow - cleared to zero.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Setting the pointers to zero makes the system believe there is no data to
    *  read and writing will resume at address 0 overwriting any data that may
    *  have remained in the RAM.
    *
    * Side Effects:
    *  Any received data not read from the RAM or FIFO buffer will be lost.
    *******************************************************************************/
    void LIN_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* clear the HW FIFO */
        /* Enter critical section */
        enableInterrupts = CyEnterCriticalSection();
        LIN_RXDATA_AUX_CTL_REG |=  LIN_RX_FIFO_CLR;
        LIN_RXDATA_AUX_CTL_REG &= (uint8)~LIN_RX_FIFO_CLR;
        /* Exit critical section */
        CyExitCriticalSection(enableInterrupts);

        #if(LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH)
            /* Disable Rx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_DisableRxInt();
            #endif /* End LIN_RX_INTERRUPT_ENABLED */

            LIN_rxBufferRead = 0u;
            LIN_rxBufferWrite = 0u;
            LIN_rxBufferLoopDetect = 0u;
            LIN_rxBufferOverflow = 0u;

            /* Enable Rx interrupt. */
            #if(LIN_RX_INTERRUPT_ENABLED)
                LIN_EnableRxInt();
            #endif /* End LIN_RX_INTERRUPT_ENABLED */
        #endif /* End LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH */

    }


    /*******************************************************************************
    * Function Name: LIN_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Set the receive addressing mode
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  LIN__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  LIN__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  LIN__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  LIN__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  LIN__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  LIN_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void LIN_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(LIN_RXHW_ADDRESS_ENABLED)
            #if(LIN_CONTROL_REG_REMOVED)
                if(addressMode != 0u) { }     /* release compiler warning */
            #else /* LIN_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = LIN_CONTROL_REG & (uint8)~LIN_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << LIN_CTRL_RXADDR_MODE0_SHIFT);
                LIN_CONTROL_REG = tmpCtrl;
                #if(LIN_RX_INTERRUPT_ENABLED && \
                   (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH) )
                    LIN_rxAddressMode = addressMode;
                    LIN_rxAddressDetected = 0u;
                #endif /* End LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH*/
            #endif /* End LIN_CONTROL_REG_REMOVED */
        #else /* LIN_RXHW_ADDRESS_ENABLED */
            if(addressMode != 0u) { }     /* release compiler warning */
        #endif /* End LIN_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: LIN_SetRxAddress1
    ********************************************************************************
    *
    * Summary:
    *  Set the first hardware address compare value
    *
    * Parameters:
    *  address
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void LIN_SetRxAddress1(uint8 address) 

    {
        LIN_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: LIN_SetRxAddress2
    ********************************************************************************
    *
    * Summary:
    *  Set the second hardware address compare value
    *
    * Parameters:
    *  address
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void LIN_SetRxAddress2(uint8 address) 
    {
        LIN_RXADDRESS2_REG = address;
    }

#endif  /* LIN_RX_ENABLED || LIN_HD_ENABLED*/


#if( (LIN_TX_ENABLED) || (LIN_HD_ENABLED) )

    #if(LIN_TX_INTERRUPT_ENABLED)

        /*******************************************************************************
        * Function Name: LIN_EnableTxInt
        ********************************************************************************
        *
        * Summary:
        *  Enable TX interrupt generation
        *
        * Parameters:
        *  None.
        *
        * Return:
        *  None.
        *
        * Theory:
        *  Enable the interrupt output -or- the interrupt component itself
        *
        *******************************************************************************/
        void LIN_EnableTxInt(void) 
        {
            CyIntEnable(LIN_TX_VECT_NUM);
        }


        /*******************************************************************************
        * Function Name: LIN_DisableTxInt
        ********************************************************************************
        *
        * Summary:
        *  Disable TX interrupt generation
        *
        * Parameters:
        *  None.
        *
        * Return:
        *  None.
        *
        * Theory:
        *  Disable the interrupt output -or- the interrupt component itself
        *
        *******************************************************************************/
        void LIN_DisableTxInt(void) 
        {
            CyIntDisable(LIN_TX_VECT_NUM);
        }

    #endif /* LIN_TX_INTERRUPT_ENABLED */


    /*******************************************************************************
    * Function Name: LIN_SetTxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configure which status bits trigger an interrupt event
    *
    * Parameters:
    *  intSrc: An or'd combination of the desired status bit masks (defined in
    *          the header file)
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void LIN_SetTxInterruptMode(uint8 intSrc) 
    {
        LIN_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: LIN_WriteTxData
    ********************************************************************************
    *
    * Summary:
    *  Write a byte of data to the Transmit FIFO or TX buffer to be sent when the
    *  bus is available. WriteTxData sends a byte without checking for buffer room
    *  or status. It is up to the user to separately check status.
    *
    * Parameters:
    *  TXDataByte: byte of data to place in the transmit FIFO
    *
    * Return:
    * void
    *
    * Global Variables:
    *  LIN_txBuffer - RAM buffer pointer for save data for transmission
    *  LIN_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  LIN_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  LIN_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void LIN_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(LIN_initVar != 0u)
        {
            #if(LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH)

                /* Disable Tx interrupt. */
                /* Protect variables that could change on interrupt. */
                #if(LIN_TX_INTERRUPT_ENABLED)
                    LIN_DisableTxInt();
                #endif /* End LIN_TX_INTERRUPT_ENABLED */

                if( (LIN_txBufferRead == LIN_txBufferWrite) &&
                    ((LIN_TXSTATUS_REG & LIN_TX_STS_FIFO_FULL) == 0u) )
                {
                    /* Add directly to the FIFO. */
                    LIN_TXDATA_REG = txDataByte;
                }
                else
                {
                    if(LIN_txBufferWrite >= LIN_TXBUFFERSIZE)
                    {
                        LIN_txBufferWrite = 0u;
                    }

                    LIN_txBuffer[LIN_txBufferWrite] = txDataByte;

                    /* Add to the software buffer. */
                    LIN_txBufferWrite++;

                }

                /* Enable Tx interrupt. */
                #if(LIN_TX_INTERRUPT_ENABLED)
                    LIN_EnableTxInt();
                #endif /* End LIN_TX_INTERRUPT_ENABLED */

            #else /* LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */

                /* Add directly to the FIFO. */
                LIN_TXDATA_REG = txDataByte;

            #endif /* End LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */
        }
    }


    /*******************************************************************************
    * Function Name: LIN_ReadTxStatus
    ********************************************************************************
    *
    * Summary:
    *  Read the status register for the component
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Contents of the status register
    *
    * Theory:
    *  This function reads the status register which is clear on read. It is up to
    *  the user to handle all bits in this return value accordingly, even if the bit
    *  was not enabled as an interrupt source the event happened and must be handled
    *  accordingly.
    *
    *******************************************************************************/
    uint8 LIN_ReadTxStatus(void) 
    {
        return(LIN_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: LIN_PutChar
    ********************************************************************************
    *
    * Summary:
    *  Wait to send byte until TX register or buffer has room.
    *
    * Parameters:
    *  txDataByte: The 8-bit data value to send across the UART.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_txBuffer - RAM buffer pointer for save data for transmission
    *  LIN_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  LIN_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  LIN_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void LIN_PutChar(uint8 txDataByte) 
    {
            #if(LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH)
                /* The temporary output pointer is used since it takes two instructions
                *  to increment with a wrap, and we can't risk doing that with the real
                *  pointer and getting an interrupt in between instructions.
                */
                uint8 loc_txBufferWrite;
                uint8 loc_txBufferRead;

                do{
                    /* Block if software buffer is full, so we don't overwrite. */
                    #if ((LIN_TXBUFFERSIZE > LIN_MAX_BYTE_VALUE) && (CY_PSOC3))
                        /* Disable TX interrupt to protect variables that could change on interrupt */
                        CyIntDisable(LIN_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                    loc_txBufferWrite = LIN_txBufferWrite;
                    loc_txBufferRead = LIN_txBufferRead;
                    #if ((LIN_TXBUFFERSIZE > LIN_MAX_BYTE_VALUE) && (CY_PSOC3))
                        /* Enable interrupt to continue transmission */
                        CyIntEnable(LIN_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                }while( (loc_txBufferWrite < loc_txBufferRead) ? (loc_txBufferWrite == (loc_txBufferRead - 1u)) :
                                        ((loc_txBufferWrite - loc_txBufferRead) ==
                                        (uint8)(LIN_TXBUFFERSIZE - 1u)) );

                if( (loc_txBufferRead == loc_txBufferWrite) &&
                    ((LIN_TXSTATUS_REG & LIN_TX_STS_FIFO_FULL) == 0u) )
                {
                    /* Add directly to the FIFO. */
                    LIN_TXDATA_REG = txDataByte;
                }
                else
                {
                    if(loc_txBufferWrite >= LIN_TXBUFFERSIZE)
                    {
                        loc_txBufferWrite = 0u;
                    }
                    /* Add to the software buffer. */
                    LIN_txBuffer[loc_txBufferWrite] = txDataByte;
                    loc_txBufferWrite++;

                    /* Finally, update the real output pointer */
                    #if ((LIN_TXBUFFERSIZE > LIN_MAX_BYTE_VALUE) && (CY_PSOC3))
                        CyIntDisable(LIN_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                    LIN_txBufferWrite = loc_txBufferWrite;
                    #if ((LIN_TXBUFFERSIZE > LIN_MAX_BYTE_VALUE) && (CY_PSOC3))
                        CyIntEnable(LIN_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                }

            #else /* LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */

                while((LIN_TXSTATUS_REG & LIN_TX_STS_FIFO_FULL) != 0u)
                {
                    ; /* Wait for room in the FIFO. */
                }

                /* Add directly to the FIFO. */
                LIN_TXDATA_REG = txDataByte;

            #endif /* End LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */
    }


    /*******************************************************************************
    * Function Name: LIN_PutString
    ********************************************************************************
    *
    * Summary:
    *  Write a Sequence of bytes on the Transmit line. Data comes from RAM or ROM.
    *
    * Parameters:
    *  string: char pointer to character string of Data to Send.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  This function will block if there is not enough memory to place the whole
    *  string, it will block until the entire string has been written to the
    *  transmit buffer.
    *
    *******************************************************************************/
    void LIN_PutString(const char8 string[]) 
    {
        uint16 buf_index = 0u;
        /* If not Initialized then skip this function*/
        if(LIN_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent*/
            while(string[buf_index] != (char8)0)
            {
                LIN_PutChar((uint8)string[buf_index]);
                buf_index++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: LIN_PutArray
    ********************************************************************************
    *
    * Summary:
    *  Write a Sequence of bytes on the Transmit line. Data comes from RAM or ROM.
    *
    * Parameters:
    *  string: Address of the memory array residing in RAM or ROM.
    *  byteCount: Number of Bytes to be transmitted.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void LIN_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 buf_index = 0u;
        /* If not Initialized then skip this function*/
        if(LIN_initVar != 0u)
        {
            do
            {
                LIN_PutChar(string[buf_index]);
                buf_index++;
            }while(buf_index < byteCount);
        }
    }


    /*******************************************************************************
    * Function Name: LIN_PutCRLF
    ********************************************************************************
    *
    * Summary:
    *  Write a character and then carriage return and line feed.
    *
    * Parameters:
    *  txDataByte: uint8 Character to send.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void LIN_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(LIN_initVar != 0u)
        {
            LIN_PutChar(txDataByte);
            LIN_PutChar(0x0Du);
            LIN_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: LIN_GetTxBufferSize
    ********************************************************************************
    *
    * Summary:
    *  Determine the amount of space left in the TX buffer and return the count in
    *  bytes
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Integer count of the number of bytes left in the TX buffer
    *
    * Global Variables:
    *  LIN_txBufferWrite - used to calculate left space.
    *  LIN_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 LIN_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

        #if(LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH)

            /* Disable Tx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(LIN_TX_INTERRUPT_ENABLED)
                LIN_DisableTxInt();
            #endif /* End LIN_TX_INTERRUPT_ENABLED */

            if(LIN_txBufferRead == LIN_txBufferWrite)
            {
                size = 0u;
            }
            else if(LIN_txBufferRead < LIN_txBufferWrite)
            {
                size = (LIN_txBufferWrite - LIN_txBufferRead);
            }
            else
            {
                size = (LIN_TXBUFFERSIZE - LIN_txBufferRead) + LIN_txBufferWrite;
            }

            /* Enable Tx interrupt. */
            #if(LIN_TX_INTERRUPT_ENABLED)
                LIN_EnableTxInt();
            #endif /* End LIN_TX_INTERRUPT_ENABLED */

        #else /* LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */

            size = LIN_TXSTATUS_REG;

            /* Is the fifo is full. */
            if((size & LIN_TX_STS_FIFO_FULL) != 0u)
            {
                size = LIN_FIFO_LENGTH;
            }
            else if((size & LIN_TX_STS_FIFO_EMPTY) != 0u)
            {
                size = 0u;
            }
            else
            {
                /* We only know there is data in the fifo. */
                size = 1u;
            }

        #endif /* End LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */

        return(size);
    }


    /*******************************************************************************
    * Function Name: LIN_ClearTxBuffer
    ********************************************************************************
    *
    * Summary:
    *  Clears the TX RAM buffer by setting the read and write pointers both to zero.
    *  Clears the hardware TX FIFO.  Any data present in the FIFO will not be sent.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_txBufferWrite - cleared to zero.
    *  LIN_txBufferRead - cleared to zero.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Setting the pointers to zero makes the system believe there is no data to
    *  read and writing will resume at address 0 overwriting any data that may have
    *  remained in the RAM.
    *
    * Side Effects:
    *  Any received data not read from the RAM buffer will be lost when overwritten.
    *
    *******************************************************************************/
    void LIN_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Enter critical section */
        enableInterrupts = CyEnterCriticalSection();
        /* clear the HW FIFO */
        LIN_TXDATA_AUX_CTL_REG |=  LIN_TX_FIFO_CLR;
        LIN_TXDATA_AUX_CTL_REG &= (uint8)~LIN_TX_FIFO_CLR;
        /* Exit critical section */
        CyExitCriticalSection(enableInterrupts);

        #if(LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH)

            /* Disable Tx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(LIN_TX_INTERRUPT_ENABLED)
                LIN_DisableTxInt();
            #endif /* End LIN_TX_INTERRUPT_ENABLED */

            LIN_txBufferRead = 0u;
            LIN_txBufferWrite = 0u;

            /* Enable Tx interrupt. */
            #if(LIN_TX_INTERRUPT_ENABLED)
                LIN_EnableTxInt();
            #endif /* End LIN_TX_INTERRUPT_ENABLED */

        #endif /* End LIN_TXBUFFERSIZE > LIN_FIFO_LENGTH */
    }


    /*******************************************************************************
    * Function Name: LIN_SendBreak
    ********************************************************************************
    *
    * Summary:
    *  Write a Break command to the UART
    *
    * Parameters:
    *  uint8 retMode:  Wait mode,
    *   0 - Initialize registers for Break, sends the Break signal and return
    *       imediately.
    *   1 - Wait until Break sending is complete, reinitialize registers to normal
    *       transmission mode then return.
    *   2 - Reinitialize registers to normal transmission mode then return.
    *   3 - both steps: 0 and 1
    *       init registers for Break, send Break signal
    *       wait until Break sending is complete, reinit registers to normal
    *       transmission mode then return.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  LIN_initVar - checked to identify that the component has been
    *     initialized.
    *  tx_period - static variable, used for keeping TX period configuration.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  SendBreak function initializes registers to send 13-bit break signal. It is
    *  important to return the registers configuration to normal for continue 8-bit
    *  operation.
    *  Trere are 3 variants for this API usage:
    *  1) SendBreak(3) - function will send the Break signal and take care on the
    *     configuration returning. Funcition will block CPU untill transmition
    *     complete.
    *  2) User may want to use bloking time if UART configured to the low speed
    *     operation
    *     Emample for this case:
    *     SendBreak(0);     - init Break signal transmition
    *         Add your code here to use CPU time
    *     SendBreak(1);     - complete Break operation
    *  3) Same to 2) but user may want to init and use the interrupt for complete
    *     break operation.
    *     Example for this case:
    *     Init TX interrupt whith "TX - On TX Complete" parameter
    *     SendBreak(0);     - init Break signal transmition
    *         Add your code here to use CPU time
    *     When interrupt appear with UART_TX_STS_COMPLETE status:
    *     SendBreak(2);     - complete Break operation
    *
    * Side Effects:
    *   Uses static variable to keep registers configuration.
    *
    *******************************************************************************/
    void LIN_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(LIN_initVar != 0u)
        {
            /*Set the Counter to 13-bits and transmit a 00 byte*/
            /*When that is done then reset the counter value back*/
            uint8 tmpStat;

            #if(LIN_HD_ENABLED) /* Half Duplex mode*/

                if( (retMode == LIN_SEND_BREAK) ||
                    (retMode == LIN_SEND_WAIT_REINIT ) )
                {
                    /* CTRL_HD_SEND_BREAK - sends break bits in HD mode*/
                    LIN_WriteControlRegister(LIN_ReadControlRegister() |
                                                          LIN_CTRL_HD_SEND_BREAK);
                    /* Send zeros*/
                    LIN_TXDATA_REG = 0u;

                    do /*wait until transmit starts*/
                    {
                        tmpStat = LIN_TXSTATUS_REG;
                    }while((tmpStat & LIN_TX_STS_FIFO_EMPTY) != 0u);
                }

                if( (retMode == LIN_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == LIN_SEND_WAIT_REINIT) )
                {
                    do /*wait until transmit complete*/
                    {
                        tmpStat = LIN_TXSTATUS_REG;
                    }while(((uint8)~tmpStat & LIN_TX_STS_COMPLETE) != 0u);
                }

                if( (retMode == LIN_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == LIN_REINIT) ||
                    (retMode == LIN_SEND_WAIT_REINIT) )
                {
                    LIN_WriteControlRegister(LIN_ReadControlRegister() &
                                                  (uint8)~LIN_CTRL_HD_SEND_BREAK);
                }

            #else /* LIN_HD_ENABLED Full Duplex mode */

                static uint8 tx_period;

                if( (retMode == LIN_SEND_BREAK) ||
                    (retMode == LIN_SEND_WAIT_REINIT) )
                {
                    /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode*/
                    #if( (LIN_PARITY_TYPE != LIN__B_UART__NONE_REVB) || \
                                        (LIN_PARITY_TYPE_SW != 0u) )
                        LIN_WriteControlRegister(LIN_ReadControlRegister() |
                                                              LIN_CTRL_HD_SEND_BREAK);
                    #endif /* End LIN_PARITY_TYPE != LIN__B_UART__NONE_REVB  */

                    #if(LIN_TXCLKGEN_DP)
                        tx_period = LIN_TXBITCLKTX_COMPLETE_REG;
                        LIN_TXBITCLKTX_COMPLETE_REG = LIN_TXBITCTR_BREAKBITS;
                    #else
                        tx_period = LIN_TXBITCTR_PERIOD_REG;
                        LIN_TXBITCTR_PERIOD_REG = LIN_TXBITCTR_BREAKBITS8X;
                    #endif /* End LIN_TXCLKGEN_DP */

                    /* Send zeros*/
                    LIN_TXDATA_REG = 0u;

                    do /* wait until transmit starts */
                    {
                        tmpStat = LIN_TXSTATUS_REG;
                    }while((tmpStat & LIN_TX_STS_FIFO_EMPTY) != 0u);
                }

                if( (retMode == LIN_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == LIN_SEND_WAIT_REINIT) )
                {
                    do /*wait until transmit complete*/
                    {
                        tmpStat = LIN_TXSTATUS_REG;
                    }while(((uint8)~tmpStat & LIN_TX_STS_COMPLETE) != 0u);
                }

                if( (retMode == LIN_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == LIN_REINIT) ||
                    (retMode == LIN_SEND_WAIT_REINIT) )
                {

                    #if(LIN_TXCLKGEN_DP)
                        LIN_TXBITCLKTX_COMPLETE_REG = tx_period;
                    #else
                        LIN_TXBITCTR_PERIOD_REG = tx_period;
                    #endif /* End LIN_TXCLKGEN_DP */

                    #if( (LIN_PARITY_TYPE != LIN__B_UART__NONE_REVB) || \
                         (LIN_PARITY_TYPE_SW != 0u) )
                        LIN_WriteControlRegister(LIN_ReadControlRegister() &
                                                      (uint8)~LIN_CTRL_HD_SEND_BREAK);
                    #endif /* End LIN_PARITY_TYPE != NONE */
                }
            #endif    /* End LIN_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: LIN_SetTxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Set the transmit addressing mode
    *
    * Parameters:
    *  addressMode: 0 -> Space
    *               1 -> Mark
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void LIN_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable*/
        if(addressMode != 0u)
        {
            #if( LIN_CONTROL_REG_REMOVED == 0u )
                LIN_WriteControlRegister(LIN_ReadControlRegister() |
                                                      LIN_CTRL_MARK);
            #endif /* End LIN_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
            #if( LIN_CONTROL_REG_REMOVED == 0u )
                LIN_WriteControlRegister(LIN_ReadControlRegister() &
                                                    (uint8)~LIN_CTRL_MARK);
            #endif /* End LIN_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndLIN_TX_ENABLED */

#if(LIN_HD_ENABLED)


    /*******************************************************************************
    * Function Name: LIN_LoadTxConfig
    ********************************************************************************
    *
    * Summary:
    *  Unloads the Rx configuration if required and loads the
    *  Tx configuration. It is the users responsibility to ensure that any
    *  transaction is complete and it is safe to unload the Tx
    *  configuration.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Valid only for half duplex UART.
    *
    * Side Effects:
    *  Disable RX interrupt mask, when software buffer has been used.
    *
    *******************************************************************************/
    void LIN_LoadTxConfig(void) 
    {
        #if((LIN_RX_INTERRUPT_ENABLED) && (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH))
            /* Disable RX interrupts before set TX configuration */
            LIN_SetRxInterruptMode(0u);
        #endif /* LIN_RX_INTERRUPT_ENABLED */

        LIN_WriteControlRegister(LIN_ReadControlRegister() | LIN_CTRL_HD_SEND);
        LIN_RXBITCTR_PERIOD_REG = LIN_HD_TXBITCTR_INIT;
        #if(CY_UDB_V0) /* Manually clear status register when mode has been changed */
            /* Clear status register */
            CY_GET_REG8(LIN_RXSTATUS_PTR);
        #endif /* CY_UDB_V0 */
    }


    /*******************************************************************************
    * Function Name: LIN_LoadRxConfig
    ********************************************************************************
    *
    * Summary:
    *  Unloads the Tx configuration if required and loads the
    *  Rx configuration. It is the users responsibility to ensure that any
    *  transaction is complete and it is safe to unload the Rx
    *  configuration.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Valid only for half duplex UART
    *
    * Side Effects:
    *  Set RX interrupt mask based on customizer settings, when software buffer
    *  has been used.
    *
    *******************************************************************************/
    void LIN_LoadRxConfig(void) 
    {
        LIN_WriteControlRegister(LIN_ReadControlRegister() &
                                                (uint8)~LIN_CTRL_HD_SEND);
        LIN_RXBITCTR_PERIOD_REG = LIN_HD_RXBITCTR_INIT;
        #if(CY_UDB_V0) /* Manually clear status register when mode has been changed */
            /* Clear status register */
            CY_GET_REG8(LIN_RXSTATUS_PTR);
        #endif /* CY_UDB_V0 */

        #if((LIN_RX_INTERRUPT_ENABLED) && (LIN_RXBUFFERSIZE > LIN_FIFO_LENGTH))
            /* Enable RX interrupt after set RX configuration */
            LIN_SetRxInterruptMode(LIN_INIT_RX_INTERRUPTS_MASK);
        #endif /* LIN_RX_INTERRUPT_ENABLED */
    }

#endif  /* LIN_HD_ENABLED */


/* [] END OF FILE */
