/*******************************************************************************
* File Name: AHI.h  
* Version 2.0
*
* Description:
*  This file containts Control Register function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_AHI_H) /* Pins AHI_H */
#define CY_PINS_AHI_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "AHI_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    AHI_Write(uint8 value) ;
void    AHI_SetDriveMode(uint8 mode) ;
uint8   AHI_ReadDataReg(void) ;
uint8   AHI_Read(void) ;
uint8   AHI_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define AHI_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define AHI_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define AHI_DM_RES_UP          PIN_DM_RES_UP
#define AHI_DM_RES_DWN         PIN_DM_RES_DWN
#define AHI_DM_OD_LO           PIN_DM_OD_LO
#define AHI_DM_OD_HI           PIN_DM_OD_HI
#define AHI_DM_STRONG          PIN_DM_STRONG
#define AHI_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define AHI_MASK               AHI__MASK
#define AHI_SHIFT              AHI__SHIFT
#define AHI_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define AHI_PS                     (* (reg8 *) AHI__PS)
/* Data Register */
#define AHI_DR                     (* (reg8 *) AHI__DR)
/* Port Number */
#define AHI_PRT_NUM                (* (reg8 *) AHI__PRT) 
/* Connect to Analog Globals */                                                  
#define AHI_AG                     (* (reg8 *) AHI__AG)                       
/* Analog MUX bux enable */
#define AHI_AMUX                   (* (reg8 *) AHI__AMUX) 
/* Bidirectional Enable */                                                        
#define AHI_BIE                    (* (reg8 *) AHI__BIE)
/* Bit-mask for Aliased Register Access */
#define AHI_BIT_MASK               (* (reg8 *) AHI__BIT_MASK)
/* Bypass Enable */
#define AHI_BYP                    (* (reg8 *) AHI__BYP)
/* Port wide control signals */                                                   
#define AHI_CTL                    (* (reg8 *) AHI__CTL)
/* Drive Modes */
#define AHI_DM0                    (* (reg8 *) AHI__DM0) 
#define AHI_DM1                    (* (reg8 *) AHI__DM1)
#define AHI_DM2                    (* (reg8 *) AHI__DM2) 
/* Input Buffer Disable Override */
#define AHI_INP_DIS                (* (reg8 *) AHI__INP_DIS)
/* LCD Common or Segment Drive */
#define AHI_LCD_COM_SEG            (* (reg8 *) AHI__LCD_COM_SEG)
/* Enable Segment LCD */
#define AHI_LCD_EN                 (* (reg8 *) AHI__LCD_EN)
/* Slew Rate Control */
#define AHI_SLW                    (* (reg8 *) AHI__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define AHI_PRTDSI__CAPS_SEL       (* (reg8 *) AHI__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define AHI_PRTDSI__DBL_SYNC_IN    (* (reg8 *) AHI__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define AHI_PRTDSI__OE_SEL0        (* (reg8 *) AHI__PRTDSI__OE_SEL0) 
#define AHI_PRTDSI__OE_SEL1        (* (reg8 *) AHI__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define AHI_PRTDSI__OUT_SEL0       (* (reg8 *) AHI__PRTDSI__OUT_SEL0) 
#define AHI_PRTDSI__OUT_SEL1       (* (reg8 *) AHI__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define AHI_PRTDSI__SYNC_OUT       (* (reg8 *) AHI__PRTDSI__SYNC_OUT) 


#if defined(AHI__INTSTAT)  /* Interrupt Registers */

    #define AHI_INTSTAT                (* (reg8 *) AHI__INTSTAT)
    #define AHI_SNAP                   (* (reg8 *) AHI__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins AHI_H */


/* [] END OF FILE */
