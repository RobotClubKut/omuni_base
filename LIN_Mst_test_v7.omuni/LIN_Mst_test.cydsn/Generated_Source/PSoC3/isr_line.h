/*******************************************************************************
* File Name: isr_line.h
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

#if !defined(CY_ISR_isr_line_H)
#define CY_ISR_isr_line_H

#include <cytypes.h>
#include <cyfitter.h>

/* Interrupt Controller API. */
void isr_line_Start(void) ;
void isr_line_StartEx(cyisraddress address) ;
void isr_line_Stop(void) ;

CY_ISR_PROTO(isr_line_Interrupt);

void isr_line_SetVector(cyisraddress address) ;
cyisraddress isr_line_GetVector(void) ;

void isr_line_SetPriority(uint8 priority) ;
uint8 isr_line_GetPriority(void) ;

void isr_line_Enable(void) ;
uint8 isr_line_GetState(void) ;
void isr_line_Disable(void) ;

void isr_line_SetPending(void) ;
void isr_line_ClearPending(void) ;


/* Interrupt Controller Constants */

/* Address of the INTC.VECT[x] register that contains the Address of the isr_line ISR. */
#define isr_line_INTC_VECTOR            ((reg16 *) isr_line__INTC_VECT)

/* Address of the isr_line ISR priority. */
#define isr_line_INTC_PRIOR             ((reg8 *) isr_line__INTC_PRIOR_REG)

/* Priority of the isr_line interrupt. */
#define isr_line_INTC_PRIOR_NUMBER      isr_line__INTC_PRIOR_NUM

/* Address of the INTC.SET_EN[x] byte to bit enable isr_line interrupt. */
#define isr_line_INTC_SET_EN            ((reg8 *) isr_line__INTC_SET_EN_REG)

/* Address of the INTC.CLR_EN[x] register to bit clear the isr_line interrupt. */
#define isr_line_INTC_CLR_EN            ((reg8 *) isr_line__INTC_CLR_EN_REG)

/* Address of the INTC.SET_PD[x] register to set the isr_line interrupt state to pending. */
#define isr_line_INTC_SET_PD            ((reg8 *) isr_line__INTC_SET_PD_REG)

/* Address of the INTC.CLR_PD[x] register to clear the isr_line interrupt. */
#define isr_line_INTC_CLR_PD            ((reg8 *) isr_line__INTC_CLR_PD_REG)



#endif /* CY_ISR_isr_line_H */


/* [] END OF FILE */
