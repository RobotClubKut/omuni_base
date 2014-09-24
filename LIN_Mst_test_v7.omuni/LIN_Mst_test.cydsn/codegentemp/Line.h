/*******************************************************************************
* File Name: Line.h
* Version 2.30
*
* Description:
*  Contains the function prototypes and constants available to the UART
*  user module.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(CY_UART_Line_H)
#define CY_UART_Line_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h"


/***************************************
* Conditional Compilation Parameters
***************************************/

#define Line_RX_ENABLED                     (1u)
#define Line_TX_ENABLED                     (1u)
#define Line_HD_ENABLED                     (0u)
#define Line_RX_INTERRUPT_ENABLED           (1u)
#define Line_TX_INTERRUPT_ENABLED           (0u)
#define Line_INTERNAL_CLOCK_USED            (1u)
#define Line_RXHW_ADDRESS_ENABLED           (0u)
#define Line_OVER_SAMPLE_COUNT              (8u)
#define Line_PARITY_TYPE                    (3u)
#define Line_PARITY_TYPE_SW                 (0u)
#define Line_BREAK_DETECT                   (0u)
#define Line_BREAK_BITS_TX                  (13u)
#define Line_BREAK_BITS_RX                  (13u)
#define Line_TXCLKGEN_DP                    (1u)
#define Line_USE23POLLING                   (1u)
#define Line_FLOW_CONTROL                   (0u)
#define Line_CLK_FREQ                       (0u)
#define Line_TXBUFFERSIZE                   (4u)
#define Line_RXBUFFERSIZE                   (16u)

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component UART_v2_30 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

#ifdef Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Line_CONTROL_REG_REMOVED            (0u)
#else
    #define Line_CONTROL_REG_REMOVED            (1u)
#endif /* End Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */


/***************************************
*      Data Struct Definition
***************************************/

/* Sleep Mode API Support */
typedef struct Line_backupStruct_
{
    uint8 enableState;

    #if(Line_CONTROL_REG_REMOVED == 0u)
        uint8 cr;
    #endif /* End Line_CONTROL_REG_REMOVED */
    #if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
        uint8 rx_period;
        #if (CY_UDB_V0)
            uint8 rx_mask;
            #if (Line_RXHW_ADDRESS_ENABLED)
                uint8 rx_addr1;
                uint8 rx_addr2;
            #endif /* End Line_RXHW_ADDRESS_ENABLED */
        #endif /* End CY_UDB_V0 */
    #endif  /* End (Line_RX_ENABLED) || (Line_HD_ENABLED)*/

    #if(Line_TX_ENABLED)
        #if(Line_TXCLKGEN_DP)
            uint8 tx_clk_ctr;
            #if (CY_UDB_V0)
                uint8 tx_clk_compl;
            #endif  /* End CY_UDB_V0 */
        #else
            uint8 tx_period;
        #endif /*End Line_TXCLKGEN_DP */
        #if (CY_UDB_V0)
            uint8 tx_mask;
        #endif  /* End CY_UDB_V0 */
    #endif /*End Line_TX_ENABLED */
} Line_BACKUP_STRUCT;


/***************************************
*       Function Prototypes
***************************************/

void Line_Start(void) ;
void Line_Stop(void) ;
uint8 Line_ReadControlRegister(void) ;
void Line_WriteControlRegister(uint8 control) ;

void Line_Init(void) ;
void Line_Enable(void) ;
void Line_SaveConfig(void) ;
void Line_RestoreConfig(void) ;
void Line_Sleep(void) ;
void Line_Wakeup(void) ;

