/*******************************************************************************
* File Name: Rx_UART.h  
* Version 1.90
*
* Description:
*  This file containts Control Register function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Rx_UART_H) /* Pins Rx_UART_H */
#define CY_PINS_Rx_UART_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Rx_UART_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    Rx_UART_Write(uint8 value) ;
void    Rx_UART_SetDriveMode(uint8 mode) ;
uint8   Rx_UART_ReadDataReg(void) ;
uint8   Rx_UART_Read(void) ;
uint8   Rx_UART_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define Rx_UART_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define Rx_UART_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define Rx_UART_DM_RES_UP          PIN_DM_RES_UP
#define Rx_UART_DM_RES_DWN         PIN_DM_RES_DWN
#define Rx_UART_DM_OD_LO           PIN_DM_OD_LO
#define Rx_UART_DM_OD_HI           PIN_DM_OD_HI
#define Rx_UART_DM_STRONG          PIN_DM_STRONG
#define Rx_UART_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define Rx_UART_MASK               Rx_UART__MASK
#define Rx_UART_SHIFT              Rx_UART__SHIFT
#define Rx_UART_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Rx_UART_PS                     (* (reg8 *) Rx_UART__PS)
/* Data Register */
#define Rx_UART_DR                     (* (reg8 *) Rx_UART__DR)
/* Port Number */
#define Rx_UART_PRT_NUM                (* (reg8 *) Rx_UART__PRT) 
/* Connect to Analog Globals */                                                  
#define Rx_UART_AG                     (* (reg8 *) Rx_UART__AG)                       
/* Analog MUX bux enable */
#define Rx_UART_AMUX                   (* (reg8 *) Rx_UART__AMUX) 
/* Bidirectional Enable */                                                        
#define Rx_UART_BIE                    (* (reg8 *) Rx_UART__BIE)
/* Bit-mask for Aliased Register Access */
#define Rx_UART_BIT_MASK               (* (reg8 *) Rx_UART__BIT_MASK)
/* Bypass Enable */
#define Rx_UART_BYP                    (* (reg8 *) Rx_UART__BYP)
/* Port wide control signals */                                                   
#define Rx_UART_CTL                    (* (reg8 *) Rx_UART__CTL)
/* Drive Modes */
#define Rx_UART_DM0                    (* (reg8 *) Rx_UART__DM0) 
#define Rx_UART_DM1                    (* (reg8 *) Rx_UART__DM1)
#define Rx_UART_DM2                    (* (reg8 *) Rx_UART__DM2) 
/* Input Buffer Disable Override */
#define Rx_UART_INP_DIS                (* (reg8 *) Rx_UART__INP_DIS)
/* LCD Common or Segment Drive */
#define Rx_UART_LCD_COM_SEG            (* (reg8 *) Rx_UART__LCD_COM_SEG)
/* Enable Segment LCD */
#define Rx_UART_LCD_EN                 (* (reg8 *) Rx_UART__LCD_EN)
/* Slew Rate Control */
#define Rx_UART_SLW                    (* (reg8 *) Rx_UART__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Rx_UART_PRTDSI__CAPS_SEL       (* (reg8 *) Rx_UART__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Rx_UART_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Rx_UART__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Rx_UART_PRTDSI__OE_SEL0        (* (reg8 *) Rx_UART__PRTDSI__OE_SEL0) 
#define Rx_UART_PRTDSI__OE_SEL1        (* (reg8 *) Rx_UART__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Rx_UART_PRTDSI__OUT_SEL0       (* (reg8 *) Rx_UART__PRTDSI__OUT_SEL0) 
#define Rx_UART_PRTDSI__OUT_SEL1       (* (reg8 *) Rx_UART__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Rx_UART_PRTDSI__SYNC_OUT       (* (reg8 *) Rx_UART__PRTDSI__SYNC_OUT) 


#if defined(Rx_UART__INTSTAT)  /* Interrupt Registers */

    #define Rx_UART_INTSTAT                (* (reg8 *) Rx_UART__INTSTAT)
    #define Rx_UART_SNAP                   (* (reg8 *) Rx_UART__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins Rx_UART_H */


/* [] END OF FILE */
