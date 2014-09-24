/*******************************************************************************
* File Name: LIN_1.h
* Version 1.30
*
* Description:
*  This file contains implementation of LIN.
*
********************************************************************************
* Copyright 2011-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_LIN_LIN_1_H)
#define CY_LIN_LIN_1_H

#include "cyfitter.h"
#include "CyLib.h"
#include "cytypes.h"

#include "LIN_1_UART.h"
#include "LIN_1_IntClk.h"


/* Check to see if required defines such as CY_PSOC5A are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5A)
    #error Component LIN_v1_30 requires cy_boot v3.0 or later
#endif /* (CY_ PSOC5A) */


/***************************************
* Conditional Compilation Parameters
***************************************/

/* General */
#define LIN_1_RESPONSE_ERROR_SIGNAL  (1u)
#define LIN_1_SAE_J2602              (0u)
#define LIN_1_LIN_2_0                (0u)

#define LIN_1_INACTIVITY_ENABLED     (1u)
#define LIN_1_INACTIVITY_THRESHOLD   (7000u)
#define LIN_1_BREAK_THRESHOLD        (11)

/* Baud Rate */
#define LIN_1_AUTO_BAUD_RATE_SYNC    (1u)
#define LIN_1_BAUD_RATE              (19200u)
#define LIN_1_INTERNAL_CLOCK         (u)

/* Transport Layer */
#define LIN_1_TL_ENABLED             (0u)
#define LIN_1_TL_INITIAL_NAD         (1u)
#define LIN_1_TL_API_FORMAT          (1u)
#define LIN_1_TL_BUF_LEN_MAX         (512u)
#define LIN_1_TL_TX_QUEUE_LEN        (32u)
#define LIN_1_TL_RX_QUEUE_LEN        (32u)

/* Config. Services */
#define LIN_1_CS_ENABLED             (0u)
#define LIN_1_CS_SUPPLIER_ID         (32766u)
#define LIN_1_CS_FUNCTION_ID         (65534u)
#define LIN_1_CS_VARIANT             (0u)
#define LIN_1_CS_SAVE_ADDR           (u)
#define LIN_1_CS_SEL_SERVICES01      (128u)

/* Interface handle constant  for LIN_1 */
#define LIN_1_IFC_HANDLE             (0x00u)

/* General frames and signals information */
#define LIN_1_NUM_FRAMES            (0x03u)
#define LIN_1_NUM_ET_FRAMES         (0x00u)
#define LIN_1_NUM_SIGNALS           (0x05u)
#define LIN_1_NUM_UNIQUE_SIGNALS    (0x05u)
#define LIN_1_RESPONSE_ERROR_FRAME_INDEX    (0x00u)


#define LIN_1_NUM_SERVICE_FRAMES     (0x02u)     /* MRF and SRF */

#define LIN_1_SIG_FRAME_FLAGS_SIZE   ((LIN_1_NUM_FRAMES         + \
                                                  LIN_1_NUM_SIGNALS        + \
                                                  LIN_1_NUM_UNIQUE_SIGNALS + \
                                                  LIN_1_NUM_SERVICE_FRAMES) / 8u + 1u)

#define LIN_1_ET_FRAMES_FLAGS_SIZE   (LIN_1_NUM_ET_FRAMES / 8u + 1u)

#define LIN_1_INACTIVITY_THRESHOLD_IN_100_MS    (LIN_1_INACTIVITY_THRESHOLD / 100u)

#if(1u == LIN_1_TL_ENABLED)
    #if(1u == LIN_1_CS_ENABLED)

        /* Bit masks for selected services */
        #define LIN_1_NCS_0xB0_SEL               (0x01u)
        #define LIN_1_NCS_0xB1_SEL               (0x02u)
        #define LIN_1_NCS_0xB2_SEL               (0x04u)
        #define LIN_1_NCS_0xB3_SEL               (0x08u)
        #define LIN_1_NCS_0xB4_SEL               (0x10u)
        #define LIN_1_NCS_0xB5_SEL               (0x20u)
        #define LIN_1_NCS_0xB6_SEL               (0x40u)
        #define LIN_1_NCS_0xB7_SEL               (0x80u)

    #endif /*(1u == LIN_1_CS_ENABLED) */
#endif /* (1u == LIN_1_TL_ENABLED) */


/***************************************
*     Data Types Definitions
***************************************/

/* LIN Core Types */
typedef unsigned char   l_bool;
typedef unsigned char   l_u8;
typedef unsigned short  l_u16;
typedef unsigned char   l_irqmask;
typedef unsigned char   l_ioctl_op;
typedef unsigned char   l_signal_handle;
typedef unsigned char   l_ifc_handle;
typedef unsigned char   l_flag_handle;

/* Detailed PID information */
typedef struct
{
    l_u8   param;
    volatile l_u8   *dataPtr;
}   LIN_1_PID_INFO_TABLE;

