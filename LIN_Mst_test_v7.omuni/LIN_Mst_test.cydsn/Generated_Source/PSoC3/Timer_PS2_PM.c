/*******************************************************************************
* File Name: Timer_PS2_PM.c
* Version 2.50
*
*  Description:
*     This file provides the power management source code to API for the
*     Timer.
*
*   Note:
*     None
*
*******************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "Timer_PS2.h"
static Timer_PS2_backupStruct Timer_PS2_backup;


/*******************************************************************************
* Function Name: Timer_PS2_SaveConfig
********************************************************************************
*
* Summary:
*     Save the current user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Timer_PS2_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void Timer_PS2_SaveConfig(void) 
{
    #if (!Timer_PS2_UsingFixedFunction)
        /* Backup the UDB non-rentention registers for CY_UDB_V0 */
        #if (CY_UDB_V0)
            Timer_PS2_backup.TimerUdb = Timer_PS2_ReadCounter();
            Timer_PS2_backup.TimerPeriod = Timer_PS2_ReadPeriod();
            Timer_PS2_backup.InterruptMaskValue = Timer_PS2_STATUS_MASK;
            #if (Timer_PS2_UsingHWCaptureCounter)
                Timer_PS2_backup.TimerCaptureCounter = Timer_PS2_ReadCaptureCount();
            #endif /* Backup the UDB non-rentention register capture counter for CY_UDB_V0 */
        #endif /* Backup the UDB non-rentention registers for CY_UDB_V0 */

        #if (CY_UDB_V1)
            Timer_PS2_backup.TimerUdb = Timer_PS2_ReadCounter();
            Timer_PS2_backup.InterruptMaskValue = Timer_PS2_STATUS_MASK;
            #if (Timer_PS2_UsingHWCaptureCounter)
                Timer_PS2_backup.TimerCaptureCounter = Timer_PS2_ReadCaptureCount();
            #endif /* Back Up capture counter register  */
        #endif /* Backup non retention registers, interrupt mask and capture counter for CY_UDB_V1 */

        #if(!Timer_PS2_ControlRegRemoved)
            Timer_PS2_backup.TimerControlRegister = Timer_PS2_ReadControlRegister();
        #endif /* Backup the enable state of the Timer component */
    #endif /* Backup non retention registers in UDB implementation. All fixed function registers are retention */
}


/*******************************************************************************
* Function Name: Timer_PS2_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Timer_PS2_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void Timer_PS2_RestoreConfig(void) 
{   
    #if (!Timer_PS2_UsingFixedFunction)
        /* Restore the UDB non-rentention registers for CY_UDB_V0 */
        #if (CY_UDB_V0)
            /* Interrupt State Backup for Critical Region*/
            uint8 Timer_PS2_interruptState;

            Timer_PS2_WriteCounter(Timer_PS2_backup.TimerUdb);
            Timer_PS2_WritePeriod(Timer_PS2_backup.TimerPeriod);
            /* CyEnterCriticalRegion and CyExitCriticalRegion are used to mark following region critical*/
            /* Enter Critical Region*/
            Timer_PS2_interruptState = CyEnterCriticalSection();
            /* Use the interrupt output of the status register for IRQ output */
            Timer_PS2_STATUS_AUX_CTRL |= Timer_PS2_STATUS_ACTL_INT_EN_MASK;
            /* Exit Critical Region*/
            CyExitCriticalSection(Timer_PS2_interruptState);
            Timer_PS2_STATUS_MASK =Timer_PS2_backup.InterruptMaskValue;
            #if (Timer_PS2_UsingHWCaptureCounter)
                Timer_PS2_SetCaptureCount(Timer_PS2_backup.TimerCaptureCounter);
            #endif /* Restore the UDB non-rentention register capture counter for CY_UDB_V0 */
        #endif /* Restore the UDB non-rentention registers for CY_UDB_V0 */

        #if (CY_UDB_V1)
            Timer_PS2_WriteCounter(Timer_PS2_backup.TimerUdb);
            Timer_PS2_STATUS_MASK =Timer_PS2_backup.InterruptMaskValue;
            #if (Timer_PS2_UsingHWCaptureCounter)
                Timer_PS2_SetCaptureCount(Timer_PS2_backup.TimerCaptureCounter);
            #endif /* Restore Capture counter register*/
        #endif /* Restore up non retention registers, interrupt mask and capture counter for CY_UDB_V1 */

        #if(!Timer_PS2_ControlRegRemoved)
            Timer_PS2_WriteControlRegister(Timer_PS2_backup.TimerControlRegister);
        #endif /* Restore the enable state of the Timer component */
    #endif /* Restore non retention registers in the UDB implementation only */
}


/*******************************************************************************
* Function Name: Timer_PS2_Sleep
********************************************************************************
*
* Summary:
*     Stop and Save the user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Timer_PS2_backup.TimerEnableState:  Is modified depending on the
*  enable state of the block before entering sleep mode.
*
*******************************************************************************/
void Timer_PS2_Sleep(void) 
{
    #if(!Timer_PS2_ControlRegRemoved)
        /* Save Counter's enable state */
        if(Timer_PS2_CTRL_ENABLE == (Timer_PS2_CONTROL & Timer_PS2_CTRL_ENABLE))
        {
            /* Timer is enabled */
            Timer_PS2_backup.TimerEnableState = 1u;
        }
        else
        {
            /* Timer is disabled */
            Timer_PS2_backup.TimerEnableState = 0u;
        }
    #endif /* Back up enable state from the Timer control register */
    Timer_PS2_Stop();
    Timer_PS2_SaveConfig();
}


/*******************************************************************************
* Function Name: Timer_PS2_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Timer_PS2_backup.enableState:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void Timer_PS2_Wakeup(void) 
{
    Timer_PS2_RestoreConfig();
    #if(!Timer_PS2_ControlRegRemoved)
        if(Timer_PS2_backup.TimerEnableState == 1u)
        {     /* Enable Timer's operation */
                Timer_PS2_Enable();
        } /* Do nothing if Timer was disabled before */
    #endif /* Remove this code section if Control register is removed */
}


/* [] END OF FILE */
