/*******************************************************************************
* File Name: Line.c
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

#include "Line.h"
#include "CyLib.h"
#if(Line_INTERNAL_CLOCK_USED)
    #include "Line_IntClock.h"
#endif /* End Line_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 Line_initVar = 0u;
#if( Line_TX_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
    volatile uint8 Line_txBuffer[Line_TXBUFFERSIZE];
    volatile uint8 Line_txBufferRead = 0u;
    uint8 Line_txBufferWrite = 0u;
#endif /* End Line_TX_ENABLED */
#if( ( Line_RX_ENABLED || Line_HD_ENABLED ) && \
     (Line_RXBUFFERSIZE > Line_FIFO_LENGTH) )
    volatile uint8 Line_rxBuffer[Line_RXBUFFERSIZE];
    volatile uint8 Line_rxBufferRead = 0u;
    volatile uint8 Line_rxBufferWrite = 0u;
    volatile uint8 Line_rxBufferLoopDetect = 0u;
    volatile uint8 Line_rxBufferOverflow = 0u;
    #if (Line_RXHW_ADDRESS_ENABLED)
        volatile uint8 Line_rxAddressMode = Line_RXADDRESSMODE;
        volatile uint8 Line_rxAddressDetected = 0u;
    #endif /* End EnableHWAddress */
#endif /* End Line_RX_ENABLED */


/*******************************************************************************
* Function Name: Line_Start
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
*  The Line_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time UART_Start() is called. This allows for
*  component initialization without re-initialization in all subsequent calls
*  to the Line_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Line_Start(void) 
{
    /* If not Initialized then initialize all required hardware and software */
    if(Line_initVar == 0u)
    {
        Line_Init();
        Line_initVar = 1u;
    }
    Line_Enable();
}


