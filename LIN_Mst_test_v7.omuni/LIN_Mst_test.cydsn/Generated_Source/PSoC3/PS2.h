/*******************************************************************************
* File Name: PS2.h
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


#if !defined(CY_UART_PS2_H)
#define CY_UART_PS2_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h"


/***************************************
* Conditional Compilation Parameters
***************************************/

#define PS2_RX_ENABLED                     (1u)
#define PS2_TX_ENABLED                     (0u)
#define PS2_HD_ENABLED                     (0u)
#define PS2_RX_INTERRUPT_ENABLED           (1u)
#define PS2_TX_INTERRUPT_ENABLED           (0u)
#define PS2_INTERNAL_CLOCK_USED            (1u)
#define PS2_RXHW_ADDRESS_ENABLED           (0u)
#define PS2_OVER_SAMPLE_COUNT              (8u)
#define PS2_PARITY_TYPE                    (0u)
#define PS2_PARITY_TYPE_SW                 (0u)
#define PS2_BREAK_DETECT                   (0u)
#define PS2_BREAK_BITS_TX                  (13u)
#define PS2_BREAK_BITS_RX                  (13u)
#define PS2_TXCLKGEN_DP                    (1u)
#define PS2_USE23POLLING                   (1u)
#define PS2_FLOW_CONTROL                   (0u)
#define PS2_CLK_FREQ                       (0u)
#define PS2_TXBUFFERSIZE                   (4u)
#define PS2_RXBUFFERSIZE                   (30u)

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component UART_v2_30 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

#ifdef PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define PS2_CONTROL_REG_REMOVED            (0u)
#else
    #define PS2_CONTROL_REG_REMOVED            (1u)
#endif /* End PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */


/***************************************
*      Data Struct Definition
***************************************/

/* Sleep Mode API Support */
typedef struct PS2_backupStruct_
{
    uint8 enableState;

    #if(PS2_CONTROL_REG_REMOVED == 0u)
        uint8 cr;
    #endif /* End PS2_CONTROL_REG_REMOVED */
    #if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
        uint8 rx_period;
        #if (CY_UDB_V0)
            uint8 rx_mask;
            #if (PS2_RXHW_ADDRESS_ENABLED)
                uint8 rx_addr1;
                uint8 rx_addr2;
            #endif /* End PS2_RXHW_ADDRESS_ENABLED */
        #endif /* End CY_UDB_V0 */
    #endif  /* End (PS2_RX_ENABLED) || (PS2_HD_ENABLED)*/

    #if(PS2_TX_ENABLED)
        #if(PS2_TXCLKGEN_DP)
            uint8 tx_clk_ctr;
            #if (CY_UDB_V0)
                uint8 tx_clk_compl;
            #endif  /* End CY_UDB_V0 */
        #else
            uint8 tx_period;
        #endif /*End PS2_TXCLKGEN_DP */
        #if (CY_UDB_V0)
            uint8 tx_mask;
        #endif  /* End CY_UDB_V0 */
    #endif /*End PS2_TX_ENABLED */
} PS2_BACKUP_STRUCT;


/***************************************
*       Function Prototypes
***************************************/

void PS2_Start(void) ;
void PS2_Stop(void) ;
uint8 PS2_ReadControlRegister(void) ;
void PS2_WriteControlRegister(uint8 control) ;

void PS2_Init(void) ;
void PS2_Enable(void) ;
void PS2_SaveConfig(void) ;
void PS2_RestoreConfig(void) ;
void PS2_Sleep(void) ;
void PS2_Wakeup(void) ;

