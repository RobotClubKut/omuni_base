/*******************************************************************************
* File Name: LIN_1_INT.c
* Version 1.30
*
* Description:
*  This file contains the code that operates during the interrupt service
*  routine.
*
********************************************************************************
* Copyright 2011-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "LIN_1.h"

CY_ISR(LIN_1_BLIN_ISR)
{
    l_ifc_aux_LIN_1();
}


CY_ISR(LIN_1_UART_ISR)
{
    l_ifc_rx_LIN_1();
}


/* [] END OF FILE */
