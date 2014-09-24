/*******************************************************************************
* File Name: LIN_1.c
* Version 1.30
*
* Description:
*  This file contains implementation of LIN Slave component.
*
********************************************************************************
* Copyright 2011-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "LIN_1.h"


/*******************************************************************************
*  Place your includes, defines and variable declarations here
*******************************************************************************/
/* `#START LIN_1_DECLARATIONS` */

/* `#END` */

/* Internal APIs */
static l_u8    LIN_1_FindPidIndex(l_u8 pid);
static void    LIN_1_EndFrame(l_u8 status);
static void    LIN_1_SetAssociatedFlags(l_u8 pIndex);

/* SAE J2602 is disabled */
#if(0u == LIN_1_SAE_J2602)

    static l_bool  LIN_1_GetEtFlagValue(l_u8 pidIndex);
    static void    LIN_1_ClearEtFlagValue(l_u8 pidIndex);

#endif /* (0u == LIN_1_SAE_J2602) */

#if(1u == LIN_1_TL_ENABLED)

    static void LIN_1_ProcessMrf(l_u8 frame[]);

    #if(1u == LIN_1_CS_ENABLED)
    
        #if((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB0_SEL)) || \
            (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)))

            static l_bool LIN_1_LinProductId(volatile const l_u8 frameData[]);
            
        #endif /*((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB0_SEL)) || \
               * (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)))
               */
               
        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB1_SEL))
            #if(1u == LIN_1_LIN_2_0)

                static l_u8 LIN_1_MessageId(volatile const l_u8* frameData);

            #endif /* (1u == LIN_1_LIN_2_0) */
        #endif /* (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB1_SEL)) */

    #endif /* (1u == LIN_1_CS_ENABLED) */

    /* RAM copy of the slave NAD */
    static volatile l_u8 LIN_1_nad;

#endif /* (1u == LIN_1_TL_ENABLED) */

#if(1u == LIN_1_TL_ENABLED)

    #if(1u == LIN_1_LIN_2_0)
        #if(0u != (LIN_1_CS_SEL_SERVICES01 & (LIN_1_NCS_0xB2_SEL | \
                LIN_1_NCS_0xB1_SEL)))

        

        #endif /* (0u != (LIN_1_CS_SEL_SERVICES01 & (LIN_1_NCS_0xB2_SEL |
               * LIN_1_NCS_0xB1_SEL)))
               */

    #endif /* (1u == LIN_1_LIN_2_0) */

#endif /* (1u == LIN_1_TL_ENABLED) */

#if(1u == LIN_1_INACTIVITY_ENABLED)

    /* Free-running timer */
    static l_u8    LIN_1_periodCounter;

#endif /* (1u == LIN_1_INACTIVITY_ENABLED) */

static volatile l_u8  LIN_1_status;       /* Internal Status                  */
static volatile l_u8  LIN_1_syncCounts;   /* Sync Field Timer Counts          */
static volatile l_u8  LIN_1_auxStatus;    /* Internal AUX ISR shadow status   */
static volatile l_u16 LIN_1_ioctlStatus;  /* Status used by l_ifc_ioctl()     */
static volatile l_u16 LIN_1_ifcStatus;    /* Interface communication status   */
static volatile l_u8  LIN_1_uartFsmState; /* Current state of the UART ISR    */
static volatile l_u8  LIN_1_fsmFlags;

#if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

    /* Initial clock divider */
    static l_u16   LIN_1_initialClockDivider;

#endif  /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */

#if(1u == LIN_1_SAE_J2602)

    /* J2602 status variable */
    static l_u8 LIN_1_j2602Status;

#endif  /* (1u == LIN_1_SAE_J2602) */

/* Notification API statuses */
static l_u8 LIN_1_statusFlagArray[LIN_1_SIG_FRAME_FLAGS_SIZE];

/* SAE J2602 is disabled */
#if(0u == LIN_1_SAE_J2602)

    #if (0u != LIN_1_NUM_ET_FRAMES)
        static l_u8 LIN_1_etFrameFlags[LIN_1_ET_FRAMES_FLAGS_SIZE];
    #endif /* (0u != LIN_1_NUM_ET_FRAMES) */

#endif /* (0u == LIN_1_SAE_J2602) */

/* RAM copy of the slave configuration data */
static volatile l_u8 LIN_1_volatileConfig[LIN_1_NUM_FRAMES];


/****************************************************
*   Transport Layer API
*****************************************************/
#if(1u == LIN_1_TL_ENABLED)

    #if(1u == LIN_1_CS_ENABLED)
        
        #if(0u != (LIN_1_CS_SEL_SERVICES01 & (LIN_1_NCS_0xB0_SEL | LIN_1_NCS_0xB1_SEL \
                    | LIN_1_NCS_0xB2_SEL | LIN_1_NCS_0xB3_SEL)))
        
            /* LIN Slave Identification */
            static const LIN_1_SLAVE_ID CYCODE LIN_1_slaveId =
            {
                LIN_1_CS_SUPPLIER_ID,
                LIN_1_CS_FUNCTION_ID,
                LIN_1_CS_VARIANT
            };
            
        #endif /*(0u != (LIN_1_CS_SEL_SERVICES01 & (LIN_1_NCS_0xB0_SEL 
               * | LIN_1_NCS_0xB1_SEL | LIN_1_NCS_0xB2_SEL | 
               * LIN_1_NCS_0xB3_SEL))
               */


        /* Serial Number */
        static l_u8* LIN_1_serialNumber;

    #endif /* (1u == LIN_1_CS_ENABLED) */

    /* SRF buffer */
    static volatile l_u8 LIN_1_srfBuffer[LIN_1_FRAME_BUFF_LEN];

    /* Transport Layer Rx and Tx Statuses */
    static volatile l_u8   LIN_1_txTlStatus;
    static volatile l_u8   LIN_1_rxTlStatus;

    /* Flags that are used for Transport Layer */
    static volatile l_u8   LIN_1_tlFlags;

    #if(1u == LIN_1_TL_API_FORMAT)

        /* Internal variable used to store the PCI of the previous frame */
        static l_u8 LIN_1_prevPci;
    
        static const    l_u8*   LIN_1_txTlDataPointer     = NULL;
        static volatile l_u16   LIN_1_txTlDataCount;
        static volatile l_u8*   LIN_1_rxTlDataPointer     = NULL;
        static volatile l_u8*   LIN_1_rxTlInitDataPointer = NULL;
        static volatile l_u8*   LIN_1_tlNadPointer      = NULL;
        static volatile l_u16*  LIN_1_tlLengthPointer   = NULL;

        static volatile l_u16   LIN_1_txMessageLength = 0u;
        static volatile l_u16   LIN_1_rxMessageLength = 0u;

        static volatile l_u8 LIN_1_txFrameCounter = 0u;
        static l_u8 LIN_1_rxFrameCounter = 0u;
        static volatile l_u8 LIN_1_tlTimeoutCnt   = 0u;

    #else

        /* Internal variables for buffer indexing */
        static l_u8 LIN_1_txBufDepth;
        static l_u8 LIN_1_rxBufDepth;

        /* The Master Request Frame (MRF) buffer */
        static volatile l_u8 LIN_1_rawRxQueue[LIN_1_TL_RX_QUEUE_LEN];

        /* The Slave Response Frame (SRF) buffer */
        static volatile l_u8 LIN_1_rawTxQueue[LIN_1_TL_TX_QUEUE_LEN];

        /* TX buffer indexes */
        static volatile l_u16 LIN_1_txWrIndex;
        static volatile l_u16 LIN_1_txRdIndex;

        /* RX buffer indexes */
        static volatile l_u16 LIN_1_rxWrIndex;
        static volatile l_u16 LIN_1_rxRdIndex;

    #endif /* (1u == LIN_1_TL_API_FORMAT) */

#endif /* (1u == LIN_1_TL_ENABLED) */


/* LIN slave configuration data */
static LIN_1_SLAVE_CONFIG const CYCODE LIN_1_LinSlaveConfig = {
    /* Frame PID Table */
    {0xC1u, 0x80u, 0xC4u}
};


/* Frames declaration with initial signals values */
static volatile l_u8  LIN_1_Buffer_Frame1[1u] = {0xFEu};
static volatile l_u8  LIN_1_Buffer_rpm[6u] = {0xFFu, 0xFFu, 0x00u, 0x00u, 0xFFu, 0xFFu};
static volatile l_u8  LIN_1_Buffer_PID[3u] = {0x00u, 0x00u, 0x00u};



/*******************************************************************************
*  Place your code here
*******************************************************************************/
/* `#START LIN_1_CODE` */

/* `#END` */


/*******************************************************************************
* Function Name: LIN_1_Start
********************************************************************************
*
* Summary:
*  Starts the component operation. This function is not required to be used.
*
* Parameters:
*  None
*
* Return:
*  Zero     - if the initialization succeeded.
*  Non-zero - if the initialization failed.
*
* Reentrant:
*  No
*
*******************************************************************************/
l_bool LIN_1_Start(void) 
{
    l_bool returnValue;

    /* Call initialization function */
    returnValue = l_sys_init();

    /* Start LIN components. Ignore the return value. */
    (void) l_ifc_init_LIN_1();

    return (returnValue);
}


/*******************************************************************************
* Function Name: LIN_1_Stop
********************************************************************************
*
* Summary:
*  Starts the component operation. This function is not required to be used.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
l_u8 LIN_1_Stop(void) 
{
    /* Stop UART */
    LIN_1_UART_Stop();

    /* Disable hardware blocks operation */
    LIN_1_CONTROL_REG &= (l_u8) ~LIN_1_CONTROL_ENABLE;

    /* Disable interrupts */
    return(l_sys_irq_disable());
}


/*******************************************************************************
* Function Name: l_sys_init
********************************************************************************
*
* Summary:
*  Performs the initialization of the LIN core. This function does nothing and
*  and always returns zero.
*
* Parameters:
*  None
*
* Return:
*  Zero     - if the initialization succeeded.
*  Non-zero - if the initialization failed.
*
*******************************************************************************/
l_bool l_sys_init(void)  
{
    return (LIN_1_FALSE);
}


/* Signal interaction and notification API */


/*******************************************************************************
* Function Name: l_u8_rd
********************************************************************************
*
* Summary:
*  Reads and returns the current value of the signal for signals of the size
*  2 - 8 bits. If an invalid signal handle is passed into the function, no
*  action is done.
*
* Parameters:
*  sss - signal handle of the signal to read.
*
* Return:
*  Returns current value of signal.
*
*******************************************************************************/
l_u8 l_u8_rd(l_signal_handle sss) 
{
    l_u8 returnValue;

    switch(sss)
    {
        case Kp_SIGNAL_HANDLE:
            returnValue = l_u8_rd_Kp();
        break;

        case Ki_SIGNAL_HANDLE:
            returnValue = l_u8_rd_Ki();
        break;

        case Kd_SIGNAL_HANDLE:
            returnValue = l_u8_rd_Kd();
        break;

        default:
            returnValue = (l_u8) 0xFFu;
        break;
    }

    return(returnValue);
}


/*******************************************************************************
* Function Name: l_u8_rd_Kp
********************************************************************************
*
* Summary:
*  Reads and returns the current value of the signal for signals of the size
*  2 - 8 bits.
*
* Parameters:
*  None
*
* Return:
*  Returns current value of signal.
*
*******************************************************************************/
l_u8 l_u8_rd_Kp(void) 
{
    /* Get one complete byte signal from frame */
    return(LIN_1_Buffer_PID[LIN_1_Kp_PID_SIG_BYTE_OFFSET]);
}

/*******************************************************************************
* Function Name: l_u8_rd_Ki
********************************************************************************
*
* Summary:
*  Reads and returns the current value of the signal for signals of the size
*  2 - 8 bits.
*
* Parameters:
*  None
*
* Return:
*  Returns current value of signal.
*
*******************************************************************************/
l_u8 l_u8_rd_Ki(void) 
{
    /* Get one complete byte signal from frame */
    return(LIN_1_Buffer_PID[LIN_1_Ki_PID_SIG_BYTE_OFFSET]);
}

/*******************************************************************************
* Function Name: l_u8_rd_Kd
********************************************************************************
*
* Summary:
*  Reads and returns the current value of the signal for signals of the size
*  2 - 8 bits.
*
* Parameters:
*  None
*
* Return:
*  Returns current value of signal.
*
*******************************************************************************/
l_u8 l_u8_rd_Kd(void) 
{
    /* Get one complete byte signal from frame */
    return(LIN_1_Buffer_PID[LIN_1_Kd_PID_SIG_BYTE_OFFSET]);
}






/*******************************************************************************
* Function Name: l_u16_rd
********************************************************************************
*
* Summary:
*  Reads and returns the current value of the signal for signals of the size
*  2 - 8 bits. If an invalid signal handle is passed into the function, no
*  action is done.
*
* Parameters:
*  sss - signal handle of the signal to read.
*
* Return:
*  Returns current value of signal.
*
*******************************************************************************/
l_u16 l_u16_rd(l_signal_handle sss) 
{
    l_u16 returnValue;

    switch(sss)
    {
        case target_SIGNAL_HANDLE:
            returnValue = l_u16_rd_target();
        break;

        default:
            returnValue = (l_u16) 0xFFFFu;
        break;
    }

    return (returnValue);
}


/*******************************************************************************
* Function Name: l_u16_rd_target
********************************************************************************
*
* Summary:
*  Reads and returns the current value of the signal for signals of the size
*  2 - 8 bits.
*
* Parameters:
*  None
*
* Return:
*  Returns current value of signal.
*
*******************************************************************************/
l_u16 l_u16_rd_target(void) 
{
    l_u16 returnValue;
    uint8 interruptState;
    l_u16 tmpByte1;
    l_u16 tmpByte2;

    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

    tmpByte1 = (l_u16) LIN_1_Buffer_rpm[LIN_1_target_rpm_SIG_BYTE_OFFSET + 1u];
    tmpByte2 = (l_u16) LIN_1_Buffer_rpm[LIN_1_target_rpm_SIG_BYTE_OFFSET];

    /* Swap MSB and LSB */
    returnValue = LIN_1_SWAP_U8_TO_U16(tmpByte1, tmpByte2);

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);

    return(returnValue);
}








/*******************************************************************************
* Function Name: l_bool_wr
********************************************************************************
*
* Summary:
*  Sets the current value of the signal for one bit signals to "v".
*  If an invalid signal handle is passed into the function, no action is done. 
*
* Parameters:
*  sss - signal handle of the signal to be set.
*  v - value of the signal to be set.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_bool_wr(l_signal_handle sss, l_bool v)
{
    switch(sss)
    {
        case Response_Error_SIGNAL_HANDLE:
            l_bool_wr_Response_Error(v);
        break;

        default:
        break;
    }
}


/*******************************************************************************
* Function Name: l_bool_wr_Response_Error
********************************************************************************
*
* Summary:
*  Sets the current value of the signal for one bit signals to v.
*
* Parameters:
*  v - value of the signal to be set.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_bool_wr_Response_Error(l_bool v)
{
    uint8 interruptState;

    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

    if(0u != v)
    {
        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;
    }
    else
    {
        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] &= 
                            (l_u8) ~LIN_1_Response_Error_Frame1_SIG_MASK_0;
    }

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);
}











/*******************************************************************************
* Function Name: l_flg_tst
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  fff - is the name of the flag handle.
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst(l_flag_handle fff)
{
    l_bool returnValue;

    switch(fff)
    {
        case target_FLAG_HANDLE:
            returnValue = l_flg_tst_target();
        break;

        case Kp_FLAG_HANDLE:
            returnValue = l_flg_tst_Kp();
        break;

        case Ki_FLAG_HANDLE:
            returnValue = l_flg_tst_Ki();
        break;

        case Kd_FLAG_HANDLE:
            returnValue = l_flg_tst_Kd();
        break;

        case Frame1_FLAG_HANDLE:
            returnValue = l_flg_tst_Frame1();
        break;

        case rpm_FLAG_HANDLE:
            returnValue = l_flg_tst_rpm();
        break;

        case PID_FLAG_HANDLE:
            returnValue = l_flg_tst_PID();
        break;

        default:
            returnValue = LIN_1_TRUE;
        break;
    }

    return(returnValue);
}


/*******************************************************************************
* Function Name: l_flg_tst_target
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_target(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_target_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_target_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}

/*******************************************************************************
* Function Name: l_flg_tst_Kp
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_Kp(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_Kp_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_Kp_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}

/*******************************************************************************
* Function Name: l_flg_tst_Ki
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_Ki(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_Ki_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_Ki_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}

/*******************************************************************************
* Function Name: l_flg_tst_Kd
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_Kd(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_Kd_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_Kd_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}

/*******************************************************************************
* Function Name: l_flg_tst_Frame1
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_Frame1(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_Frame1_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_Frame1_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}

/*******************************************************************************
* Function Name: l_flg_tst_rpm
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_rpm(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_rpm_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_rpm_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}

/*******************************************************************************
* Function Name: l_flg_tst_PID
********************************************************************************
*
* Summary:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff, i.e. returns zero if the flag is cleared, non-zero otherwise.
*
* Parameters:
*  None
*
* Return:
*  Returns a C boolean indicating the current state of the flag specified by
*  the name fff:
*  False - if the flag is cleared;
*  True - if the flag is not cleared.
*
*******************************************************************************/
l_bool l_flg_tst_PID(void)
{
    l_bool result = LIN_1_FALSE;

    if(0u != (LIN_1_statusFlagArray[LIN_1_PID_FRAME_FLAG_BYTE_OFFSET_0] &
    LIN_1_PID_FRAME_FLAG_MASK_0))
    {
        result = LIN_1_TRUE;
    }

    return(result);
}






