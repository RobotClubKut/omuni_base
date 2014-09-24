/*******************************************************************************
* File Name: UART_PS2_PM.c
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

#include "UART_PS2.h"


/***************************************
* Local data allocation
***************************************/

static UART_PS2_BACKUP_STRUCT  UART_PS2_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: UART_PS2_SaveConfig
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
*  UART_PS2_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void UART_PS2_SaveConfig(void)
{
    #if (CY_UDB_V0)

        #if(UART_PS2_CONTROL_REG_REMOVED == 0u)
            UART_PS2_backup.cr = UART_PS2_CONTROL_REG;
        #endif /* End UART_PS2_CONTROL_REG_REMOVED */

        #if( (UART_PS2_RX_ENABLED) || (UART_PS2_HD_ENABLED) )
            UART_PS2_backup.rx_period = UART_PS2_RXBITCTR_PERIOD_REG;
            UART_PS2_backup.rx_mask = UART_PS2_RXSTATUS_MASK_REG;
            #if (UART_PS2_RXHW_ADDRESS_ENABLED)
                UART_PS2_backup.rx_addr1 = UART_PS2_RXADDRESS1_REG;
                UART_PS2_backup.rx_addr2 = UART_PS2_RXADDRESS2_REG;
            #endif /* End UART_PS2_RXHW_ADDRESS_ENABLED */
        #endif /* End UART_PS2_RX_ENABLED | UART_PS2_HD_ENABLED*/

        #if(UART_PS2_TX_ENABLED)
            #if(UART_PS2_TXCLKGEN_DP)
                UART_PS2_backup.tx_clk_ctr = UART_PS2_TXBITCLKGEN_CTR_REG;
                UART_PS2_backup.tx_clk_compl = UART_PS2_TXBITCLKTX_COMPLETE_REG;
            #else
                UART_PS2_backup.tx_period = UART_PS2_TXBITCTR_PERIOD_REG;
            #endif /*End UART_PS2_TXCLKGEN_DP */
            UART_PS2_backup.tx_mask = UART_PS2_TXSTATUS_MASK_REG;
        #endif /*End UART_PS2_TX_ENABLED */


    #else /* CY_UDB_V1 */

        #if(UART_PS2_CONTROL_REG_REMOVED == 0u)
            UART_PS2_backup.cr = UART_PS2_CONTROL_REG;
        #endif /* End UART_PS2_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: UART_PS2_RestoreConfig
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
*  UART_PS2_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void UART_PS2_RestoreConfig(void)
{

    #if (CY_UDB_V0)

        #if(UART_PS2_CONTROL_REG_REMOVED == 0u)
            UART_PS2_CONTROL_REG = UART_PS2_backup.cr;
        #endif /* End UART_PS2_CONTROL_REG_REMOVED */

        #if( (UART_PS2_RX_ENABLED) || (UART_PS2_HD_ENABLED) )
            UART_PS2_RXBITCTR_PERIOD_REG = UART_PS2_backup.rx_period;
            UART_PS2_RXSTATUS_MASK_REG = UART_PS2_backup.rx_mask;
            #if (UART_PS2_RXHW_ADDRESS_ENABLED)
                UART_PS2_RXADDRESS1_REG = UART_PS2_backup.rx_addr1;
                UART_PS2_RXADDRESS2_REG = UART_PS2_backup.rx_addr2;
            #endif /* End UART_PS2_RXHW_ADDRESS_ENABLED */
        #endif  /* End (UART_PS2_RX_ENABLED) || (UART_PS2_HD_ENABLED) */

        #if(UART_PS2_TX_ENABLED)
            #if(UART_PS2_TXCLKGEN_DP)
                UART_PS2_TXBITCLKGEN_CTR_REG = UART_PS2_backup.tx_clk_ctr;
                UART_PS2_TXBITCLKTX_COMPLETE_REG = UART_PS2_backup.tx_clk_compl;
            #else
                UART_PS2_TXBITCTR_PERIOD_REG = UART_PS2_backup.tx_period;
            #endif /*End UART_PS2_TXCLKGEN_DP */
            UART_PS2_TXSTATUS_MASK_REG = UART_PS2_backup.tx_mask;
        #endif /*End UART_PS2_TX_ENABLED */

    #else /* CY_UDB_V1 */

        #if(UART_PS2_CONTROL_REG_REMOVED == 0u)
            UART_PS2_CONTROL_REG = UART_PS2_backup.cr;
        #endif /* End UART_PS2_CONTROL_REG_REMOVED */

    #endif  /* End CY_UDB_V0 */
}


/*******************************************************************************
* Function Name: UART_PS2_Sleep
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
*  UART_PS2_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void UART_PS2_Sleep(void)
{

    #if(UART_PS2_RX_ENABLED || UART_PS2_HD_ENABLED)
        if((UART_PS2_RXSTATUS_ACTL_REG  & UART_PS2_INT_ENABLE) != 0u)
        {
            UART_PS2_backup.enableState = 1u;
        }
        else
        {
            UART_PS2_backup.enableState = 0u;
        }
    #else
        if((UART_PS2_TXSTATUS_ACTL_REG  & UART_PS2_INT_ENABLE) !=0u)
        {
            UART_PS2_backup.enableState = 1u;
        }
        else
        {
            UART_PS2_backup.enableState = 0u;
        }
    #endif /* End UART_PS2_RX_ENABLED || UART_PS2_HD_ENABLED*/

    UART_PS2_Stop();
    UART_PS2_SaveConfig();
}


/*******************************************************************************
* Function Name: UART_PS2_Wakeup
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
*  UART_PS2_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void UART_PS2_Wakeup(void)
{
    UART_PS2_RestoreConfig();
    #if( (UART_PS2_RX_ENABLED) || (UART_PS2_HD_ENABLED) )
        UART_PS2_ClearRxBuffer();
    #endif /* End (UART_PS2_RX_ENABLED) || (UART_PS2_HD_ENABLED) */
    #if(UART_PS2_TX_ENABLED || UART_PS2_HD_ENABLED)
        UART_PS2_ClearTxBuffer();
    #endif /* End UART_PS2_TX_ENABLED || UART_PS2_HD_ENABLED */

    if(UART_PS2_backup.enableState != 0u)
    {
        UART_PS2_Enable();
    }
}


/* [] END OF FILE */
