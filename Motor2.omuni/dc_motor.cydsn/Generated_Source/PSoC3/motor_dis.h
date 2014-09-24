/*******************************************************************************
* File Name: motor_dis.h  
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

#if !defined(CY_PINS_motor_dis_H) /* Pins motor_dis_H */
#define CY_PINS_motor_dis_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "motor_dis_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    motor_dis_Write(uint8 value) ;
void    motor_dis_SetDriveMode(uint8 mode) ;
uint8   motor_dis_ReadDataReg(void) ;
uint8   motor_dis_Read(void) ;
uint8   motor_dis_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define motor_dis_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define motor_dis_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define motor_dis_DM_RES_UP          PIN_DM_RES_UP
#define motor_dis_DM_RES_DWN         PIN_DM_RES_DWN
#define motor_dis_DM_OD_LO           PIN_DM_OD_LO
#define motor_dis_DM_OD_HI           PIN_DM_OD_HI
#define motor_dis_DM_STRONG          PIN_DM_STRONG
#define motor_dis_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define motor_dis_MASK               motor_dis__MASK
#define motor_dis_SHIFT              motor_dis__SHIFT
#define motor_dis_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define motor_dis_PS                     (* (reg8 *) motor_dis__PS)
/* Data Register */
#define motor_dis_DR                     (* (reg8 *) motor_dis__DR)
/* Port Number */
#define motor_dis_PRT_NUM                (* (reg8 *) motor_dis__PRT) 
/* Connect to Analog Globals */                                                  
#define motor_dis_AG                     (* (reg8 *) motor_dis__AG)                       
/* Analog MUX bux enable */
#define motor_dis_AMUX                   (* (reg8 *) motor_dis__AMUX) 
/* Bidirectional Enable */                                                        
#define motor_dis_BIE                    (* (reg8 *) motor_dis__BIE)
/* Bit-mask for Aliased Register Access */
#define motor_dis_BIT_MASK               (* (reg8 *) motor_dis__BIT_MASK)
/* Bypass Enable */
#define motor_dis_BYP                    (* (reg8 *) motor_dis__BYP)
/* Port wide control signals */                                                   
#define motor_dis_CTL                    (* (reg8 *) motor_dis__CTL)
/* Drive Modes */
#define motor_dis_DM0                    (* (reg8 *) motor_dis__DM0) 
#define motor_dis_DM1                    (* (reg8 *) motor_dis__DM1)
#define motor_dis_DM2                    (* (reg8 *) motor_dis__DM2) 
/* Input Buffer Disable Override */
#define motor_dis_INP_DIS                (* (reg8 *) motor_dis__INP_DIS)
/* LCD Common or Segment Drive */
#define motor_dis_LCD_COM_SEG            (* (reg8 *) motor_dis__LCD_COM_SEG)
/* Enable Segment LCD */
#define motor_dis_LCD_EN                 (* (reg8 *) motor_dis__LCD_EN)
/* Slew Rate Control */
#define motor_dis_SLW                    (* (reg8 *) motor_dis__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define motor_dis_PRTDSI__CAPS_SEL       (* (reg8 *) motor_dis__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define motor_dis_PRTDSI__DBL_SYNC_IN    (* (reg8 *) motor_dis__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define motor_dis_PRTDSI__OE_SEL0        (* (reg8 *) motor_dis__PRTDSI__OE_SEL0) 
#define motor_dis_PRTDSI__OE_SEL1        (* (reg8 *) motor_dis__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define motor_dis_PRTDSI__OUT_SEL0       (* (reg8 *) motor_dis__PRTDSI__OUT_SEL0) 
#define motor_dis_PRTDSI__OUT_SEL1       (* (reg8 *) motor_dis__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define motor_dis_PRTDSI__SYNC_OUT       (* (reg8 *) motor_dis__PRTDSI__SYNC_OUT) 


#if defined(motor_dis__INTSTAT)  /* Interrupt Registers */

    #define motor_dis_INTSTAT                (* (reg8 *) motor_dis__INTSTAT)
    #define motor_dis_SNAP                   (* (reg8 *) motor_dis__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins motor_dis_H */


/* [] END OF FILE */