/* LIN Slave Configuration */
typedef struct
{
    #if(1u == LIN_1_TL_ENABLED)

        /* Initial NAD */
        l_u8   initialNad;

    #endif /* (1u == LIN_1_TL_ENABLED) */

    /* PID Table */
    l_u8 pidTable[LIN_1_NUM_FRAMES];

 }   LIN_1_SLAVE_CONFIG;

#if(1u == LIN_1_CS_ENABLED)

    /* LIN Slave Identification */
    typedef struct
    {
        l_u16  supplierId;
        l_u16  functionId;
        l_u8   variant;
    }   LIN_1_SLAVE_ID;

#endif /* (1u == LIN_1_CS_ENABLED) */


/***************************************
*      Data Struct Definition
***************************************/

/* Sleep Mode API Support */
typedef struct
{
    l_u8 enableState;
    l_u8 control;

    l_u8 statusMask;

    #if(1u == LIN_1_INACTIVITY_ENABLED)

        l_u8 inactivityDiv0;
        l_u8 inactivityDiv1;

    #endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */

} LIN_1_BACKUP_STRUCT;


/**************************************
*  Function Prototypes
**************************************/

l_bool  l_sys_init(void)  ;
l_bool  LIN_1_Start(void) ;
l_u8    LIN_1_Stop(void) ;


/* Core API: Signal Interaction  Functions */
l_u8 l_u8_rd_Kp(void);
l_u8 l_u8_rd_Ki(void);
l_u8 l_u8_rd_Kd(void);
l_u8 l_u8_rd(l_signal_handle sss);
l_u16 l_u16_rd_target(void);
l_u16 l_u16_rd(l_signal_handle sss);

void l_bool_wr_Response_Error(l_bool v);
void l_bool_wr(l_signal_handle sss, l_bool v);

/* Core API Functions: Notification */
l_bool l_flg_tst_target(void);
l_bool l_flg_tst_Kp(void);
l_bool l_flg_tst_Ki(void);
l_bool l_flg_tst_Kd(void);
l_bool l_flg_tst_Frame1(void);
l_bool l_flg_tst_rpm(void);
l_bool l_flg_tst_PID(void);
l_bool l_flg_tst(l_flag_handle fff);

void   l_flg_clr_target(void);
void   l_flg_clr_Kp(void);
void   l_flg_clr_Ki(void);
void   l_flg_clr_Kd(void);
void   l_flg_clr_Frame1(void);
void   l_flg_clr_rpm(void);
void   l_flg_clr_PID(void);
void l_flg_clr(l_flag_handle fff);


l_bool l_ifc_init_LIN_1(void) ;
l_bool l_ifc_init(l_ifc_handle iii);

void l_ifc_wake_up_LIN_1(void) ;
void l_ifc_wake_up(l_ifc_handle iii) ;

l_u16 l_ifc_ioctl_LIN_1(l_ioctl_op op, void* pv) ;
l_u16 l_ifc_ioctl(l_ifc_handle iii, l_ioctl_op op, void* pv) ;

void l_ifc_rx_LIN_1(void) ;
void l_ifc_rx(l_ifc_handle iii) ;

void l_ifc_tx_LIN_1(void) ;
void l_ifc_tx(l_ifc_handle iii) ;

void l_ifc_aux_LIN_1(void) ;
void l_ifc_aux(l_ifc_handle iii) ;

l_u16 l_ifc_read_status_LIN_1(void) ;
l_u16 l_ifc_read_status(l_ifc_handle iii) ;

l_irqmask   l_sys_irq_disable(void)  ;
void        l_sys_irq_restore(l_irqmask previous) ;


#if(1u == LIN_1_TL_ENABLED)

    #if(1u == LIN_1_CS_ENABLED)
        #if((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)) || \
            (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL)))

        l_u8 ld_read_by_id_callout(l_ifc_handle iii, l_u8 id, l_u8* frameData)
                                                        ;

        #endif /*((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)) ||
               * (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL)))
               */

    #endif /* (1u == LIN_1_CS_ENABLED) */

    /* Transport Layer Functions: Initialization */
    void        ld_init(l_ifc_handle iii) ;

    /* Transport Layer Functions: Node Configuration Functions */
    l_u8        ld_read_configuration(l_ifc_handle iii, l_u8* const pData, l_u8* const length)
                                                    ;
    l_u8        ld_set_configuration(l_ifc_handle iii, const l_u8* const pData, l_u16 length)
                                                    ;

    #if(1u == LIN_1_TL_API_FORMAT)

        /* Transport Layer Functions: Cooked Transport Layer API */
        void        ld_send_message(l_ifc_handle iii, l_u16 length, l_u8 nad, const l_u8* const ld_data)
                                                    ;
        void        ld_receive_message(l_ifc_handle iii, l_u16* const length, l_u8* const nad, l_u8* const ld_data)
                        ;
        l_u8        ld_tx_status(l_ifc_handle iii) ;
        l_u8        ld_rx_status(l_ifc_handle iii) ;

    #else

        /* Transport Layer Functions: Raw Transport Layer API */
        void        ld_put_raw(l_ifc_handle iii, const l_u8* const ld_data) ;
        void        ld_get_raw(l_ifc_handle iii, l_u8* const ld_data) ;
        l_u8        ld_raw_tx_status(l_ifc_handle iii) ;
        l_u8        ld_raw_rx_status(l_ifc_handle iii) ;

    #endif /* (1u == LIN_1_TL_API_FORMAT) */