/* Only if RX is enabled */
#if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )

    #if(Line_RX_INTERRUPT_ENABLED)
        void  Line_EnableRxInt(void) ;
        void  Line_DisableRxInt(void) ;
        CY_ISR_PROTO(Line_RXISR);
    #endif /* Line_RX_INTERRUPT_ENABLED */

    void Line_SetRxAddressMode(uint8 addressMode)
                                                           ;
    void Line_SetRxAddress1(uint8 address) ;
    void Line_SetRxAddress2(uint8 address) ;

    void  Line_SetRxInterruptMode(uint8 intSrc) ;
    uint8 Line_ReadRxData(void) ;
    uint8 Line_ReadRxStatus(void) ;
    uint8 Line_GetChar(void) ;
    uint16 Line_GetByte(void) ;
    uint8 Line_GetRxBufferSize(void)
                                                            ;
    void Line_ClearRxBuffer(void) ;

    /* Obsolete functions, defines for backward compatible */
    #define Line_GetRxInterruptSource   Line_ReadRxStatus

#endif /* End (Line_RX_ENABLED) || (Line_HD_ENABLED) */

/* Only if TX is enabled */
#if(Line_TX_ENABLED || Line_HD_ENABLED)

    #if(Line_TX_INTERRUPT_ENABLED)
        void Line_EnableTxInt(void) ;
        void Line_DisableTxInt(void) ;
        CY_ISR_PROTO(Line_TXISR);
    #endif /* Line_TX_INTERRUPT_ENABLED */

    void Line_SetTxInterruptMode(uint8 intSrc) ;
    void Line_WriteTxData(uint8 txDataByte) ;
    uint8 Line_ReadTxStatus(void) ;
    void Line_PutChar(uint8 txDataByte) ;
    void Line_PutString(const char8 string[]) ;
    void Line_PutArray(const uint8 string[], uint8 byteCount)
                                                            ;
    void Line_PutCRLF(uint8 txDataByte) ;
    void Line_ClearTxBuffer(void) ;
    void Line_SetTxAddressMode(uint8 addressMode) ;
    void Line_SendBreak(uint8 retMode) ;
    uint8 Line_GetTxBufferSize(void)
                                                            ;
    /* Obsolete functions, defines for backward compatible */
    #define Line_PutStringConst         Line_PutString
    #define Line_PutArrayConst          Line_PutArray
    #define Line_GetTxInterruptSource   Line_ReadTxStatus

#endif /* End Line_TX_ENABLED || Line_HD_ENABLED */

#if(Line_HD_ENABLED)
    void Line_LoadRxConfig(void) ;
    void Line_LoadTxConfig(void) ;
#endif /* End Line_HD_ENABLED */


/* Communication bootloader APIs */
#if defined(CYDEV_BOOTLOADER_IO_COMP) && ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Line) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))
    /* Physical layer functions */
    void    Line_CyBtldrCommStart(void) CYSMALL ;
    void    Line_CyBtldrCommStop(void) CYSMALL ;
    void    Line_CyBtldrCommReset(void) CYSMALL ;
    cystatus Line_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;
    cystatus Line_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;

    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Line)
        #define CyBtldrCommStart    Line_CyBtldrCommStart
        #define CyBtldrCommStop     Line_CyBtldrCommStop
        #define CyBtldrCommReset    Line_CyBtldrCommReset
        #define CyBtldrCommWrite    Line_CyBtldrCommWrite
        #define CyBtldrCommRead     Line_CyBtldrCommRead
    #endif  /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Line) */

    /* Byte to Byte time out for detecting end of block data from host */
    #define Line_BYTE2BYTE_TIME_OUT (25u)

#endif /* CYDEV_BOOTLOADER_IO_COMP */


/***************************************
*          API Constants
***************************************/
/* Parameters for SetTxAddressMode API*/
#define Line_SET_SPACE                              (0x00u)
#define Line_SET_MARK                               (0x01u)

