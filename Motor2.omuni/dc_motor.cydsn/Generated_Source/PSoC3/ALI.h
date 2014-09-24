/*******************************************************************************
* File Name: ALI.h  
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

#if !defined(CY_PINS_ALI_H) /* Pins ALI_H */
#define CY_PINS_ALI_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "ALI_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    ALI_Write(uint8 value) ;
void    ALI_SetDriveMode(uint8 mode) ;
uint8   ALI_ReadDataReg(void) ;
uint8   ALI_Read(void) ;
uint8   ALI_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define ALI_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define ALI_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define ALI_DM_RES_UP          PIN_DM_RES_UP
#define ALI_DM_RES_DWN         PIN_DM_RES_DWN
#define ALI_DM_OD_LO           PIN_DM_OD_LO
#define ALI_DM_OD_HI           PIN_DM_OD_HI
#define ALI_DM_STRONG          PIN_DM_STRONG
#define ALI_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define ALI_MASK               ALI__MASK
#define ALI_SHIFT              ALI__SHIFT
#define ALI_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define ALI_PS                     (* (reg8 *) ALI__PS)
/* Data Register */
#define ALI_DR                     (* (reg8 *) ALI__DR)
/* Port Number */
#define ALI_PRT_NUM                (* (reg8 *) ALI__PRT) 
/* Connect to Analog Globals */                                                  
#define ALI_AG                     (* (reg8 *) ALI__AG)                       
/* Analog MUX bux enable */
#define ALI_AMUX                   (* (reg8 *) ALI__AMUX) 
/* Bidirectional Enable */                                                        
#define ALI_BIE                    (* (reg8 *) ALI__BIE)
/* Bit-mask for Aliased Register Access */
#define ALI_BIT_MASK               (* (reg8 *) ALI__BIT_MASK)
/* Bypass Enable */
#define ALI_BYP                    (* (reg8 *) ALI__BYP)
/* Port wide control signals */                                                   
#define ALI_CTL                    (* (reg8 *) ALI__CTL)
/* Drive Modes */
#define ALI_DM0                    (* (reg8 *) ALI__DM0) 
#define ALI_DM1                    (* (reg8 *) ALI__DM1)
#define ALI_DM2                    (* (reg8 *) ALI__DM2) 
/* Input Buffer Disable Override */
#define ALI_INP_DIS                (* (reg8 *) ALI__INP_DIS)
/* LCD Common or Segment Drive */
#define ALI_LCD_COM_SEG            (* (reg8 *) ALI__LCD_COM_SEG)
/* Enable Segment LCD */
#define ALI_LCD_EN                 (* (reg8 *) ALI__LCD_EN)
/* Slew Rate Control */
#define ALI_SLW                    (* (reg8 *) ALI__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define ALI_PRTDSI__CAPS_SEL       (* (reg8 *) ALI__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define ALI_PRTDSI__DBL_SYNC_IN    (* (reg8 *) ALI__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define ALI_PRTDSI__OE_SEL0        (* (reg8 *) ALI__PRTDSI__OE_SEL0) 
#define ALI_PRTDSI__OE_SEL1        (* (reg8 *) ALI__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define ALI_PRTDSI__OUT_SEL0       (* (reg8 *) ALI__PRTDSI__OUT_SEL0) 
#define ALI_PRTDSI__OUT_SEL1       (* (reg8 *) ALI__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define ALI_PRTDSI__SYNC_OUT       (* (reg8 *) ALI__PRTDSI__SYNC_OUT) 


#if defined(ALI__INTSTAT)  /* Interrupt Registers */

    #define ALI_INTSTAT                (* (reg8 *) ALI__INTSTAT)
    #define ALI_SNAP                   (* (reg8 *) ALI__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins ALI_H */


/* [] END OF FILE */