#endif /* (1u == LIN_1_TL_ENABLED) */

/* ISR Prototypes */
CY_ISR_PROTO(LIN_1_BLIN_ISR);
CY_ISR_PROTO(LIN_1_UART_ISR);

/* Swap U16 macro */
#define LIN_1_SWAP_U8_TO_U16(x,y)    (l_u16) ((l_u16) (((l_u16) (x)) << 8u) | (l_u16) (y))


/***************************************
*   API Constants
***************************************/
/* Constants for l_bool_wr_Response_Error() */
#define LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET    (0x00u)
#define LIN_1_Response_Error_Frame1_SIG_BIT_OFFSET     (0x00u)
#define LIN_1_Response_Error_Frame1_SIG_MASK_0         (0x01u)

/* Constants for l_u16_rd_target */
#define LIN_1_target_rpm_SIG_BYTE_OFFSET     (0x04u)

/* Constants for l_u8_rd_Kp */
#define LIN_1_Kp_PID_SIG_BYTE_OFFSET    (0x00u)

/* Constants for l_u8_rd_Ki */
#define LIN_1_Ki_PID_SIG_BYTE_OFFSET    (0x01u)

/* Constants for l_u8_rd_Kd */
#define LIN_1_Kd_PID_SIG_BYTE_OFFSET    (0x02u)


/* l_flg_tst_target() and l_flg_clr_target() */
#define LIN_1_target_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_target_FRAME_FLAG_MASK_0           (0x01u)

/* l_flg_tst_Kp() and l_flg_clr_Kp() */
#define LIN_1_Kp_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_Kp_FRAME_FLAG_MASK_0           (0x02u)

/* l_flg_tst_Ki() and l_flg_clr_Ki() */
#define LIN_1_Ki_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_Ki_FRAME_FLAG_MASK_0           (0x04u)

/* l_flg_tst_Kd() and l_flg_clr_Kd() */
#define LIN_1_Kd_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_Kd_FRAME_FLAG_MASK_0           (0x08u)

/* l_flg_tst_Frame1() and l_flg_clr_Frame1() */
#define LIN_1_Frame1_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_Frame1_FRAME_FLAG_MASK_0           (0x10u)

/* l_flg_tst_rpm() and l_flg_clr_rpm() */
#define LIN_1_rpm_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_rpm_FRAME_FLAG_MASK_0           (0x20u)

/* l_flg_tst_PID() and l_flg_clr_PID() */
#define LIN_1_PID_FRAME_FLAG_BYTE_OFFSET_0    (0x00u)
#define LIN_1_PID_FRAME_FLAG_MASK_0           (0x40u)




#define Response_Error_SIGNAL_HANDLE        (0x00u)
#define target_SIGNAL_HANDLE        (0x01u)
#define Kp_SIGNAL_HANDLE        (0x02u)
#define Ki_SIGNAL_HANDLE        (0x03u)
#define Kd_SIGNAL_HANDLE        (0x04u)



#define target_FLAG_HANDLE        (0x00u)
#define Kp_FLAG_HANDLE        (0x01u)
#define Ki_FLAG_HANDLE        (0x02u)
#define Kd_FLAG_HANDLE        (0x03u)
#define Frame1_FLAG_HANDLE        (0x04u)
#define rpm_FLAG_HANDLE        (0x05u)
#define PID_FLAG_HANDLE        (0x06u)



#define LIN_1_ISR_AUX_SET_FLAGS_Frame1_FLAG_BYTE_OFFSET_0        (0x00u)
#define LIN_1_ISR_AUX_SET_FLAGS_Frame1_FLAG_MASK_0        (0x10u)
#define LIN_1_ISR_AUX_SET_FLAGS_rpm_FLAG_BYTE_OFFSET_0        (0x00u)
#define LIN_1_ISR_AUX_SET_FLAGS_rpm_FLAG_MASK_0        (0x21u)
#define LIN_1_ISR_AUX_SET_FLAGS_PID_FLAG_BYTE_OFFSET_0        (0x00u)
#define LIN_1_ISR_AUX_SET_FLAGS_PID_FLAG_MASK_0        (0x4Eu)



/* Break threshold detection value assuming 16x oversampling */
#define LIN_1_BREAK_THRESHOLD_VALUE      (16 * LIN_1_BREAK_THRESHOLD)