/* Status Register definitions */
#if( (Line_TX_ENABLED) || (Line_HD_ENABLED) )
    #if(Line_TX_INTERRUPT_ENABLED)
        #define Line_TX_VECT_NUM            (uint8)Line_TXInternalInterrupt__INTC_NUMBER
        #define Line_TX_PRIOR_NUM           (uint8)Line_TXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Line_TX_INTERRUPT_ENABLED */
    #if(Line_TX_ENABLED)
        #define Line_TX_STS_COMPLETE_SHIFT          (0x00u)
        #define Line_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
        #define Line_TX_STS_FIFO_FULL_SHIFT         (0x02u)
        #define Line_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #endif /* Line_TX_ENABLED */
    #if(Line_HD_ENABLED)
        #define Line_TX_STS_COMPLETE_SHIFT          (0x00u)
        #define Line_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
        #define Line_TX_STS_FIFO_FULL_SHIFT         (0x05u)  /*needs MD=0*/
        #define Line_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #endif /* Line_HD_ENABLED */
    #define Line_TX_STS_COMPLETE            (uint8)(0x01u << Line_TX_STS_COMPLETE_SHIFT)
    #define Line_TX_STS_FIFO_EMPTY          (uint8)(0x01u << Line_TX_STS_FIFO_EMPTY_SHIFT)
    #define Line_TX_STS_FIFO_FULL           (uint8)(0x01u << Line_TX_STS_FIFO_FULL_SHIFT)
    #define Line_TX_STS_FIFO_NOT_FULL       (uint8)(0x01u << Line_TX_STS_FIFO_NOT_FULL_SHIFT)
#endif /* End (Line_TX_ENABLED) || (Line_HD_ENABLED)*/

#if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
    #if(Line_RX_INTERRUPT_ENABLED)
        #define Line_RX_VECT_NUM            (uint8)Line_RXInternalInterrupt__INTC_NUMBER
        #define Line_RX_PRIOR_NUM           (uint8)Line_RXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Line_RX_INTERRUPT_ENABLED */
    #define Line_RX_STS_MRKSPC_SHIFT            (0x00u)
    #define Line_RX_STS_BREAK_SHIFT             (0x01u)
    #define Line_RX_STS_PAR_ERROR_SHIFT         (0x02u)
    #define Line_RX_STS_STOP_ERROR_SHIFT        (0x03u)
    #define Line_RX_STS_OVERRUN_SHIFT           (0x04u)
    #define Line_RX_STS_FIFO_NOTEMPTY_SHIFT     (0x05u)
    #define Line_RX_STS_ADDR_MATCH_SHIFT        (0x06u)
    #define Line_RX_STS_SOFT_BUFF_OVER_SHIFT    (0x07u)

    #define Line_RX_STS_MRKSPC           (uint8)(0x01u << Line_RX_STS_MRKSPC_SHIFT)
    #define Line_RX_STS_BREAK            (uint8)(0x01u << Line_RX_STS_BREAK_SHIFT)
    #define Line_RX_STS_PAR_ERROR        (uint8)(0x01u << Line_RX_STS_PAR_ERROR_SHIFT)
    #define Line_RX_STS_STOP_ERROR       (uint8)(0x01u << Line_RX_STS_STOP_ERROR_SHIFT)
    #define Line_RX_STS_OVERRUN          (uint8)(0x01u << Line_RX_STS_OVERRUN_SHIFT)
    #define Line_RX_STS_FIFO_NOTEMPTY    (uint8)(0x01u << Line_RX_STS_FIFO_NOTEMPTY_SHIFT)
    #define Line_RX_STS_ADDR_MATCH       (uint8)(0x01u << Line_RX_STS_ADDR_MATCH_SHIFT)
    #define Line_RX_STS_SOFT_BUFF_OVER   (uint8)(0x01u << Line_RX_STS_SOFT_BUFF_OVER_SHIFT)
    #define Line_RX_HW_MASK                     (0x7Fu)
#endif /* End (Line_RX_ENABLED) || (Line_HD_ENABLED) */