/* Only if RX is enabled */
#if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )

    #if(PS2_RX_INTERRUPT_ENABLED)
        void  PS2_EnableRxInt(void) ;
        void  PS2_DisableRxInt(void) ;
        CY_ISR_PROTO(PS2_RXISR);
    #endif /* PS2_RX_INTERRUPT_ENABLED */

    void PS2_SetRxAddressMode(uint8 addressMode)
                                                           ;
    void PS2_SetRxAddress1(uint8 address) ;
    void PS2_SetRxAddress2(uint8 address) ;

    void  PS2_SetRxInterruptMode(uint8 intSrc) ;
    uint8 PS2_ReadRxData(void) ;
    uint8 PS2_ReadRxStatus(void) ;
    uint8 PS2_GetChar(void) ;
    uint16 PS2_GetByte(void) ;
    uint8 PS2_GetRxBufferSize(void)
                                                            ;
    void PS2_ClearRxBuffer(void) ;

    /* Obsolete functions, defines for backward compatible */
    #define PS2_GetRxInterruptSource   PS2_ReadRxStatus

#endif /* End (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */

/* Only if TX is enabled */
#if(PS2_TX_ENABLED || PS2_HD_ENABLED)

    #if(PS2_TX_INTERRUPT_ENABLED)
        void PS2_EnableTxInt(void) ;
        void PS2_DisableTxInt(void) ;
        CY_ISR_PROTO(PS2_TXISR);
    #endif /* PS2_TX_INTERRUPT_ENABLED */

    void PS2_SetTxInterruptMode(uint8 intSrc) ;
    void PS2_WriteTxData(uint8 txDataByte) ;
    uint8 PS2_ReadTxStatus(void) ;
    void PS2_PutChar(uint8 txDataByte) ;
    void PS2_PutString(const char8 string[]) ;
    void PS2_PutArray(const uint8 string[], uint8 byteCount)
                                                            ;
    void PS2_PutCRLF(uint8 txDataByte) ;
    void PS2_ClearTxBuffer(void) ;
    void PS2_SetTxAddressMode(uint8 addressMode) ;
    void PS2_SendBreak(uint8 retMode) ;
    uint8 PS2_GetTxBufferSize(void)
                                                            ;
    /* Obsolete functions, defines for backward compatible */
    #define PS2_PutStringConst         PS2_PutString
    #define PS2_PutArrayConst          PS2_PutArray
    #define PS2_GetTxInterruptSource   PS2_ReadTxStatus

#endif /* End PS2_TX_ENABLED || PS2_HD_ENABLED */

#if(PS2_HD_ENABLED)
    void PS2_LoadRxConfig(void) ;
    void PS2_LoadTxConfig(void) ;
#endif /* End PS2_HD_ENABLED */


/* Communication bootloader APIs */
#if defined(CYDEV_BOOTLOADER_IO_COMP) && ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_PS2) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))
    /* Physical layer functions */
    void    PS2_CyBtldrCommStart(void) CYSMALL ;
    void    PS2_CyBtldrCommStop(void) CYSMALL ;
    void    PS2_CyBtldrCommReset(void) CYSMALL ;
    cystatus PS2_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;
    cystatus PS2_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;

    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_PS2)
        #define CyBtldrCommStart    PS2_CyBtldrCommStart
        #define CyBtldrCommStop     PS2_CyBtldrCommStop
        #define CyBtldrCommReset    PS2_CyBtldrCommReset
        #define CyBtldrCommWrite    PS2_CyBtldrCommWrite
        #define CyBtldrCommRead     PS2_CyBtldrCommRead
    #endif  /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_PS2) */

    /* Byte to Byte time out for detecting end of block data from host */
    #define PS2_BYTE2BYTE_TIME_OUT (25u)

#endif /* CYDEV_BOOTLOADER_IO_COMP */


/***************************************
*          API Constants
***************************************/
/* Parameters for SetTxAddressMode API*/
#define PS2_SET_SPACE                              (0x00u)
#define PS2_SET_MARK                               (0x01u)