/* 8 bits with 16x oversampling rate */
#define LIN_1_EXPECTED_TIME_COUNTS       (128u)

/* Bus inactivity block reconfiguration */
#if(1u == LIN_1_INACTIVITY_ENABLED)

    /* Bus inactivity block configuration to issue interrupt */
    #define LIN_1_INACT_100MS_IN_S           (10u)
    #define LIN_1_INACT_OVERSAMPLE_RATE      (16u)
    #define LIN_1_INACT_DIVIDE_FACTOR        (256u)

    #define LIN_1_INACT_TIME_FACTOR          \
        ((l_u16) ((LIN_1_INACT_OVERSAMPLE_RATE) * ((LIN_1_BAUD_RATE) / \
        (LIN_1_INACT_100MS_IN_S))))

    /* Divider 1 for specified interrupt rate */
    #define LIN_1_INACT_DIV1                 (HI8(LIN_1_INACT_TIME_FACTOR))

    /* Divider 0 for specified interrupt rate */
    #define LIN_1_INACT_DIV0                 (LO8(LIN_1_INACT_TIME_FACTOR))

#endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */

#define LIN_1_FALSE                      (0x00u)
#define LIN_1_TRUE                       (0x01u)

#define LIN_1_ALL_IRQ_ENABLE             (0x03u)


#define LIN_1_FRAME_PID_MRF              (0x3Cu)
#define LIN_1_FRAME_PID_SRF              (0x7Du)
#define LIN_1_FRAME_SYNC_BYTE            (0x55u)

#if(1u == LIN_1_SAE_J2602)

    #define LIN_1_FRAME_PID_MRF_J2602    (0xFEu)

#endif /* (1u == LIN_1_SAE_J2602) */


/* Numbers and priorities of bLIN and UART interrupts */
#define LIN_1_BLIN_ISR_NUMBER            (LIN_1_BLIN_ISR__INTC_NUMBER)
#define LIN_1_BLIN_ISR_PRIORITY          (LIN_1_BLIN_ISR__INTC_PRIOR_NUM)

#define LIN_1_UART_ISR_NUMBER            (LIN_1_UART_ISR__INTC_NUMBER)
#define LIN_1_UART_ISR_PRIORITY          (LIN_1_UART_ISR__INTC_PRIOR_NUM)

/* Wake up signal length in us */
#define LIN_1_WAKE_UP_SIGNAL_LENGTH      (300u)
#define LIN_1_INVALID_FRAME_PID          (0xFFu)

/* Used to clear UART Rx FIFO */
#define LIN_1_UART_RX_FIFO_CLEAR         (0x01u)

/*******************************************************************************
*                       UART State Machine                                     *
*******************************************************************************/
/* Auto Baud Rate Sync Enabled */
#if(0u == LIN_1_AUTO_BAUD_RATE_SYNC)

    /* Sync Field detection */
    #define LIN_1_UART_ISR_STATE_0_SNC       (0x00u)

#endif  /* (0u == LIN_1_AUTO_BAUD_RATE_SYNC) */

/* Receive PID was detected. Analize recieved PID and determine action */
#define LIN_1_UART_ISR_STATE_1_PID       (0x01u)

/* Transmit data and checksum byte to the LIN master */
#define LIN_1_UART_ISR_STATE_2_TX        (0x02u)

/* Read data from the LIN master */
#define LIN_1_UART_ISR_STATE_3_RX        (0x03u)

/* Checksum verification */
#define LIN_1_UART_ISR_STATE_4_CHS       (0x04u)

/* PID Parity Error Mask */
#define LIN_1_PID_PARITY_MASK            (0x3Fu)


/*******************************************************************************
*                     LIN_1_PID_INFO_TABLE                          *
*******************************************************************************/
#define LIN_1_FRAME_DATA_SIZE_1          (0x01u)
#define LIN_1_FRAME_DATA_SIZE_2          (0x02u)
#define LIN_1_FRAME_DATA_SIZE_3          (0x03u)
#define LIN_1_FRAME_DATA_SIZE_4          (0x04u)
#define LIN_1_FRAME_DATA_SIZE_5          (0x05u)
#define LIN_1_FRAME_DATA_SIZE_6          (0x06u)
#define LIN_1_FRAME_DATA_SIZE_7          (0x07u)
#define LIN_1_FRAME_DATA_SIZE_8          (0x08u)

/* LIN slave direction = subscribe  */
#define LIN_1_FRAME_DIR_SUBSCRIBE        (0x00u)

/* LIN slave direction = publish    */
#define LIN_1_FRAME_DIR_PUBLISH          (0x10u)

/* LIN frame type = event-triggered */
#define LIN_1_FRAME_TYPE_EVENT           (0x20u)

/* LIN frame type = unconditional   */
#define LIN_1_FRAME_TYPE_UNCOND          (0x00u)