/* Control Register definitions */
#define Line_CTRL_HD_SEND_SHIFT                 (0x00u) /* 1 enable TX part in Half Duplex mode */
#define Line_CTRL_HD_SEND_BREAK_SHIFT           (0x01u) /* 1 send BREAK signal in Half Duplez mode */
#define Line_CTRL_MARK_SHIFT                    (0x02u) /* 1 sets mark, 0 sets space */
#define Line_CTRL_PARITY_TYPE0_SHIFT            (0x03u) /* Defines the type of parity implemented */
#define Line_CTRL_PARITY_TYPE1_SHIFT            (0x04u) /* Defines the type of parity implemented */
#define Line_CTRL_RXADDR_MODE0_SHIFT            (0x05u)
#define Line_CTRL_RXADDR_MODE1_SHIFT            (0x06u)
#define Line_CTRL_RXADDR_MODE2_SHIFT            (0x07u)

#define Line_CTRL_HD_SEND               (uint8)(0x01u << Line_CTRL_HD_SEND_SHIFT)
#define Line_CTRL_HD_SEND_BREAK         (uint8)(0x01u << Line_CTRL_HD_SEND_BREAK_SHIFT)
#define Line_CTRL_MARK                  (uint8)(0x01u << Line_CTRL_MARK_SHIFT)
#define Line_CTRL_PARITY_TYPE_MASK      (uint8)(0x03u << Line_CTRL_PARITY_TYPE0_SHIFT)
#define Line_CTRL_RXADDR_MODE_MASK      (uint8)(0x07u << Line_CTRL_RXADDR_MODE0_SHIFT)

/* StatusI Register Interrupt Enable Control Bits. As defined by the Register map for the AUX Control Register */
#define Line_INT_ENABLE                         (0x10u)

/* Bit Counter (7-bit) Control Register Bit Definitions. As defined by the Register map for the AUX Control Register */
#define Line_CNTR_ENABLE                        (0x20u)

/*   Constants for SendBreak() "retMode" parameter  */
#define Line_SEND_BREAK                         (0x00u)
#define Line_WAIT_FOR_COMPLETE_REINIT           (0x01u)
#define Line_REINIT                             (0x02u)
#define Line_SEND_WAIT_REINIT                   (0x03u)

#define Line_OVER_SAMPLE_8                      (8u)
#define Line_OVER_SAMPLE_16                     (16u)

#define Line_BIT_CENTER                         (Line_OVER_SAMPLE_COUNT - 1u)

#define Line_FIFO_LENGTH                        (4u)
#define Line_NUMBER_OF_START_BIT                (1u)
#define Line_MAX_BYTE_VALUE                     (0xFFu)

/* 8X always for count7 implementation*/
#define Line_TXBITCTR_BREAKBITS8X   ((Line_BREAK_BITS_TX * Line_OVER_SAMPLE_8) - 1u)
/* 8X or 16X for DP implementation*/
#define Line_TXBITCTR_BREAKBITS ((Line_BREAK_BITS_TX * Line_OVER_SAMPLE_COUNT) - 1u)

#define Line_HALF_BIT_COUNT   \
                            (((Line_OVER_SAMPLE_COUNT / 2u) + (Line_USE23POLLING * 1u)) - 2u)
#if (Line_OVER_SAMPLE_COUNT == Line_OVER_SAMPLE_8)
    #define Line_HD_TXBITCTR_INIT   (((Line_BREAK_BITS_TX + \
                            Line_NUMBER_OF_START_BIT) * Line_OVER_SAMPLE_COUNT) - 1u)

    /* This parameter is increased on the 2 in 2 out of 3 mode to sample voting in the middle */
    #define Line_RXBITCTR_INIT  ((((Line_BREAK_BITS_RX + Line_NUMBER_OF_START_BIT) \
                            * Line_OVER_SAMPLE_COUNT) + Line_HALF_BIT_COUNT) - 1u)