/* Status Register definitions */
#if( (PS2_TX_ENABLED) || (PS2_HD_ENABLED) )
    #if(PS2_TX_INTERRUPT_ENABLED)
        #define PS2_TX_VECT_NUM            (uint8)PS2_TXInternalInterrupt__INTC_NUMBER
        #define PS2_TX_PRIOR_NUM           (uint8)PS2_TXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* PS2_TX_INTERRUPT_ENABLED */
    #if(PS2_TX_ENABLED)
        #define PS2_TX_STS_COMPLETE_SHIFT          (0x00u)
        #define PS2_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
        #define PS2_TX_STS_FIFO_FULL_SHIFT         (0x02u)
        #define PS2_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #endif /* PS2_TX_ENABLED */
    #if(PS2_HD_ENABLED)
        #define PS2_TX_STS_COMPLETE_SHIFT          (0x00u)
        #define PS2_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
        #define PS2_TX_STS_FIFO_FULL_SHIFT         (0x05u)  /*needs MD=0*/
        #define PS2_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #endif /* PS2_HD_ENABLED */
    #define PS2_TX_STS_COMPLETE            (uint8)(0x01u << PS2_TX_STS_COMPLETE_SHIFT)
    #define PS2_TX_STS_FIFO_EMPTY          (uint8)(0x01u << PS2_TX_STS_FIFO_EMPTY_SHIFT)
    #define PS2_TX_STS_FIFO_FULL           (uint8)(0x01u << PS2_TX_STS_FIFO_FULL_SHIFT)
    #define PS2_TX_STS_FIFO_NOT_FULL       (uint8)(0x01u << PS2_TX_STS_FIFO_NOT_FULL_SHIFT)
#endif /* End (PS2_TX_ENABLED) || (PS2_HD_ENABLED)*/

#if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
    #if(PS2_RX_INTERRUPT_ENABLED)
        #define PS2_RX_VECT_NUM            (uint8)PS2_RXInternalInterrupt__INTC_NUMBER
        #define PS2_RX_PRIOR_NUM           (uint8)PS2_RXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* PS2_RX_INTERRUPT_ENABLED */
    #define PS2_RX_STS_MRKSPC_SHIFT            (0x00u)
    #define PS2_RX_STS_BREAK_SHIFT             (0x01u)
    #define PS2_RX_STS_PAR_ERROR_SHIFT         (0x02u)
    #define PS2_RX_STS_STOP_ERROR_SHIFT        (0x03u)
    #define PS2_RX_STS_OVERRUN_SHIFT           (0x04u)
    #define PS2_RX_STS_FIFO_NOTEMPTY_SHIFT     (0x05u)
    #define PS2_RX_STS_ADDR_MATCH_SHIFT        (0x06u)
    #define PS2_RX_STS_SOFT_BUFF_OVER_SHIFT    (0x07u)

    #define PS2_RX_STS_MRKSPC           (uint8)(0x01u << PS2_RX_STS_MRKSPC_SHIFT)
    #define PS2_RX_STS_BREAK            (uint8)(0x01u << PS2_RX_STS_BREAK_SHIFT)
    #define PS2_RX_STS_PAR_ERROR        (uint8)(0x01u << PS2_RX_STS_PAR_ERROR_SHIFT)
    #define PS2_RX_STS_STOP_ERROR       (uint8)(0x01u << PS2_RX_STS_STOP_ERROR_SHIFT)
    #define PS2_RX_STS_OVERRUN          (uint8)(0x01u << PS2_RX_STS_OVERRUN_SHIFT)
    #define PS2_RX_STS_FIFO_NOTEMPTY    (uint8)(0x01u << PS2_RX_STS_FIFO_NOTEMPTY_SHIFT)
    #define PS2_RX_STS_ADDR_MATCH       (uint8)(0x01u << PS2_RX_STS_ADDR_MATCH_SHIFT)
    #define PS2_RX_STS_SOFT_BUFF_OVER   (uint8)(0x01u << PS2_RX_STS_SOFT_BUFF_OVER_SHIFT)
    #define PS2_RX_HW_MASK                     (0x7Fu)
#endif /* End (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */

