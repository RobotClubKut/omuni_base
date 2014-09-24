/*******************************************************************************
* File Name: PS2_PM.c
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

#include "PS2.h"


/***************************************
* Local data allocation
***************************************/

static PS2_BACKUP_STRUCT  PS2_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: PS2_SaveConfig
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
*  PS2_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void PS2_SaveConfig(void)
{
    #if (CY_UDB_V0)

        #if(PS2_CONTROL_REG_REMOVED == 0u)
            PS2_backup.cr = PS2_CONTROL_REG;
        #endif /* End PS2_CONTROL_REG_REMOVED */

        #if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
            PS2_backup.rx_period = PS2_RXBITCTR_PERIOD_REG;
            PS2_backup.rx_mask = PS2_RXSTATUS_MASK_REG;
            #if (PS2_RXHW_ADDRESS_ENABLED)
                PS2_backup.rx_addr1 = PS2_RXADDRESS1_REG;
                PS2_backup.rx_addr2 = PS2_RXADDRESS2_REG;
            #endif /* End PS2_RXHW_ADDRESS_ENABLED */
        #endif /* End PS2_RX_ENABLED | PS2_HD_ENABLED*/

        #if(PS2_TX_ENABLED)
            #if(PS2_TXCLKGEN_DP)
                PS2_backup.tx_clk_ctr = PS2_TXBITCLKGEN_CTR_REG;
                PS2_backup.tx_clk_compl = PS2_TXBITCLKTX_COMPLETE_REG;
            #else
                PS2_backup.tx_period = PS2_TXBITCTR_PERIOD_REG;
            #endif /*End PS2_TXCLKGEN_DP */
            PS2_backup.tx_mask = PS2_TXSTATUS_MASK_REG;
        #endif /*End PS2_TX_ENABLED */


    #else /* CY_UDB_V1 */

        #if(PS2_CONTROL_REG_REMOVED == 0u)
            PS2_backup.cr = PS2_CONTROL_REG;
        #endif /* End PS2_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: PS2_RestoreConfig
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
*  PS2_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void PS2_RestoreConfig(void)
{

    #if (CY_UDB_V0)

        #if(PS2_CONTROL_REG_REMOVED == 0u)
            PS2_CONTROL_REG = PS2_backup.cr;
        #endif /* End PS2_CONTROL_REG_REMOVED */

        #if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
            PS2_RXBITCTR_PERIOD_REG = PS2_backup.rx_period;
            PS2_RXSTATUS_MASK_REG = PS2_backup.rx_mask;
            #if (PS2_RXHW_ADDRESS_ENABLED)
                PS2_RXADDRESS1_REG = PS2_backup.rx_addr1;
                PS2_RXADDRESS2_REG = PS2_backup.rx_addr2;
            #endif /* End PS2_RXHW_ADDRESS_ENABLED */
        #endif  /* End (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */

        #if(PS2_TX_ENABLED)
            #if(PS2_TXCLKGEN_DP)
                PS2_TXBITCLKGEN_CTR_REG = PS2_backup.tx_clk_ctr;
                PS2_TXBITCLKTX_COMPLETE_REG = PS2_backup.tx_clk_compl;
            #else
                PS2_TXBITCTR_PERIOD_REG = PS2_backup.tx_period;
            #endif /*End PS2_TXCLKGEN_DP */
            PS2_TXSTATUS_MASK_REG = PS2_backup.tx_mask;
        #endif /*End PS2_TX_ENABLED */

    #else /* CY_UDB_V1 */

        #if(PS2_CONTROL_REG_REMOVED == 0u)
            PS2_CONTROL_REG = PS2_backup.cr;
        #endif /* End PS2_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: PS2_Sleep
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
*  PS2_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void PS2_Sleep(void)
{

    #if(PS2_RX_ENABLED || PS2_HD_ENABLED)
        if((PS2_RXSTATUS_ACTL_REG  & PS2_INT_ENABLE) != 0u)
        {
            PS2_backup.enableState = 1u;
        }
        else
        {
            PS2_backup.enableState = 0u;
        }
    #else
        if((PS2_TXSTATUS_ACTL_REG  & PS2_INT_ENABLE) !=0u)
        {
            PS2_backup.enableState = 1u;
        }
        else
        {
            PS2_backup.enableState = 0u;
        }
    #endif /* End PS2_RX_ENABLED || PS2_HD_ENABLED*/

    PS2_Stop();
    PS2_SaveConfig();
}


/*******************************************************************************
* Function Name: PS2_Wakeup
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
*  PS2_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void PS2_Wakeup(void)
{
    PS2_RestoreConfig();
    #if( (PS2_RX_ENABLED) || (PS2_HD_ENABLED) )
        PS2_ClearRxBuffer();
    #endif /* End (PS2_RX_ENABLED) || (PS2_HD_ENABLED) */
    #if(PS2_TX_ENABLED || PS2_HD_ENABLED)
        PS2_ClearTxBuffer();
    #endif /* End PS2_TX_ENABLED || PS2_HD_ENABLED */

    if(PS2_backup.enableState != 0u)
    {
        PS2_Enable();
    }
}


/* [] END OF FILE */
