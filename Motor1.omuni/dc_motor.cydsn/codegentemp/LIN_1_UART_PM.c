/*******************************************************************************
* File Name: LIN_1_UART_PM.c
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

#include "LIN_1_UART.h"


/***************************************
* Local data allocation
***************************************/

static LIN_1_UART_BACKUP_STRUCT  LIN_1_UART_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: LIN_1_UART_SaveConfig
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
*  LIN_1_UART_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_1_UART_SaveConfig(void)
{
    #if (CY_UDB_V0)

        #if(LIN_1_UART_CONTROL_REG_REMOVED == 0u)
            LIN_1_UART_backup.cr = LIN_1_UART_CONTROL_REG;
        #endif /* End LIN_1_UART_CONTROL_REG_REMOVED */

        #if( (LIN_1_UART_RX_ENABLED) || (LIN_1_UART_HD_ENABLED) )
            LIN_1_UART_backup.rx_period = LIN_1_UART_RXBITCTR_PERIOD_REG;
            LIN_1_UART_backup.rx_mask = LIN_1_UART_RXSTATUS_MASK_REG;
            #if (LIN_1_UART_RXHW_ADDRESS_ENABLED)
                LIN_1_UART_backup.rx_addr1 = LIN_1_UART_RXADDRESS1_REG;
                LIN_1_UART_backup.rx_addr2 = LIN_1_UART_RXADDRESS2_REG;
            #endif /* End LIN_1_UART_RXHW_ADDRESS_ENABLED */
        #endif /* End LIN_1_UART_RX_ENABLED | LIN_1_UART_HD_ENABLED*/

        #if(LIN_1_UART_TX_ENABLED)
            #if(LIN_1_UART_TXCLKGEN_DP)
                LIN_1_UART_backup.tx_clk_ctr = LIN_1_UART_TXBITCLKGEN_CTR_REG;
                LIN_1_UART_backup.tx_clk_compl = LIN_1_UART_TXBITCLKTX_COMPLETE_REG;
            #else
                LIN_1_UART_backup.tx_period = LIN_1_UART_TXBITCTR_PERIOD_REG;
            #endif /*End LIN_1_UART_TXCLKGEN_DP */
            LIN_1_UART_backup.tx_mask = LIN_1_UART_TXSTATUS_MASK_REG;
        #endif /*End LIN_1_UART_TX_ENABLED */


    #else /* CY_UDB_V1 */

        #if(LIN_1_UART_CONTROL_REG_REMOVED == 0u)
            LIN_1_UART_backup.cr = LIN_1_UART_CONTROL_REG;
        #endif /* End LIN_1_UART_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: LIN_1_UART_RestoreConfig
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
*  LIN_1_UART_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_1_UART_RestoreConfig(void)
{

    #if (CY_UDB_V0)

        #if(LIN_1_UART_CONTROL_REG_REMOVED == 0u)
            LIN_1_UART_CONTROL_REG = LIN_1_UART_backup.cr;
        #endif /* End LIN_1_UART_CONTROL_REG_REMOVED */

        #if( (LIN_1_UART_RX_ENABLED) || (LIN_1_UART_HD_ENABLED) )
            LIN_1_UART_RXBITCTR_PERIOD_REG = LIN_1_UART_backup.rx_period;
            LIN_1_UART_RXSTATUS_MASK_REG = LIN_1_UART_backup.rx_mask;
            #if (LIN_1_UART_RXHW_ADDRESS_ENABLED)
                LIN_1_UART_RXADDRESS1_REG = LIN_1_UART_backup.rx_addr1;
                LIN_1_UART_RXADDRESS2_REG = LIN_1_UART_backup.rx_addr2;
            #endif /* End LIN_1_UART_RXHW_ADDRESS_ENABLED */
        #endif  /* End (LIN_1_UART_RX_ENABLED) || (LIN_1_UART_HD_ENABLED) */

        #if(LIN_1_UART_TX_ENABLED)
            #if(LIN_1_UART_TXCLKGEN_DP)
                LIN_1_UART_TXBITCLKGEN_CTR_REG = LIN_1_UART_backup.tx_clk_ctr;
                LIN_1_UART_TXBITCLKTX_COMPLETE_REG = LIN_1_UART_backup.tx_clk_compl;
            #else
                LIN_1_UART_TXBITCTR_PERIOD_REG = LIN_1_UART_backup.tx_period;
            #endif /*End LIN_1_UART_TXCLKGEN_DP */
            LIN_1_UART_TXSTATUS_MASK_REG = LIN_1_UART_backup.tx_mask;
        #endif /*End LIN_1_UART_TX_ENABLED */

    #else /* CY_UDB_V1 */

        #if(LIN_1_UART_CONTROL_REG_REMOVED == 0u)
            LIN_1_UART_CONTROL_REG = LIN_1_UART_backup.cr;
        #endif /* End LIN_1_UART_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: LIN_1_UART_Sleep
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
*  LIN_1_UART_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_1_UART_Sleep(void)
{

    #if(LIN_1_UART_RX_ENABLED || LIN_1_UART_HD_ENABLED)
        if((LIN_1_UART_RXSTATUS_ACTL_REG  & LIN_1_UART_INT_ENABLE) != 0u)
        {
            LIN_1_UART_backup.enableState = 1u;
        }
        else
        {
            LIN_1_UART_backup.enableState = 0u;
        }
    #else
        if((LIN_1_UART_TXSTATUS_ACTL_REG  & LIN_1_UART_INT_ENABLE) !=0u)
        {
            LIN_1_UART_backup.enableState = 1u;
        }
        else
        {
            LIN_1_UART_backup.enableState = 0u;
        }
    #endif /* End LIN_1_UART_RX_ENABLED || LIN_1_UART_HD_ENABLED*/

    LIN_1_UART_Stop();
    LIN_1_UART_SaveConfig();
}


/*******************************************************************************
* Function Name: LIN_1_UART_Wakeup
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
*  LIN_1_UART_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void LIN_1_UART_Wakeup(void)
{
    LIN_1_UART_RestoreConfig();
    #if( (LIN_1_UART_RX_ENABLED) || (LIN_1_UART_HD_ENABLED) )
        LIN_1_UART_ClearRxBuffer();
    #endif /* End (LIN_1_UART_RX_ENABLED) || (LIN_1_UART_HD_ENABLED) */
    #if(LIN_1_UART_TX_ENABLED || LIN_1_UART_HD_ENABLED)
        LIN_1_UART_ClearTxBuffer();
    #endif /* End LIN_1_UART_TX_ENABLED || LIN_1_UART_HD_ENABLED */

    if(LIN_1_UART_backup.enableState != 0u)
    {
        LIN_1_UART_Enable();
    }
}


/* [] END OF FILE */