/* Control Register definitions */
#define PS2_CTRL_HD_SEND_SHIFT                 (0x00u) /* 1 enable TX part in Half Duplex mode */
#define PS2_CTRL_HD_SEND_BREAK_SHIFT           (0x01u) /* 1 send BREAK signal in Half Duplez mode */
#define PS2_CTRL_MARK_SHIFT                    (0x02u) /* 1 sets mark, 0 sets space */
#define PS2_CTRL_PARITY_TYPE0_SHIFT            (0x03u) /* Defines the type of parity implemented */
#define PS2_CTRL_PARITY_TYPE1_SHIFT            (0x04u) /* Defines the type of parity implemented */
#define PS2_CTRL_RXADDR_MODE0_SHIFT            (0x05u)
#define PS2_CTRL_RXADDR_MODE1_SHIFT            (0x06u)
#define PS2_CTRL_RXADDR_MODE2_SHIFT            (0x07u)

#define PS2_CTRL_HD_SEND               (uint8)(0x01u << PS2_CTRL_HD_SEND_SHIFT)
#define PS2_CTRL_HD_SEND_BREAK         (uint8)(0x01u << PS2_CTRL_HD_SEND_BREAK_SHIFT)
#define PS2_CTRL_MARK                  (uint8)(0x01u << PS2_CTRL_MARK_SHIFT)
#define PS2_CTRL_PARITY_TYPE_MASK      (uint8)(0x03u << PS2_CTRL_PARITY_TYPE0_SHIFT)
#define PS2_CTRL_RXADDR_MODE_MASK      (uint8)(0x07u << PS2_CTRL_RXADDR_MODE0_SHIFT)

/* StatusI Register Interrupt Enable Control Bits. As defined by the Register map for the AUX Control Register */
#define PS2_INT_ENABLE                         (0x10u)

/* Bit Counter (7-bit) Control Register Bit Definitions. As defined by the Register map for the AUX Control Register */
#define PS2_CNTR_ENABLE                        (0x20u)

/*   Constants for SendBreak() "retMode" parameter  */
#define PS2_SEND_BREAK                         (0x00u)
#define PS2_WAIT_FOR_COMPLETE_REINIT           (0x01u)
#define PS2_REINIT                             (0x02u)
#define PS2_SEND_WAIT_REINIT                   (0x03u)

#define PS2_OVER_SAMPLE_8                      (8u)
#define PS2_OVER_SAMPLE_16                     (16u)

#define PS2_BIT_CENTER                         (PS2_OVER_SAMPLE_COUNT - 1u)

#define PS2_FIFO_LENGTH                        (4u)
#define PS2_NUMBER_OF_START_BIT                (1u)
#define PS2_MAX_BYTE_VALUE                     (0xFFu)

/* 8X always for count7 implementation*/
#define PS2_TXBITCTR_BREAKBITS8X   ((PS2_BREAK_BITS_TX * PS2_OVER_SAMPLE_8) - 1u)
/* 8X or 16X for DP implementation*/
#define PS2_TXBITCTR_BREAKBITS ((PS2_BREAK_BITS_TX * PS2_OVER_SAMPLE_COUNT) - 1u)

#define PS2_HALF_BIT_COUNT   \
                            (((PS2_OVER_SAMPLE_COUNT / 2u) + (PS2_USE23POLLING * 1u)) - 2u)
#if (PS2_OVER_SAMPLE_COUNT == PS2_OVER_SAMPLE_8)
    #define PS2_HD_TXBITCTR_INIT   (((PS2_BREAK_BITS_TX + \
                            PS2_NUMBER_OF_START_BIT) * PS2_OVER_SAMPLE_COUNT) - 1u)

    /* This parameter is increased on the 2 in 2 out of 3 mode to sample voting in the middle */
    #define PS2_RXBITCTR_INIT  ((((PS2_BREAK_BITS_RX + PS2_NUMBER_OF_START_BIT) \
                            * PS2_OVER_SAMPLE_COUNT) + PS2_HALF_BIT_COUNT) - 1u)


#else /* PS2_OVER_SAMPLE_COUNT == PS2_OVER_SAMPLE_16 */
    #define PS2_HD_TXBITCTR_INIT   ((8u * PS2_OVER_SAMPLE_COUNT) - 1u)
    /* 7bit counter need one more bit for OverSampleCount=16 */
    #define PS2_RXBITCTR_INIT      (((7u * PS2_OVER_SAMPLE_COUNT) - 1u) + \
                                                      PS2_HALF_BIT_COUNT)
