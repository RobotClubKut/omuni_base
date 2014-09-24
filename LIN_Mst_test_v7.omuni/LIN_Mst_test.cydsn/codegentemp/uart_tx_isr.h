/*******************************************************************************
* File Name: uart_tx_isr.h
* Version 1.70
*
*  Description:
*   Provides the function definitions for the Interrupt Controller.
*
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_ISR_uart_tx_isr_H)
#define CY_ISR_uart_tx_isr_H

#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void uart_tx_isr_Start(void) ;
void uart_tx_isr_StartEx(cyisraddress address) ;
void uart_tx_isr_Stop(void) ;

CY_ISR_PROTO(uart_tx_isr_Interrupt);

void uart_tx_isr_SetVector(cyisraddress address) ;
cyisraddress uart_tx_isr_GetVector(void) ;

void uart_tx_isr_SetPriority(uint8 priority) ;
uint8 uart_tx_isr_GetPriority(void) ;

void uart_tx_isr_Enable(void) ;
uint8 uart_tx_isr_GetState(void) ;
void uart_tx_isr_Disable(void) ;

void uart_tx_isr_SetPending(void) ;
void uart_tx_isr_ClearPending(void) ;


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the uart_tx_isr ISR. */
#define uart_tx_isr_INTC_VECTOR            ((reg16 *) uart_tx_isr__INTC_VECT)

/* Address of the uart_tx_isr ISR priority. */
#define uart_tx_isr_INTC_PRIOR             ((reg8 *) uart_tx_isr__INTC_PRIOR_REG)

/* Priority of the uart_tx_isr interrupt. */
#define uart_tx_isr_INTC_PRIOR_NUMBER      uart_tx_isr__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable uart_tx_isr interrupt. */
#define uart_tx_isr_INTC_SET_EN            ((reg8 *) uart_tx_isr__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the uart_tx_isr interrupt. */
#define uart_tx_isr_INTC_CLR_EN            ((reg8 *) uart_tx_isr__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the uart_tx_isr interrupt state to pending. */
#define uart_tx_isr_INTC_SET_PD            ((reg8 *) uart_tx_isr__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the uart_tx_isr interrupt. */
#define uart_tx_isr_INTC_CLR_PD            ((reg8 *) uart_tx_isr__INTC_CLR_PD_REG)



#endif /* CY_ISR_uart_tx_isr_H */


/* [] END OF FILE */