/*******************************************************************************
* Function Name: l_flg_clr
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  fff - is the name of the flag handle.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr(l_flag_handle fff)
{
    switch(fff)
    {
        case target_FLAG_HANDLE:
            l_flg_clr_target();
        break;

        case Kp_FLAG_HANDLE:
            l_flg_clr_Kp();
        break;

        case Ki_FLAG_HANDLE:
            l_flg_clr_Ki();
        break;

        case Kd_FLAG_HANDLE:
            l_flg_clr_Kd();
        break;

        case Frame1_FLAG_HANDLE:
            l_flg_clr_Frame1();
        break;

        case rpm_FLAG_HANDLE:
            l_flg_clr_rpm();
        break;

        case PID_FLAG_HANDLE:
            l_flg_clr_PID();
        break;

        default:
        break;
    }
}


/*******************************************************************************
* Function Name: l_flg_clr_target
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_target(void)
{
    LIN_1_statusFlagArray[LIN_1_target_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_target_FRAME_FLAG_MASK_0);
}

/*******************************************************************************
* Function Name: l_flg_clr_Kp
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_Kp(void)
{
    LIN_1_statusFlagArray[LIN_1_Kp_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_Kp_FRAME_FLAG_MASK_0);
}

/*******************************************************************************
* Function Name: l_flg_clr_Ki
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_Ki(void)
{
    LIN_1_statusFlagArray[LIN_1_Ki_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_Ki_FRAME_FLAG_MASK_0);
}

/*******************************************************************************
* Function Name: l_flg_clr_Kd
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_Kd(void)
{
    LIN_1_statusFlagArray[LIN_1_Kd_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_Kd_FRAME_FLAG_MASK_0);
}

/*******************************************************************************
* Function Name: l_flg_clr_Frame1
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_Frame1(void)
{
    LIN_1_statusFlagArray[LIN_1_Frame1_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_Frame1_FRAME_FLAG_MASK_0);
}

/*******************************************************************************
* Function Name: l_flg_clr_rpm
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_rpm(void)
{
    LIN_1_statusFlagArray[LIN_1_rpm_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_rpm_FRAME_FLAG_MASK_0);
}

/*******************************************************************************
* Function Name: l_flg_clr_PID
********************************************************************************
*
* Summary:
*  Sets the current value of the flag specified by the name fff to zero.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_flg_clr_PID(void)
{
    LIN_1_statusFlagArray[LIN_1_PID_FRAME_FLAG_BYTE_OFFSET_0] &=
       ((l_u8) ~LIN_1_PID_FRAME_FLAG_MASK_0);
}







/*******************************************************************************
* Function Name: l_ifc_init_LIN_1
********************************************************************************
*
* Summary:
*  The function initializes the LIN Slave component instance that is specified
*  by the name iii. It sets up internal functions such as the baud rate and
*  starts up digital blocks that are used by the LIN Slave component. This is
*  the first call that must be performed, before using any other interface-
*  related LIN Slave API functions.
*
* Parameters:
*  None
*
* Return:
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
* Reentrant:
*  No
*
*******************************************************************************/
l_bool l_ifc_init_LIN_1(void) 
{
    l_u8 interruptState;
    l_u8 i;

    /* Set bLIN interrupt priority */
    CyIntSetPriority(LIN_1_BLIN_ISR_NUMBER, LIN_1_BLIN_ISR_PRIORITY);

    /* Set bLIN interrupt vector */
    (void) CyIntSetVector(LIN_1_BLIN_ISR_NUMBER, &LIN_1_BLIN_ISR);

    /* Clear any pending bLIN interrupt */
    CyIntClearPending(LIN_1_BLIN_ISR_NUMBER);

    /* Clear any pending bLIN interrupt */
    CyIntClearPending(LIN_1_BLIN_ISR_NUMBER);

    /* Set UART interrupt priority */
    CyIntSetPriority(LIN_1_UART_ISR_NUMBER, LIN_1_UART_ISR_PRIORITY);

    /* Set UART interrupt vector */
    (void) CyIntSetVector(LIN_1_UART_ISR_NUMBER, &LIN_1_UART_ISR);

    /* Start UART */
    LIN_1_UART_Start();

    /* Set Break Detection Threshold in counts */
    LIN_1_BREAK_THRESHOLD_REG = (l_u8) LIN_1_BREAK_THRESHOLD_VALUE;

    /* Allow interrupts on break, sync and inactivity (optional) events */
    LIN_1_INT_MASK_REG |= (LIN_1_INT_MASK_BREAK     |
                                      LIN_1_INT_MASK_SYNC      |
                                      LIN_1_INT_MASK_INACTIVITY);

    interruptState = CyEnterCriticalSection();

    LIN_1_STATUS_AUX_CONTROL_REG |= LIN_1_STATUS_AUX_CONTROL_INT_EN;

    CyExitCriticalSection(interruptState);

    /* Bus inactivity block configuration */
    #if(1u == LIN_1_INACTIVITY_ENABLED)

        /* Bus inactivity block configuration to issue interrupt every 100 ms */
        LIN_1_INACTIVITY_DIV0_REG = LIN_1_INACT_DIV0;
        LIN_1_INACTIVITY_DIV1_REG = LIN_1_INACT_DIV1;

    #endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */


    /* Save clock divider */
     #if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

        /* Save default clock divider */
        LIN_1_initialClockDivider = LIN_1_IntClk_GetDividerRegister();

    #endif  /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */

    /* Enable hardware blocks operation */
    LIN_1_CONTROL_REG |= LIN_1_CONTROL_ENABLE;

    /* Copy PIDs from NVRAM to VRAM */
    for(i = 0u; i < LIN_1_NUM_FRAMES; i++)
    {
        LIN_1_volatileConfig[i] = LIN_1_LinSlaveConfig.pidTable[i];
    }

    #if(1u == LIN_1_TL_ENABLED)

        LIN_1_nad = LIN_1_LinSlaveConfig.initialNad;

    #endif /* (1u == LIN_1_TL_ENABLED) */

    /*  Clear interface status */
    LIN_1_ifcStatus &= (l_u16) ~LIN_1_IFC_STS_MASK;

    /* Enable bLIN interrupt */
    CyIntEnable(LIN_1_BLIN_ISR_NUMBER);

    /* Initialization is always expected to succeeded */
    return (LIN_1_FALSE);
}


/*******************************************************************************
* Function Name: l_ifc_init
********************************************************************************
*
* Summary:
*  The function initializes the LIN Slave component instance that is specified
*  by the name iii. It sets up internal functions such as the baud rate and
*  starts up digital blocks that are used by the LIN Slave component. This is
*  the first call that must be performed, before using any other interface-
*  related LIN Slave API functions.
*
* Parameters:
*  iii - is the name of the interface handle.
*
* Return:
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
* Reentrant:
*  No
*
*******************************************************************************/
l_bool l_ifc_init(l_ifc_handle iii) 
{
    l_bool returnValue;

    switch(iii)
    {
        case LIN_1_IFC_HANDLE:
            returnValue = l_ifc_init_LIN_1();
        break;

        default:
            /* Unknown interface handle */
            returnValue = LIN_1_TRUE;
        break;
    }

    return (returnValue);
}


/*******************************************************************************
* Function Name: l_ifc_wake_up
********************************************************************************
*
* Summary:
*  This function transmits one wakeup signal. The wakeup signal is transmitted
*  directly when this function is called. When you call this API function, the
*  application is blocked until a wakeup signal is transmitted on the LIN bus.
*  The CyDelayUs() function is used as the timing source. The delay is
*  calculated based on the clock configuration entered in PSoC Creator.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void l_ifc_wake_up_LIN_1(void) 
{
    /* Force TXD low */
    LIN_1_CONTROL_REG |= LIN_1_CONTROL_TX_DIS;

    /* Wait */
    CyDelayUs(LIN_1_WAKE_UP_SIGNAL_LENGTH);

    /* Connect TXD from UART to LIN bus line */
    LIN_1_CONTROL_REG &=  ((l_u8) ~LIN_1_CONTROL_TX_DIS);
}


/*******************************************************************************
* Function Name: l_ifc_wake_up
********************************************************************************
*
* Summary:
*  This function transmits one wakeup signal. The wakeup signal is transmitted
*  directly when this function is called. When you call this API function, the
*  application is blocked until a wakeup signal is transmitted on the LIN bus.
*  The CyDelayUs() function is used as the timing source. The delay is
*  calculated based on the clock configuration entered in PSoC Creator.
*
* Parameters:
*  iii - is the name of the interface handle.
*
* Return:
*  None
*
*******************************************************************************/
void l_ifc_wake_up(l_ifc_handle iii) 
{
    switch(iii)
    {
        case LIN_1_IFC_HANDLE:
            l_ifc_wake_up_LIN_1();
        break;

        default:
            /* Unknown interface handle - do nothing. */
        break;
    }
}


/*******************************************************************************
* Function Name: l_ifc_ioctl
********************************************************************************
*
* Summary:
*  This function controls functionality that is not covered by the other API
*  calls. It is used for protocol specific parameters or hardware specific
*  functionality.
*
* Parameters:
*  op - is the operation that should be applied.
*  pv - is the pointer to the optional parameter.
*
* Return:
*  There is no error code value returned for operation selected. This means that
*  you must ensure that the values passed into the function are correct.
*
*  L_IOCTL_READ_STATUS operation:
*  The first bit in this byte is the flag that indicates that there has been no
*  signaling on the bus for a certain elapsed time (available when
*  Bus Inactivity Timeout Detection option is enabled). If the elapsed time
*  passes a certain threshold, then this flag is set. Calling this API clears
*  all status bits after they are returned. The second bit is the flag that
*  indicates that a Targeted Reset service request (0xB5) was received
*  (when J2602-1 Compliance is enabled).
*
*  Symbolic Name : LIN_1_IOCTL_STS_BUS_INACTIVITY
*  Value         : 0x0001u
*  Description   : No signal was detected on the bus for a certain elapsed time
*
*  Symbolic Name : LIN_1_IOCTL_STS_TARGET_RESET
*  Value         : 0x0002u
*  Description   : Targeted Reset service request (0xB5) was received.
*
*  L_IOCTL_SET_BAUD_RATE operation:
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
*  L_IOCTL_SLEEP operation:
*  The CYRET_SUCCESS is returned if operation succeeded and CYRET_BAD_PARAM if
*  an invalid operation parameter was passed to the function.
*
*  L_IOCTL_WAKEUP operation:
*  The CYRET_SUCCESS is returned if operation succeeded and CYRET_BAD_PARAM if
*  an invalid operation parameter was passed to the function.
*
*  L_IOCTL_SYNC_COUNTS operation:
*  The CYRET_SUCCESS is returned if operation succeeded and CYRET_BAD_PARAM if
*  an invalid operation parameter was passed to the function.
*
*  L_IOCTL_SET_SERIAL_NUMBER operation:
*  The CYRET_SUCCESS is returned if operation succeeded and CYRET_BAD_PARAM if
*  an invalid operation parameter was passed to the function.
*
* Reentrant:
*  No
*
*******************************************************************************/
l_u16 l_ifc_ioctl_LIN_1(l_ioctl_op op, void* pv) 
{
    /* Retention registers are stored here */
    static LIN_1_BACKUP_STRUCT  LIN_1_backup = {0u, };

    l_u16 returnValue = (l_u16) CYRET_SUCCESS;
    l_u8 interruptState;

    switch (op)
    {
        /***********************************************************************
        *                           Read Status
        ***********************************************************************/
        case L_IOCTL_READ_STATUS:

            /* Return status */
            returnValue = LIN_1_ioctlStatus;

            /* Clear status */
            LIN_1_ioctlStatus = 0x0000u;

        break;

        /***********************************************************************
        *                           Set Baud Rate
        ***********************************************************************/
        case L_IOCTL_SET_BAUD_RATE:

            interruptState = CyEnterCriticalSection();

            /* Set new internal clock divider with effect on end of cycle */
            LIN_1_IntClk_SetDividerRegister(((*((l_u16*)pv)) - 1u), 0u);

            /* Bus inactivity block reconfiguration */
            #if(1u == LIN_1_INACTIVITY_ENABLED)

                /* Divider 1 for specified interrupt rate */
                LIN_1_INACTIVITY_DIV0_REG =
                    (((l_u8)(((LIN_1_INACT_OVERSAMPLE_RATE) *
                    ((*(l_u16*)pv) / LIN_1_INACT_100MS_IN_S)) /
                    LIN_1_INACT_DIVIDE_FACTOR) - 1u));

                /* Divider 1 for specified interrupt rate */
                LIN_1_INACTIVITY_DIV1_REG =
                    ((l_u8)(((LIN_1_INACT_OVERSAMPLE_RATE) *
                    ((*(l_u16*)pv) / LIN_1_INACT_100MS_IN_S)) -
                    (LIN_1_INACT_DIVIDE_FACTOR *
                    LIN_1_INACTIVITY_DIV0_REG)));

            #endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */

            CyExitCriticalSection(interruptState);

        break;


        /***********************************************************************
        *                   Prepare for the low power modes
        ***********************************************************************/
        case L_IOCTL_SLEEP:

            LIN_1_backup.control = LIN_1_CONTROL_REG;

            if(0u != (LIN_1_CONTROL_REG  & LIN_1_CONTROL_ENABLE))
            {
               LIN_1_backup.enableState = LIN_1_TRUE;
            }
            else
            {
                LIN_1_backup.enableState = LIN_1_FALSE;
            }

            /* Disable interrupts */
            (void) l_sys_irq_disable();

            /* Prepare UART for low power mode */
            LIN_1_UART_Sleep();

            /* Disable hardware blocks operation */
            LIN_1_CONTROL_REG &= ((l_u8) ~LIN_1_CONTROL_ENABLE);

            LIN_1_backup.statusMask = LIN_1_INT_MASK_REG;

            #if(1u == LIN_1_INACTIVITY_ENABLED)

                LIN_1_backup.inactivityDiv0 = LIN_1_INACTIVITY_DIV0_REG;
                LIN_1_backup.inactivityDiv1 = LIN_1_INACTIVITY_DIV1_REG;

            #endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */

        break;


        /***********************************************************************
        *             Restore after wakeup from low power modes
        ***********************************************************************/
        case L_IOCTL_WAKEUP:

            LIN_1_INT_MASK_REG = LIN_1_backup.statusMask;

            #if(1u == LIN_1_INACTIVITY_ENABLED)

                LIN_1_INACTIVITY_DIV0_REG = LIN_1_backup.inactivityDiv0;
                LIN_1_INACTIVITY_DIV1_REG = LIN_1_backup.inactivityDiv1;

            #endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */

            LIN_1_CONTROL_REG = LIN_1_backup.control;

            /* Restore UART state*/
            LIN_1_UART_Wakeup();

            if(0u != LIN_1_backup.enableState)
            {
               l_sys_irq_restore(LIN_1_ALL_IRQ_ENABLE);
            }

        break;

        #if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

            case L_IOCTL_SYNC_COUNTS:

                /* Returns current number of sync field timer counts */
                returnValue = (l_u16) LIN_1_syncCounts;

            break;

        #endif  /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */


        #if(1u == LIN_1_TL_ENABLED)
            #if(1u == LIN_1_CS_ENABLED)

                case L_IOCTL_SET_SERIAL_NUMBER:
                    LIN_1_serialNumber = (l_u8*) pv;
                break;

            #endif /* (1u == LIN_1_CS_ENABLED) */
        #endif /* (1u == LIN_1_TL_ENABLED) */


        default:
            /* Unknown operation */
            returnValue = (l_u16) CYRET_BAD_PARAM;
        break;
    }

    return (returnValue);
}


/*******************************************************************************
* Function Name: l_ifc_ioctl
********************************************************************************
*
* Summary:
*  This function controls functionality that is not covered by the other API
*  calls. It is used for protocol specific parameters or hardware specific
*  functionality. Example of such functionality can be to switch on/off the
*  wake up signal detection.
*
* Parameters:
*  iii - is the name of the interface handle.
*  op - is the operation that should be applied.
*  pv - is the pointer to the optional parameter.
*
* Return:
*  There is no error code value returned for operation selected. This means that
*  you must ensure that the values passed into the function are correct.
*
*  L_IOCTL_READ_STATUS operation
*  The first bit in this byte is the flag that indicates that there has been no
*  signaling on the bus for a certain elapsed time (available when
*  Bus Inactivity Timeout Detection option is enabled). If the elapsed time
*  passes a certain threshold, then this flag is set. Calling this API clears
*  all status bits after they are returned. The second bit is the flag that
*  indicates that a Targeted Reset service request (0xB5) was received
*  (when J2602-1 Compliance is enabled).
*
*  Symbolic Name : LIN_1_IOCTL_STS_BUS_INACTIVITY
*  Value         : 0x0001u
*  Description   : No signal was detected on the bus for a certain elapsed time
*
*  Symbolic Name : LIN_1_IOCTL_STS_TARGET_RESET
*  Value         : 0x0002u
*  Description   : Targeted Reset service request (0xB5) was received.
*
*  L_IOCTL_SET_BAUD_RATE operation
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
*  L_IOCTL_SLEEP operation
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
*  L_IOCTL_WAKEUP operation
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
*  L_IOCTL_SYNC_COUNTS operation
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
*  L_IOCTL_SET_SERIAL_NUMBER operation
*  The 0 is returned if operation succeeded and 1 if an invalid operation
*  parameter was passed to the function.
*
* Reentrant:
*  No
*
*******************************************************************************/
l_u16 l_ifc_ioctl(l_ifc_handle iii, l_ioctl_op op, void* pv) 
{
    l_u16 returnValue;

    switch(iii)
    {
        case LIN_1_IFC_HANDLE:
            returnValue = l_ifc_ioctl_LIN_1(op, pv);
        break;

        default:
            /* Unknown operation */
            returnValue = (l_u16) CYRET_BAD_PARAM;
        break;
    }

    return (returnValue);
}