#define LIN_1_FRAME_DATA_SIZE_MASK       (0x0Fu)
#define LIN_1_FRAME_DIR_MASK             (0xEFu)
#define LIN_1_FRAME_TYPE_MASK            (0xDEu)


/*******************************************************************************
*                                Interface Status
*                          LIN_1_ifcStatus
*******************************************************************************/
/* Error in respons */
#define LIN_1_IFC_STS_ERROR_IN_RESPONSE  (0x0001u)

/* Successful frame transfer  */
#define LIN_1_IFC_STS_SUCCESSFUL_TRANSFER (0x0002u)

/* Overrun */
#define LIN_1_IFC_STS_OVERRUN            (0x0004u)

/* Go to sleep */
#define LIN_1_IFC_STS_GO_TO_SLEEP        (0x0008u)

/* Bus activity */
#define LIN_1_IFC_STS_BUS_ACTIVITY       (0x0010u)

/* Event-triggered frame collision  */
#define LIN_1_IFC_STS_EVTRIG_COLLISION   (0x0020u)

/* Save configuration */
#define LIN_1_IFC_STS_SAVE_CONFIG        (0x0040u)

/* Last frame PID mask */
#define LIN_1_IFC_STS_PID_MASK           ((l_u16) (0xFF00u))

/* Status mask */
#define LIN_1_IFC_STS_MASK               ((l_u16) (0xFFFFu))

/*******************************************************************************
*                          Internal LIN Slave Status
*                           LIN_1_status
*******************************************************************************/

/* This bit indicates that there is a response for ACRH service
*  and it is ready to be sent to master node.
*/
#define LIN_1_STATUS_SRVC_RSP_RDY        ((l_u8) (0x80u))


#define LIN_1_STATUS_ERROR_MIRROR        (0x01u)
#define LIN_1_STATUS_NOT_CHECKED         (0x04u)
#define LIN_1_STATUS_RESPONSE_ACTIVE     (0x10u)



#if(1u == LIN_1_SAE_J2602)
    /***************************************************************************
    *                           SAE J2602-1 Error states
    *                          LIN_1_j2602Status
    ***************************************************************************/

    /* Clear all error buts in the j2602 status word mask */
    #define LIN_1_J2602_CLEAR_ERR_BITS_MASK  (0x1Fu)

    /* Data error */
    #define LIN_1_J2602_STS_DATA_ERR         (0x80u)

    /* Checksum error  */
    #define LIN_1_J2602_STS_CHECKSUM_ERR     (0xA0u)

    /* Framing error  */
    #define LIN_1_J2602_STS_FRAMING_ERR      (0xC0u)

    /* Parity error  */
    #define LIN_1_J2602_STS_PARITY_ERR       (0xE0u)

#endif  /* (1u == LIN_1_SAE_J2602) */


/*******************************************************************************
*                       LIN_1_EndFrame(l_u16 status)
*******************************************************************************/
#define LIN_1_HANDLING_RESET_FSM_ERR     (0x01u)
#define LIN_1_HANDLING_DONT_SAVE_PID     (0x10u)
#define LIN_1_HANDLING_SKIP_OVERRUN      (0x20u)
#define LIN_1_HANDLING_PID_ERR           (0x40u)


/*******************************************************************************
*                           LIN_1_fsmFlags
*******************************************************************************/

/* Break signal detected  */
#define LIN_1_FSM_BREAK_FLAG             (0x01u)

/* Sleep API was called */
#define LIN_1_FSM_SLEEP_API_FLAG         (0x02u)

/* UART receives at least 1 BYTE of data */
#define LIN_1_FSM_DATA_RECEIVE           (0x04u)

/* UART enable flag */
#define LIN_1_FSM_UART_ENABLE_FLAG       (0x08u)

/* Overrun Flag is used for status word */
#define LIN_1_FSM_OVERRUN                (0x40u)

/* Framing error */
#define LIN_1_FSM_FRAMING_ERROR_FLAG     (0x80u)


/*******************************************************************************
*                           Transport Layer
*******************************************************************************/