#else /* Line_OVER_SAMPLE_COUNT == Line_OVER_SAMPLE_16 */
    #define Line_HD_TXBITCTR_INIT   ((8u * Line_OVER_SAMPLE_COUNT) - 1u)
    /* 7bit counter need one more bit for OverSampleCount=16 */
    #define Line_RXBITCTR_INIT      (((7u * Line_OVER_SAMPLE_COUNT) - 1u) + \
                                                      Line_HALF_BIT_COUNT)
#endif /* End Line_OVER_SAMPLE_COUNT */
#define Line_HD_RXBITCTR_INIT                   Line_RXBITCTR_INIT


/***************************************
* Global variables external identifier
***************************************/

extern uint8 Line_initVar;
#if( Line_TX_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
    extern volatile uint8 Line_txBuffer[Line_TXBUFFERSIZE];
    extern volatile uint8 Line_txBufferRead;
    extern uint8 Line_txBufferWrite;
#endif /* End Line_TX_ENABLED */
#if( ( Line_RX_ENABLED || Line_HD_ENABLED ) && \
     (Line_RXBUFFERSIZE > Line_FIFO_LENGTH) )
    extern volatile uint8 Line_rxBuffer[Line_RXBUFFERSIZE];
    extern volatile uint8 Line_rxBufferRead;
    extern volatile uint8 Line_rxBufferWrite;
    extern volatile uint8 Line_rxBufferLoopDetect;
    extern volatile uint8 Line_rxBufferOverflow;
    #if (Line_RXHW_ADDRESS_ENABLED)
        extern volatile uint8 Line_rxAddressMode;
        extern volatile uint8 Line_rxAddressDetected;
    #endif /* End EnableHWAddress */
#endif /* End Line_RX_ENABLED */


/***************************************
* Enumerated Types and Parameters
***************************************/

#define Line__B_UART__AM_SW_BYTE_BYTE 1
#define Line__B_UART__AM_SW_DETECT_TO_BUFFER 2
#define Line__B_UART__AM_HW_BYTE_BY_BYTE 3
#define Line__B_UART__AM_HW_DETECT_TO_BUFFER 4
#define Line__B_UART__AM_NONE 0

#define Line__B_UART__NONE_REVB 0
#define Line__B_UART__EVEN_REVB 1
#define Line__B_UART__ODD_REVB 2
#define Line__B_UART__MARK_SPACE_REVB 3



/***************************************
*    Initial Parameter Constants
***************************************/

/* UART shifts max 8 bits, Mark/Space functionality working if 9 selected */
#define Line_NUMBER_OF_DATA_BITS    ((9u > 8u) ? 8u : 9u)
#define Line_NUMBER_OF_STOP_BITS    (1u)

#if (Line_RXHW_ADDRESS_ENABLED)
    #define Line_RXADDRESSMODE      (0u)
    #define Line_RXHWADDRESS1       (0u)
    #define Line_RXHWADDRESS2       (0u)
    /* Backward compatible define */
    #define Line_RXAddressMode      Line_RXADDRESSMODE
#endif /* End EnableHWAddress */

#define Line_INIT_RX_INTERRUPTS_MASK \
                                  (uint8)((1 << Line_RX_STS_FIFO_NOTEMPTY_SHIFT) \
                                        | (0 << Line_RX_STS_MRKSPC_SHIFT) \
                                        | (0 << Line_RX_STS_ADDR_MATCH_SHIFT) \
                                        | (0 << Line_RX_STS_PAR_ERROR_SHIFT) \
                                        | (0 << Line_RX_STS_STOP_ERROR_SHIFT) \
                                        | (0 << Line_RX_STS_BREAK_SHIFT) \
                                        | (0 << Line_RX_STS_OVERRUN_SHIFT))

#define Line_INIT_TX_INTERRUPTS_MASK \
                                  (uint8)((0 << Line_TX_STS_COMPLETE_SHIFT) \
                                        | (0 << Line_TX_STS_FIFO_EMPTY_SHIFT) \
                                        | (0 << Line_TX_STS_FIFO_FULL_SHIFT) \
                                        | (0 << Line_TX_STS_FIFO_NOT_FULL_SHIFT))