/*******************************************************************************
* Function Name: l_ifc_rx
********************************************************************************
*
* Summary:
*  The LIN Slave component takes care of calling this API routine automatically.
*  Therefore, this API routine must not be called by the application code.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_ifc_rx_LIN_1(void) 
{
    l_u8 i;
    l_u8 interruptState;

    static l_u16 LIN_1_interimChecksum;   /* Holds interim checksum value     */
    static l_u8 LIN_1_framePid;           /* PID of the current frame         */
    static l_u8 LIN_1_frameSize;          /* Size of frame being processed    */
    static l_u8 LIN_1_bytesTransferred;   /* Number of transferred bytes       */
    static l_u8 LIN_1_tmpData;            /* Used to store transmitted byte   */
    static l_u8 LIN_1_pidIndex;           /* Index in pidInfoTable            */
    static l_u8 LIN_1_tmpRxFrameData[8u]; /* RXed data before checksum checked */
    static volatile l_u8* LIN_1_frameData = NULL;  /* Pointer to frame data. Points to the byte to be sent.*/

    #if(1u == LIN_1_TL_ENABLED)

        #if(1u == LIN_1_TL_API_FORMAT)
            l_u8 tmpPci;
        #endif /* 1u == LIN_1_TL_API_FORMAT */

        static l_u8 LIN_1_mrfBuffer[LIN_1_FRAME_BUFF_LEN];
    #endif /* (1u == LIN_1_TL_ENABLED) */

    /*******************************************************************************
    * Parity lookup table.  Given a six bit identifier as an index, the indexed
    * value will provide the correct value with the parity bit set.
    *******************************************************************************/
    static const l_u8 CYCODE LIN_1_parityTable[] =
    {
        0x80u, 0xC1u, 0x42u, 0x03u, 0xC4u, 0x85u, 0x06u, 0x47u, 0x08u, 0x49u, 0xCAu,
        0x8Bu, 0x4Cu, 0x0Du, 0x8Eu, 0xCFu, 0x50u, 0x11u, 0x92u, 0xD3u, 0x14u, 0x55u,
        0xD6u, 0x97u, 0xD8u, 0x99u, 0x1Au, 0x5Bu, 0x9Cu, 0xDDu, 0x5Eu, 0x1Fu, 0x20u,
        0x61u, 0xE2u, 0xA3u, 0x64u, 0x25u, 0xA6u, 0xE7u, 0xA8u, 0xE9u, 0x6Au, 0x2Bu,
        0xECu, 0xADu, 0x2Eu, 0x6Fu, 0xF0u, 0xB1u, 0x32u, 0x73u, 0xB4u, 0xF5u, 0x76u,
        0x37u, 0x78u, 0x39u, 0xBAu, 0xFBu, 0x3Cu, 0x7Du, 0xFEu, 0xBFu
    };

    /* PID information table */
    static LIN_1_PID_INFO_TABLE const CYCODE LIN_1_pidInfoTable[LIN_1_NUM_FRAMES] =
{
    {(LIN_1_FRAME_DIR_PUBLISH | LIN_1_FRAME_DATA_SIZE_1 | LIN_1_FRAME_TYPE_UNCOND),
     LIN_1_Buffer_Frame1}, 

    {(LIN_1_FRAME_DIR_SUBSCRIBE | LIN_1_FRAME_DATA_SIZE_6 | LIN_1_FRAME_TYPE_UNCOND),
     LIN_1_Buffer_rpm}, 

    {(LIN_1_FRAME_DIR_SUBSCRIBE | LIN_1_FRAME_DATA_SIZE_3 | LIN_1_FRAME_TYPE_UNCOND),
     LIN_1_Buffer_PID}
};


    /* Check for correctness data for UART */
    if(0u == (LIN_1_FSM_UART_ENABLE_FLAG & LIN_1_fsmFlags))
    {
        /* Reset UART state machine */
        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
    }

    /* Check for UART framing error */
    if(0u != (LIN_1_UART_ReadRxStatus() & LIN_1_UART_RX_STS_STOP_ERROR))
    {
        #if(1u == LIN_1_SAE_J2602)

            /* Set framing error bits */
            LIN_1_j2602Status |= LIN_1_J2602_STS_FRAMING_ERR;

        #endif /* (1u == LIN_1_SAE_J2602) */

        /* Set framing error  */
        LIN_1_fsmFlags |= LIN_1_FSM_FRAMING_ERROR_FLAG;

        
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


        /* Finish frame processing */
        LIN_1_EndFrame(LIN_1_HANDLING_DONT_SAVE_PID);
    }

    switch(LIN_1_uartFsmState)
    {

        /***********************************************************************
        *                       Sync Field Byte Receive
        * State description:
        *  - Available if Automatic Baud Rate Synchronization is disabled
        *  - Receives sync byte and verifies its correctness
        *  - Next state is PID Field Byte Receive (state 1)
        ***********************************************************************/
        #if(0u == LIN_1_AUTO_BAUD_RATE_SYNC)

            case LIN_1_UART_ISR_STATE_0_SNC:

                /* Handle Sync field correctness */
                if(LIN_1_FRAME_SYNC_BYTE != LIN_1_UART_ReadRxData())
                {
                    /* Set response error */
                    LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                    #if(1u == LIN_1_SAE_J2602)

                        /* Set data error bit */
                        LIN_1_j2602Status |= LIN_1_J2602_STS_DATA_ERR;

                    #endif /* (1u == LIN_1_SAE_J2602) */

                    
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                    /* Reset UART State Machine */
                    LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                }
                else
                {
                    /* Next step is reception of the frame PID field */
                    LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_1_PID;
                }
            break;

        #endif  /* (0u == LIN_1_AUTO_BAUD_RATE_SYNC) */


        /***********************************************************************
        *                       PID Field Byte Receive
        * State description:
        *  - Receives protected identifier (PID)
        *  - Checks PID parity
        *  - Set flags
        *  - Determine next state (RX or TX)
        ***********************************************************************/
        case LIN_1_UART_ISR_STATE_1_PID:

            /* Save PID */
            LIN_1_framePid = LIN_1_UART_ReadRxData();

            /* Reset number of transferred bytes */
            LIN_1_bytesTransferred = 0u;

            /* Verify PID parity */
            if(LIN_1_parityTable[LIN_1_framePid & LIN_1_PID_PARITY_MASK] !=
                LIN_1_framePid)
            {
                /* Set response error */
                LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                #if(1u == LIN_1_SAE_J2602)

                    /* Set ERR2, ERR1 and ERR0 bits */
                    LIN_1_j2602Status |= LIN_1_J2602_STS_PARITY_ERR;

                #endif /* (1u == LIN_1_SAE_J2602) */

                
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                /* Reset UART State Machine */
                LIN_1_EndFrame(LIN_1_HANDLING_DONT_SAVE_PID);
            }
            else    /* PID parity is correct */
            {
                /* Check if MRF or SRF frame */
                if((LIN_1_FRAME_PID_MRF == LIN_1_framePid) ||
                (LIN_1_FRAME_PID_SRF == LIN_1_framePid))
                {
                    /*  Transport Layer section. MRF and SRF detection */
                    #if(1u == LIN_1_TL_ENABLED)

                        #if(1u == LIN_1_SAE_J2602)

                            if(LIN_1_FRAME_PID_MRF_J2602 == LIN_1_framePid)
                            {
                                /* Process Master Request */

                                /* Nothing need to be done for Transport Layer */

                                /* Set response error */
                                LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                                /* Set ERR2, ERR1 and ERR0 bits */
                                LIN_1_j2602Status |= LIN_1_J2602_STS_PARITY_ERR;

                                
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                                /* Check for framing error */
                                if(0u == (LIN_1_fsmFlags & LIN_1_FSM_FRAMING_ERROR_FLAG))
                                {
                                    /* Save the last processed PID on the bus to the status variable */
                                    LIN_1_ifcStatus &= ((l_u16) ~LIN_1_IFC_STS_PID_MASK);
                                    LIN_1_ifcStatus |= ((l_u16) (((l_u16)LIN_1_framePid) << 8u));
                                }

                                /* Reset UART State Machine */
                                LIN_1_EndFrame(LIN_1_HANDLING_DONT_SAVE_PID);
                            }

                        #endif  /* (1u == LIN_1_SAE_J2602) */

                        if(LIN_1_FRAME_PID_MRF == LIN_1_framePid)
                        {
                            /* Indicate that slave is required to receive the data */
                            LIN_1_tlFlags |= LIN_1_TL_RX_DIRECTION;

                            /*******************************************************
                            *               Cooked & RAW API
                            *******************************************************/

                            /* If the MRF PID is detected then pass a pointer to a start of a
                            * Frame Buffer and size of data to RX state to handle data receiving.
                            */

                            /* Frame equals 8 bytes */
                            LIN_1_frameSize = LIN_1_FRAME_DATA_SIZE_8;

                            /* Set frame data pointer to a start of a frame buffer */
                            LIN_1_frameData = LIN_1_mrfBuffer;

                            /* Switch to the subscribe data state. */
                            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_3_RX;
                        }

                        if(LIN_1_FRAME_PID_SRF == LIN_1_framePid)
                        {
                            /* Indicate that slave is required to transmit the data */
                            LIN_1_tlFlags |= LIN_1_TL_TX_DIRECTION;

                            if(0u != (LIN_1_status & LIN_1_STATUS_SRVC_RSP_RDY))
                            {
                                /* Clear Service Response ready status bit */
                                LIN_1_status &= ((l_u8) ~LIN_1_STATUS_SRVC_RSP_RDY);

                                /* Frame always equal to 8 bytes for TL */
                                LIN_1_frameSize = LIN_1_FRAME_DATA_SIZE_8;

                                /* Set frame data pointer to a start of a frame buffer */
                                LIN_1_frameData = LIN_1_srfBuffer;

                                /* Send first byte to the LIN master */
                                LIN_1_tmpData = *LIN_1_frameData;
                                LIN_1_frameData++;
                                LIN_1_UART_WriteTxData(LIN_1_tmpData);
                                LIN_1_bytesTransferred = 1u;

                                /* Switch to the publish data state. */
                                LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_2_TX;
                            }
                            else
                            {
                                #if(1u == LIN_1_TL_API_FORMAT)

                                    /***************************************************
                                    *                    Cooked API
                                    ***************************************************/

                                    /* Send one frame of a message if there is a message pending */
                                    if(LIN_1_txTlStatus == LD_IN_PROGRESS)
                                    {
                                        /* This part of code will handle PDU packing for Cooked API */
                                        /* Check length it shows if the message already sent */
                                        if(LIN_1_txMessageLength == 0u)
                                        {
                                            /* Reset UART State Machine */
                                            LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                                        }
                                        /* Process the message sending */
                                        else
                                        {
                                            /* Fill Frame NAD field */
                                            LIN_1_srfBuffer[0u] = LIN_1_nad;

                                            /* Analyze length to find the type of frame the message should be sent */
                                            if(LIN_1_txMessageLength > LIN_1_FRAME_DATA_SIZE_6)
                                            {
                                                /* Process the FF Frame */
                                                if(LIN_1_prevPci == LIN_1_PDU_PCI_TYPE_UNKNOWN)
                                                {
                                                    /* Fill Frame PCI field */
                                                    LIN_1_srfBuffer[1u] = (LIN_1_PDU_PCI_TYPE_FF |
                                                        (HI8(LIN_1_txMessageLength)));

                                                    /* Fill Frame LEN field */
                                                    LIN_1_srfBuffer[2u] =
                                                        LO8(LIN_1_txMessageLength);

                                                    /* Fill Frame Data fields */
                                                    for(i = 3u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                                                    {
                                                        LIN_1_srfBuffer[i] =
                                                            LIN_1_txTlDataPointer[(i +
                                                                LIN_1_txTlDataCount) - 3u];
                                                    }

                                                    /* Update the user buffer pointer */
                                                    LIN_1_txTlDataCount +=
                                                        LIN_1_FRAME_DATA_SIZE_5;

                                                    /* Save the previous PCI */
                                                    LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_FF;

                                                    LIN_1_txMessageLength -=
                                                        LIN_1_FRAME_DATA_SIZE_5;
                                                }
                                                /* Process the CF Frame */
                                                else
                                                {
                                                     /* Fill Frame PCI field */
                                                    LIN_1_srfBuffer[1u] =
                                                        (LIN_1_PDU_PCI_TYPE_CF |
                                                            LIN_1_txFrameCounter);

                                                    /* Fill Frame Data fields */
                                                    for(i = 2u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                                                    {
                                                        LIN_1_srfBuffer[i] =
                                                            LIN_1_txTlDataPointer[(i +
                                                                LIN_1_txTlDataCount) - 2u];
                                                    }

                                                    /* Update the user buffer pointer */
                                                    LIN_1_txTlDataCount +=
                                                            LIN_1_FRAME_DATA_SIZE_6;

                                                    /* Save the previous PCI */
                                                    LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_CF;

                                                    /* Update length pointer properly */
                                                    LIN_1_txMessageLength -=
                                                        LIN_1_FRAME_DATA_SIZE_6;
                                                }
                                            }
                                            /* Process the SF Frame or last CF Frame */
                                            else
                                            {
                                                /* Check if the Previous frame is unknown which indicates that current
                                                * frame is SF, otherwise it is last CF frame. Fill Frame PCI field
                                                * properly.
                                                */
                                                if(LIN_1_PDU_PCI_TYPE_UNKNOWN == LIN_1_prevPci)
                                                {
                                                    LIN_1_srfBuffer[1u] = (l_u8)
                                                        LIN_1_txMessageLength;

                                                    /* Save the previous PCI */
                                                    LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_SF;
                                                }
                                                else
                                                {
                                                    LIN_1_srfBuffer[1u] =
                                                        (LIN_1_PDU_PCI_TYPE_CF |
                                                            LIN_1_txFrameCounter);

                                                    /* Save the previous PCI */
                                                    LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_CF;
                                                }

                                                /* Fill Frame Data fields */
                                                for(i = 2u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                                                {
                                                    if(LIN_1_txMessageLength >= ((l_u8) (i - 1u)))
                                                    {
                                                        LIN_1_srfBuffer[i] =
                                                            LIN_1_txTlDataPointer[(i +
                                                                LIN_1_txTlDataCount) - 2u];
                                                    }
                                                    else
                                                    {
                                                        /* Fill unused data bytes with FFs */
                                                        LIN_1_srfBuffer[i] = 0xFFu;
                                                    }
                                                }

                                                /* Update the user buffer pointer */
                                                LIN_1_txTlDataCount +=
                                                            LIN_1_FRAME_DATA_SIZE_6;

                                                /* Update length pointer properly */
                                                if(LIN_1_txMessageLength >=
                                                        LIN_1_FRAME_DATA_SIZE_6)
                                                {
                                                    LIN_1_txMessageLength -=
                                                        LIN_1_FRAME_DATA_SIZE_6;
                                                }
                                                else
                                                {
                                                    LIN_1_txMessageLength = 0u;
                                                }
                                            }

                                            /* Update the frame counter */
                                            if(LIN_1_txFrameCounter != 15u)
                                            {
                                                LIN_1_txFrameCounter++;
                                            }
                                            else
                                            {
                                                /* If frame counter is larger then 15 then reset it */
                                                LIN_1_txFrameCounter = 0u;
                                            }
                                        }

                                        /* Frame equals 8 bytes */
                                        LIN_1_frameSize = LIN_1_FRAME_DATA_SIZE_8;

                                        /* Set frame data pointer to a start of a frame buffer */
                                        LIN_1_frameData = LIN_1_srfBuffer;

                                        /* Send first byte to the LIN master */
                                        LIN_1_tmpData = *LIN_1_frameData;
                                        LIN_1_frameData++;
                                        LIN_1_UART_WriteTxData(LIN_1_tmpData);
                                        LIN_1_bytesTransferred = 1u;

                                        /* Switch to the publish data state. */
                                        LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_2_TX;
                                    }
                                    else
                                    {
                                        /* Reset UART State Machine */
                                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                                    }

                                #else

                                    /***************************************************
                                    *                     Raw API
                                    ***************************************************/

                                    if(0u != LIN_1_txBufDepth)
                                    {
                                        /* Fill the frame buffer from SRF*/
                                        for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                                        {
                                            LIN_1_srfBuffer[i] =
                                                LIN_1_rawTxQueue[LIN_1_txRdIndex];

                                            /* Update the index to TX queue */
                                            LIN_1_txRdIndex++;
                                        }

                                        /* Read index should point to the next byte in MRF */
                                        if(LIN_1_TL_TX_QUEUE_LEN == LIN_1_txRdIndex)
                                        {
                                            LIN_1_txRdIndex = 0u;
                                        }

                                        /* 8 bytes were read from the SRF so decrement the buffer depth */
                                        LIN_1_txBufDepth--;

                                        /* Update status properly */
                                        if(0u == LIN_1_txBufDepth)
                                        {
                                            LIN_1_txTlStatus = LD_QUEUE_EMPTY;
                                        }
                                        else
                                        {
                                            LIN_1_txTlStatus = LD_QUEUE_AVAILABLE;
                                        }

                                        /* Frame equals 8 bytes */
                                        LIN_1_frameSize = LIN_1_FRAME_DATA_SIZE_8;

                                        /* Set frame data pointer to a start of a frame buffer */
                                        LIN_1_frameData = LIN_1_srfBuffer;

                                        /* Send first byte to the LIN master */
                                        LIN_1_tmpData = *LIN_1_frameData;
                                        LIN_1_frameData++;
                                        LIN_1_UART_WriteTxData(LIN_1_tmpData);
                                        LIN_1_bytesTransferred = 1u;

                                        /* Switch to the publish data state. */
                                        LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_2_TX;
                                    }
                                    else
                                    {
                                        /* Reset UART State Machine */
                                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                                    }

                                #endif /* (1u == LIN_1_TL_API_FORMAT) */
                            }


                        }

                    #else

                        /* These are invalid PIDs when TL is disabled - reset UART state machine */

                        /* Set response error */
                        LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                        #if(1u == LIN_1_SAE_J2602)

                            /* Set ERR2, ERR1 and ERR0 bits */
                            LIN_1_j2602Status |= LIN_1_J2602_STS_PARITY_ERR;

                        #endif /* (1u == LIN_1_SAE_J2602) */

                        
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                        /* Reset UART State Machine */
                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);

                    #endif /* (1u == LIN_1_TL_ENABLED) */
                }
                else    /* Not MRF and SRF */
                {
                    /* Get PID index in LIN_1_pidInfoTable */
                    LIN_1_pidIndex = LIN_1_FindPidIndex(LIN_1_framePid);

                    if(LIN_1_INVALID_FRAME_PID != LIN_1_pidIndex)
                    {
                        /* Valid ID */

                        /* Start enhanced checksum calculation  */
                        LIN_1_interimChecksum = LIN_1_framePid;

                        /* Get size of frame */
                        LIN_1_frameSize = LIN_1_pidInfoTable[LIN_1_pidIndex].param &
                                                        LIN_1_FRAME_DATA_SIZE_MASK;

                        /* TX response (publish action) was requested by Master */
                        if(0u != (LIN_1_FRAME_DIR_PUBLISH &
                                 LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                        {
                            /* SAE J2602 is disabled */
                            #if(0u == LIN_1_SAE_J2602)

                                /* This frame is event-triggered */
                                if(0u  != (LIN_1_FRAME_TYPE_EVENT &
                                           LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                                {
                                    /* Check do we need to process event-triggered frame */
                                    if(0u == LIN_1_GetEtFlagValue(LIN_1_pidIndex))
                                    {
                                        /* Reset UART State Machine */
                                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                                    }
                                }

                            #endif /* (0u == LIN_1_SAE_J2602) */

                            /* Get pointer to the frame data */
                            LIN_1_frameData =
                                LIN_1_pidInfoTable[LIN_1_pidIndex].dataPtr;

                            /* Send first byte to the LIN master */
                            LIN_1_tmpData = *LIN_1_frameData;
                            LIN_1_frameData++;
                            LIN_1_UART_WriteTxData(LIN_1_tmpData);
                            LIN_1_bytesTransferred = 1u;

                            /* Switch to the publish data state. */
                            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_2_TX;

                        }
                        else    /* RX response (subscribe action) was requested by Master */
                        {
                            /* Get pointer to the temp RX frame data buffer */
                            LIN_1_frameData = LIN_1_tmpRxFrameData;

                            /* Switch to the subscribe data state. */
                            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_3_RX;
                        }
                    }
                    else    /* Invalid ID */
                    {
                        /* Set response error */
                        LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                        #if(1u == LIN_1_SAE_J2602)

                            /* Set data error bit */
                            LIN_1_j2602Status |= LIN_1_J2602_STS_DATA_ERR;

                        #endif /* (1u == LIN_1_SAE_J2602) */

                        
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                        /* Reset UART State Machine */
                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                    }
                }
            }

        break;


        /***********************************************************************
        *                       TX response (Publish)
        * State description:
        *  - Transmits data to LIN Master
        *  - Transmits next data byte if there were no any errors
        *  - Transmits checksum when data was send correctly
        ***********************************************************************/
        case LIN_1_UART_ISR_STATE_2_TX:

            /* Set the response active flag */
            LIN_1_status |= LIN_1_STATUS_RESPONSE_ACTIVE;

            /* Previously transmitted and read back bytes are not equal */
            if(LIN_1_tmpData != LIN_1_UART_ReadRxData())
            {
                /* Mismatch Error */

                #if(1u == LIN_1_SAE_J2602)

                    /* Set ERR2 bit */
                    LIN_1_j2602Status |= LIN_1_J2602_STS_DATA_ERR;

                    /* Readback error - set response error flag */
                    LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                #else

                    /* Skip event-triggered frame */
                    if(0u  == (LIN_1_FRAME_TYPE_EVENT &
                               LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                    {
                        /* Readback error - set response error flag */
                        LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;
                    }

                #endif  /* (1u == LIN_1_SAE_J2602) */

                
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                /* Check for framing error */
                if(0u == (LIN_1_fsmFlags & LIN_1_FSM_FRAMING_ERROR_FLAG))
                {
                    /* Save the last processed PID on the bus to the status variable */
                    LIN_1_ifcStatus &= ((l_u16) ~LIN_1_IFC_STS_PID_MASK);
                    LIN_1_ifcStatus |= ((l_u16) (((l_u16)LIN_1_framePid) << 8u));
                }

                /* End frame with response error */
                LIN_1_EndFrame(LIN_1_HANDLING_DONT_SAVE_PID);
            }
            else    /* If readback was successful than continue transmitting */
            {
                /* Add transmitted byte to the interim checksum */
                LIN_1_interimChecksum += LIN_1_tmpData;
                if(LIN_1_interimChecksum >= 256u)
                {
                    LIN_1_interimChecksum -= 255u;
                }

                /* Check to see if all data bytes were sent */
                if(LIN_1_frameSize > LIN_1_bytesTransferred)
                {
                    /* Send out the next byte of the buffer */
                    LIN_1_tmpData = *LIN_1_frameData;
                    LIN_1_frameData++;
                    LIN_1_UART_WriteTxData(LIN_1_tmpData);
                    LIN_1_bytesTransferred++;
                }
                else    /* All data bytes were sent - compute and transmit checksum */
                {
                    /* Compute and send out the checksum byte */
                    LIN_1_UART_WriteTxData((((l_u8) LIN_1_interimChecksum) ^ 0xFFu));

                    LIN_1_bytesTransferred = 0u;

                    /* Switch to the checksum state */
                    LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_4_CHS;
                }
            }
        break;


        /***********************************************************************
        *                       RX response (Subscribe)
        * State description:
        *  - Receives data from LIN Master
        *  - Received data are saved to the temporary buffer
        ***********************************************************************/
        case LIN_1_UART_ISR_STATE_3_RX:

            /* Save received byte */
            LIN_1_tmpData = LIN_1_UART_ReadRxData();
            *LIN_1_frameData = LIN_1_tmpData;
            LIN_1_frameData++;
            LIN_1_bytesTransferred++;

            /* Set response active flag */
            LIN_1_status |= LIN_1_STATUS_RESPONSE_ACTIVE;

            /* One or more data bytes have been received */
            LIN_1_fsmFlags |= LIN_1_FSM_DATA_RECEIVE;

            /* Add received byte to the interim checksum */
            LIN_1_interimChecksum += LIN_1_tmpData;
            if(LIN_1_interimChecksum >= 256u)
            {
                LIN_1_interimChecksum -= 255u;
            }

            /* Check to see if the data section has not finished */
            if(LIN_1_frameSize > LIN_1_bytesTransferred)
            {
                /* There is data to be sent */
            }
            else
            {
                /* There is no data to be sent */

                LIN_1_bytesTransferred = 0u;

                /* Switch to the checksum state */
                LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_4_CHS;
            }

        break;



        /***********************************************************************
        *                              Checksum
        ***********************************************************************/
        case LIN_1_UART_ISR_STATE_4_CHS:

            /* Previously transmitted and read back bytes are not equal */
            if((((l_u8) LIN_1_interimChecksum) ^ 0xFFu) != LIN_1_UART_ReadRxData())
            {
                /* Mismatch or Ckechsum Error */

                /* Set response error */
                LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

                #if(1u == LIN_1_SAE_J2602)

                    /* Set ERR2 and ERR0 bits */
                    LIN_1_j2602Status |= LIN_1_J2602_STS_CHECKSUM_ERR;

                #endif /* (1u == LIN_1_SAE_J2602) */

                
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


                /* Check for framing error */
                if(0u == (LIN_1_fsmFlags & LIN_1_FSM_FRAMING_ERROR_FLAG))
                {
                    /* Save the last processed PID on the bus to the status variable */
                    LIN_1_ifcStatus &= ((l_u16) ~LIN_1_IFC_STS_PID_MASK);
                    LIN_1_ifcStatus |= ((l_u16) (((l_u16)LIN_1_framePid) << 8u));
                }

                /* Reset UART state machine with checksum or mismatch error */
                LIN_1_EndFrame(LIN_1_HANDLING_DONT_SAVE_PID);
            }
            else
            {
                /*  Clear all error bits in interface status */
                #if(1u == LIN_1_SAE_J2602)

                    LIN_1_j2602Status &= LIN_1_J2602_CLEAR_ERR_BITS_MASK;

                #endif  /* (1u == LIN_1_SAE_J2602) */


                /* Clear framing error and data receive flags */
                LIN_1_fsmFlags &=
                   ((l_u8) ~(LIN_1_FSM_FRAMING_ERROR_FLAG | LIN_1_FSM_DATA_RECEIVE));

                /* Set successful transfer interface flag */
                LIN_1_ifcStatus |= LIN_1_IFC_STS_SUCCESSFUL_TRANSFER;

                /* Save the last processed PID on the bus to the status variable */
                LIN_1_ifcStatus &= ((l_u16) ~LIN_1_IFC_STS_PID_MASK);
                LIN_1_ifcStatus |= ((l_u16) (((l_u16)LIN_1_framePid) << 8u));

                /* Set overrun interface flag */
                if(0u != (LIN_1_FSM_OVERRUN & LIN_1_fsmFlags))
                {
                    LIN_1_ifcStatus |= LIN_1_IFC_STS_OVERRUN;
                }

                /* Set Overrun flag */
                LIN_1_fsmFlags |= LIN_1_FSM_OVERRUN;

                /* Clear response error signal if frame contains RESPONSE ERROR signal */
                #if(1u == LIN_1_RESPONSE_ERROR_SIGNAL)

                    if(LIN_1_RESPONSE_ERROR_FRAME_INDEX == LIN_1_pidIndex)
                    {
                        
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] &= 
                            (l_u8) ~LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);

                    }

                #endif /* (1u == LIN_1_RESPONSE_ERROR_SIGNAL) */


                #if(1u == LIN_1_TL_ENABLED)

                    if(!((LIN_1_FRAME_PID_MRF == LIN_1_framePid) ||
                         (LIN_1_FRAME_PID_SRF == LIN_1_framePid)))
                    {
                        /* SAE J2602 is disabled */
                        #if(0u == LIN_1_SAE_J2602)

                            /* This frame is event-triggered */
                            if(0u  != (LIN_1_FRAME_TYPE_EVENT &
                                       LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                            {
                                /* Clear event-triggered flags */
                                LIN_1_ClearEtFlagValue(LIN_1_pidIndex);

                                /* Reset UART State Machine */
                               LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                            }

                        #endif /* (0u == LIN_1_SAE_J2602) */

                        /* Set associated with current frame flags */
                        LIN_1_SetAssociatedFlags(LIN_1_pidIndex);
                    }

                #else

                    /* SAE J2602 is disabled */
                    #if(0u == LIN_1_SAE_J2602)

                        /* This frame is event-triggered */
                        if(0u  != (LIN_1_FRAME_TYPE_EVENT &
                                   LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                        {
                            /* Clear event-triggered flags */
                            LIN_1_ClearEtFlagValue(LIN_1_pidIndex);

                            /* Reset UART State Machine */
                           LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                        }

                    #endif /* (0u == LIN_1_SAE_J2602) */

                    /* Set associated with current frame flags */
                    LIN_1_SetAssociatedFlags(LIN_1_pidIndex);

                #endif  /* (1u == LIN_1_TL_ENABLED) */

                #if(1u == LIN_1_TL_ENABLED)

                    /* Check to see if received data was a "master request frame" */
                    if(LIN_1_FRAME_PID_MRF == LIN_1_framePid)
                    {
                        /* Process master request frame data here */
                        LIN_1_ProcessMrf(LIN_1_mrfBuffer);

                        #if(1u == LIN_1_TL_API_FORMAT)

                            /* Store previous PCI to avoid MISRA warning */
                            tmpPci = LIN_1_prevPci;

                            if((LIN_1_PDU_PCI_TYPE_FF == tmpPci) ||
                                (LIN_1_PDU_PCI_TYPE_CF == tmpPci))
                            {
                                LIN_1_tlFlags |= LIN_1_TL_N_CR_TIMEOUT_ON;
                                LIN_1_tlTimeoutCnt = 0u;
                            }

                            if((0u == LIN_1_rxMessageLength) &&
                                ( 0u != (LIN_1_tlFlags & LIN_1_TL_RX_REQUESTED)))
                            {
                                /* Indicate that message is received */
                                LIN_1_rxTlStatus = LD_COMPLETED;

                                /* Reset the frame counter */
                                LIN_1_rxFrameCounter = 0u;

                                /* Previous PCI is required to be unknown at the beginning of a new message */
                                LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_UNKNOWN;

                                /* Clear TX requested flag as the message was transferred */
                                LIN_1_tlFlags &= ((l_u8) ~LIN_1_TL_RX_REQUESTED);

                                LIN_1_tlFlags &= ((l_u8) ~LIN_1_TL_N_CR_TIMEOUT_ON);
                            }

                        #endif /* (1u == LIN_1_TL_API_FORMAT) */

                        /* Clear the TL RX direction flag */
                        LIN_1_tlFlags &= ((l_u8) ~LIN_1_TL_RX_DIRECTION);

                        /* Reset UART state machine */
                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                    }
                    else if(LIN_1_FRAME_PID_SRF == LIN_1_framePid)
                    {
                        #if(1u == LIN_1_TL_API_FORMAT)

                            /* Store previous PCI to avoid MISRA warning */
                            tmpPci = LIN_1_prevPci;

                            if((LIN_1_PDU_PCI_TYPE_FF == tmpPci) ||
                                (LIN_1_PDU_PCI_TYPE_CF == tmpPci))
                            {
                                LIN_1_tlFlags |= LIN_1_TL_N_AS_TIMEOUT_ON;
                                LIN_1_tlTimeoutCnt = 0u;
                            }

                            if((0u == LIN_1_txMessageLength) &&
                                ( 0u != (LIN_1_tlFlags & LIN_1_TL_TX_REQUESTED)))
                            {
                                /* Indicate that message is sent */
                                LIN_1_txTlStatus = LD_COMPLETED;

                                /* Reset the frame counter */
                                LIN_1_txFrameCounter = 0u;

                                /* Previous PCI is required to be unknown at the beginning of a new message */
                                LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_UNKNOWN;

                                /* Clear TX requested flag as the message was transferred */
                                LIN_1_tlFlags &= (l_u8) ~LIN_1_TL_TX_REQUESTED;

                                LIN_1_tlFlags &= (l_u8) ~LIN_1_TL_N_AS_TIMEOUT_ON;
                            }

                        #endif /* (1u == LIN_1_TL_API_FORMAT) */

                        /* Clear the TL TX direction flag */
                        LIN_1_tlFlags &= ((l_u8) ~LIN_1_TL_TX_DIRECTION);

                        /* Reset UART state machine */
                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                    }
                    else
                    {
                        /* RX response (subscribe action) was requested by Master */
                        if(0u == (LIN_1_FRAME_DIR_PUBLISH &
                                 LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                        {
                            interruptState = CyEnterCriticalSection();

                            /* Copy received data from tempororary buffer to the frame one */
                            for(i = 0u; i < LIN_1_frameSize; i++)
                            {
                                *(LIN_1_pidInfoTable[LIN_1_pidIndex].dataPtr + i) =
                                    LIN_1_tmpRxFrameData[i];
                            }

                            CyExitCriticalSection(interruptState);
                        }

                        /* Reset UART state machine */
                        LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
                    }

                #else

                    /* RX response (subscribe action) was requested by Master */
                    if(0u == (LIN_1_FRAME_DIR_PUBLISH &
                             LIN_1_pidInfoTable[LIN_1_pidIndex].param))
                    {
                        interruptState = CyEnterCriticalSection();

                        /* Copy received data from tempororary buffer to the frame one */
                        for(i = 0u; i < LIN_1_frameSize; i++)
                        {
                            *(LIN_1_pidInfoTable[LIN_1_pidIndex].dataPtr + i) =
                                LIN_1_tmpRxFrameData[i];
                        }

                        CyExitCriticalSection(interruptState);
                    }

                    /* Reset UART state machine */
                    LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);

                #endif  /* (1u == LIN_1_TL_ENABLED) */

            }

            LIN_1_interimChecksum = 0u;

        break;


        default:
            /* Reset UART state machine */
            LIN_1_EndFrame(LIN_1_HANDLING_RESET_FSM_ERR);
        break;
    }


    /***************************************************************************
    *  Place your UART ISR code here
    ***************************************************************************/
    /* `#START LIN_1_UART_ISR_CODE` */

    /* `#END` */
}


/*******************************************************************************
* Function Name: l_ifc_rx
********************************************************************************
*
* Summary:
*  The LIN Slave component takes care of calling this API routine automatically.
*  Therefore, this API routine must not be called by the application code.
*
* Parameters:
*  iii - is the name of the interface handle.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_ifc_rx(l_ifc_handle iii) 
{
    switch(iii)
    {
        case LIN_1_IFC_HANDLE:
            l_ifc_rx_LIN_1();
        break;

        default:
        break;
    }
}


/*******************************************************************************
* Function Name: l_ifc_rx
********************************************************************************
*
* Summary:
*  The LIN Slave component takes care of calling this API routine automatically.
*  Therefore, this API routine must not be called by the application code.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_ifc_tx_LIN_1(void) 
{
    l_ifc_rx_LIN_1();
}


/*******************************************************************************
* Function Name: l_ifc_tx
********************************************************************************
*
* Summary:
*  The LIN Slave component takes care of calling this API routine automatically.
*  Therefore, this API routine must not be called by the application code.
*
* Parameters:
*  iii - is the name of the interface handle.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_ifc_tx(l_ifc_handle iii) 
{
    switch(iii)
    {
        case LIN_1_IFC_HANDLE:
            l_ifc_tx_LIN_1();
        break;

        default:
        break;
    }
}


/*******************************************************************************
* Function Name: l_ifc_aux
********************************************************************************
*
* Summary:
*  The LIN Slave component takes care of calling this API routine automatically.
*  Therefore, this API routine must not be called by the application code.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_ifc_aux_LIN_1(void) 
{
    l_u8 interruptState;

    /* Update shadow status register with the hardware status */
    LIN_1_auxStatus |= LIN_1_STATUS_REG;

    /***************************************************************************
    *                             Edge Detected                                *
    ***************************************************************************/
    if(0u != (LIN_1_auxStatus & LIN_1_STATUS_EDGE_DETECTED))
    {
        /* Set bus activity interface status bit */
        LIN_1_ifcStatus |= LIN_1_IFC_STS_BUS_ACTIVITY;

        #if(1u == LIN_1_INACTIVITY_ENABLED)

            /* Clear period timer counter */
            LIN_1_periodCounter = 0x00u;

        #endif /* (1u == LIN_1_INACTIVITY_ENABLED) */
    }


    /***************************************************************************
    *                       Bus Inactivity Interrupt Detected
    ***************************************************************************/
    #if(1u == LIN_1_INACTIVITY_ENABLED)

        if(0u != (LIN_1_auxStatus & LIN_1_STATUS_INACTIVITY_INT))
        {
            #if(1u == LIN_1_TL_ENABLED)              /* TL enabled */
                #if(1u == LIN_1_TL_API_FORMAT)       /* Cooked API */

                    /* if the timeout is enabled then proceed timeout manage */
                    if(0u !=(LIN_1_tlFlags & LIN_1_TL_N_AS_TIMEOUT_ON))
                    {
                        /* Increment timeout */
                        LIN_1_tlTimeoutCnt++;

                        if(LIN_1_TL_N_AS_TIMEOUT_VALUE <= LIN_1_tlTimeoutCnt)
                        {
                            /* Set error status as the timeout occurred */
                            LIN_1_txTlStatus = LD_N_AS_TIMEOUT;
                        }
                    }
                    else if(0u !=(LIN_1_tlFlags & LIN_1_TL_N_CR_TIMEOUT_ON))
                    {
                        /* Increment timeout */
                        LIN_1_tlTimeoutCnt++;

                        if(LIN_1_TL_N_CR_TIMEOUT_VALUE <= LIN_1_tlTimeoutCnt)
                        {
                            /* Set error status as the timeout occurred */
                            LIN_1_rxTlStatus = LD_N_CR_TIMEOUT;
                        }
                    }
                    else
                    {
                        /* Reset timeout counter */
                        LIN_1_tlTimeoutCnt = 0u;
                    }

                #endif /* (1u == LIN_1_TL_API_FORMAT) */
            #endif /* (1u == LIN_1_TL_ENABLED) */

            if(LIN_1_INACTIVITY_THRESHOLD_IN_100_MS == LIN_1_periodCounter)
            {
                /* Inactivity threshold achieved */

                /* Set bus inactivity ioctl status bit */
                LIN_1_ioctlStatus |= LIN_1_IOCTL_STS_BUS_INACTIVITY;
            }
            else
            {
                LIN_1_periodCounter++;
            }
        }

    #endif  /* (1u == LIN_1_INACTIVITY_ENABLED) */


    /***************************************************************************
    *                       Break Field Detected
    ***************************************************************************/
    if(0u != (LIN_1_auxStatus & LIN_1_STATUS_BREAK_DETECTED))
    {
        /* Framing error or data transfer was aborted */
        if(0u  != ((LIN_1_FSM_FRAMING_ERROR_FLAG | LIN_1_FSM_DATA_RECEIVE) &
                    LIN_1_fsmFlags))
        {
            /* Set response error */
            LIN_1_ifcStatus |= LIN_1_IFC_STS_ERROR_IN_RESPONSE;

            /* Clear framing error */
            LIN_1_fsmFlags &= ((l_u8) ~LIN_1_FSM_FRAMING_ERROR_FLAG);

            #if(1u == LIN_1_SAE_J2602)

                /* Set ERR2 bit */
                LIN_1_j2602Status |= LIN_1_J2602_STS_DATA_ERR;

            #endif  /* (1u == LIN_1_SAE_J2602) */

            
    /* Save current global interrupt enable and disable it */
    interruptState = CyEnterCriticalSection();

        LIN_1_Buffer_Frame1[LIN_1_Response_Error_Frame1_SIG_BYTE_OFFSET] |= 
                            LIN_1_Response_Error_Frame1_SIG_MASK_0;

    /* Restore global interrupt enable state */
    CyExitCriticalSection(interruptState);


        }   /* No response error, continue */

        /* Set break detected flag */
        LIN_1_fsmFlags |= LIN_1_FSM_BREAK_FLAG;

        /***********************************************************************
        *  This will make bus RXD signal to be automatically routed to the
        *  UART's input when rising edge of the break field occurs (in case
        *  when Auto Baud Rate Sync is disabled; that will make UART to be
        *  able to receive sync byte) or when sync field is already processed
        *  by hardware (in case when Auto Baud Rate Sync is enabled; that will
        *  make UART to be able to receive frame's PID).
        ***********************************************************************/
        LIN_1_CONTROL_REG &= ((l_u8) ~LIN_1_CONTROL_RX_DIS);

        /* Auto Baud Rate Sync Enabled */
        #if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

            /* Restore initial clock divider */
            LIN_1_IntClk_SetDividerRegister(LIN_1_initialClockDivider, 0u);

        #else   /* Auto Baud Rate Sync Disabled */

            /* Clear one or more data bytes have been received internal flag */
            LIN_1_fsmFlags &= ((l_u8) ~LIN_1_FSM_DATA_RECEIVE);

            /* Set UART ISR FSM to sync byte receive state */
            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_0_SNC;

            /* Set UART enable flag */
            LIN_1_fsmFlags |= LIN_1_FSM_UART_ENABLE_FLAG;

            /* Clear any pending UART interrupt */
            CyIntClearPending(LIN_1_UART_ISR_NUMBER);

            /* Enable UART ISR interrupt */
            CyIntEnable(LIN_1_UART_ISR_NUMBER);

        #endif  /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */
    }


    /* Auto Baud Rate Sync Enabled */
    #if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

        /***********************************************************************
        *                       Sync Field Complete                            *
        ***********************************************************************/
        if(0u != (LIN_1_auxStatus & LIN_1_STATUS_SYNC_COMPLETED))
        {
            /* Save actual sync field timer counts */
            LIN_1_syncCounts = LIN_1_LOW_BIT_LENGTH_SUM_REG;
            LIN_1_syncCounts = LIN_1_LOW_BIT_LENGTH_SUM_REG;
            LIN_1_syncCounts = LIN_1_LOW_BIT_LENGTH_SUM_REG;
            LIN_1_syncCounts = LIN_1_LOW_BIT_LENGTH_SUM_REG +
                                          LIN_1_HIGH_BITS_LENGTH_SUM_REG;

            /* Set new clock divider */
            if(LIN_1_syncCounts != LIN_1_EXPECTED_TIME_COUNTS)
            {
                LIN_1_IntClk_SetDividerRegister((uint16)(((uint32)LIN_1_initialClockDivider *
                    (uint32) LIN_1_syncCounts) / LIN_1_EXPECTED_TIME_COUNTS), 0u);
            }

            /* Clear one or more data bytes have been received internal flag */
            LIN_1_fsmFlags &= ((l_u8) ~LIN_1_FSM_DATA_RECEIVE);

            /* This will reset the UART ISR FSM to state 1 */
            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_1_PID;

            /* Set UART enabled flag */
            LIN_1_fsmFlags |= LIN_1_FSM_UART_ENABLE_FLAG;

            /* Clear any pending UART interrupt */
            CyIntClearPending(LIN_1_UART_ISR_NUMBER);

            /* Enable UART ISR interrupt */
            CyIntEnable(LIN_1_UART_ISR_NUMBER);
        }

    #endif /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */


    /***************************************************************************
    *  Place your BASE ISR code here
    ***************************************************************************/
    /* `#START LIN_1_BASE_ISR_CODE` */

    /* `#END` */


    /* Clear software shadow register file  */
    LIN_1_auxStatus = 0x00u;
}


/*******************************************************************************
* Function Name: l_ifc_aux
********************************************************************************
*
* Summary:
*  The LIN Slave component takes care of calling this API routine automatically.
*  Therefore, this API routine must not be called by the application code.
*
* Parameters:
*  iii - is the name of the interface handle.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void l_ifc_aux(l_ifc_handle iii) 
{
    switch(iii)
    {
        case LIN_1_IFC_HANDLE:
            l_ifc_aux_LIN_1();
        break;

        default:
        break;
    }
}


/*******************************************************************************
* Function Name: l_ifc_read_status_LIN_1
********************************************************************************
*
* Summary:
*  This function is defined by the LIN specification. This returns the status of
*  the specified LIN interface and then clears all status bits for that
*  interface. See Section 7.2.5.8 of the LIN 2.1 specification.
*
* Parameters:
*  None
*
* Return:
*  The status bits of the specified LIN interface are returned. These bits have
*  the following meanings:
*    [15:8]    Last Received PID
*    [7]        0
*    [6]        Save Configuration flag
*    [5]        0
*    [4]        Bus Activity flag
*    [3]        Go To Sleep flag
*    [2]        Overrun flag
*    [1]        Successful Transfer flag
*    [0]        Error in Response flag
*
* Reentrant:
*  No
*
*******************************************************************************/
l_u16 l_ifc_read_status_LIN_1(void) 
{
    l_u16 returnValue;
    l_u8 interruptState;

    interruptState = CyEnterCriticalSection();

    /***************************************************************************
    * Update software shadow status register with the value of the hardware
    * status register to obtain current value of the edge detected bit. The
    * edge detected status bit is cleared immediately and the rest of the
    * software shadow status register bits are expected to be cleared in ISR.
    ***************************************************************************/
    LIN_1_auxStatus |= LIN_1_STATUS_REG;

    /* Optionally update interface status variable */
    if(0u != (LIN_1_auxStatus & LIN_1_STATUS_EDGE_DETECTED))
    {
        /* Set bus activity bit in IFC status */
        LIN_1_ifcStatus |= LIN_1_IFC_STS_BUS_ACTIVITY;

        /* Clear edge detected bit in shadow register variable */
        LIN_1_auxStatus &= ((l_u8) ~LIN_1_STATUS_EDGE_DETECTED);

        #if(1u == LIN_1_INACTIVITY_ENABLED)

            /* Clear period timer counter */
            LIN_1_periodCounter = 0x00u;

        #endif /* (1u == LIN_1_INACTIVITY_ENABLED) */
    }

    /* Copy the global status variable to the local temp variable */
    returnValue = LIN_1_ifcStatus;

    /* Clear status variable */
    LIN_1_ifcStatus &= (l_u16) ~LIN_1_IFC_STS_MASK;

    CyExitCriticalSection(interruptState);

    /* Clear the "stats not checked" flag, since the status is now being checked by the user */
    LIN_1_status &= ((l_u8) ~LIN_1_STATUS_NOT_CHECKED);

    /* Return the status in the temp variable */
    return (returnValue);
}


/*******************************************************************************
* Function Name: l_ifc_read_status
********************************************************************************
*
* Summary:
*  This function is defined by the LIN specification. This returns the status of
*  the specified LIN interface and then clears all status bits for that
*  interface. See Section 7.2.5.8 of the LIN 2.1 specification.
*
*
* Parameters:
*  iii - is the name of the interface handle.
*
* Return:
*  The status bits of the specified LIN interface are returned. These bits have
*  the following meanings:
*    [15:8]    Last Received PID
*    [7]        0
*    [6]        Save Configuration flag
*    [5]        0
*    [4]        Bus Activity flag
*    [3]        Go To Sleep flag
*    [2]        Overrun flag
*    [1]        Successful Transfer flag
*    [0]        Error in Response flag
*
* Reentrant:
*  No
*
*******************************************************************************/
l_u16 l_ifc_read_status(l_ifc_handle iii) 
{
    l_u16 returnValue;

    /* Determine which interface is specified */
    switch(iii)
    {
        /* Go here is interface 0 is specified */
        case LIN_1_IFC_HANDLE:
            returnValue = l_ifc_read_status_LIN_1();
        break;

        default:
            returnValue = (l_u16) CYRET_BAD_PARAM;
        break;
    }

    return (returnValue);
}


/*******************************************************************************
* Function Name: l_sys_irq_disable
********************************************************************************
*
* Summary:
*  This function disables all interrupts for the component. It returns a mask of
*  the state that the interruptmask bits were in. This function is essentially
*  equivalent to the DisableInt API of most components.
*
*  However, the returned value must be saved and later used with the
*  l_sys_irq_restore function to restore the interrupt state properly. It is
*  highly recommended that great care be taken when using this API routine. It
*  is likely that LIN communication failures will occur if the interrupts for
*  this component are disabled for too long.
*
*  This routine is supposed to be provided by the application. However, the LIN
*  Slave component implements this routine automatically. You can modify the
*  code in the routine if necessary.
*
* Parameters:
*  None
*
* Return:
*  Returns an interrupt register mask that defines the digital blocks for which
*  interrupts were disabled.
*
*******************************************************************************/
l_irqmask l_sys_irq_disable(void)
{
    l_irqmask irqMask = 0u;

    if(1u == CyIntGetState(LIN_1_UART_ISR_NUMBER))
    {
        irqMask = 0x01u;
        CyIntDisable(LIN_1_UART_ISR_NUMBER);
    }

    if(1u == CyIntGetState(LIN_1_BLIN_ISR_NUMBER))
    {
        irqMask |= ((l_u8) (0x01u << 1u));
        CyIntDisable(LIN_1_BLIN_ISR_NUMBER);
    }

    return (irqMask);
}


/*******************************************************************************
* Function Name: l_sys_irq_restore
********************************************************************************
*
* Summary:
*  This function restores interrupts for the component. It should be used in
*  conjunction with l_sys_irq_disable. This function is essentially equivalent
*  to the EnableInt API of most components. However, it should not be called
*  when the component is being started.
*
*  This routine is supposed to be provided by the application. However, the LIN
*  Slave component implements this routine automatically. You can modify the
*  code in the routine if necessary.
*
* Parameters:
*  previous - interrupt mask that defines the digital blocks for which
*  interrupts will be enabled.
*
* Return:
*  None
*
*******************************************************************************/
void l_sys_irq_restore(l_irqmask previous)
{
    if(0u != (previous & 0x01u))
    {
        CyIntEnable(LIN_1_UART_ISR_NUMBER);
    }

    if(0u != (previous & ((l_u8) (0x01u << 1u))))
    {
        CyIntEnable(LIN_1_BLIN_ISR_NUMBER);
    }
}


/*******************************************************************************
* Function Name: LIN_1_FindPidIndex
********************************************************************************
*
* Summary:
*  Returns index of the PID in LIN_1_volatileConfig.
*
* Parameters:
*  l_u8 pid - PID of the frame which index required.
*
* Return:
*  Index if the PID in LIN_1_volatileConfig,
*  0xFFu - if PID is not found
*
*******************************************************************************/
static l_u8 LIN_1_FindPidIndex(l_u8 pid)
{
    l_u8 i;
    l_u8 returnValue = LIN_1_INVALID_FRAME_PID;

    for(i = 0u; i < LIN_1_NUM_FRAMES; i++)
    {
        if(pid == LIN_1_volatileConfig[i])
        {
            returnValue = i;

            /* Break the for loop */
            break;
        }
    }

    return (returnValue);
}


/*******************************************************************************
* Function Name: LIN_1_EndFrame
********************************************************************************
*
* Summary:
*  Finishes the frame transmission.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
static void LIN_1_EndFrame(l_u8 status) 
{
    l_u8 interruptState;

    if(status == LIN_1_HANDLING_DONT_SAVE_PID)
    {
        /* Clear data received flag */
        LIN_1_fsmFlags &= ((l_u8) ~LIN_1_FSM_DATA_RECEIVE);

        if(0u != (LIN_1_fsmFlags & LIN_1_FSM_OVERRUN))
        {
            /* Set overrun */
            LIN_1_ifcStatus |= LIN_1_IFC_STS_OVERRUN;
        }
    }
    else if(status ==LIN_1_HANDLING_SKIP_OVERRUN)
    {
        /* Set Overrun flag */
        LIN_1_fsmFlags |= LIN_1_FSM_OVERRUN;

        #if(1u == LIN_1_TL_ENABLED)

            if(0u != (LIN_1_tlFlags & LIN_1_TL_TX_DIRECTION))
            {
                /* Transport Layer Functions: Cooked Transport Layer API */
                #if(1u == LIN_1_TL_API_FORMAT)

                    /* Set TL TX error status */
                    LIN_1_txTlStatus = LD_FAILED;

                #else   /* Transport Layer Functions: Raw Transport Layer API */

                    /* Set TL TX error status */
                    LIN_1_txTlStatus = LD_TRANSMIT_ERROR;

                #endif /* (1u == LIN_1_TL_API_FORMAT) */

                /* Clear TL flags register */
                LIN_1_tlFlags = 0u;
            }

            if(0u != (LIN_1_tlFlags & LIN_1_TL_RX_DIRECTION))
            {
                /* Transport Layer Functions: Cooked Transport Layer API */
                #if(1u == LIN_1_TL_API_FORMAT)

                    /* Set TL RX error status */
                    LIN_1_rxTlStatus = LD_FAILED;

                #else   /* Transport Layer Functions: Raw Transport Layer API */

                    /* Set TL RX error status */
                    LIN_1_rxTlStatus = LD_RECEIVE_ERROR;

                #endif /* (1u == LIN_1_TL_API_FORMAT) */

                /* Clear TL flags register */
                LIN_1_tlFlags = 0u;
            }

        #endif /* (1u == LIN_1_TL_ENABLED) */
    }
    else if(status == LIN_1_HANDLING_RESET_FSM_ERR)
    {
        /* Clear UART enable flag */
        LIN_1_fsmFlags &= ((l_u8) ~LIN_1_FSM_UART_ENABLE_FLAG);

        /* Shutdown and disconnect UART, clear pending interrupts */
        CyIntDisable(LIN_1_UART_ISR_NUMBER);

        /* Disconnect bus RX from UART */
        LIN_1_CONTROL_REG |=  LIN_1_CONTROL_RX_DIS;

        /*******************************************************************
        * Clear UART Rx FIFO.
        * This should be done by calling UART_ClearRxBuffer() function, but
        * its current implementation clears only memory buffer, but not FIFO
        * one.
        *******************************************************************/
        interruptState = CyEnterCriticalSection();

        LIN_1_UART_RX_FIFO_REG |= LIN_1_UART_RX_FIFO_CLEAR;
        LIN_1_UART_RX_FIFO_REG &= ((l_u8) ~LIN_1_UART_RX_FIFO_CLEAR);

        CyExitCriticalSection(interruptState);

        /* Clear any pending UART interrupt */
        CyIntClearPending(LIN_1_UART_ISR_NUMBER);

        /* Update UART ISR FSM state */
         #if(1u == LIN_1_AUTO_BAUD_RATE_SYNC)

            /* Auto Baud Rate Sync Enabled */
            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_1_PID;

        #else

            /* Auto Baud Rate Sync disabled */
            LIN_1_uartFsmState = LIN_1_UART_ISR_STATE_0_SNC;

        #endif  /* (1u == LIN_1_AUTO_BAUD_RATE_SYNC) */
    }
    else
    {
        /* Do nothing */
    }
}


/*******************************************************************************
* Function Name: LIN_1_SetAssociatedFlags
********************************************************************************
*
* Summary:
*  This function sets appropriate flags. Used by ISR.
*
* Parameters:
*  Index of the PID in LIN_1_LinSlaveConfig.
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
static void LIN_1_SetAssociatedFlags(l_u8 pIndex)
{
    switch(pIndex)
    {
        case 0u:
            /* Mark frame "Frame1" (Type:Unconditional,Associated With:None) and associated signals processed. */ 
            LIN_1_statusFlagArray[LIN_1_ISR_AUX_SET_FLAGS_Frame1_FLAG_BYTE_OFFSET_0] |=
                LIN_1_ISR_AUX_SET_FLAGS_Frame1_FLAG_MASK_0;
        break;

        case 1u:
            /* Mark frame "rpm" (Type:Unconditional,Associated With:None) and associated signals processed. */ 
            LIN_1_statusFlagArray[LIN_1_ISR_AUX_SET_FLAGS_rpm_FLAG_BYTE_OFFSET_0] |=
                LIN_1_ISR_AUX_SET_FLAGS_rpm_FLAG_MASK_0;
        break;

        case 2u:
            /* Mark frame "PID" (Type:Unconditional,Associated With:None) and associated signals processed. */ 
            LIN_1_statusFlagArray[LIN_1_ISR_AUX_SET_FLAGS_PID_FLAG_BYTE_OFFSET_0] |=
                LIN_1_ISR_AUX_SET_FLAGS_PID_FLAG_MASK_0;
        break;


        default:
        break;    
    }
}



/* SAE J2602 is disabled */
#if(0u == LIN_1_SAE_J2602)


    /*******************************************************************************
    * Function Name: LIN_1_GetEtFlagValue
    ********************************************************************************
    *
    * Summary:
    *  This function gets value of appropriate event-triggered frame flag.
    *
    * Parameters:
    *  Index of the PID in LIN_1_LinSlaveConfig.
    *
    * Return:
    *  Current flag value.
    *
    *******************************************************************************/
    static l_bool LIN_1_GetEtFlagValue(l_u8 pidIndex)
{
    l_bool returnValue;

    switch(pidIndex)
    {
        default:
             returnValue = LIN_1_FALSE;
        break;    
    }

    return (returnValue);
}



    /*******************************************************************************
    * Function Name: LIN_1_ClearEtFlagValue
    ********************************************************************************
    *
    * Summary:
    *  This function clears value of appropriate event-triggered frame flag.
    *
    * Parameters:
    *  Index of the PID in LIN_1_LinSlaveConfig.
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    static void LIN_1_ClearEtFlagValue(l_u8 pidIndex)
{
    switch(pidIndex)
    {
        default:
        break;    
    }
}



#endif /* (0u == LIN_1_SAE_J2602) */


#if(1u == LIN_1_TL_ENABLED)

    /*******************************************************************************
    * Function Name: ld_init
    ********************************************************************************
    *
    * Summary:
    *  This call will (re)initialize the raw and the cooked layers on the interface
    *  iii.
    *
    *  All transport layer buffers will be initialized.
    *
    *  If there is an ongoing diagnostic frame transporting a cooked or raw message
    *  on the bus, it will not be aborted.
    *
    * Parameters:
    *  iii - Interface.
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    void ld_init(l_ifc_handle iii) 
    {
        l_u8 interruptState;

        /* To remove unreferenced local variable warning */
        iii = iii;

        #if(1u == LIN_1_TL_API_FORMAT)

            while(LIN_1_rxTlStatus == LD_IN_PROGRESS)
            {
                /* Wait until current message will be processed */
            }

            while(LIN_1_txTlStatus == LD_IN_PROGRESS)
            {
                /* Wait until current message will be processed */
            }

            /* Save interrupt state and disable interrupts */
            interruptState = CyEnterCriticalSection();

            /* Initialize Tx and Rx status variables correctly */
            LIN_1_txTlStatus = LD_COMPLETED;
            LIN_1_rxTlStatus = LD_COMPLETED;

            LIN_1_tlLengthPointer = NULL;

            /* Reset the frame counters */
            LIN_1_rxMessageLength = 0u;
            LIN_1_txMessageLength = 0u;

            /* Reset the frame counters */
            LIN_1_txFrameCounter = 0u;
            LIN_1_rxFrameCounter = 0u;

            LIN_1_rxTlDataPointer = NULL;
            LIN_1_rxTlInitDataPointer = NULL;
            
            /* Previous PCI requires to be unknown after the initialization */
            LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_UNKNOWN;

        #else

            /* Save interrupt state and disable interupts */
            interruptState = CyEnterCriticalSection();

            /* Reset buffers depth to 0, it will indicate the buffers are empty */
            LIN_1_txBufDepth = 0u;
            LIN_1_rxBufDepth = 0u;

            /* Raw API buffers initialization */

            LIN_1_txWrIndex = 0u;
            LIN_1_txRdIndex = 0u;

            LIN_1_rxWrIndex = 0u;
            LIN_1_rxRdIndex = 0u;

            LIN_1_txTlStatus = LD_QUEUE_EMPTY;
            LIN_1_rxTlStatus = LD_NO_DATA;

        #endif /* (1u == LIN_1_TL_API_FORMAT) */

        /* Set initial NAD as a current active NAD before initializing TL */
        LIN_1_nad = LIN_1_LinSlaveConfig.initialNad;

        LIN_1_tlFlags = 0u;

        /* Enable interrupts */
        CyExitCriticalSection(interruptState);
    }


    /*******************************************************************************
    * Function Name: ld_read_configuration
    ********************************************************************************
    *
    * Summary:
    *  This function is used to read the NAD and PID values from volatile memory.
    *  This function can be used to read the current configuration data, and then
    *  save this data into non-volatile (flash) memory. The application should save
    *  the configuration data to flash when the "Save Configuration" bit is set in
    *  the LIN status register (returned by l_ifc_read_status_LIN_1).
    *  The configuration data that is read is a series of bytes. The first byte is
    *  the current NAD of the slave. The next bytes are the current PID values for
    *  the frames that the slave responds to. The PID values are in the order in
    *  which the frames appear in the LDF or NCF file.
    *
    * Parameters:
    *  iii - Interface.
    *
    * Return:
    *  LD_READ_OK - If the service was successful.
    *
    *  LD_LENGTH_TOO_SHORT - If the configuration size is greater than the length.
    *                        It means that the data area does not contain a valid
    *                        configuration.
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    l_u8 ld_read_configuration(l_ifc_handle iii, l_u8* const pData, l_u8* const length) 
                                                
    {
        l_u8 i;
        l_u8 result = LIN_1_LD_READ_OK;

        if(*length < (LIN_1_NUM_FRAMES + 1u))
        {
            /* Return with no action when the requested length is smaller
            *  than the configuration data length.
            */
            result = LIN_1_LD_LENGTH_TOO_SHORT;
        }
        else
        {
            /* Copy over the configured NAD */
            pData[0u] = LIN_1_nad;

            /* Copy the data from the PID array to the data array */
            for (i = 0u; i < LIN_1_NUM_FRAMES; i++)
            {
                pData[i + 1u] = LIN_1_volatileConfig[i];
            }

            /* Set the length parameter to the actual length of the configuration data */
            *length = LIN_1_NUM_FRAMES + 1u;
        }

        /* To remove unreferenced local variable warning */
        iii = iii;

        /* Return status */
        return (result);
    }


    /*******************************************************************************
    * Function Name: ld_set_configuration
    ********************************************************************************
    *
    * Summary:
    *  This call will not transport anything on the bus.
    *
    *  The function will configure the NAD and the PIDs according to the
    *  configuration given by data. The intended usage is to restore a saved
    *  configuration or set an initial configuration (e.g. coded by I/O pins).
    *
    *  The function shall be called after calling l_ifc_init.
    *
    *  The caller shall set the size of the data area before calling the function.
    *
    *  The data contains the NAD and the PIDs and occupies one byte each.
    *  The structure of the data is: NAD and then all PIDs for the frames.
    *  The order of the PIDs are the same as the frame list in the LDF,
    *  Section 9.2.2.2, and NCF, Section 8.2.5.
    *
    * Parameters:
    *  iii - Interface.
    *
    * Return:
    *  LD_SET_OK - If the service was successful.
    *
    *  LD_LENGTH_NOT_CORRECT - If the required size of the configuration is not
    *                          equal to the given length.
    *
    *  LD_DATA_ERROR - The set of configuration could not be set. A an error
    *                  occurred while setting the configuration and the read back
    *                  configuration settings doesn't match required settings.
    *
    *  LIN_1_volatileConfig - new frame PIDs are copied.
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    l_u8 ld_set_configuration(l_ifc_handle iii, const l_u8* const pData, l_u16 length) 
                                                     
    {
        l_u8 i;
        l_u8 result = LIN_1_LD_SET_OK;

        if(length != (LIN_1_NUM_FRAMES + 1u))
        {
            /* Return error if the length isn't correct */
            result = LIN_1_LD_LENGTH_NOT_CORRECT;
        }
        else
        {
            /* Copy NAD to a volatile memory */
            LIN_1_nad = pData[0u];

            /* Data read back */
            if(LIN_1_nad != pData[0u])
            {
                /* Indicate data error if NAD is not set correctly */
                result = LIN_1_LD_DATA_ERROR;
            }

            /* Copy Frame PIDs to a volatile memory */
            for(i = 0u; i < LIN_1_NUM_FRAMES; i++)
            {
                LIN_1_volatileConfig[i] = pData[i + 1u];

                /* Data read back */
                if(LIN_1_volatileConfig[i] != pData[i + 1u])
                {
                    /* Indicate data error if NAD is not set correctly */
                    result = LIN_1_LD_DATA_ERROR;
                }
            }
        }

        /* To remove unreferenced local variable warning */
        iii = iii;

        /* Return success code if the copy has completed */
        return(result);
    }

    #if(1u == LIN_1_CS_ENABLED)

        #if((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB0_SEL)) || \
            (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)))
            
            /*******************************************************************************
            * Function Name: LIN_1_LinProductId
            ********************************************************************************
            *
            * Summary:
            *  Verify that received LIN product identification matches.
            *
            * Parameters:
            *  frameData - pointer to a 4 bytes that holds LIN product ID.
            *
            * Return:
            *  0 - in case when LIN product IDs don't match.
            *  1 - in case when LIN product IDs do match.
            *
            * Reentrant:
            *  No
            *
            *******************************************************************************/
            static l_bool LIN_1_LinProductId(volatile const l_u8 frameData[])
            {
                l_u8 i = 1u;

                if((frameData[0u] != LO8(LIN_1_slaveId.supplierId)) &&
                    (frameData[0u] != LO8(LIN_1_CS_SUPPLIER_ID_WILDCARD)))
                {
                    i = 0u;        /* Zero out 'i' if the data isn't for this slave */
                }

                if((frameData[1u] != HI8(LIN_1_slaveId.supplierId)) &&
                    (frameData[1u] != HI8(LIN_1_CS_SUPPLIER_ID_WILDCARD)))
                {
                    i = 0u;        /* Zero out 'i' if the data isn't for this slave */
                }

                if((frameData[2u] != LO8(LIN_1_slaveId.functionId)) &&
                    (frameData[2u] != LO8(LIN_1_CS_FUNCTION_ID_WILDCARD)))
                {
                    i = 0u;        /* Zero out 'i' if the data isn't for this slave */
                }

                if((frameData[3u] != HI8(LIN_1_slaveId.functionId)) &&
                    (frameData[3u] != HI8(LIN_1_CS_FUNCTION_ID_WILDCARD)))
                {
                    i = 0u;        /* Zero out 'i' if the data isn't for this slave */
                }

                return(i);
            }
            
        #endif /*((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB0_SEL)) ||
               * (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)))
               */
                
        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB1_SEL))

            #if(1u == LIN_1_LIN_2_0)

                /*******************************************************************************
                * Function Name: LIN_1_MessageId
                ********************************************************************************
                *
                * Summary:
                *  Search for message ID in the LIN message ID table in case of success returns
                *  message ID index in the table.
                *
                * Parameters:
                *  frameData - The data pointer points to a data area with 2 bytes.
                *
                * Return:
                *  message ID index - in case of successful operation;
                *  LD_INVALID_MESSAGE_INDEX - in case when message ID wasn't found.
                *
                * Reentrant:
                *  No
                *
                *******************************************************************************/
                static l_u8 LIN_1_MessageId(volatile const l_u8* frameData)
                {
                    l_u8 i = 0u;
                    l_u8 result = LD_INVALID_MESSAGE_INDEX;

                    while((i < LIN_1_NUM_FRAMES) && (result == LD_INVALID_MESSAGE_INDEX))
                    {
                        /* If LSB of the message ID from the table is equal to received one then
                        * Compare the MSB and in case of success set result to message index.
                        */
                        if(frameData[0u] == LO8(messageIdTable[i]))
                        {
                            if(frameData[1u] == HI8(messageIdTable[i]))
                            {
                                result = i;
                            }
                        }

                        i++;
                    }

                    return(result);
                }

            #endif /* (1u == LIN_1_LIN_2_0) */

        #endif /*(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB1_SEL)) */

        #if((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL)) || \
            (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL)))

            /*******************************************************************************
            * Function Name: ld_read_by_id_callout
            ********************************************************************************
            *
            * Summary:
            *  This callout is used when the master node transmits a read by identifier
            *  request with an identifier in the user defined area. The slave node
            *  application will be called from the driver when such request is received.
            *
            * Parameters:
            *  iii - Interface.
            *  id - The id parameter is the identifier in the user defined area (32 to 63),
            *  from the read by identifier configuration request.
            *  frameData - The data pointer points to a data area with 5 bytes. This area
            *  will be used by the application to set up the positive response.
            *
            * Return:
            *  LD_NEGATIVE_RESPONSE - The slave node will respond with a negative response.
            *  In this case the data area is not considered
            *
            *  LD_POSTIVE_RESPONSE - The slave node will setup a positive response using
            *  the data provided by the application.
            *
            *  LD_NO_RESPONSE - The slave node will not answer.
            *
            * Reentrant:
            *  No
            *
            *******************************************************************************/
            l_u8 ld_read_by_id_callout(l_ifc_handle iii, l_u8 id, l_u8* frameData)
                                                        
            {
                l_u8 result = LD_NEGATIVE_RESPONSE;

                /* User code required to handle user defined identification (Optional) */
                /* `#START LD_READ_BY_ID_CALLOUT_USER_SECTION` */

                /* `#END` */

                /* To remove unreferenced local variable warning */
                iii = iii;
                id = id;
                frameData = frameData;

                return(result);
            }

        #endif /* ((0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL))
               * ||
               * (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL)))
               */

    #endif /* (1u == LIN_1_CS_ENABLED) */


    /*******************************************************************************
    * Function Name: LIN_1_ProcessMrf
    ********************************************************************************
    *
    * Summary:
    *  This API is called from the ISR. It is responsible to parse the RX frames
    *  that come from a LIN Master. This API handles Automatic Configuration
    *  requests and receiving both Raw and Cooked API frames.
    *
    * Parameters:
    *  frame - Pointer to an array that contains a last received frame.
    *
    * Return:
    *  None
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    static void LIN_1_ProcessMrf(l_u8 frame[])
    {
        l_u8 i;
        l_u8 tempStatus = 0u;

        #if(1u == LIN_1_TL_API_FORMAT) /* Cooked API */
            l_u8 tmpPci;
            l_u16 tmpWord;
            l_u8 tmpByte;
        #endif /* 1u == LIN_1_TL_API_FORMAT */

        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL))

            l_u8 idByte;
            l_u8 idMask;
            l_u8 idInvert;

        #endif /* (0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL)) */


        if(LIN_1_NAD_GO_TO_SLEEP == frame[LIN_1_PDU_NAD_IDX])
        {
            LIN_1_ifcStatus |= LIN_1_IFC_STS_GO_TO_SLEEP;
        }
        else if((LIN_1_nad == frame[LIN_1_PDU_NAD_IDX]) ||
                    (LIN_1_NAD_BROADCAST == frame[LIN_1_PDU_NAD_IDX]) ||
                        (LIN_1_NAD_FUNCTIONAL == frame[LIN_1_PDU_NAD_IDX]))
        {
            /* Single Frame detected */
            if(0u == (frame[LIN_1_PDU_PCI_IDX] & LIN_1_PDU_PCI_TYPE_MASK))
            {
                /* SID used for node configuration */
                switch(frame[LIN_1_PDU_SID_IDX])
                {
                    #if(1u == LIN_1_CS_ENABLED)

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB0_SEL))

                            case LIN_1_NCS_ASSIGN_NAD:

                                if(1u == LIN_1_LinProductId(&frame[3u]))  /* Check LIN Product ID */
                                {
                                    /* Save received NAD */
                                    LIN_1_nad = frame[LIN_1_PDU_D5_NEW_NAD_IDX];

                                    /* Fill the SRF Buffer with response for a service */
                                    /* Nad field should contain initial NAD */
                                    LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                        LIN_1_LinSlaveConfig.initialNad;

                                    /* PCI is 0 so only length required */
                                    LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                    /* The RSID for a positive response is always SID + 0x40 */
                                    LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                        LIN_1_NCS_POS_RESP_ASSIGN_NAD;

                                    /* Fill unused data bytes with 0xFFs */
                                    for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                    {
                                        LIN_1_srfBuffer[i] = 0xFFu;
                                    }

                                    /* Set the service response bit that indicates that the response is
                                    * ready to be sent to master node.
                                    */
                                    LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                }
                                else
                                {
                                    /* No response as the Supplier ID and the Function ID is invalid */
                                }

                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB0_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB1_SEL))

                            case LIN_1_NCS_ASSIGN_FRAME_ID:  /* Obsolete, not implemented for LIN lin 2.1 */
                                                                        /* LIN 2.0 specification Only */
                                #if(1u == LIN_1_LIN_2_0)

                                    /* tempStatus is used in Supplier ID verification */
                                    tempStatus = 1u;

                                    if((frame[LIN_1_PDU_D1_IDX] !=
                                        LO8(LIN_1_slaveId.supplierId)) &&
                                            (frame[LIN_1_PDU_D1_IDX] !=
                                                LO8(LIN_1_CS_SUPPLIER_ID_WILDCARD)))
                                    {
                                        tempStatus = 0u;  /* Zero out tempStatus if the data isn't for this slave */
                                    }

                                    if((frame[LIN_1_PDU_D2_IDX] !=
                                        HI8(LIN_1_slaveId.supplierId)) &&
                                            (frame[LIN_1_PDU_D2_IDX] !=
                                                HI8(LIN_1_CS_SUPPLIER_ID_WILDCARD)))
                                    {
                                        tempStatus = 0u;   /* Zero out tempStatus if the data isn't for this slave */
                                    }

                                    /* If tempStatus is not 0 then verification passed successfully */
                                    if(tempStatus != 0u)
                                    {
                                        /* Now tempStatus is used to hold Message ID Index */
                                        tempStatus = LIN_1_MessageId(frame + 5u);

                                        /* If Message ID index is valid then process the request and
                                        * prepare positive answer
                                        */
                                        if(tempStatus != LD_INVALID_MESSAGE_INDEX)
                                        {
                                            /* Set the PID in the position where the valid index points */
                                            LIN_1_volatileConfig[tempStatus] =
                                                frame[LIN_1_PDU_D5_IDX];

                                            /* Fill the SRF Buffer with response for a service */
                                            /* Nad field should contain current NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* PCI is 0 so only length required */
                                            LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                            /* The RSID for a positive response is always SID + 0x40 */
                                            LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                LIN_1_NCS_POS_RESP_ASSIGN_FRAME_ID;

                                            /* Fill unused data bytes with 0xFFs */
                                            for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                            {
                                                LIN_1_srfBuffer[i] = 0xFFu;
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }
                                    }

                                #else

                                    /* Do nothing ignore obsolete request */

                                #endif /* (1u == LIN_1_LIN_2_0) */
                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB1_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL))

                            case LIN_1_NCS_READ_BY_ID:

                                /* tempStatus is used to hold a status of LIN_1_LinProductId() */
                                tempStatus = LIN_1_LinProductId(&frame[4u]);

                                /* LIN Product Identification (the only identifier is supported) */
                                if(LIN_1_NCS_READ_BY_ID_ID == frame[LIN_1_PDU_D1_IDX])
                                {
                                    if(1u == tempStatus)
                                    {
                                        /* Fill the SRF Buffer with response for a service */
                                        /* Nad field should contain current NAD */
                                        LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                            LIN_1_nad;

                                        /* PCI is 0 so only length required */
                                        LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 6u;

                                        /* The RSID for a positive response is always SID + 0x40 */
                                        LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                            LIN_1_NCS_POS_RESP_READ_BY_ID;

                                        /* Fill data fields with Supplier and function IDs */
                                        LIN_1_srfBuffer[LIN_1_PDU_D1_IDX] =
                                           LO8(LIN_1_slaveId.supplierId);
                                        LIN_1_srfBuffer[LIN_1_PDU_D2_IDX] =
                                            HI8(LIN_1_slaveId.supplierId);
                                        LIN_1_srfBuffer[LIN_1_PDU_D3_IDX] =
                                            LO8(LIN_1_slaveId.functionId);
                                        LIN_1_srfBuffer[LIN_1_PDU_D4_IDX] =
                                            HI8(LIN_1_slaveId.functionId);
                                        LIN_1_srfBuffer[LIN_1_PDU_D5_IDX] =
                                            LIN_1_slaveId.variant;

                                        /* Set the service response bit that indicates that the response is
                                        * ready to be sent to master node.
                                        */
                                        LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                    }
                                    else
                                    {
                                        /* No action */
                                    }
                                }
                                else if(LIN_1_NCS_READ_BY_ID_SERIAL == frame[LIN_1_PDU_D1_IDX])
                                {
                                    /* Serial number identification*/

                                    /* If the slave serial number matches received one
                                    * then prepare positive response.
                                    */
                                    if(1u == tempStatus)
                                    {
                                        /* Fill the SRF Buffer with response for a service */
                                        /* Nad field should contain current NAD */
                                        LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                            LIN_1_nad;

                                        /* PCI is 0 so only length required */
                                        LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 5u;

                                        /* The RSID for a positive response is always SID + 0x40 */
                                        LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                            LIN_1_NCS_POS_RESP_READ_BY_ID;

                                        /* Fill unused data bytes with serial number ID */
                                        for(i = 3u; i < (LIN_1_FRAME_LEN - 1u); i++)
                                        {
                                            LIN_1_srfBuffer[i] = LIN_1_serialNumber[i - 3u];
                                        }

                                        /* The serial number is 4 byte length so set to 0xFF last unusued byte */
                                        LIN_1_srfBuffer[LIN_1_PDU_D5_IDX] = 0xFFu;

                                        /* Set the service response bit that indicates that the response is
                                        * ready to be sent to master node.
                                        */
                                        LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                    }
                                    else
                                    {
                                         /* Do nothing serial number is invalid */
                                    }

                                }
                                else if((frame[LIN_1_PDU_D1_IDX] >= 32u) &&
                                (frame[LIN_1_PDU_D1_IDX] <= 63u))     /* User defined identification */
                                {

                                    if(1u == tempStatus)
                                    {
                                        /* Copy five bytes of data from volatile location into non-volatile memory
                                        *  location. This is done to suppress MISRA violation of rule 11.5.
                                        */
 /*                                       for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_5; i++)
                                        {
                                            tmpFrameBuff[i] = frame[i + LIN_1_FRAME_DATA_SIZE_3];
                                        }*/

                                        /* If user didn't reassign the status of ld_read_by_id_callout() then
                                        * LD_NEGATIVE_RESPONSE will always be returned by ld_read_by_id_callout().
                                        * This will indicate to master that the service by user defined identification
                                        * is not supported. tempStatus will be used to hold status of
                                        * ld_read_by_id_callout().
                                        */
                                        tempStatus = ld_read_by_id_callout(LIN_1_IFC_HANDLE,
                                            frame[LIN_1_PDU_D1_IDX],
                                                frame + LIN_1_FRAME_DATA_SIZE_3);

                                        if(tempStatus == LD_NEGATIVE_RESPONSE)
                                        {
                                            /* Fill the SRF Buffer with negative response for the service */
                                            /* Nad field should contain current NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* PCI is 0 so only length required */
                                            LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 3u;

                                            /* The RSID for a positive response is always SID + 0x40 */
                                            LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                LIN_1_NCS_RSID_NEG_REPLY;

                                            /* D1 will hold the service ID */
                                            LIN_1_srfBuffer[LIN_1_PDU_D1_ID_IDX] =
                                                LIN_1_NCS_READ_BY_ID;

                                            /* D2 contains error code */
                                            LIN_1_srfBuffer[LIN_1_PDU_D2_IDX] = 0x12u;

                                            /* Fill unused data bytes with 0xFFs */
                                            for(i = 5u; i < LIN_1_FRAME_LEN; i++)
                                            {
                                                LIN_1_srfBuffer[i] = 0xFFu;
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }
                                        else if(tempStatus == LD_POSITIVE_RESPONSE)
                                        {
                                            /* Fill the SRF Buffer with response for a service */
                                            /* Nad field should contain current NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* PCI is 0 so only length required */
                                            LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 6u;

                                            /* The RSID for a positive response is always SID + 0x40 */
                                            LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                LIN_1_NCS_POS_RESP_READ_BY_ID;

                                            /* Fill unused data bytes with user defined information */
                                            for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                            {
                                                LIN_1_srfBuffer[i] = frame[i];
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }
                                        else
                                        {
                                            /* Do nothing as there no response from user */
                                        }
                                    }
                                    else
                                    {
                                         /* Do nothing serial number is invalid */
                                    }
                                }
                                else if((frame[LIN_1_PDU_D1_IDX] >= 16u) &&
                                    (frame[LIN_1_PDU_D1_IDX] <= 31u))     /* Message ID identification*/
                                {
                                    /* LIN 2.0 specification Only */
                                    #if(1u == LIN_1_LIN_2_0)

                                        /* If the slave serial number matchs received one
                                        * then prepare positive response.
                                        */
                                        if(1u == tempStatus)
                                        {
                                            /* Fill the SRF Buffer with response for a service */
                                            /* Nad field should contain current NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* tempStatus now used to store calculated Message ID index */
                                            tempStatus = frame[LIN_1_PDU_D1_IDX] - LD_MESSAGE_ID_BASE;

                                            if(LIN_1_NUM_FRAMES > tempStatus)
                                            {
                                                /* PCI is 0 so only length required */
                                                LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 4u;

                                                /* The RSID for a positive response is always SID + 0x40 */
                                                LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                    LIN_1_NCS_POS_RESP_READ_BY_ID;

                                                /* D1 = Message ID LSB */
                                                LIN_1_srfBuffer[LIN_1_PDU_D1_IDX] =
                                                    HI8(messageIdTable[tempStatus]);

                                                /* D2 = Message ID MSB */
                                                LIN_1_srfBuffer[LIN_1_PDU_D2_IDX] =
                                                    LO8(messageIdTable[tempStatus]);

                                                /* D3 = PID */
                                                LIN_1_srfBuffer[LIN_1_PDU_D3_IDX] =
                                                    LIN_1_volatileConfig[tempStatus];

                                                /* The Message ID response is 3 byte length so set last two bytes
                                                * to 0xFF.
                                                */
                                                LIN_1_srfBuffer[LIN_1_PDU_D4_IDX] = 0xFFu;
                                                LIN_1_srfBuffer[LIN_1_PDU_D5_IDX] = 0xFFu;
                                            }
                                            else
                                            {
                                                /* PCI is 0 so only length required */
                                                LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 3u;

                                                /* The RSID for a positive response is always SID + 0x40 */
                                                LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                    LIN_1_NCS_RSID_NEG_REPLY;

                                                /* D1 will hold the service ID */
                                                LIN_1_srfBuffer[LIN_1_PDU_D1_ID_IDX] =
                                                    LIN_1_NCS_READ_BY_ID;

                                                /* D2 contains error code */
                                                LIN_1_srfBuffer[LIN_1_PDU_D2_IDX] = 0x12u;

                                                /* Fill unused data bytes with 0xFFs */
                                                for(i = 5u; i < LIN_1_FRAME_LEN; i++)
                                                {
                                                    LIN_1_srfBuffer[i] = 0xFFu;
                                                }
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }

                                    #else

                                        /* Do nothing ignore errorneous request */

                                    #endif /* (1u == LIN_1_LIN_2_0) */
                                }
                                else
                                {
                                    /* Do nothing */
                                }

                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB2_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL))

                            case LIN_1_NCS_COND_CHANGE_NAD:

                                if(LIN_1_NCS_READ_BY_ID_ID == frame[LIN_1_PDU_D1_ID_IDX])
                                {
                                    /* LIN Product Identification */
                                    if((6u > frame[LIN_1_PDU_D2_BYTE_IDX]) &&
                                        (0u != frame[LIN_1_PDU_D2_BYTE_IDX]))
                                    {
                                        switch (frame[LIN_1_PDU_D2_BYTE_IDX])
                                        {
                                            /* tempStatus will be used as a teporary variable to store ID byte */

                                            case LIN_1_CS_BYTE_SUPPLIER_ID1:

                                                tempStatus = LO8(LIN_1_slaveId.supplierId);

                                            break;

                                            case LIN_1_CS_BYTE_SUPPLIER_ID2:

                                                tempStatus = HI8(LIN_1_slaveId.supplierId);

                                            break;

                                            case LIN_1_CS_BYTE_FUNCTION_ID1:

                                                tempStatus = LO8(LIN_1_slaveId.functionId);

                                            break;

                                            case LIN_1_CS_BYTE_FUNCTION_ID2:

                                                tempStatus = HI8(LIN_1_slaveId.functionId);

                                            break;

                                            case LIN_1_CS_BYTE_VARIANT:

                                                tempStatus = LIN_1_slaveId.variant;

                                            break;

                                            default:

                                                /* This state should never be used */

                                            break;
                                        }

                                        if(0u == ((tempStatus ^ frame[LIN_1_PDU_D4_INVERT_IDX]) &
                                            frame[LIN_1_PDU_D3_MASK_IDX]))
                                        {
                                            /* Change NAD to new NAD */
                                            LIN_1_nad = frame[LIN_1_PDU_D5_NEW_NAD_IDX];

                                            /* Fill the SRF Buffer with response for a service */
                                            /* Nad field should contain current NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* PCI is 0 so only length required */
                                            LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                            /* The RSID for a positive response is always SID + 0x40 */
                                            LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                LIN_1_NCS_POS_RESP_COND_CHANGE_NAD;

                                            /* Fill unused bytes with 0xFF */
                                            for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                            {
                                                LIN_1_srfBuffer[i] = 0xFFu;
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }
                                        else
                                        {
                                            /* Do nothing ignore errorneous request */
                                        }
                                    }
                                    else
                                    {
                                        /* Do nothing ignore errorneous request */
                                    }
                                }
                                else if(LIN_1_NCS_READ_BY_ID_SERIAL == frame[LIN_1_PDU_D1_ID_IDX])
                                {
                                    if((5u < frame[LIN_1_PDU_D2_BYTE_IDX]) &&
                                        (0u != frame[LIN_1_PDU_D2_BYTE_IDX]))
                                    {
                                        /* Byte = 1 corresponds to the first byte (LIN_1_serialNumber[0]) */
                                        if(0u == ((
                                        LIN_1_serialNumber[frame[LIN_1_PDU_D2_BYTE_IDX] - 1u] ^
                                            frame[LIN_1_PDU_D4_INVERT_IDX]) &
                                                frame[LIN_1_PDU_D3_MASK_IDX]))
                                        {
                                            /* Change NAD to new NAD */
                                            LIN_1_nad = frame[LIN_1_PDU_D5_NEW_NAD_IDX];

                                            /* Fill the SRF Buffer with response for a service */
                                            /* Nad field should contain current NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* PCI is 0 so only length required */
                                            LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                            /* The RSID for a positive response is always SID + 0x40 */
                                            LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                LIN_1_NCS_POS_RESP_COND_CHANGE_NAD;

                                            /* Fill unused bytes with 0xFF */
                                            for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                            {
                                                LIN_1_srfBuffer[i] = 0xFFu;
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }
                                        else
                                        {
                                            /* Do nothing ignore errorneous request */
                                        }
                                    }
                                    else
                                    {
                                        /* Do nothing ignore errorneous request */
                                    }
                                }
                                else if((frame[LIN_1_PDU_D1_IDX] >= 32u) &&
                                    (frame[LIN_1_PDU_D1_IDX] <= 63u))     /* User defined identification*/
                                {
                                    /* Need to store Byte, Invert and Mask in a variables for user defined
                                    * identification as frame[] should contain user data after execution of
                                    * ld_read_by_id_callout();
                                    */
                                    idByte = frame[LIN_1_PDU_D2_BYTE_IDX] - 1u;
                                    idInvert = frame[LIN_1_PDU_D4_INVERT_IDX];
                                    idMask = frame[LIN_1_PDU_D3_MASK_IDX];

                                    /* If user didn't reassign the status of ld_read_by_id_callout() then
                                    * LD_NEGATIVE_RESPONSE will alwas be returned by ld_read_by_id_callout(). This will
                                    * indicate to master that the service by user defined identification is not
                                    * supported. tempStatus will be used to hold status of ld_read_by_id_callout().
                                    */
                                    tempStatus = ld_read_by_id_callout(LIN_1_IFC_HANDLE,
                                        frame[LIN_1_PDU_D1_IDX], frame + LIN_1_PDU_D1_IDX);

                                    if((tempStatus == LD_NEGATIVE_RESPONSE) || (tempStatus == LD_NO_RESPONSE))
                                    {
                                        /* Do nothing as there no response from user */
                                    }
                                    else
                                    {
                                        if(0u == ((frame[idByte + LIN_1_PDU_D1_IDX] ^ idInvert) & idMask))
                                        {
                                            /* Change NAD to new NAD */
                                            LIN_1_nad = frame[LIN_1_PDU_D5_NEW_NAD_IDX];

                                            /* Fill the SRF Buffer with response for a service */
                                            /* Nad field should contain changed NAD */
                                            LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                                LIN_1_nad;

                                            /* PCI is 0 so only length required */
                                            LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                            /* The RSID for a positive response is always SID + 0x40 */
                                            LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                                LIN_1_NCS_POS_RESP_COND_CHANGE_NAD;

                                            /* Fill unused bytes with 0xFF */
                                            for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                            {
                                                LIN_1_srfBuffer[i] = 0xFFu;
                                            }

                                            /* Set the service response bit that indicates that the response is
                                            * ready to be sent to master node.
                                            */
                                            LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                        }
                                        else
                                        {
                                            /* Do nothing ignore errorneous request */
                                        }
                                    }
                                }
                                else
                                {
                                    /* Do nothing ignore errorneous request */
                                }
                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB3_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB4_SEL))

                            case LIN_1_NCS_DATA_DUMP:

                                /* Not Supported */

                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB4_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB5_SEL))

                            /* LIN Slave Node Position Detection */
                            case LIN_1_NCS_ASSIGN_NAD_SNPD:

                                #if(1u == LIN_1_SAE_J2602)

                                    LIN_1_ioctlStatus = LIN_1_IOCTL_STS_TARGET_RESET;

                                #endif /* (1u == LIN_1_SAE_J2602) */

                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB5_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB6_SEL))

                            case LIN_1_NCS_SAVE_CONFIG:

                                /* Set save configuration bit in the status register */
                                LIN_1_ifcStatus |= LIN_1_IFC_STS_SAVE_CONFIG;

                                /* Fill the SRF Buffer with response for a service */
                                /* Nad field should contain current NAD */
                                LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                    LIN_1_nad;

                                /* PCI is 0 so only length required */
                                LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                /* The RSID for a positive response is always SID + 0x40 */
                                LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                    LIN_1_NCS_POS_RESP_SAVE_CONFIG;

                                /* Fill unused data bytes with 0xFFs */
                                for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                {
                                    LIN_1_srfBuffer[i] = 0xFFu;
                                }

                                /* Set the service response bit that indicates that the response is
                                * ready to be sent to master node.
                                */
                                LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;

                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB6_SEL) */

                        #if(0u != (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB7_SEL))

                            case LIN_1_NCS_ASSIGN_FRAME_ID_RANGE:

                                /* Zero out the temp satus. It will be used as error counter. */
                                tempStatus = 0u;

                                for(i = 0u; i < LIN_1_NCS_MAX_FRAME_ID_RANGE; i++)
                                {
                                    if((i + frame[LIN_1_PDU_D1_START_IDX]) < LIN_1_NUM_FRAMES)
                                    {
                                        if((frame[i + LIN_1_PDU_D2_PID_IDX] !=
                                            LIN_1_INVALID_FRAME_PID) &&
                                                ((frame[i + LIN_1_PDU_D2_PID_IDX] &
                                                    LIN_1_PID_PARITY_MASK) < LIN_1_FRAME_PID_MRF))
                                        {
                                            /* The unassigned value "0" is used to invalidate
                                            *  this frame for transportation on the bus.
                                            */
                                            /* Set the new received PID value */
                                            LIN_1_volatileConfig[i +
                                                frame[LIN_1_PDU_D1_START_IDX]] =
                                                    frame[i + LIN_1_PDU_D2_PID_IDX];
                                        }
                                        else if(frame[i + LIN_1_PDU_D2_PID_IDX] == 0xFFu)
                                        {
                                            /* Do nothing. */
                                        }
                                        else
                                        {
                                            /* Indicate an error by changing the status other then 0
                                            * if the Frame ID is reserved.
                                            */
                                            tempStatus++;
                                        }
                                    }
                                    else
                                    {
                                        if(frame[i + LIN_1_PDU_D2_PID_IDX] != 0xFFu)
                                        {
                                            tempStatus++;  /* Indicate an error by changing the status other then 0 */
                                        }
                                    }
                                }

                                if(tempStatus == 0u) /* No errors condition check */
                                {
                                    /* Fill the SRF Buffer with response for a service */
                                    /* Nad field should contain current NAD */
                                    LIN_1_srfBuffer[LIN_1_PDU_NAD_IDX] =
                                        LIN_1_nad;

                                    /* PCI is 0 so only length required */
                                    LIN_1_srfBuffer[LIN_1_PDU_PCI_IDX] = 1u;

                                    /* The RSID for a positive response is always SID + 0x40 */
                                    LIN_1_srfBuffer[LIN_1_PDU_SID_IDX] =
                                        LIN_1_NCS_POS_RESP_ASSIGN_FRAME_ID_RANGE;

                                    /* Fill unused data bytes with 0xFFs */
                                    for(i = 3u; i < LIN_1_FRAME_LEN; i++)
                                    {
                                        LIN_1_srfBuffer[i] = 0xFFu;
                                    }

                                    /* Set the service response bit that indicates that the response is
                                    * ready to be sent to master node.
                                    */
                                    LIN_1_status |= LIN_1_STATUS_SRVC_RSP_RDY;
                                }
                                else
                                {
                                    /* Do nothing ignore errorneous request */
                                }

                            break;

                        #endif /* (LIN_1_CS_SEL_SERVICES01 & LIN_1_NCS_0xB7_SEL) */

                    #endif /* (1u == LIN_1_CS_ENABLED) */

                    default:

                       /* This will indicate that requested service is disabled and the Frame
                       * will be "passed" to TL. This means user should process this Frame
                       * properly using TL API.
                       */
                       LIN_1_tlFlags |= LIN_1_TL_CS_SERVICE_DISABLED;

                       /* Indicates that detected SID is a diagnostic SID and it should be
                       * passed to Transport Layer.
                       */
                      LIN_1_tlFlags |= LIN_1_TL_DIAG_FRAME_DETECTED;

                    break;
                }

                if((0u != (LIN_1_tlFlags & LIN_1_TL_CS_SERVICE_DISABLED)) ||
                    (0u != (LIN_1_tlFlags & LIN_1_TL_DIAG_FRAME_DETECTED)))
                {
                    /* SID used for diagnostics */
                    if(frame[LIN_1_PDU_PCI_IDX] <= LIN_1_PDU_SF_DATA_LEN)
                    {
                        #if(1u == LIN_1_TL_API_FORMAT) /* Cooked API */

                            /* Get one frame of a message if there is a message pending */
                            if(LIN_1_rxTlStatus == LD_IN_PROGRESS)
                            {
                                /* Make sure the pointer is points to the beginning of
                                * receive buffer.
                                */
                                if(0u != (LIN_1_tlFlags & LIN_1_TL_CS_SERVICE_DISABLED))
                                {
                                    LIN_1_rxTlDataPointer = LIN_1_rxTlInitDataPointer;
                                }
                                else
                                {
                                    /* Do nothing */
                                }

                                /* Copy data to user buffer */
                                for(i = 0u; i < frame[LIN_1_PDU_PCI_IDX]; i++)
                                {
                                    *LIN_1_rxTlDataPointer = frame[i + 2u];
                                    LIN_1_rxTlDataPointer++;
                                }

                                /* Store the NAD */
                                *LIN_1_tlNadPointer = frame[LIN_1_PDU_NAD_IDX];

                                /* Get length of the data bytes */
                                *LIN_1_tlLengthPointer = (l_u16) frame[LIN_1_PDU_PCI_IDX];

                                /* Update length pointer properly */
                                LIN_1_rxMessageLength = 0u;

                                /* The SF message is received, so set the proper status */
                                LIN_1_rxTlStatus = LD_COMPLETED;
                            }

                        #else /* Raw API */

                            if(LIN_1_rxBufDepth < (LIN_1_TL_RX_QUEUE_LEN / 8u))
                            {
                                /* Fill the RX queue from a MRF buffer */
                                for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                                {
                                    LIN_1_rawRxQueue[LIN_1_rxWrIndex] = frame[i];
                                    LIN_1_rxWrIndex++;
                                }

                                /* Read index should point to the next byte in MRF */
                                if(LIN_1_rxWrIndex == LIN_1_TL_RX_QUEUE_LEN)
                                {
                                    LIN_1_rxWrIndex = 0u;
                                }

                                /* 8 Bytes copied to MRF - increment buffer depth */
                                LIN_1_rxBufDepth++;

                                /* Specification doesn't require status of queue full
                                * so unconditionally set the status to data available
                                */
                                LIN_1_rxTlStatus = LD_DATA_AVAILABLE;
                            }

                        #endif /* (1u == LIN_1_TL_API_FORMAT) */
                    }
                    else
                    {
                        /* Do nothing. Length is not valid the data should not be trusted. */
                    }

                    /* Clear Service Disabled and Diagnostic Frame detected bits
                    * and process diagnostic frame receiving into a user buffer or MRF.
                    */
                    LIN_1_tlFlags &=
                        ((l_u8) ~(LIN_1_TL_CS_SERVICE_DISABLED | LIN_1_TL_DIAG_FRAME_DETECTED));
                }
                else
                {
                    /* Do nothing. Length is not valid the data should not be trusted. */
                }
            }
            else if((frame[LIN_1_PDU_PCI_IDX] & LIN_1_PDU_PCI_TYPE_MASK) ==
                LIN_1_PDU_PCI_TYPE_FF)                                 /* First Frame detected */
            {
                if(frame[LIN_1_PDU_LEN_IDX] > LIN_1_FRAME_DATA_SIZE_7)
                {
                    #if(1u == LIN_1_TL_API_FORMAT)    /* Cooked API */

                        /* Get one frame of a message if there is a message pending
                        * and the PCI is valid
                        */
                        if((LIN_1_rxTlStatus == LD_IN_PROGRESS) &&
                            (LIN_1_prevPci == LIN_1_PDU_PCI_TYPE_UNKNOWN ))
                        {
                            /* Store the result of condition check operand in temporary
                            * variable. This will improve code readability and will remove
                            * MISRA warning of uneven typecast.
                            */
                            tmpByte = (frame[LIN_1_PDU_PCI_IDX] &
                                ((l_u8) ~LIN_1_PDU_PCI_TYPE_MASK));

                            /* Following line allows to avoid violation of MISRA Rule 10.3:
                            * "A composite expression of 'essentially unsigned' type (unsigned char)
                            * is being cast to a wider unsigned type, 'unsigned int'."
                            */
                            tmpWord = (l_u16) tmpByte;

                            tmpWord = ((l_u16) (tmpWord << 8u)) | ((l_u16) frame[LIN_1_PDU_LEN_IDX]);

                            if(LIN_1_TL_BUF_LEN_MAX >= tmpWord)
                            {
                                /* Get First Frame Length with following two operations */
                                *LIN_1_tlLengthPointer = (((l_u16)
                                    frame[LIN_1_PDU_PCI_IDX]) &
                                        ((l_u16) ((l_u8) ~LIN_1_PDU_PCI_TYPE_MASK)));

                                *LIN_1_tlLengthPointer =
                                    ((l_u16) (*LIN_1_tlLengthPointer << 8u)) |
                                        ((l_u16) frame[LIN_1_PDU_LEN_IDX]);

                                /* Copy Length to current length variable */
                                LIN_1_rxMessageLength = *LIN_1_tlLengthPointer;

                                for(i = 3u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                                {
                                    *LIN_1_rxTlDataPointer = frame[i];   /* Get Frame Data */
                                    LIN_1_rxTlDataPointer++;
                                }

                                /* Update length pointer properly */
                                LIN_1_rxMessageLength -= LIN_1_FRAME_DATA_SIZE_5;

                                /* Save the state of the Frame Counter for monitor future possible errors */
                                LIN_1_rxFrameCounter = 0u;

                                /* Save the PCI type */
                                LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_FF;
                            }
                            else
                            {
                                /* Do nothing Length is invalid. */
                            }
                        }

                    #else /* Raw API */

                        if(LIN_1_rxBufDepth < (LIN_1_TL_RX_QUEUE_LEN / 8u))
                        {
                            /* Fill the MRF from a frame buffer */
                            for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                            {
                                LIN_1_rawRxQueue[LIN_1_rxWrIndex] = frame[i];
                                LIN_1_rxWrIndex++;
                            }

                            /* Read index should point to the next byte in MRF */
                            if(LIN_1_rxWrIndex == LIN_1_TL_RX_QUEUE_LEN)
                            {
                                LIN_1_rxWrIndex = 0u;
                            }

                            /* 8 Bytes copied to MRF - increment buffer depth */
                            LIN_1_rxBufDepth++;

                            /* Specification doesn't require status of queue full
                            * so unconditionally set the status to data available
                            */
                            LIN_1_rxTlStatus = LD_DATA_AVAILABLE;
                        }

                    #endif /* (1u == LIN_1_TL_API_FORMAT) */
                }
                else
                {
                    /* Do nothing. Length is not valid the data should not be trusted. */
                }
            }
            else if((frame[LIN_1_PDU_PCI_IDX] & LIN_1_PDU_PCI_TYPE_MASK) ==
                LIN_1_PDU_PCI_TYPE_CF)                                 /* Consecutive Frames detected */
            {
                #if(1u == LIN_1_TL_API_FORMAT) /* Cooked API */

                    /* Store previous PCI to avoid MISRA warning */
                    tmpPci = LIN_1_prevPci;

                    /* Get one frame of a message if there is a message pending and the
                    * PCI is valid.
                    */
                    if((LIN_1_rxTlStatus == LD_IN_PROGRESS) &&
                        ((tmpPci == LIN_1_PDU_PCI_TYPE_FF) ||
                            (tmpPci == LIN_1_PDU_PCI_TYPE_CF)))
                    {
                        /* Store the result of condition check operand in temporary
                        * variable. This will improve code readability and will remove
                        * MISRA warning of uneven typecast.
                        */
                        tmpByte = frame[LIN_1_PDU_PCI_IDX] & ((l_u8) ~LIN_1_PDU_PCI_TYPE_MASK);

                        /* Check if frame counter is valid */
                        if(((LIN_1_rxFrameCounter + 1u) == tmpByte) ||
                                (((LIN_1_rxFrameCounter + 1u) == 16u) && (tmpByte == 0u)))
                        {
                            for(i = 2u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                            {
                                *LIN_1_rxTlDataPointer = frame[i];    /* Get Frame Data */
                                LIN_1_rxTlDataPointer++;
                            }

                            /* Save current Frame Counter */
                            LIN_1_rxFrameCounter =
                                frame[LIN_1_PDU_PCI_IDX] & ((l_u8) ~LIN_1_PDU_PCI_TYPE_MASK);

                            /* Save the PCI type */
                            LIN_1_prevPci = LIN_1_PDU_PCI_TYPE_CF;

                            /* Update length pointer properly */
                            if(LIN_1_rxMessageLength >= LIN_1_FRAME_DATA_SIZE_6)
                            {
                                LIN_1_rxMessageLength -= LIN_1_FRAME_DATA_SIZE_6;
                            }
                            else
                            {
                                LIN_1_rxMessageLength = 0u;
                            }
                        }
                        else
                        {
                            /* Indicate an error if Frame Counter is invalid. */
                            LIN_1_rxTlStatus = LD_WRONG_SN;
                        }
                    }

                #else /* Raw API */

                    if(LIN_1_rxBufDepth < (LIN_1_TL_RX_QUEUE_LEN / 8u))
                    {
                        /* Fill the MRF from a frame buffer */
                        for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                        {
                            LIN_1_rawRxQueue[LIN_1_rxWrIndex] = frame[i];
                            LIN_1_rxWrIndex++;
                        }

                        /* Read index should point to the next byte in MRF */
                        if(LIN_1_rxWrIndex == LIN_1_TL_RX_QUEUE_LEN)
                        {
                            LIN_1_rxWrIndex = 0u;
                        }

                        /* 8 Bytes copied to MRF - increment buffer depth */
                        LIN_1_rxBufDepth++;

                        /* Specification doesn't require status of queue full
                        * so unconditionally set the status to data available
                        */
                        LIN_1_rxTlStatus = LD_DATA_AVAILABLE;
                    }

                #endif /* (1u == LIN_1_TL_API_FORMAT) */
            }
            else
            {
                /* Do nothing SID is invalid. */
            }
        }
        else  /* Indicate an error */
        {
            #if(1u == LIN_1_TL_API_FORMAT) /* Cooked API */

                /* Reception failed */
                if((0u != (LIN_1_tlFlags & LIN_1_TL_RX_DIRECTION)) &&
                    (0u != (LIN_1_tlFlags & LIN_1_TL_RX_REQUESTED)))
                {
                    LIN_1_rxTlStatus = LD_FAILED;
                }

            #endif /* (1u == LIN_1_TL_API_FORMAT) */
        }
    }

    #if(1u == LIN_1_TL_API_FORMAT)

        /*******************************************************************************
        * Function Name: ld_send_message
        ********************************************************************************
        *
        * Summary:
        *  The call packs the information specified by data and length into one or
        *  multiple diagnostic frames. If the call is made in a master node application
        *  the frames are transmitted to the slave node with the address NAD. If the
        *  call is made in a slave node application the frames are transmitted to the
        *  master node with the address NAD. The parameter NAD is not used in slave
        *  nodes.
        *
        *  The value of the SID (or RSID) shall be the first byte in the data area.
        *
        *  Length must be in the range of 1 to 4095 bytes. The length shall also include
        *  the SID (or RSID) value, i.e. message length plus one.
        *
        *  The call is asynchronous, i.e. not suspended until the message has been sent,
        *  and the buffer may not be changed by the application as long as calls to
        *  ld_tx_status returns LD_IN_PROGRESS.
        *
        *  The data is transmitted in suitable frames (master request frame for master
        *  nodes and slave response frame for slave nodes).
        *
        *  If there is a message in progress, the call will return with no action.
        *
        * Parameters:
        *  iii - Interface.
        *  length - Size of data to be sent in bytes.
        *  nad - Address of the slave node to which data is sent.
        *  data - Array of data to be sent. The value of the RSID is the first byte in
        *  the data area.
        *
        * Return:
        *  None
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        void ld_send_message(l_ifc_handle iii, l_u16 length, l_u8 nad, const l_u8* const ld_data)
                                                        
        { volatile
            l_u8 interruptState;

            /* To remove unreferenced local variable warning */
            iii = iii;

            /* NAD isn't used in the slave node */
            nad = nad;

            if(LIN_1_txTlStatus != LD_IN_PROGRESS)
            {
                /* Interrupts can be disabled as the global variables used by LIN ISR
                * is used below.
                */
                interruptState = CyEnterCriticalSection();

                LIN_1_txTlDataPointer = ld_data;

                /* Reset the data count */
                LIN_1_txTlDataCount = 0u;

                /* Set up the length pointer, Length shouldn't be greater then
                * LIN_1_TL_BUF_LEN_MAX.
                */
                LIN_1_txMessageLength = length;

                /* Indicates that there is a message in progress */
                LIN_1_txTlStatus = LD_IN_PROGRESS;

                /* Indicates that Cooked API requested transmit data */
                LIN_1_tlFlags |= LIN_1_TL_TX_REQUESTED;

                /* Restore the interrupt state */
                CyExitCriticalSection(interruptState);
            }
        }


        /*******************************************************************************
        * Function Name: ld_receive_message
        ********************************************************************************
        *
        * Summary:
        *  The call prepares the LIN diagnostic module to receive one message and store
        *  it in the buffer pointed to by data. At the call, length shall specify the
        *  maximum length allowed. When the reception has completed, length is changed
        *  to the actual length and NAD to the NAD in the message.
        *
        *  SID (or RSID) will be the first byte in the data area.
        *
        *  Length will be in the range of 1 to 4095 bytes, but never more than the value
        *  originally set in the call. SID (or RSID) is included in the length.
        *
        *  The parameter NAD is not used in slave nodes.
        *
        *  The call is asynchronous, i.e. not suspended until the message has been
        *  received, and the buffer may not be changed by the application as long as
        *  calls to ld_rx_status returns LD_IN_PROGRESS. If the call is made after the
        *  message transmission has commenced on the bus (i.e. the SF or FF is already
        *  transmitted), this message will not be received. Instead the function will
        *  wait until next message commence.
        *
        *  The data is received from the succeeding suitable frames (master request
        *  frame for slave nodes and slave response frame for master nodes).
        *
        *  The application shall monitor the ld_rx_status and shall not call this
        *  function until the status is LD_COMPLETED. Otherwise this function may
        *  return inconsistent data in the parameters.
        *
        * Parameters:
        *  iii - Interface.
        *  length: Size of data to be received in bytes.
        *  nad: Address of the slave node from which data is received.
        *  data: Array of data to be received. The value of the SID is the first byte
        *  in the data area.
        *
        * Return:
        *  None
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        void ld_receive_message(l_ifc_handle iii, l_u16* const length, l_u8* const nad, l_u8* const ld_data)
                                                        
        {
            l_u8 interruptState;

            /* To remove unreferenced local variable warning */
            iii = iii;

            if(LIN_1_rxTlStatus != LD_IN_PROGRESS)
            {
                /* Interrupts can be disabled as the global variables used by LIN ISR
                * is used below.
                */
                interruptState = CyEnterCriticalSection();

                /* Set the user status bits */
                LIN_1_rxTlStatus = LD_IN_PROGRESS;

                /* Set up the data pointer */
                LIN_1_rxTlDataPointer = ld_data;

                /* Set up the initial data pointer that should
                * always point to the beginning of a user buffer.
                */
                LIN_1_rxTlInitDataPointer = ld_data;

                /* Set up the NAD pointer */
                LIN_1_tlNadPointer = nad;

                /* Set up the length pointer */
                LIN_1_tlLengthPointer = length;

                LIN_1_rxMessageLength = *length;

                /* Indicates that Cooked API requested receive data */
                LIN_1_tlFlags |= LIN_1_TL_RX_REQUESTED;

                /* Restore the interrupt state */
                CyExitCriticalSection(interruptState);
            }
        }


        /*******************************************************************************
        * Function Name: ld_tx_status
        ********************************************************************************
        *
        * Summary:
        *  The call returns the status of the last made call to ld_send_message.
        *
        * Parameters:
        *  iii - Interface.
        *
        * Return:
        *  LD_IN_PROGRESS - The transmission is not yet completed.
        *
        *  LD_COMPLETED - The transmission has completed successfully (and you can
        *                 issue a new ld_send_message call). This value is also
        *                 returned after initialization of the Transport Layer.
        *
        *  LD_FAILED - The transmission ended in an error. The data was only partially
        *              sent. The transport layer shall be reinitialized before
        *              processing further messages. To find out why a transmission has
        *              failed, check the status management function l_read_status.
        *
        *  LD_N_AS_TIMEOUT - The transmission failed because of a N_As timeout,
        *
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        l_u8 ld_tx_status(l_ifc_handle iii) 
        {
            /* To remove unreferenced local variable warning */
            iii = iii;

            return(LIN_1_txTlStatus);
        }


        /*******************************************************************************
        * Function Name: ld_rx_status
        ********************************************************************************
        *
        * Summary:
        *  The call returns the status of the last made call to ld_receive_message.
        *
        * Parameters:
        *  iii - Interface.
        *
        * Return:
        *  LD_IN_PROGRESS - The reception is not yet completed.
        *
        *  LD_COMPLETED - The reception has completed successfully and all information
        *                 (length, NAD, data) is available. (You can also issue a new
        *                 ld_receive_message call). This value is also returned after
        *                 initialization of the Tansport Layer.
        *
        *  LD_FAILED - The reception ended in an error. The data was only partially
        *              received and should not be trusted. Initialize before processing
        *              further transport layer messages. To find out why a reception
        *              has failed, check the status management function l_read_status.
        *
        *  LD_N_CR_TIMEOUT - The reception failed because of a N_Cr timeout,
        *
        *  LD_WRONG_SN - The reception failed because of an unexpected sequence
        *                number.
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        l_u8 ld_rx_status(l_ifc_handle iii) 
        {
            /* To remove unreferenced local variable warning */
            iii = iii;

            return(LIN_1_rxTlStatus);
        }


    #else


        /*******************************************************************************
        * Function Name: ld_put_raw
        ********************************************************************************
        *
        * Summary:
        *  The call queues the transmission of 8 bytes of data in one frame. The data
        *  is sent in the next suitable frame (slave response frame). The data area
        *  will be copied in the call, the pointer will not be memorized. If no more
        *  queue resources are available, the data may be jettisoned and the
        *  appropriate error status will be set.
        *
        * Parameters:
        *  iii - Interface.
        *  data - Array of data to be sent.
        *
        * Return:
        *  None
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        void ld_put_raw(l_ifc_handle iii, const l_u8* const ld_data) 
        {
            l_u8 interruptState;
            l_u8 i;

            /* To remove unreferenced local variable warning */
            iii = iii;

            /* Interrupts can be disabled as the global variables used by LIN ISR
            * is used below.
            */
            interruptState = CyEnterCriticalSection();

            /* Copy data only when the buffer is not full */
            if(LIN_1_txBufDepth < (LIN_1_TL_TX_QUEUE_LEN / 8u))
            {
                /* Copy 8 bytes of data to Raw TX queue buffer */
                for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                {
                    /* Copy one byte of data to SRF buffer */
                    LIN_1_rawTxQueue[LIN_1_txWrIndex] = *(ld_data + i);
                    LIN_1_txWrIndex++;
                }

                /* If the end of the buffer is reached then reset the write index */
                if(LIN_1_txWrIndex == LIN_1_TL_TX_QUEUE_LEN)
                {
                    LIN_1_txWrIndex = 0u;
                }

                /* 8 bytes of data were copied so increment a buffer depth */
                LIN_1_txBufDepth++;

                /* Update the status properly */
                if(LIN_1_txBufDepth == (LIN_1_TL_TX_QUEUE_LEN / 8u))
                {
                    LIN_1_txTlStatus = LD_QUEUE_FULL;
                }
                else
                {
                    LIN_1_txTlStatus = LD_QUEUE_AVAILABLE;
                }
            }

            /* Restore the interrupt state */
            CyExitCriticalSection(interruptState);
        }


        /*******************************************************************************
        * Function Name: ld_get_raw
        ********************************************************************************
        *
        * Summary:
        *  The call copies the oldest received diagnostic frame data to the memory
        *  specified by data. The data returned is received from master request frame
        *  If the receive queue is empty no data will be copied.
        *
        * Parameters:
        *  iii - Interface.
        *  data - Array to which the oldest received diagnostic frame data will be
        *  copied.
        *
        * Return:
        *  None
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        void ld_get_raw(l_ifc_handle iii, l_u8* const ld_data) 
        {
            l_u8 interruptState;
            l_u8 i;

            /* To remove unreferenced local variable warning */
            iii = iii;

            /* Interrupts can be disabled as the global variables used by LIN ISR
            * is used below.
            */
            interruptState = CyEnterCriticalSection();

            /* If queue is empty then do not copy anything */
            if(LIN_1_rxBufDepth != 0u)
            {
                /* Copy 8 bytes of data from Raw RX queue buffer */
                for(i = 0u; i < LIN_1_FRAME_DATA_SIZE_8; i++)
                {
                    *(ld_data + i) = LIN_1_rawRxQueue[LIN_1_rxRdIndex];
                    LIN_1_rxRdIndex++;
                }

                /* 8 bytes of data were copied so decrement a buffer depth */
                LIN_1_rxBufDepth--;

                /* If the end of the buffer is reached then go to the start */
                if(LIN_1_rxRdIndex == LIN_1_TL_RX_QUEUE_LEN)
                {
                    LIN_1_rxRdIndex = 0u;
                }

                /* Update the status properly */
                if(LIN_1_rxBufDepth == 0u)
                {
                    LIN_1_rxTlStatus = LD_NO_DATA;
                }
                else
                {
                    LIN_1_rxTlStatus = LD_DATA_AVAILABLE;
                }
            }

            /* Restore the interrupt state */
            CyExitCriticalSection(interruptState);
        }


        /*******************************************************************************
        * Function Name: ld_raw_tx_status
        ********************************************************************************
        *
        * Summary:
        *  The call returns the status of the raw frame transmission function.
        *
        * Parameters:
        *  iii - Interface.
        *
        * Return:
        *  Status:
        *   LD_QUEUE_EMPTY - The transmit queue is empty. In case previous calls to
        *   ld_put_raw, all frames in the queue have been transmitted.
        *
        *   LD_QUEUE_AVAILABLE - The transmit queue contains entries, but is not full.
        *
        *   LD_QUEUE_FULL - The transmit queue is full and can not accept further
        *   frames.
        *
        *   LD_TRANSMIT_ERROR - LIN protocol errors occurred during the transfer;
        *   initialize and redo the transfer.
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        l_u8 ld_raw_tx_status(l_ifc_handle iii) 
        {
            /* To remove unreferenced local variable warning */
            iii = iii;

            return(LIN_1_txTlStatus);
        }


        /*******************************************************************************
        * Function Name: LIN_1_ld_raw_rx_status
        ********************************************************************************
        *
        * Summary:
        *  The call returns the status of the raw frame receive function.
        *
        * Parameters:
        *  iii - Interface.
        *
        * Return:
        *  Status:
        *   LD_NO_DATA - The receive queue is empty.
        *
        *   LD_DATA_AVAILABLE - The receive queue contains data that can be read.
        *
        *   LD_RECEIVE_ERROR - LIN protocol errors occurred during the transfer;
        *
        *   initialize and redo the transfer.
        *
        * Reentrant:
        *  No
        *
        *******************************************************************************/
        l_u8 ld_raw_rx_status(l_ifc_handle iii) 
        {
            /* To remove unreferenced local variable warning */
            iii = iii;

            return(LIN_1_rxTlStatus);
        }

    #endif /* (1u == LIN_1_TL_API_FORMAT) */

#endif /* (1u == LIN_1_TL_ENABLED) */


/* [] END OF FILE */