#if(1u == LIN_1_TL_ENABLED)

    #if(1u == LIN_1_CS_ENABLED)

        /* Wilcard ID understandable for every slave node */
        #define LIN_1_CS_SUPPLIER_ID_WILDCARD    (0x7FFFu)
        #define LIN_1_CS_FUNCTION_ID_WILDCARD    (0xFFFFu)

        #define LIN_1_CS_BYTE_SUPPLIER_ID1       (0x01u)
        #define LIN_1_CS_BYTE_SUPPLIER_ID2       (0x02u)
        #define LIN_1_CS_BYTE_FUNCTION_ID1       (0x03u)
        #define LIN_1_CS_BYTE_FUNCTION_ID2       (0x04u)
        #define LIN_1_CS_BYTE_VARIANT            (0x05u)

    #endif /* (1u == LIN_1_CS_ENABLED) */

    #if(1u == LIN_1_TL_API_FORMAT)

        /* Specifies the value for the TL timeouts */
        #define LIN_1_TL_N_AS_TIMEOUT_VALUE      (10u)
        #define LIN_1_TL_N_CR_TIMEOUT_VALUE      (10u)

    #endif /* (1u == LIN_1_TL_API_FORMAT) */

    /* Specifies the Frame buffer length for Transport Layer */
    #define LIN_1_FRAME_BUFF_LEN             (8u)

    /* Specifies the Frame length for Transport Layer */
    #define LIN_1_FRAME_LEN                  (8u)

    /* Packet Data Unit (PDU) Offsets */
    #define LIN_1_PDU_NAD_IDX                (0u)
    #define LIN_1_PDU_PCI_IDX                (1u)
    #define LIN_1_PDU_SID_IDX                (2u)
    #define LIN_1_PDU_LEN_IDX                (2u)
    #define LIN_1_PDU_D1_IDX                 (3u)
    #define LIN_1_PDU_D1_START_IDX           (3u)
    #define LIN_1_PDU_D1_ID_IDX              (3u)
    #define LIN_1_PDU_D2_IDX                 (4u)
    #define LIN_1_PDU_D2_PID_IDX             (4u)
    #define LIN_1_PDU_D2_BYTE_IDX            (4u)
    #define LIN_1_PDU_D3_IDX                 (5u)
    #define LIN_1_PDU_D3_MASK_IDX            (5u)
    #define LIN_1_PDU_D4_IDX                 (6u)
    #define LIN_1_PDU_D4_INVERT_IDX          (6u)
    #define LIN_1_PDU_D5_IDX                 (7u)
    #define LIN_1_PDU_D5_NEW_NAD_IDX         (7u)

    /* Single Frame data length  */
    #define LIN_1_PDU_SF_DATA_LEN            (6u)

    /* Protocol Control Information (PCI) Types */
    #define LIN_1_PDU_PCI_TYPE_SF            (0x00u)     /* Single       Frame */
    #define LIN_1_PDU_PCI_TYPE_FF            (0x10u)     /* First        Frame */
    #define LIN_1_PDU_PCI_TYPE_CF            (0x20u)     /* Consecutive  Frame */
    #define LIN_1_PDU_PCI_TYPE_UNKNOWN       (0xFFu)
    #define LIN_1_PDU_PCI_TYPE_MASK          (0xF0u)

    #define LIN_1_NAD_FUNCTIONAL             (0x7Eu)

    /* Wildcard NAD */
    #define LIN_1_NAD_BROADCAST              (0x7Fu)

    /* Go to sleep command ID */
    #define LIN_1_NAD_GO_TO_SLEEP            (0x00u)

    /* Max and min Config Services IDs */
    #define LIN_1_SID_CONF_MIN               (0xB0u)
    #define LIN_1_SID_CONF_MAX               (0xB7u)

    #if(1u == LIN_1_CS_ENABLED)

        /* Node Configuration Services */
        #define LIN_1_NCS_ASSIGN_NAD             (0xB0u)
        #define LIN_1_NCS_ASSIGN_FRAME_ID        (0xB1u) /* Used only in LIN 2.0 */
        #define LIN_1_NCS_READ_BY_ID             (0xB2u)
        #define LIN_1_NCS_COND_CHANGE_NAD        (0xB3u)
        #define LIN_1_NCS_DATA_DUMP              (0xB4u) /* Not supported */
        #define LIN_1_NCS_ASSIGN_NAD_SNPD        (0xB5u) /* Not supported */
        #define LIN_1_NCS_SAVE_CONFIG            (0xB6u)
        #define LIN_1_NCS_ASSIGN_FRAME_ID_RANGE  (0xB7u)

        /* Positive responses for Node Configuration Services requests */
        #define LIN_1_NCS_POS_RESP_ASSIGN_NAD             (0xF0u)
        #define LIN_1_NCS_POS_RESP_ASSIGN_FRAME_ID        (0xF1u) /* Used only in LIN 2.0 */
        #define LIN_1_NCS_POS_RESP_READ_BY_ID             (0xF2u)
        #define LIN_1_NCS_POS_RESP_COND_CHANGE_NAD        (0xF3u)
        #define LIN_1_NCS_POS_RESP_DATA_DUMP              (0xF4u) /* Not supported */
        #define LIN_1_NCS_POS_RESP_ASSIGN_NAD_SNPD        (0xF5u) /* Not supported */
        #define LIN_1_NCS_POS_RESP_SAVE_CONFIG            (0xF6u)
        #define LIN_1_NCS_POS_RESP_ASSIGN_FRAME_ID_RANGE  (0xF7u)

        /* Other LIN TL constants */
        #define LIN_1_NCS_READ_BY_ID_ID          (0x00u)
        #define LIN_1_NCS_READ_BY_ID_SERIAL      (0x01u)
        #define LIN_1_NCS_RSID_NEG_REPLY         (0x7Fu)
        #define LIN_1_NCS_MAX_FRAME_ID_RANGE     (0x04u)

        /* ld_read_by_id_callout() return constants */
        #define LD_NEGATIVE_RESPONSE                        (0x00u)
        #define LD_NO_RESPONSE                              (0x01u)
        #define LD_POSITIVE_RESPONSE                        (0x02u)

        #if(1u == LIN_1_LIN_2_0)

            #define LD_INVALID_MESSAGE_INDEX                (0xFFu)
            #define LD_MESSAGE_ID_BASE                      (0x10u)

        #endif /* (1u == LIN_1_LIN_2_0) */


    #endif /* (1u == LIN_1_CS_ENABLED) */

    /*******************************************************************************
    *       LIN_1_txTlStatus and LIN_1_rxTlStatus
    *******************************************************************************/

    /* Reception or transmission is not yet completed */
    #define LD_IN_PROGRESS             (0x01u)

    /* Reception or transmission has completed successfully */
    #define LD_COMPLETED               (0x02u)

    /* Reception or transmittion ended in an error */
    #define LD_FAILED                  (0x03u)

    /* Transmittion failed because of a N_As timeout */
    #define LD_N_AS_TIMEOUT            (0x04u)

    /* Reception failed because of a N_Cr timeout */
    #define LD_N_CR_TIMEOUT            (0x05u)

    /* Reception failed because of unexpected sequence number */
    #define LD_WRONG_SN                (0x06u)

    /* The transmit queue is empty */
    #define LD_QUEUE_EMPTY             (0x07u)

    /* The transmit queue contains entires */
    #define LD_QUEUE_AVAILABLE         (0x08u)

    /* The transmit queue is full and cant accept further frames */
    #define LD_QUEUE_FULL              (0x09u)

    /* LIN protocol errors occured during the transfer */
    #define LD_TRANSMIT_ERROR          (0x0Au)

    /* The receive queue is empty */
    #define LD_NO_DATA                 (0x0Bu)

    /* The receive queue contains data that can be read */
    #define LD_DATA_AVAILABLE          (0x0Cu)

    /* LIN protocol errors occured during the transfer */
    #define LD_RECEIVE_ERROR           (0x0Du)

    /*******************************************************************************
    *                           LIN_1_tlFlags
    *******************************************************************************/

    /* The requested service is disabled and diagnostic frame will
    * be "passed" to Transport Layer.
    */
    #define LIN_1_TL_CS_SERVICE_DISABLED       (0x01u)

    /* The SID isn't ACRH but a simple diagnostic SID */
    #define LIN_1_TL_DIAG_FRAME_DETECTED       (0x02u)

    /* The last PID that occured is SRF PID */
    #define LIN_1_TL_TX_DIRECTION              (0x04u)

    /* The last PID that occured is MRF PID */
    #define LIN_1_TL_RX_DIRECTION              (0x08u)

    /* Indicates that Cooked API requested trasmit data */
    #define LIN_1_TL_TX_REQUESTED              (0x10u)

    /* Indicates that Cooked API requested receive data */
    #define LIN_1_TL_RX_REQUESTED              (0x20u)

    /* In dicates that N_AS timeout is monitoring is in progress */
    #define LIN_1_TL_N_AS_TIMEOUT_ON           (0x40u)

    /* In dicates that N_CR timeout is monitoring is in progress */
    #define LIN_1_TL_N_CR_TIMEOUT_ON           (0x80u)


    /*******************************************************************************
    *            ld_read_configuration() and ld_set_configuration()
    *******************************************************************************/

    /* Read configuration is complete */
    #define LIN_1_LD_READ_OK                 (0x01u)

    /* Read configuration is fail. Configuration size is greater than the length. */
    #define LIN_1_LD_LENGTH_TOO_SHORT        (0x02u)

    /* Set configuration is complete */
    #define LIN_1_LD_SET_OK                  (0x01u)

    /* set configuration is fail.
    *  Required size of the configuration is not equal to the given length.
    */
    #define LIN_1_LD_LENGTH_NOT_CORRECT      (0x02u)

    /* Set configuration is fail. Set conf could not be made. */
    #define LIN_1_LD_DATA_ERROR              (0x04u)