/***************************************
*              Registers
***************************************/

#ifdef Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Line_CONTROL_REG \
                            (* (reg8 *) Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
    #define Line_CONTROL_PTR \
                            (  (reg8 *) Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
#endif /* End Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Line_TX_ENABLED)
    #define Line_TXDATA_REG          (* (reg8 *) Line_BUART_sTX_TxShifter_u0__F0_REG)
    #define Line_TXDATA_PTR          (  (reg8 *) Line_BUART_sTX_TxShifter_u0__F0_REG)
    #define Line_TXDATA_AUX_CTL_REG  (* (reg8 *) Line_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Line_TXDATA_AUX_CTL_PTR  (  (reg8 *) Line_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Line_TXSTATUS_REG        (* (reg8 *) Line_BUART_sTX_TxSts__STATUS_REG)
    #define Line_TXSTATUS_PTR        (  (reg8 *) Line_BUART_sTX_TxSts__STATUS_REG)
    #define Line_TXSTATUS_MASK_REG   (* (reg8 *) Line_BUART_sTX_TxSts__MASK_REG)
    #define Line_TXSTATUS_MASK_PTR   (  (reg8 *) Line_BUART_sTX_TxSts__MASK_REG)
    #define Line_TXSTATUS_ACTL_REG   (* (reg8 *) Line_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)
    #define Line_TXSTATUS_ACTL_PTR   (  (reg8 *) Line_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)

    /* DP clock */
    #if(Line_TXCLKGEN_DP)
        #define Line_TXBITCLKGEN_CTR_REG        \
                                        (* (reg8 *) Line_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Line_TXBITCLKGEN_CTR_PTR        \
                                        (  (reg8 *) Line_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Line_TXBITCLKTX_COMPLETE_REG    \
                                        (* (reg8 *) Line_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
        #define Line_TXBITCLKTX_COMPLETE_PTR    \
                                        (  (reg8 *) Line_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
    #else     /* Count7 clock*/
        #define Line_TXBITCTR_PERIOD_REG    \
                                        (* (reg8 *) Line_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Line_TXBITCTR_PERIOD_PTR    \
                                        (  (reg8 *) Line_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Line_TXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Line_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Line_TXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Line_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Line_TXBITCTR_COUNTER_REG   \
                                        (* (reg8 *) Line_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
        #define Line_TXBITCTR_COUNTER_PTR   \
                                        (  (reg8 *) Line_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
    #endif /* Line_TXCLKGEN_DP */

#endif /* End Line_TX_ENABLED */

#if(Line_HD_ENABLED)

    #define Line_TXDATA_REG             (* (reg8 *) Line_BUART_sRX_RxShifter_u0__F1_REG )
    #define Line_TXDATA_PTR             (  (reg8 *) Line_BUART_sRX_RxShifter_u0__F1_REG )
    #define Line_TXDATA_AUX_CTL_REG     (* (reg8 *) Line_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
    #define Line_TXDATA_AUX_CTL_PTR     (  (reg8 *) Line_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Line_TXSTATUS_REG           (* (reg8 *) Line_BUART_sRX_RxSts__STATUS_REG )
    #define Line_TXSTATUS_PTR           (  (reg8 *) Line_BUART_sRX_RxSts__STATUS_REG )
    #define Line_TXSTATUS_MASK_REG      (* (reg8 *) Line_BUART_sRX_RxSts__MASK_REG )
    #define Line_TXSTATUS_MASK_PTR      (  (reg8 *) Line_BUART_sRX_RxSts__MASK_REG )
    #define Line_TXSTATUS_ACTL_REG      (* (reg8 *) Line_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Line_TXSTATUS_ACTL_PTR      (  (reg8 *) Line_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End Line_HD_ENABLED */

#if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
    #define Line_RXDATA_REG             (* (reg8 *) Line_BUART_sRX_RxShifter_u0__F0_REG )
    #define Line_RXDATA_PTR             (  (reg8 *) Line_BUART_sRX_RxShifter_u0__F0_REG )
    #define Line_RXADDRESS1_REG         (* (reg8 *) Line_BUART_sRX_RxShifter_u0__D0_REG )
    #define Line_RXADDRESS1_PTR         (  (reg8 *) Line_BUART_sRX_RxShifter_u0__D0_REG )
    #define Line_RXADDRESS2_REG         (* (reg8 *) Line_BUART_sRX_RxShifter_u0__D1_REG )
    #define Line_RXADDRESS2_PTR         (  (reg8 *) Line_BUART_sRX_RxShifter_u0__D1_REG )
    #define Line_RXDATA_AUX_CTL_REG     (* (reg8 *) Line_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Line_RXBITCTR_PERIOD_REG    (* (reg8 *) Line_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Line_RXBITCTR_PERIOD_PTR    (  (reg8 *) Line_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Line_RXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Line_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Line_RXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Line_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Line_RXBITCTR_COUNTER_REG   (* (reg8 *) Line_BUART_sRX_RxBitCounter__COUNT_REG )
    #define Line_RXBITCTR_COUNTER_PTR   (  (reg8 *) Line_BUART_sRX_RxBitCounter__COUNT_REG )

    #define Line_RXSTATUS_REG           (* (reg8 *) Line_BUART_sRX_RxSts__STATUS_REG )
    #define Line_RXSTATUS_PTR           (  (reg8 *) Line_BUART_sRX_RxSts__STATUS_REG )
    #define Line_RXSTATUS_MASK_REG      (* (reg8 *) Line_BUART_sRX_RxSts__MASK_REG )
    #define Line_RXSTATUS_MASK_PTR      (  (reg8 *) Line_BUART_sRX_RxSts__MASK_REG )
    #define Line_RXSTATUS_ACTL_REG      (* (reg8 *) Line_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Line_RXSTATUS_ACTL_PTR      (  (reg8 *) Line_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End  (Line_RX_ENABLED) || (Line_HD_ENABLED) */

#if(Line_INTERNAL_CLOCK_USED)
    /* Register to enable or disable the digital clocks */
    #define Line_INTCLOCK_CLKEN_REG     (* (reg8 *) Line_IntClock__PM_ACT_CFG)
    #define Line_INTCLOCK_CLKEN_PTR     (  (reg8 *) Line_IntClock__PM_ACT_CFG)

    /* Clock mask for this clock. */
    #define Line_INTCLOCK_CLKEN_MASK    Line_IntClock__PM_ACT_MSK
#endif /* End Line_INTERNAL_CLOCK_USED */


/***************************************
*       Register Constants
***************************************/

#if(Line_TX_ENABLED)
    #define Line_TX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End Line_TX_ENABLED */

#if(Line_HD_ENABLED)
    #define Line_TX_FIFO_CLR            (0x02u) /* FIFO1 CLR */
#endif /* End Line_HD_ENABLED */

#if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
    #define Line_RX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End  (Line_RX_ENABLED) || (Line_HD_ENABLED) */


/***************************************
* Renamed global variables or defines
* for backward compatible
***************************************/

#define Line_initvar                    Line_initVar

#define Line_RX_Enabled                 Line_RX_ENABLED
#define Line_TX_Enabled                 Line_TX_ENABLED
#define Line_HD_Enabled                 Line_HD_ENABLED
#define Line_RX_IntInterruptEnabled     Line_RX_INTERRUPT_ENABLED
#define Line_TX_IntInterruptEnabled     Line_TX_INTERRUPT_ENABLED
#define Line_InternalClockUsed          Line_INTERNAL_CLOCK_USED
#define Line_RXHW_Address_Enabled       Line_RXHW_ADDRESS_ENABLED
#define Line_OverSampleCount            Line_OVER_SAMPLE_COUNT
#define Line_ParityType                 Line_PARITY_TYPE

#if( Line_TX_ENABLED && (Line_TXBUFFERSIZE > Line_FIFO_LENGTH))
    #define Line_TXBUFFER               Line_txBuffer
    #define Line_TXBUFFERREAD           Line_txBufferRead
    #define Line_TXBUFFERWRITE          Line_txBufferWrite
#endif /* End Line_TX_ENABLED */
#if( ( Line_RX_ENABLED || Line_HD_ENABLED ) && \
     (Line_RXBUFFERSIZE > Line_FIFO_LENGTH) )
    #define Line_RXBUFFER               Line_rxBuffer
    #define Line_RXBUFFERREAD           Line_rxBufferRead
    #define Line_RXBUFFERWRITE          Line_rxBufferWrite
    #define Line_RXBUFFERLOOPDETECT     Line_rxBufferLoopDetect
    #define Line_RXBUFFER_OVERFLOW      Line_rxBufferOverflow
#endif /* End Line_RX_ENABLED */

#ifdef Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Line_CONTROL                Line_CONTROL_REG
#endif /* End Line_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Line_TX_ENABLED)
    #define Line_TXDATA                 Line_TXDATA_REG
    #define Line_TXSTATUS               Line_TXSTATUS_REG
    #define Line_TXSTATUS_MASK          Line_TXSTATUS_MASK_REG
    #define Line_TXSTATUS_ACTL          Line_TXSTATUS_ACTL_REG
    /* DP clock */
    #if(Line_TXCLKGEN_DP)
        #define Line_TXBITCLKGEN_CTR        Line_TXBITCLKGEN_CTR_REG
        #define Line_TXBITCLKTX_COMPLETE    Line_TXBITCLKTX_COMPLETE_REG
    #else     /* Count7 clock*/
        #define Line_TXBITCTR_PERIOD        Line_TXBITCTR_PERIOD_REG
        #define Line_TXBITCTR_CONTROL       Line_TXBITCTR_CONTROL_REG
        #define Line_TXBITCTR_COUNTER       Line_TXBITCTR_COUNTER_REG
    #endif /* Line_TXCLKGEN_DP */
#endif /* End Line_TX_ENABLED */

#if(Line_HD_ENABLED)
    #define Line_TXDATA                 Line_TXDATA_REG
    #define Line_TXSTATUS               Line_TXSTATUS_REG
    #define Line_TXSTATUS_MASK          Line_TXSTATUS_MASK_REG
    #define Line_TXSTATUS_ACTL          Line_TXSTATUS_ACTL_REG
#endif /* End Line_HD_ENABLED */

#if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
    #define Line_RXDATA                 Line_RXDATA_REG
    #define Line_RXADDRESS1             Line_RXADDRESS1_REG
    #define Line_RXADDRESS2             Line_RXADDRESS2_REG
    #define Line_RXBITCTR_PERIOD        Line_RXBITCTR_PERIOD_REG
    #define Line_RXBITCTR_CONTROL       Line_RXBITCTR_CONTROL_REG
    #define Line_RXBITCTR_COUNTER       Line_RXBITCTR_COUNTER_REG
    #define Line_RXSTATUS               Line_RXSTATUS_REG
    #define Line_RXSTATUS_MASK          Line_RXSTATUS_MASK_REG
    #define Line_RXSTATUS_ACTL          Line_RXSTATUS_ACTL_REG
#endif /* End  (Line_RX_ENABLED) || (Line_HD_ENABLED) */

#if(Line_INTERNAL_CLOCK_USED)
    #define Line_INTCLOCK_CLKEN         Line_INTCLOCK_CLKEN_REG
#endif /* End Line_INTERNAL_CLOCK_USED */

#define Line_WAIT_FOR_COMLETE_REINIT    Line_WAIT_FOR_COMPLETE_REINIT

#endif  /* CY_UART_Line_H */


/* [] END OF FILE */
