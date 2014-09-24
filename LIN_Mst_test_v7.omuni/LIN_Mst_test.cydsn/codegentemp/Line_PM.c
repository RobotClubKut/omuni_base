/*******************************************************************************
* File Name: Line_PM.c
* Version 2.30
*
* Description:
*  This file provides Sleep/WakeUp APIs functionality.
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


/***************************************
* Local data allocation
***************************************/

static Line_BACKUP_STRUCT  Line_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: Line_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Line_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Line_SaveConfig(void)
{
    #if (CY_UDB_V0)

        #if(Line_CONTROL_REG_REMOVED == 0u)
            Line_backup.cr = Line_CONTROL_REG;
        #endif /* End Line_CONTROL_REG_REMOVED */

        #if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
            Line_backup.rx_period = Line_RXBITCTR_PERIOD_REG;
            Line_backup.rx_mask = Line_RXSTATUS_MASK_REG;
            #if (Line_RXHW_ADDRESS_ENABLED)
                Line_backup.rx_addr1 = Line_RXADDRESS1_REG;
                Line_backup.rx_addr2 = Line_RXADDRESS2_REG;
            #endif /* End Line_RXHW_ADDRESS_ENABLED */
        #endif /* End Line_RX_ENABLED | Line_HD_ENABLED*/

        #if(Line_TX_ENABLED)
            #if(Line_TXCLKGEN_DP)
                Line_backup.tx_clk_ctr = Line_TXBITCLKGEN_CTR_REG;
                Line_backup.tx_clk_compl = Line_TXBITCLKTX_COMPLETE_REG;
            #else
                Line_backup.tx_period = Line_TXBITCTR_PERIOD_REG;
            #endif /*End Line_TXCLKGEN_DP */
            Line_backup.tx_mask = Line_TXSTATUS_MASK_REG;
        #endif /*End Line_TX_ENABLED */


    #else /* CY_UDB_V1 */

        #if(Line_CONTROL_REG_REMOVED == 0u)
            Line_backup.cr = Line_CONTROL_REG;
        #endif /* End Line_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: Line_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Line_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Line_RestoreConfig(void)
{

    #if (CY_UDB_V0)

        #if(Line_CONTROL_REG_REMOVED == 0u)
            Line_CONTROL_REG = Line_backup.cr;
        #endif /* End Line_CONTROL_REG_REMOVED */

        #if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
            Line_RXBITCTR_PERIOD_REG = Line_backup.rx_period;
            Line_RXSTATUS_MASK_REG = Line_backup.rx_mask;
            #if (Line_RXHW_ADDRESS_ENABLED)
                Line_RXADDRESS1_REG = Line_backup.rx_addr1;
                Line_RXADDRESS2_REG = Line_backup.rx_addr2;
            #endif /* End Line_RXHW_ADDRESS_ENABLED */
        #endif  /* End (Line_RX_ENABLED) || (Line_HD_ENABLED) */

        #if(Line_TX_ENABLED)
            #if(Line_TXCLKGEN_DP)
                Line_TXBITCLKGEN_CTR_REG = Line_backup.tx_clk_ctr;
                Line_TXBITCLKTX_COMPLETE_REG = Line_backup.tx_clk_compl;
            #else
                Line_TXBITCTR_PERIOD_REG = Line_backup.tx_period;
            #endif /*End Line_TXCLKGEN_DP */
            Line_TXSTATUS_MASK_REG = Line_backup.tx_mask;
        #endif /*End Line_TX_ENABLED */

    #else /* CY_UDB_V1 */

        #if(Line_CONTROL_REG_REMOVED == 0u)
            Line_CONTROL_REG = Line_backup.cr;
        #endif /* End Line_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: Line_Sleep
********************************************************************************
*
* Summary:
*  Stops and saves the user configuration. Should be called
*  just prior to entering sleep.
*
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Line_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Line_Sleep(void)
{

    #if(Line_RX_ENABLED || Line_HD_ENABLED)
        if((Line_RXSTATUS_ACTL_REG  & Line_INT_ENABLE) != 0u)
        {
            Line_backup.enableState = 1u;
        }
        else
        {
            Line_backup.enableState = 0u;
        }
    #else
        if((Line_TXSTATUS_ACTL_REG  & Line_INT_ENABLE) !=0u)
        {
            Line_backup.enableState = 1u;
        }
        else
        {
            Line_backup.enableState = 0u;
        }
    #endif /* End Line_RX_ENABLED || Line_HD_ENABLED*/

    Line_Stop();
    Line_SaveConfig();
}


/*******************************************************************************
* Function Name: Line_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called
*  just after awaking from sleep.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Line_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Line_Wakeup(void)
{
    Line_RestoreConfig();
    #if( (Line_RX_ENABLED) || (Line_HD_ENABLED) )
        Line_ClearRxBuffer();
    #endif /* End (Line_RX_ENABLED) || (Line_HD_ENABLED) */
    #if(Line_TX_ENABLED || Line_HD_ENABLED)
        Line_ClearTxBuffer();
    #endif /* End Line_TX_ENABLED || Line_HD_ENABLED */

    if(Line_backup.enableState != 0u)
    {
        Line_Enable();
    }
}


/* [] END OF FILE */
