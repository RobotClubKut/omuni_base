/*******************************************************************************
* File Name: LIN_PM.c
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

#include "LIN.h"


/***************************************
* Local data allocation
***************************************/

static LIN_BACKUP_STRUCT  LIN_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: LIN_SaveConfig
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
*  LIN_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_SaveConfig(void)
{
    #if (CY_UDB_V0)

        #if(LIN_CONTROL_REG_REMOVED == 0u)
            LIN_backup.cr = LIN_CONTROL_REG;
        #endif /* End LIN_CONTROL_REG_REMOVED */

        #if( (LIN_RX_ENABLED) || (LIN_HD_ENABLED) )
            LIN_backup.rx_period = LIN_RXBITCTR_PERIOD_REG;
            LIN_backup.rx_mask = LIN_RXSTATUS_MASK_REG;
            #if (LIN_RXHW_ADDRESS_ENABLED)
                LIN_backup.rx_addr1 = LIN_RXADDRESS1_REG;
                LIN_backup.rx_addr2 = LIN_RXADDRESS2_REG;
            #endif /* End LIN_RXHW_ADDRESS_ENABLED */
        #endif /* End LIN_RX_ENABLED | LIN_HD_ENABLED*/

        #if(LIN_TX_ENABLED)
            #if(LIN_TXCLKGEN_DP)
                LIN_backup.tx_clk_ctr = LIN_TXBITCLKGEN_CTR_REG;
                LIN_backup.tx_clk_compl = LIN_TXBITCLKTX_COMPLETE_REG;
            #else
                LIN_backup.tx_period = LIN_TXBITCTR_PERIOD_REG;
            #endif /*End LIN_TXCLKGEN_DP */
            LIN_backup.tx_mask = LIN_TXSTATUS_MASK_REG;
        #endif /*End LIN_TX_ENABLED */


    #else /* CY_UDB_V1 */

        #if(LIN_CONTROL_REG_REMOVED == 0u)
            LIN_backup.cr = LIN_CONTROL_REG;
        #endif /* End LIN_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: LIN_RestoreConfig
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
*  LIN_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_RestoreConfig(void)
{

    #if (CY_UDB_V0)

        #if(LIN_CONTROL_REG_REMOVED == 0u)
            LIN_CONTROL_REG = LIN_backup.cr;
        #endif /* End LIN_CONTROL_REG_REMOVED */

        #if( (LIN_RX_ENABLED) || (LIN_HD_ENABLED) )
            LIN_RXBITCTR_PERIOD_REG = LIN_backup.rx_period;
            LIN_RXSTATUS_MASK_REG = LIN_backup.rx_mask;
            #if (LIN_RXHW_ADDRESS_ENABLED)
                LIN_RXADDRESS1_REG = LIN_backup.rx_addr1;
                LIN_RXADDRESS2_REG = LIN_backup.rx_addr2;
            #endif /* End LIN_RXHW_ADDRESS_ENABLED */
        #endif  /* End (LIN_RX_ENABLED) || (LIN_HD_ENABLED) */

        #if(LIN_TX_ENABLED)
            #if(LIN_TXCLKGEN_DP)
                LIN_TXBITCLKGEN_CTR_REG = LIN_backup.tx_clk_ctr;
                LIN_TXBITCLKTX_COMPLETE_REG = LIN_backup.tx_clk_compl;
            #else
                LIN_TXBITCTR_PERIOD_REG = LIN_backup.tx_period;
            #endif /*End LIN_TXCLKGEN_DP */
            LIN_TXSTATUS_MASK_REG = LIN_backup.tx_mask;
        #endif /*End LIN_TX_ENABLED */

    #else /* CY_UDB_V1 */

        #if(LIN_CONTROL_REG_REMOVED == 0u)
            LIN_CONTROL_REG = LIN_backup.cr;
        #endif /* End LIN_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: LIN_Sleep
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
*  LIN_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_Sleep(void)
{

    #if(LIN_RX_ENABLED || LIN_HD_ENABLED)
        if((LIN_RXSTATUS_ACTL_REG  & LIN_INT_ENABLE) != 0u)
        {
            LIN_backup.enableState = 1u;
        }
        else
        {
            LIN_backup.enableState = 0u;
        }
    #else
        if((LIN_TXSTATUS_ACTL_REG  & LIN_INT_ENABLE) !=0u)
        {
            LIN_backup.enableState = 1u;
        }
        else
        {
            LIN_backup.enableState = 0u;
        }
    #endif /* End LIN_RX_ENABLED || LIN_HD_ENABLED*/

    LIN_Stop();
    LIN_SaveConfig();
}


/*******************************************************************************
* Function Name: LIN_Wakeup
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
*  LIN_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_Wakeup(void)
{
    LIN_RestoreConfig();
    #if( (LIN_RX_ENABLED) || (LIN_HD_ENABLED) )
        LIN_ClearRxBuffer();
    #endif /* End (LIN_RX_ENABLED) || (LIN_HD_ENABLED) */
    #if(LIN_TX_ENABLED || LIN_HD_ENABLED)
        LIN_ClearTxBuffer();
    #endif /* End LIN_TX_ENABLED || LIN_HD_ENABLED */

    if(LIN_backup.enableState != 0u)
    {
        LIN_Enable();
    }
}


/* [] END OF FILE */
