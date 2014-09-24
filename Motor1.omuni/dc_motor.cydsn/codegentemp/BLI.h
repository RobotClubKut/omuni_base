/*******************************************************************************
* File Name: BLI.h  
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

#if !defined(CY_PINS_BLI_H) /* Pins BLI_H */
#define CY_PINS_BLI_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "BLI_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    BLI_Write(uint8 value) ;
void    BLI_SetDriveMode(uint8 mode) ;
uint8   BLI_ReadDataReg(void) ;
uint8   BLI_Read(void) ;
uint8   BLI_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define BLI_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define BLI_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define BLI_DM_RES_UP          PIN_DM_RES_UP
#define BLI_DM_RES_DWN         PIN_DM_RES_DWN
#define BLI_DM_OD_LO           PIN_DM_OD_LO
#define BLI_DM_OD_HI           PIN_DM_OD_HI
#define BLI_DM_STRONG          PIN_DM_STRONG
#define BLI_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define BLI_MASK               BLI__MASK
#define BLI_SHIFT              BLI__SHIFT
#define BLI_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define BLI_PS                     (* (reg8 *) BLI__PS)
/* Data Register */
#define BLI_DR                     (* (reg8 *) BLI__DR)
/* Port Number */
#define BLI_PRT_NUM                (* (reg8 *) BLI__PRT) 
/* Connect to Analog Globals */                                                  
#define BLI_AG                     (* (reg8 *) BLI__AG)                       
/* Analog MUX bux enable */
#define BLI_AMUX                   (* (reg8 *) BLI__AMUX) 
/* Bidirectional Enable */                                                        
#define BLI_BIE                    (* (reg8 *) BLI__BIE)
/* Bit-mask for Aliased Register Access */
#define BLI_BIT_MASK               (* (reg8 *) BLI__BIT_MASK)
/* Bypass Enable */
#define BLI_BYP                    (* (reg8 *) BLI__BYP)
/* Port wide control signals */                                                   
#define BLI_CTL                    (* (reg8 *) BLI__CTL)
/* Drive Modes */
#define BLI_DM0                    (* (reg8 *) BLI__DM0) 
#define BLI_DM1                    (* (reg8 *) BLI__DM1)
#define BLI_DM2                    (* (reg8 *) BLI__DM2) 
/* Input Buffer Disable Override */
#define BLI_INP_DIS                (* (reg8 *) BLI__INP_DIS)
/* LCD Common or Segment Drive */
#define BLI_LCD_COM_SEG            (* (reg8 *) BLI__LCD_COM_SEG)
/* Enable Segment LCD */
#define BLI_LCD_EN                 (* (reg8 *) BLI__LCD_EN)
/* Slew Rate Control */
#define BLI_SLW                    (* (reg8 *) BLI__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define BLI_PRTDSI__CAPS_SEL       (* (reg8 *) BLI__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define BLI_PRTDSI__DBL_SYNC_IN    (* (reg8 *) BLI__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define BLI_PRTDSI__OE_SEL0        (* (reg8 *) BLI__PRTDSI__OE_SEL0) 
#define BLI_PRTDSI__OE_SEL1        (* (reg8 *) BLI__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define BLI_PRTDSI__OUT_SEL0       (* (reg8 *) BLI__PRTDSI__OUT_SEL0) 
#define BLI_PRTDSI__OUT_SEL1       (* (reg8 *) BLI__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define BLI_PRTDSI__SYNC_OUT       (* (reg8 *) BLI__PRTDSI__SYNC_OUT) 


#if defined(BLI__INTSTAT)  /* Interrupt Registers */

    #define BLI_INTSTAT                (* (reg8 *) BLI__INTSTAT)
    #define BLI_SNAP                   (* (reg8 *) BLI__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins BLI_H */


/* [] END OF FILE */