/*******************************************************************************
* Function Name: Line_Init
********************************************************************************
*
* Summary:
*  Initialize component's parameters to the parameters set by user in the
*  customizer of the component placed onto schematic. Usually called in
*  Line_Start().
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void Line_Init(void) 
{
    #if(Line_RX_ENABLED || Line_HD_ENABLED)

        #if(Line_RX_INTERRUPT_ENABLED && (Line_RXBUFFERSIZE > Line_FIFO_LENGTH))
            /* Set the RX Interrupt. */
            (void)CyIntSetVector(Line_RX_VECT_NUM, &Line_RXISR);
            CyIntSetPriority(Line_RX_VECT_NUM, Line_RX_PRIOR_NUM);
        #endif /* End Line_RX_INTERRUPT_ENABLED */

        #if (Line_RXHW_ADDRESS_ENABLED)
            Line_SetRxAddressMode(Line_RXAddressMode);
            Line_SetRxAddress1(Line_RXHWADDRESS1);
            Line_SetRxAddress2(Line_RXHWADDRESS2);
        #endif /* End Line_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        Line_RXBITCTR_PERIOD_REG = Line_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        Line_RXSTATUS_MASK_REG  = Line_INIT_RX_INTERRUPTS_MASK;
    #endif /* End Line_RX_ENABLED || Line_HD_ENABLED*/

    #if(Line_TX_ENABLED)
        #if(Line_TX_INTERRUPT_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
            /* Set the TX Interrupt. */
            (void)CyIntSetVector(Line_TX_VECT_NUM, &Line_TXISR);
            CyIntSetPriority(Line_TX_VECT_NUM, Line_TX_PRIOR_NUM);
        #endif /* End Line_TX_INTERRUPT_ENABLED */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if(Line_TXCLKGEN_DP)
            Line_TXBITCLKGEN_CTR_REG = Line_BIT_CENTER;
            Line_TXBITCLKTX_COMPLETE_REG = (Line_NUMBER_OF_DATA_BITS +
                        Line_NUMBER_OF_START_BIT) * Line_OVER_SAMPLE_COUNT;
        #else
            Line_TXBITCTR_PERIOD_REG = ((Line_NUMBER_OF_DATA_BITS +
                        Line_NUMBER_OF_START_BIT) * Line_OVER_SAMPLE_8) - 1u;
        #endif /* End Line_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if(Line_TX_INTERRUPT_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
            Line_TXSTATUS_MASK_REG = Line_TX_STS_FIFO_EMPTY;
        #else
            Line_TXSTATUS_MASK_REG = Line_INIT_TX_INTERRUPTS_MASK;
        #endif /*End Line_TX_INTERRUPT_ENABLED*/

    #endif /* End Line_TX_ENABLED */

    #if(Line_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        Line_WriteControlRegister( \
            (Line_ReadControlRegister() & (uint8)~Line_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(Line_PARITY_TYPE << Line_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End Line_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: Line_Enable
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
*  Line_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void Line_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if(Line_RX_ENABLED || Line_HD_ENABLED)
        /*RX Counter (Count7) Enable */
        Line_RXBITCTR_CONTROL_REG |= Line_CNTR_ENABLE;
        /* Enable the RX Interrupt. */
        Line_RXSTATUS_ACTL_REG  |= Line_INT_ENABLE;
        #if(Line_RX_INTERRUPT_ENABLED && (Line_RXBUFFERSIZE > Line_FIFO_LENGTH))
            CyIntEnable(Line_RX_VECT_NUM);
            #if (Line_RXHW_ADDRESS_ENABLED)
                Line_rxAddressDetected = 0u;
            #endif /* End Line_RXHW_ADDRESS_ENABLED */
        #endif /* End Line_RX_INTERRUPT_ENABLED */
    #endif /* End Line_RX_ENABLED || Line_HD_ENABLED*/

    #if(Line_TX_ENABLED)
        /*TX Counter (DP/Count7) Enable */
        #if(!Line_TXCLKGEN_DP)
            Line_TXBITCTR_CONTROL_REG |= Line_CNTR_ENABLE;
        #endif /* End Line_TXCLKGEN_DP */
        /* Enable the TX Interrupt. */
        Line_TXSTATUS_ACTL_REG |= Line_INT_ENABLE;
        #if(Line_TX_INTERRUPT_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
            CyIntEnable(Line_TX_VECT_NUM);
        #endif /* End Line_TX_INTERRUPT_ENABLED*/
     #endif /* End Line_TX_ENABLED */

    #if(Line_INTERNAL_CLOCK_USED)
        /* Enable the clock. */
        Line_IntClock_Start();
    #endif /* End Line_INTERNAL_CLOCK_USED */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Line_Stop
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
void Line_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if(Line_RX_ENABLED || Line_HD_ENABLED)
        Line_RXBITCTR_CONTROL_REG &= (uint8)~Line_CNTR_ENABLE;
    #endif /* End Line_RX_ENABLED */

    #if(Line_TX_ENABLED)
        #if(!Line_TXCLKGEN_DP)
            Line_TXBITCTR_CONTROL_REG &= (uint8)~Line_CNTR_ENABLE;
        #endif /* End Line_TXCLKGEN_DP */
    #endif /* Line_TX_ENABLED */

    #if(Line_INTERNAL_CLOCK_USED)
        /* Disable the clock. */
        Line_IntClock_Stop();
    #endif /* End Line_INTERNAL_CLOCK_USED */

    /* Disable internal interrupt component */
    #if(Line_RX_ENABLED || Line_HD_ENABLED)
        Line_RXSTATUS_ACTL_REG  &= (uint8)~Line_INT_ENABLE;
        #if(Line_RX_INTERRUPT_ENABLED && (Line_RXBUFFERSIZE > Line_FIFO_LENGTH))
            Line_DisableRxInt();
        #endif /* End Line_RX_INTERRUPT_ENABLED */
    #endif /* End Line_RX_ENABLED */

    #if(Line_TX_ENABLED)
        Line_TXSTATUS_ACTL_REG &= (uint8)~Line_INT_ENABLE;
        #if(Line_TX_INTERRUPT_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
            Line_DisableTxInt();
        #endif /* End Line_TX_INTERRUPT_ENABLED */
    #endif /* End Line_TX_ENABLED */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Line_ReadControlRegister
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
uint8 Line_ReadControlRegister(void) 
{
    #if( Line_CONTROL_REG_REMOVED )
        return(0u);
    #else
        return(Line_CONTROL_REG);
    #endif /* End Line_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Line_WriteControlRegister
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
void  Line_WriteControlRegister(uint8 control) 
{
    #if( Line_CONTROL_REG_REMOVED )
        if(control != 0u) { }      /* release compiler warning */
    #else
       Line_CONTROL_REG = control;
    #endif /* End Line_CONTROL_REG_REMOVED */
}


#if(Line_RX_ENABLED || Line_HD_ENABLED)

    #if(Line_RX_INTERRUPT_ENABLED)

        /*******************************************************************************
        * Function Name: Line_EnableRxInt
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
        void Line_EnableRxInt(void) 
        {
            CyIntEnable(Line_RX_VECT_NUM);
        }


        /*******************************************************************************
        * Function Name: Line_DisableRxInt
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
        void Line_DisableRxInt(void) 
        {
            CyIntDisable(Line_RX_VECT_NUM);
        }

    #endif /* Line_RX_INTERRUPT_ENABLED */


    /*******************************************************************************
    * Function Name: Line_SetRxInterruptMode
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
    void Line_SetRxInterruptMode(uint8 intSrc) 
    {
        Line_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: Line_ReadRxData
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
    *  Line_rxBuffer - RAM buffer pointer for save received data.
    *  Line_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Line_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Line_rxBufferLoopDetect - creared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Line_ReadRxData(void) 
    {
        uint8 rxData;

        #if(Line_RXBUFFERSIZE > Line_FIFO_LENGTH)
            uint8 loc_rxBufferRead;
            uint8 loc_rxBufferWrite;
            /* Protect variables that could change on interrupt. */
            /* Disable Rx interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_DisableRxInt();
            #endif /* Line_RX_INTERRUPT_ENABLED */
            loc_rxBufferRead = Line_rxBufferRead;
            loc_rxBufferWrite = Line_rxBufferWrite;

            if( (Line_rxBufferLoopDetect != 0u) || (loc_rxBufferRead != loc_rxBufferWrite) )
            {
                rxData = Line_rxBuffer[loc_rxBufferRead];
                loc_rxBufferRead++;

                if(loc_rxBufferRead >= Line_RXBUFFERSIZE)
                {
                    loc_rxBufferRead = 0u;
                }
                /* Update the real pointer */
                Line_rxBufferRead = loc_rxBufferRead;

                if(Line_rxBufferLoopDetect != 0u )
                {
                    Line_rxBufferLoopDetect = 0u;
                    #if( (Line_RX_INTERRUPT_ENABLED) && (Line_FLOW_CONTROL != 0u) && \
                         (Line_RXBUFFERSIZE > Line_FIFO_LENGTH) )
                        /* When Hardware Flow Control selected - return RX mask */
                        #if( Line_HD_ENABLED )
                            if((Line_CONTROL_REG & Line_CTRL_HD_SEND) == 0u)
                            {   /* In Half duplex mode return RX mask only in RX
                                *  configuration set, otherwise
                                *  mask will be returned in LoadRxConfig() API.
                                */
                                Line_RXSTATUS_MASK_REG  |= Line_RX_STS_FIFO_NOTEMPTY;
                            }
                        #else
                            Line_RXSTATUS_MASK_REG  |= Line_RX_STS_FIFO_NOTEMPTY;
                        #endif /* end Line_HD_ENABLED */
                    #endif /* Line_RX_INTERRUPT_ENABLED and Hardware flow control*/
                }
            }
            else
            {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit*/
                rxData = Line_RXDATA_REG;
            }

            /* Enable Rx interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_EnableRxInt();
            #endif /* End Line_RX_INTERRUPT_ENABLED */

        #else /* Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

            /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit*/
            rxData = Line_RXDATA_REG;

        #endif /* Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Line_ReadRxStatus
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
    *  Line_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn?t free space in
    *   Line_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   Line_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 Line_ReadRxStatus(void) 
    {
        uint8 status;

        status = Line_RXSTATUS_REG & Line_RX_HW_MASK;

        #if(Line_RXBUFFERSIZE > Line_FIFO_LENGTH)
            if( Line_rxBufferOverflow != 0u )
            {
                status |= Line_RX_STS_SOFT_BUFF_OVER;
                Line_rxBufferOverflow = 0u;
            }
        #endif /* Line_RXBUFFERSIZE */

        return(status);
    }


    /*******************************************************************************
    * Function Name: Line_GetChar
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
    *  Line_rxBuffer - RAM buffer pointer for save received data.
    *  Line_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Line_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Line_rxBufferLoopDetect - creared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Line_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

        #if(Line_RXBUFFERSIZE > Line_FIFO_LENGTH)
            uint8 loc_rxBufferRead;
            uint8 loc_rxBufferWrite;
            /* Protect variables that could change on interrupt. */
            /* Disable Rx interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_DisableRxInt();
            #endif /* Line_RX_INTERRUPT_ENABLED */
            loc_rxBufferRead = Line_rxBufferRead;
            loc_rxBufferWrite = Line_rxBufferWrite;

            if( (Line_rxBufferLoopDetect != 0u) || (loc_rxBufferRead != loc_rxBufferWrite) )
            {
                rxData = Line_rxBuffer[loc_rxBufferRead];
                loc_rxBufferRead++;
                if(loc_rxBufferRead >= Line_RXBUFFERSIZE)
                {
                    loc_rxBufferRead = 0u;
                }
                /* Update the real pointer */
                Line_rxBufferRead = loc_rxBufferRead;

                if(Line_rxBufferLoopDetect > 0u )
                {
                    Line_rxBufferLoopDetect = 0u;
                    #if( (Line_RX_INTERRUPT_ENABLED) && (Line_FLOW_CONTROL != 0u) )
                        /* When Hardware Flow Control selected - return RX mask */
                        #if( Line_HD_ENABLED )
                            if((Line_CONTROL_REG & Line_CTRL_HD_SEND) == 0u)
                            {   /* In Half duplex mode return RX mask only if
                                *  RX configuration set, otherwise
                                *  mask will be returned in LoadRxConfig() API.
                                */
                                Line_RXSTATUS_MASK_REG  |= Line_RX_STS_FIFO_NOTEMPTY;
                            }
                        #else
                            Line_RXSTATUS_MASK_REG  |= Line_RX_STS_FIFO_NOTEMPTY;
                        #endif /* end Line_HD_ENABLED */
                    #endif /* Line_RX_INTERRUPT_ENABLED and Hardware flow control*/
                }

            }
            else
            {   rxStatus = Line_RXSTATUS_REG;
                if((rxStatus & Line_RX_STS_FIFO_NOTEMPTY) != 0u)
                {   /* Read received data from FIFO*/
                    rxData = Line_RXDATA_REG;
                    /*Check status on error*/
                    if((rxStatus & (Line_RX_STS_BREAK | Line_RX_STS_PAR_ERROR |
                                   Line_RX_STS_STOP_ERROR | Line_RX_STS_OVERRUN)) != 0u)
                    {
                        rxData = 0u;
                    }
                }
            }

            /* Enable Rx interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_EnableRxInt();
            #endif /* Line_RX_INTERRUPT_ENABLED */

        #else /* Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

            rxStatus =Line_RXSTATUS_REG;
            if((rxStatus & Line_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO*/
                rxData = Line_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (Line_RX_STS_BREAK | Line_RX_STS_PAR_ERROR |
                               Line_RX_STS_STOP_ERROR | Line_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        #endif /* Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Line_GetByte
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
    uint16 Line_GetByte(void) 
    {
        return ( ((uint16)Line_ReadRxStatus() << 8u) | Line_ReadRxData() );
    }


    /*******************************************************************************
    * Function Name: Line_GetRxBufferSize
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
    *  Line_rxBufferWrite - used to calculate left bytes.
    *  Line_rxBufferRead - used to calculate left bytes.
    *  Line_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint8 Line_GetRxBufferSize(void)
                                                            
    {
        uint8 size;

        #if(Line_RXBUFFERSIZE > Line_FIFO_LENGTH)

            /* Disable Rx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_DisableRxInt();
            #endif /* Line_RX_INTERRUPT_ENABLED */

            if(Line_rxBufferRead == Line_rxBufferWrite)
            {
                if(Line_rxBufferLoopDetect > 0u)
                {
                    size = Line_RXBUFFERSIZE;
                }
                else
                {
                    size = 0u;
                }
            }
            else if(Line_rxBufferRead < Line_rxBufferWrite)
            {
                size = (Line_rxBufferWrite - Line_rxBufferRead);
            }
            else
            {
                size = (Line_RXBUFFERSIZE - Line_rxBufferRead) + Line_rxBufferWrite;
            }

            /* Enable Rx interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_EnableRxInt();
            #endif /* End Line_RX_INTERRUPT_ENABLED */

        #else /* Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

            /* We can only know if there is data in the fifo. */
            size = ((Line_RXSTATUS_REG & Line_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

        #endif /* End Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

        return(size);
    }


    /*******************************************************************************
    * Function Name: Line_ClearRxBuffer
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
    *  Line_rxBufferWrite - cleared to zero.
    *  Line_rxBufferRead - cleared to zero.
    *  Line_rxBufferLoopDetect - cleared to zero.
    *  Line_rxBufferOverflow - cleared to zero.
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
    void Line_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* clear the HW FIFO */
        /* Enter critical section */
        enableInterrupts = CyEnterCriticalSection();
        Line_RXDATA_AUX_CTL_REG |=  Line_RX_FIFO_CLR;
        Line_RXDATA_AUX_CTL_REG &= (uint8)~Line_RX_FIFO_CLR;
        /* Exit critical section */
        CyExitCriticalSection(enableInterrupts);

        #if(Line_RXBUFFERSIZE > Line_FIFO_LENGTH)
            /* Disable Rx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_DisableRxInt();
            #endif /* End Line_RX_INTERRUPT_ENABLED */

            Line_rxBufferRead = 0u;
            Line_rxBufferWrite = 0u;
            Line_rxBufferLoopDetect = 0u;
            Line_rxBufferOverflow = 0u;

            /* Enable Rx interrupt. */
            #if(Line_RX_INTERRUPT_ENABLED)
                Line_EnableRxInt();
            #endif /* End Line_RX_INTERRUPT_ENABLED */
        #endif /* End Line_RXBUFFERSIZE > Line_FIFO_LENGTH */

    }


    /*******************************************************************************
    * Function Name: Line_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Set the receive addressing mode
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  Line__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  Line__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  Line__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  Line__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  Line__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Line_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  Line_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void Line_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(Line_RXHW_ADDRESS_ENABLED)
            #if(Line_CONTROL_REG_REMOVED)
                if(addressMode != 0u) { }     /* release compiler warning */
            #else /* Line_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = Line_CONTROL_REG & (uint8)~Line_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << Line_CTRL_RXADDR_MODE0_SHIFT);
                Line_CONTROL_REG = tmpCtrl;
                #if(Line_RX_INTERRUPT_ENABLED && \
                   (Line_RXBUFFERSIZE > Line_FIFO_LENGTH) )
                    Line_rxAddressMode = addressMode;
                    Line_rxAddressDetected = 0u;
                #endif /* End Line_RXBUFFERSIZE > Line_FIFO_LENGTH*/
            #endif /* End Line_CONTROL_REG_REMOVED */
        #else /* Line_RXHW_ADDRESS_ENABLED */
            if(addressMode != 0u) { }     /* release compiler warning */
        #endif /* End Line_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Line_SetRxAddress1
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
    void Line_SetRxAddress1(uint8 address) 

    {
        Line_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: Line_SetRxAddress2
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
    void Line_SetRxAddress2(uint8 address) 
    {
        Line_RXADDRESS2_REG = address;
    }

#endif  /* Line_RX_ENABLED || Line_HD_ENABLED*/


#if( (Line_TX_ENABLED) || (Line_HD_ENABLED) )

    #if(Line_TX_INTERRUPT_ENABLED)

        /*******************************************************************************
        * Function Name: Line_EnableTxInt
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
        void Line_EnableTxInt(void) 
        {
            CyIntEnable(Line_TX_VECT_NUM);
        }


        /*******************************************************************************
        * Function Name: Line_DisableTxInt
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
        void Line_DisableTxInt(void) 
        {
            CyIntDisable(Line_TX_VECT_NUM);
        }

    #endif /* Line_TX_INTERRUPT_ENABLED */


    /*******************************************************************************
    * Function Name: Line_SetTxInterruptMode
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
    void Line_SetTxInterruptMode(uint8 intSrc) 
    {
        Line_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: Line_WriteTxData
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
    *  Line_txBuffer - RAM buffer pointer for save data for transmission
    *  Line_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  Line_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  Line_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Line_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(Line_initVar != 0u)
        {
            #if(Line_TXBUFFERSIZE > Line_FIFO_LENGTH)

                /* Disable Tx interrupt. */
                /* Protect variables that could change on interrupt. */
                #if(Line_TX_INTERRUPT_ENABLED)
                    Line_DisableTxInt();
                #endif /* End Line_TX_INTERRUPT_ENABLED */

                if( (Line_txBufferRead == Line_txBufferWrite) &&
                    ((Line_TXSTATUS_REG & Line_TX_STS_FIFO_FULL) == 0u) )
                {
                    /* Add directly to the FIFO. */
                    Line_TXDATA_REG = txDataByte;
                }
                else
                {
                    if(Line_txBufferWrite >= Line_TXBUFFERSIZE)
                    {
                        Line_txBufferWrite = 0u;
                    }

                    Line_txBuffer[Line_txBufferWrite] = txDataByte;

                    /* Add to the software buffer. */
                    Line_txBufferWrite++;

                }

                /* Enable Tx interrupt. */
                #if(Line_TX_INTERRUPT_ENABLED)
                    Line_EnableTxInt();
                #endif /* End Line_TX_INTERRUPT_ENABLED */

            #else /* Line_TXBUFFERSIZE > Line_FIFO_LENGTH */

                /* Add directly to the FIFO. */
                Line_TXDATA_REG = txDataByte;

            #endif /* End Line_TXBUFFERSIZE > Line_FIFO_LENGTH */
        }
    }


    /*******************************************************************************
    * Function Name: Line_ReadTxStatus
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
    uint8 Line_ReadTxStatus(void) 
    {
        return(Line_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: Line_PutChar
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
    *  Line_txBuffer - RAM buffer pointer for save data for transmission
    *  Line_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  Line_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  Line_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void Line_PutChar(uint8 txDataByte) 
    {
            #if(Line_TXBUFFERSIZE > Line_FIFO_LENGTH)
                /* The temporary output pointer is used since it takes two instructions
                *  to increment with a wrap, and we can't risk doing that with the real
                *  pointer and getting an interrupt in between instructions.
                */
                uint8 loc_txBufferWrite;
                uint8 loc_txBufferRead;

                do{
                    /* Block if software buffer is full, so we don't overwrite. */
                    #if ((Line_TXBUFFERSIZE > Line_MAX_BYTE_VALUE) && (CY_PSOC3))
                        /* Disable TX interrupt to protect variables that could change on interrupt */
                        CyIntDisable(Line_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                    loc_txBufferWrite = Line_txBufferWrite;
                    loc_txBufferRead = Line_txBufferRead;
                    #if ((Line_TXBUFFERSIZE > Line_MAX_BYTE_VALUE) && (CY_PSOC3))
                        /* Enable interrupt to continue transmission */
                        CyIntEnable(Line_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                }while( (loc_txBufferWrite < loc_txBufferRead) ? (loc_txBufferWrite == (loc_txBufferRead - 1u)) :
                                        ((loc_txBufferWrite - loc_txBufferRead) ==
                                        (uint8)(Line_TXBUFFERSIZE - 1u)) );

                if( (loc_txBufferRead == loc_txBufferWrite) &&
                    ((Line_TXSTATUS_REG & Line_TX_STS_FIFO_FULL) == 0u) )
                {
                    /* Add directly to the FIFO. */
                    Line_TXDATA_REG = txDataByte;
                }
                else
                {
                    if(loc_txBufferWrite >= Line_TXBUFFERSIZE)
                    {
                        loc_txBufferWrite = 0u;
                    }
                    /* Add to the software buffer. */
                    Line_txBuffer[loc_txBufferWrite] = txDataByte;
                    loc_txBufferWrite++;

                    /* Finally, update the real output pointer */
                    #if ((Line_TXBUFFERSIZE > Line_MAX_BYTE_VALUE) && (CY_PSOC3))
                        CyIntDisable(Line_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                    Line_txBufferWrite = loc_txBufferWrite;
                    #if ((Line_TXBUFFERSIZE > Line_MAX_BYTE_VALUE) && (CY_PSOC3))
                        CyIntEnable(Line_TX_VECT_NUM);
                    #endif /* End TXBUFFERSIZE > 255 */
                }

            #else /* Line_TXBUFFERSIZE > Line_FIFO_LENGTH */

                while((Line_TXSTATUS_REG & Line_TX_STS_FIFO_FULL) != 0u)
                {
                    ; /* Wait for room in the FIFO. */
                }

                /* Add directly to the FIFO. */
                Line_TXDATA_REG = txDataByte;

            #endif /* End Line_TXBUFFERSIZE > Line_FIFO_LENGTH */
    }


    /*******************************************************************************
    * Function Name: Line_PutString
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
    *  Line_initVar - checked to identify that the component has been
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
    void Line_PutString(const char8 string[]) 
    {
        uint16 buf_index = 0u;
        /* If not Initialized then skip this function*/
        if(Line_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent*/
            while(string[buf_index] != (char8)0)
            {
                Line_PutChar((uint8)string[buf_index]);
                buf_index++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Line_PutArray
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
    *  Line_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Line_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 buf_index = 0u;
        /* If not Initialized then skip this function*/
        if(Line_initVar != 0u)
        {
            do
            {
                Line_PutChar(string[buf_index]);
                buf_index++;
            }while(buf_index < byteCount);
        }
    }


    /*******************************************************************************
    * Function Name: Line_PutCRLF
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
    *  Line_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Line_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(Line_initVar != 0u)
        {
            Line_PutChar(txDataByte);
            Line_PutChar(0x0Du);
            Line_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: Line_GetTxBufferSize
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
    *  Line_txBufferWrite - used to calculate left space.
    *  Line_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 Line_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

        #if(Line_TXBUFFERSIZE > Line_FIFO_LENGTH)

            /* Disable Tx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(Line_TX_INTERRUPT_ENABLED)
                Line_DisableTxInt();
            #endif /* End Line_TX_INTERRUPT_ENABLED */

            if(Line_txBufferRead == Line_txBufferWrite)
            {
                size = 0u;
            }
            else if(Line_txBufferRead < Line_txBufferWrite)
            {
                size = (Line_txBufferWrite - Line_txBufferRead);
            }
            else
            {
                size = (Line_TXBUFFERSIZE - Line_txBufferRead) + Line_txBufferWrite;
            }

            /* Enable Tx interrupt. */
            #if(Line_TX_INTERRUPT_ENABLED)
                Line_EnableTxInt();
            #endif /* End Line_TX_INTERRUPT_ENABLED */

        #else /* Line_TXBUFFERSIZE > Line_FIFO_LENGTH */

            size = Line_TXSTATUS_REG;

            /* Is the fifo is full. */
            if((size & Line_TX_STS_FIFO_FULL) != 0u)
            {
                size = Line_FIFO_LENGTH;
            }
            else if((size & Line_TX_STS_FIFO_EMPTY) != 0u)
            {
                size = 0u;
            }
            else
            {
                /* We only know there is data in the fifo. */
                size = 1u;
            }

        #endif /* End Line_TXBUFFERSIZE > Line_FIFO_LENGTH */

        return(size);
    }


    /*******************************************************************************
    * Function Name: Line_ClearTxBuffer
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
    *  Line_txBufferWrite - cleared to zero.
    *  Line_txBufferRead - cleared to zero.
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
    void Line_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Enter critical section */
        enableInterrupts = CyEnterCriticalSection();
        /* clear the HW FIFO */
        Line_TXDATA_AUX_CTL_REG |=  Line_TX_FIFO_CLR;
        Line_TXDATA_AUX_CTL_REG &= (uint8)~Line_TX_FIFO_CLR;
        /* Exit critical section */
        CyExitCriticalSection(enableInterrupts);

        #if(Line_TXBUFFERSIZE > Line_FIFO_LENGTH)

            /* Disable Tx interrupt. */
            /* Protect variables that could change on interrupt. */
            #if(Line_TX_INTERRUPT_ENABLED)
                Line_DisableTxInt();
            #endif /* End Line_TX_INTERRUPT_ENABLED */

            Line_txBufferRead = 0u;
            Line_txBufferWrite = 0u;

            /* Enable Tx interrupt. */
            #if(Line_TX_INTERRUPT_ENABLED)
                Line_EnableTxInt();
            #endif /* End Line_TX_INTERRUPT_ENABLED */

        #endif /* End Line_TXBUFFERSIZE > Line_FIFO_LENGTH */
    }


    /*******************************************************************************
    * Function Name: Line_SendBreak
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
    *  Line_initVar - checked to identify that the component has been
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
    void Line_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(Line_initVar != 0u)
        {
            /*Set the Counter to 13-bits and transmit a 00 byte*/
            /*When that is done then reset the counter value back*/
            uint8 tmpStat;

            #if(Line_HD_ENABLED) /* Half Duplex mode*/

                if( (retMode == Line_SEND_BREAK) ||
                    (retMode == Line_SEND_WAIT_REINIT ) )
                {
                    /* CTRL_HD_SEND_BREAK - sends break bits in HD mode*/
                    Line_WriteControlRegister(Line_ReadControlRegister() |
                                                          Line_CTRL_HD_SEND_BREAK);
                    /* Send zeros*/
                    Line_TXDATA_REG = 0u;

                    do /*wait until transmit starts*/
                    {
                        tmpStat = Line_TXSTATUS_REG;
                    }while((tmpStat & Line_TX_STS_FIFO_EMPTY) != 0u);
                }

                if( (retMode == Line_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == Line_SEND_WAIT_REINIT) )
                {
                    do /*wait until transmit complete*/
                    {
                        tmpStat = Line_TXSTATUS_REG;
                    }while(((uint8)~tmpStat & Line_TX_STS_COMPLETE) != 0u);
                }

                if( (retMode == Line_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == Line_REINIT) ||
                    (retMode == Line_SEND_WAIT_REINIT) )
                {
                    Line_WriteControlRegister(Line_ReadControlRegister() &
                                                  (uint8)~Line_CTRL_HD_SEND_BREAK);
                }

            #else /* Line_HD_ENABLED Full Duplex mode */

                static uint8 tx_period;

                if( (retMode == Line_SEND_BREAK) ||
                    (retMode == Line_SEND_WAIT_REINIT) )
                {
                    /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode*/
                    #if( (Line_PARITY_TYPE != Line__B_UART__NONE_REVB) || \
                                        (Line_PARITY_TYPE_SW != 0u) )
                        Line_WriteControlRegister(Line_ReadControlRegister() |
                                                              Line_CTRL_HD_SEND_BREAK);
                    #endif /* End Line_PARITY_TYPE != Line__B_UART__NONE_REVB  */

                    #if(Line_TXCLKGEN_DP)
                        tx_period = Line_TXBITCLKTX_COMPLETE_REG;
                        Line_TXBITCLKTX_COMPLETE_REG = Line_TXBITCTR_BREAKBITS;
                    #else
                        tx_period = Line_TXBITCTR_PERIOD_REG;
                        Line_TXBITCTR_PERIOD_REG = Line_TXBITCTR_BREAKBITS8X;
                    #endif /* End Line_TXCLKGEN_DP */

                    /* Send zeros*/
                    Line_TXDATA_REG = 0u;

                    do /* wait until transmit starts */
                    {
                        tmpStat = Line_TXSTATUS_REG;
                    }while((tmpStat & Line_TX_STS_FIFO_EMPTY) != 0u);
                }

                if( (retMode == Line_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == Line_SEND_WAIT_REINIT) )
                {
                    do /*wait until transmit complete*/
                    {
                        tmpStat = Line_TXSTATUS_REG;
                    }while(((uint8)~tmpStat & Line_TX_STS_COMPLETE) != 0u);
                }

                if( (retMode == Line_WAIT_FOR_COMPLETE_REINIT) ||
                    (retMode == Line_REINIT) ||
                    (retMode == Line_SEND_WAIT_REINIT) )
                {

                    #if(Line_TXCLKGEN_DP)
                        Line_TXBITCLKTX_COMPLETE_REG = tx_period;
                    #else
                        Line_TXBITCTR_PERIOD_REG = tx_period;
                    #endif /* End Line_TXCLKGEN_DP */

                    #if( (Line_PARITY_TYPE != Line__B_UART__NONE_REVB) || \
                         (Line_PARITY_TYPE_SW != 0u) )
                        Line_WriteControlRegister(Line_ReadControlRegister() &
                                                      (uint8)~Line_CTRL_HD_SEND_BREAK);
                    #endif /* End Line_PARITY_TYPE != NONE */
                }
            #endif    /* End Line_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: Line_SetTxAddressMode
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
    void Line_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable*/
        if(addressMode != 0u)
        {
            #if( Line_CONTROL_REG_REMOVED == 0u )
                Line_WriteControlRegister(Line_ReadControlRegister() |
                                                      Line_CTRL_MARK);
            #endif /* End Line_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
            #if( Line_CONTROL_REG_REMOVED == 0u )
                Line_WriteControlRegister(Line_ReadControlRegister() &
                                                    (uint8)~Line_CTRL_MARK);
            #endif /* End Line_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndLine_TX_ENABLED */

#if(Line_HD_ENABLED)


    /*******************************************************************************
    * Function Name: Line_LoadTxConfig
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
    void Line_LoadTxConfig(void) 
    {
        #if((Line_RX_INTERRUPT_ENABLED) && (Line_RXBUFFERSIZE > Line_FIFO_LENGTH))
            /* Disable RX interrupts before set TX configuration */
            Line_SetRxInterruptMode(0u);
        #endif /* Line_RX_INTERRUPT_ENABLED */

        Line_WriteControlRegister(Line_ReadControlRegister() | Line_CTRL_HD_SEND);
        Line_RXBITCTR_PERIOD_REG = Line_HD_TXBITCTR_INIT;
        #if(CY_UDB_V0) /* Manually clear status register when mode has been changed */
            /* Clear status register */
            CY_GET_REG8(Line_RXSTATUS_PTR);
        #endif /* CY_UDB_V0 */
    }


    /*******************************************************************************
    * Function Name: Line_LoadRxConfig
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
    void Line_LoadRxConfig(void) 
    {
        Line_WriteControlRegister(Line_ReadControlRegister() &
                                                (uint8)~Line_CTRL_HD_SEND);
        Line_RXBITCTR_PERIOD_REG = Line_HD_RXBITCTR_INIT;
        #if(CY_UDB_V0) /* Manually clear status register when mode has been changed */
            /* Clear status register */
            CY_GET_REG8(Line_RXSTATUS_PTR);
        #endif /* CY_UDB_V0 */

        #if((Line_RX_INTERRUPT_ENABLED) && (Line_RXBUFFERSIZE > Line_FIFO_LENGTH))
            /* Enable RX interrupt after set RX configuration */
            Line_SetRxInterruptMode(Line_INIT_RX_INTERRUPTS_MASK);
        #endif /* Line_RX_INTERRUPT_ENABLED */
    }

#endif  /* Line_HD_ENABLED */


/* [] END OF FILE */