#endif /* End PS2_OVER_SAMPLE_COUNT */
#define PS2_HD_RXBITCTR_INIT                   PS2_RXBITCTR_INIT


/***************************************
* Global variables external identifier
***************************************/

extern uint8 PS2_initVar;
#if( PS2_TX_ENABLED && (PS2_TXBUFFERSIZE > PS2_FIFO_LENGTH))
    extern volatile uint8 PS2_txBuffer[PS2_TXBUFFERSIZE];
    extern volatile uint8 PS2_txBufferRead;
    extern uint8 PS2_txBufferWrite;
#endif /* End PS2_TX_ENABLED */
#if( ( PS2_RX_ENABLED || PS2_HD_ENABLED ) && \
     (PS2_RXBUFFERSIZE > PS2_FIFO_LENGTH) )
    extern volatile uint8 PS2_rxBuffer[PS2_RXBUFFERSIZE];
    extern volatile uint8 PS2_rxBufferRead;
    extern volatile uint8 PS2_rxBufferWrite;
    extern volatile uint8 PS2_rxBufferLoopDetect;
    extern volatile uint8 PS2_rxBufferOverflow;
    #if (PS2_RXHW_ADDRESS_ENABLED)
        extern volatile uint8 PS2_rxAddressMode;
        extern volatile uint8 PS2_rxAddressDetected;
    #endif /* End EnableHWAddress */
#endif /* End PS2_RX_ENABLED */


/***************************************
* Enumerated Types and Parameters
***************************************/

#define PS2__B_UART__AM_SW_BYTE_BYTE 1
#define PS2__B_UART__AM_SW_DETECT_TO_BUFFER 2
#define PS2__B_UART__AM_HW_BYTE_BY_BYTE 3
#define PS2__B_UART__AM_HW_DETECT_TO_BUFFER 4
#define PS2__B_UART__AM_NONE 0

#define PS2__B_UART__NONE_REVB 0
#define PS2__B_UART__EVEN_REVB 1
#define PS2__B_UART__ODD_REVB 2
#define PS2__B_UART__MARK_SPACE_REVB 3



/***************************************
*    Initial Parameter Constants
***************************************/

/* UART shifts max 8 bits, Mark/Space functionality working if 9 selected */
#define PS2_NUMBER_OF_DATA_BITS    ((8u > 8u) ? 8u : 8u)
#define PS2_NUMBER_OF_STOP_BITS    (1u)

#if (PS2_RXHW_ADDRESS_ENABLED)
    #define PS2_RXADDRESSMODE      (0u)
    #define PS2_RXHWADDRESS1       (0u)
    #define PS2_RXHWADDRESS2       (0u)
    /* Backward compatible define */
    #define PS2_RXAddressMode      PS2_RXADDRESSMODE
#endif /* End EnableHWAddress */

#define PS2_INIT_RX_INTERRUPTS_MASK \
                                  (uint8)((1 << PS2_RX_STS_FIFO_NOTEMPTY_SHIFT) \
                                        | (0 << PS2_RX_STS_MRKSPC_SHIFT) \
                                        | (0 << PS2_RX_STS_ADDR_MATCH_SHIFT) \
                                        | (0 << PS2_RX_STS_PAR_ERROR_SHIFT) \
                                        | (0 << PS2_RX_STS_STOP_ERROR_SHIFT) \
                                        | (0 << PS2_RX_STS_BREAK_SHIFT) \
                                        | (0 << PS2_RX_STS_OVERRUN_SHIFT))

#define PS2_INIT_TX_INTERRUPTS_MASK \
                                  (uint8)((0 << PS2_TX_STS_COMPLETE_SHIFT) \
                                        | (0 << PS2_TX_STS_FIFO_EMPTY_SHIFT) \
                                        | (0 << PS2_TX_STS_FIFO_FULL_SHIFT) \
                                        | (0 << PS2_TX_STS_FIFO_NOT_FULL_SHIFT))