#endif /* (1u == LIN_1_TL_ENABLED) */


/*******************************************************************************
*                   l_ifc_ioctl() parameters
*******************************************************************************/
#define L_IOCTL_READ_STATUS                     (0x00u)
#define L_IOCTL_SET_BAUD_RATE                   (0x01u)
#define L_IOCTL_SLEEP                           (0x02u)
#define L_IOCTL_WAKEUP                          (0x03u)
#define L_IOCTL_SYNC_COUNTS                     (0x04u)
#define L_IOCTL_SET_SERIAL_NUMBER               (0x05u)




/*******************************************************************************
*           Variable LIN_1_ioctlStatus flags
*******************************************************************************/
/* Bus inactivity */
#define LIN_1_IOCTL_STS_BUS_INACTIVITY (0x0001u)

/* Target reset (0xB5) */
#define LIN_1_IOCTL_STS_TARGET_RESET   (0x0002u)


/* Status Register */
#define LIN_1_STATUS_BREAK_DETECTED          (0x01u)
#define LIN_1_STATUS_EDGE_DETECTED           (0x02u)
#define LIN_1_STATUS_SYNC_COMPLETED          (0x08u)
#if(1u == LIN_1_INACTIVITY_ENABLED)
    #define LIN_1_STATUS_INACTIVITY_INT    (0x04u)
#endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */


/* Control Register */
#define LIN_1_CONTROL_ENABLE             ((l_u8) (0x01u))
#define LIN_1_CONTROL_TX_DIS             ((l_u8) (0x02u))
#define LIN_1_CONTROL_RX_DIS             (0x04u)


/* Status Mask Register */
#define LIN_1_INT_MASK_BREAK          (0x01u)
#define LIN_1_INT_MASK_EDGE           (0x02u)
#define LIN_1_INT_MASK_SYNC           (0x08u)

#if(1u == LIN_1_INACTIVITY_ENABLED)
    #define LIN_1_INT_MASK_INACTIVITY (0x04u)
#else
    #define LIN_1_INT_MASK_INACTIVITY (0x00u)
#endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */


#define LIN_1_STATUS_AUX_CONTROL_INT_EN     (0x10u)


/***************************************
*             Registers
***************************************/

#define LIN_1_CONTROL_REG            \
                                    (* (reg8 *) LIN_1_bLIN_CtrlReg__CONTROL_REG)
#define LIN_1_CONTROL_PTR            \
                                    (  (reg8 *) LIN_1_bLIN_CtrlReg__CONTROL_REG)

#define LIN_1_STATUS_REG             (* (reg8 *) LIN_1_bLIN_StsReg__STATUS_REG)
#define LIN_1_STATUS_PTR             (  (reg8 *) LIN_1_bLIN_StsReg__STATUS_REG)

#define LIN_1_INT_MASK_REG           (* (reg8 *) LIN_1_bLIN_StsReg__MASK_REG)
#define LIN_1_INT_MASK_PTR           (  (reg8 *) LIN_1_bLIN_StsReg__MASK_REG)

#define LIN_1_BREAK_THRESHOLD_REG    (* (reg8 *) LIN_1_bLIN_LINDp_u0__D0_REG)
#define LIN_1_BREAK_THRESHOLD_PTR    (  (reg8 *) LIN_1_bLIN_LINDp_u0__D0_REG)

#define LIN_1_STATUS_AUX_CONTROL_REG (* (reg8 *) LIN_1_bLIN_StsReg__STATUS_AUX_CTL_REG)
#define LIN_1_STATUS_AUX_CONTROL_PTR (  (reg8 *) LIN_1_bLIN_StsReg__STATUS_AUX_CTL_REG)


/* Auto Baud Rate Sync Enabled */
#if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

    #define LIN_1_HIGH_BITS_LENGTH_SUM_REG       (* (reg8 *) LIN_1_bLIN_LINDp_u0__F1_REG)
    #define LIN_1_HIGH_BITS_LENGTH_SUM_PTR       (  (reg8 *) LIN_1_bLIN_LINDp_u0__F1_REG)

    #define LIN_1_LOW_BIT_LENGTH_SUM_REG             (* (reg8 *) LIN_1_bLIN_LINDp_u0__F0_REG)
    #define LIN_1_LOW_BIT_LENGTH_SUM_PTR             (  (reg8 *) LIN_1_bLIN_LINDp_u0__F0_REG)

#endif  /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */


/* Bus inactivity block configuration */
#if(1u == LIN_1_INACTIVITY_ENABLED)

    #define LIN_1_INACTIVITY_DIV0_REG    (* (reg8 *) LIN_1_bLIN_InactFSM_BusInactDp_u0__D0_REG)
    #define LIN_1_INACTIVITY_DIV0_PTR    (  (reg8 *) LIN_1_bLIN_InactFSM_BusInactDp_u0__D0_REG)

    #define LIN_1_INACTIVITY_DIV1_REG    (* (reg8 *) LIN_1_bLIN_InactFSM_BusInactDp_u0__D1_REG)
    #define LIN_1_INACTIVITY_DIV1_PTR    (  (reg8 *) LIN_1_bLIN_InactFSM_BusInactDp_u0__D1_REG)

#endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */

/* Register used to clear UART RX FIFO */
#define LIN_1_UART_RX_FIFO_REG       \
                                            (* (reg8 *) LIN_1_UART_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
#define LIN_1_UART_RX_FIFO_PTR       \
                                            (  (reg8 *) LIN_1_UART_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

#endif /* CY_LIN_LIN_1_H */


/* [] END OF FILE */