/***************************************
*              Registers
***************************************/

#ifdef PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define PS2_CONTROL_REG \
                            (* (reg8 *) PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
    #define PS2_CONTROL_PTR \
                            (  (reg8 *) PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
#endif /* End PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(PS2_TX_ENABLED)
    #define PS2_TXDATA_REG          (* (reg8 *) PS2_BUART_sTX_TxShifter_u0__F0_REG)
    #define PS2_TXDATA_PTR          (  (reg8 *) PS2_BUART_sTX_TxShifter_u0__F0_REG)
    #define PS2_TXDATA_AUX_CTL_REG  (* (reg8 *) PS2_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define PS2_TXDATA_AUX_CTL_PTR  (  (reg8 *) PS2_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define PS2_TXSTATUS_REG        (* (reg8 *) PS2_BUART_sTX_TxSts__STATUS_REG)
    #define PS2_TXSTATUS_PTR        (  (reg8 *) PS2_BUART_sTX_TxSts__STATUS_REG)
    #define PS2_TXSTATUS_MASK_REG   (* (reg8 *) PS2_BUART_sTX_TxSts__MASK_REG)
    #define PS2_TXSTATUS_MASK_PTR   (  (reg8 *) PS2_BUART_sTX_TxSts__MASK_REG)
    #define PS2_TXSTATUS_ACTL_REG   (* (reg8 *) PS2_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)
    #define PS2_TXSTATUS_ACTL_PTR   (  (reg8 *) PS2_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)

    /* DP clock */
    #if(PS2_TXCLKGEN_DP)
        #define PS2_TXBITCLKGEN_CTR_REG        \
                                        (* (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define PS2_TXBITCLKGEN_CTR_PTR        \
                                        (  (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define PS2_TXBITCLKTX_COMPLETE_REG    \
                                        (* (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
        #define PS2_TXBITCLKTX_COMPLETE_PTR    \
                                        (  (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
    #else     /* Count7 clock*/
        #define PS2_TXBITCTR_PERIOD_REG    \
                                        (* (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define PS2_TXBITCTR_PERIOD_PTR    \
                                        (  (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define PS2_TXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define PS2_TXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define PS2_TXBITCTR_COUNTER_REG   \
                                        (* (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
        #define PS2_TXBITCTR_COUNTER_PTR   \
                                        (  (reg8 *) PS2_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
    #endif /* PS2_TXCLKGEN_DP */

#endif /* End PS2_TX_ENABLED */

#if(PS2_HD_ENABLED)

    #define PS2_TXDATA_REG             (* (reg8 *) PS2_BUART_sRX_RxShifter_u0__F1_REG )
    #define PS2_TXDATA_PTR             (  (reg8 *) PS2_BUART_sRX_RxShifter_u0__F1_REG )
    #define PS2_TXDATA_AUX_CTL_REG     (* (reg8 *) PS2_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
    #define PS2_TXDATA_AUX_CTL_PTR     (  (reg8 *) PS2_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define PS2_TXSTATUS_REG           (* (reg8 *) PS2_BUART_sRX_RxSts__STATUS_REG )
    #define PS2_TXSTATUS_PTR           (  (reg8 *) PS2_BUART_sRX_RxSts__STATUS_REG )
    #define PS2_TXSTATUS_MASK_REG      (* (reg8 *) PS2_BUART_sRX_RxSts__MASK_REG )
    #define PS2_TXSTATUS_MASK_PTR      (  (reg8 *) PS2_BUART_sRX_RxSts__MASK_REG )
    #define PS2_TXSTATUS_ACTL_REG      (* (reg8 *) PS2_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define PS2_TXSTATUS_ACTL_PTR      (  (reg8 *) PS2_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End PS2_HD_ENABLED */

#if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
    #define PS2_RXDATA_REG             (* (reg8 *) PS2_BUART_sRX_RxShifter_u0__F0_REG )
    #define PS2_RXDATA_PTR             (  (reg8 *) PS2_BUART_sRX_RxShifter_u0__F0_REG )
    #define PS2_RXADDRESS1_REG         (* (reg8 *) PS2_BUART_sRX_RxShifter_u0__D0_REG )
    #define PS2_RXADDRESS1_PTR         (  (reg8 *) PS2_BUART_sRX_RxShifter_u0__D0_REG )
    #define PS2_RXADDRESS2_REG         (* (reg8 *) PS2_BUART_sRX_RxShifter_u0__D1_REG )
    #define PS2_RXADDRESS2_PTR         (  (reg8 *) PS2_BUART_sRX_RxShifter_u0__D1_REG )
    #define PS2_RXDATA_AUX_CTL_REG     (* (reg8 *) PS2_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define PS2_RXBITCTR_PERIOD_REG    (* (reg8 *) PS2_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define PS2_RXBITCTR_PERIOD_PTR    (  (reg8 *) PS2_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define PS2_RXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) PS2_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define PS2_RXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) PS2_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define PS2_RXBITCTR_COUNTER_REG   (* (reg8 *) PS2_BUART_sRX_RxBitCounter__COUNT_REG )
    #define PS2_RXBITCTR_COUNTER_PTR   (  (reg8 *) PS2_BUART_sRX_RxBitCounter__COUNT_REG )

    #define PS2_RXSTATUS_REG           (* (reg8 *) PS2_BUART_sRX_RxSts__STATUS_REG )
    #define PS2_RXSTATUS_PTR           (  (reg8 *) PS2_BUART_sRX_RxSts__STATUS_REG )
    #define PS2_RXSTATUS_MASK_REG      (* (reg8 *) PS2_BUART_sRX_RxSts__MASK_REG )
    #define PS2_RXSTATUS_MASK_PTR      (  (reg8 *) PS2_BUART_sRX_RxSts__MASK_REG )
    #define PS2_RXSTATUS_ACTL_REG      (* (reg8 *) PS2_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define PS2_RXSTATUS_ACTL_PTR      (  (reg8 *) PS2_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End  (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */

#if(PS2_INTERNAL_CLOCK_USED)
    /* Register to enable or disable the digital clocks */
    #define PS2_INTCLOCK_CLKEN_REG     (* (reg8 *) PS2_IntClock__PM_ACT_CFG)
    #define PS2_INTCLOCK_CLKEN_PTR     (  (reg8 *) PS2_IntClock__PM_ACT_CFG)

    /* Clock mask for this clock. */
    #define PS2_INTCLOCK_CLKEN_MASK    PS2_IntClock__PM_ACT_MSK
#endif /* End PS2_INTERNAL_CLOCK_USED */


/***************************************
*       Register Constants
***************************************/

#if(PS2_TX_ENABLED)
    #define PS2_TX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End PS2_TX_ENABLED */

#if(PS2_HD_ENABLED)
    #define PS2_TX_FIFO_CLR            (0x02u) /* FIFO1 CLR */
#endif /* End PS2_HD_ENABLED */

#if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
    #define PS2_RX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End  (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */


/***************************************
* Renamed global variables or defines
* for backward compatible
***************************************/

#define PS2_initvar                    PS2_initVar

#define PS2_RX_Enabled                 PS2_RX_ENABLED
#define PS2_TX_Enabled                 PS2_TX_ENABLED
#define PS2_HD_Enabled                 PS2_HD_ENABLED
#define PS2_RX_IntInterruptEnabled     PS2_RX_INTERRUPT_ENABLED
#define PS2_TX_IntInterruptEnabled     PS2_TX_INTERRUPT_ENABLED
#define PS2_InternalClockUsed          PS2_INTERNAL_CLOCK_USED
#define PS2_RXHW_Address_Enabled       PS2_RXHW_ADDRESS_ENABLED
#define PS2_OverSampleCount            PS2_OVER_SAMPLE_COUNT
#define PS2_ParityType                 PS2_PARITY_TYPE

#if( PS2_TX_ENABLED && (PS2_TXBUFFERSIZE > PS2_FIFO_LENGTH))
    #define PS2_TXBUFFER               PS2_txBuffer
    #define PS2_TXBUFFERREAD           PS2_txBufferRead
    #define PS2_TXBUFFERWRITE          PS2_txBufferWrite
#endif /* End PS2_TX_ENABLED */
#if( ( PS2_RX_ENABLED || PS2_HD_ENABLED ) && \
     (PS2_RXBUFFERSIZE > PS2_FIFO_LENGTH) )
    #define PS2_RXBUFFER               PS2_rxBuffer
    #define PS2_RXBUFFERREAD           PS2_rxBufferRead
    #define PS2_RXBUFFERWRITE          PS2_rxBufferWrite
    #define PS2_RXBUFFERLOOPDETECT     PS2_rxBufferLoopDetect
    #define PS2_RXBUFFER_OVERFLOW      PS2_rxBufferOverflow
#endif /* End PS2_RX_ENABLED */

#ifdef PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define PS2_CONTROL                PS2_CONTROL_REG
#endif /* End PS2_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(PS2_TX_ENABLED)
    #define PS2_TXDATA                 PS2_TXDATA_REG
    #define PS2_TXSTATUS               PS2_TXSTATUS_REG
    #define PS2_TXSTATUS_MASK          PS2_TXSTATUS_MASK_REG
    #define PS2_TXSTATUS_ACTL          PS2_TXSTATUS_ACTL_REG
    /* DP clock */
    #if(PS2_TXCLKGEN_DP)
        #define PS2_TXBITCLKGEN_CTR        PS2_TXBITCLKGEN_CTR_REG
        #define PS2_TXBITCLKTX_COMPLETE    PS2_TXBITCLKTX_COMPLETE_REG
    #else     /* Count7 clock*/
        #define PS2_TXBITCTR_PERIOD        PS2_TXBITCTR_PERIOD_REG
        #define PS2_TXBITCTR_CONTROL       PS2_TXBITCTR_CONTROL_REG
        #define PS2_TXBITCTR_COUNTER       PS2_TXBITCTR_COUNTER_REG
    #endif /* PS2_TXCLKGEN_DP */
#endif /* End PS2_TX_ENABLED */

#if(PS2_HD_ENABLED)
    #define PS2_TXDATA                 PS2_TXDATA_REG
    #define PS2_TXSTATUS               PS2_TXSTATUS_REG
    #define PS2_TXSTATUS_MASK          PS2_TXSTATUS_MASK_REG
    #define PS2_TXSTATUS_ACTL          PS2_TXSTATUS_ACTL_REG
#endif /* End PS2_HD_ENABLED */

#if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
    #define PS2_RXDATA                 PS2_RXDATA_REG
    #define PS2_RXADDRESS1             PS2_RXADDRESS1_REG
    #define PS2_RXADDRESS2             PS2_RXADDRESS2_REG
    #define PS2_RXBITCTR_PERIOD        PS2_RXBITCTR_PERIOD_REG
    #define PS2_RXBITCTR_CONTROL       PS2_RXBITCTR_CONTROL_REG
    #define PS2_RXBITCTR_COUNTER       PS2_RXBITCTR_COUNTER_REG
    #define PS2_RXSTATUS               PS2_RXSTATUS_REG
    #define PS2_RXSTATUS_MASK          PS2_RXSTATUS_MASK_REG
    #define PS2_RXSTATUS_ACTL          PS2_RXSTATUS_ACTL_REG
#endif /* End  (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */

#if(PS2_INTERNAL_CLOCK_USED)
    #define PS2_INTCLOCK_CLKEN         PS2_INTCLOCK_CLKEN_REG
#endif /* End PS2_INTERNAL_CLOCK_USED */

#define PS2_WAIT_FOR_COMLETE_REINIT    PS2_WAIT_FOR_COMPLETE_REINIT

#endif  /* CY_UART_PS2_H */


/* [] END OF FILE */
