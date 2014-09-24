/*******************************************************************************
* File Name: BHI.h  
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

#if !defined(CY_PINS_BHI_H) /* Pins BHI_H */
#define CY_PINS_BHI_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "BHI_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    BHI_Write(uint8 value) ;
void    BHI_SetDriveMode(uint8 mode) ;
uint8   BHI_ReadDataReg(void) ;
uint8   BHI_Read(void) ;
uint8   BHI_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define BHI_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define BHI_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define BHI_DM_RES_UP          PIN_DM_RES_UP
#define BHI_DM_RES_DWN         PIN_DM_RES_DWN
#define BHI_DM_OD_LO           PIN_DM_OD_LO
#define BHI_DM_OD_HI           PIN_DM_OD_HI
#define BHI_DM_STRONG          PIN_DM_STRONG
#define BHI_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define BHI_MASK               BHI__MASK
#define BHI_SHIFT              BHI__SHIFT
#define BHI_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define BHI_PS                     (* (reg8 *) BHI__PS)
/* Data Register */
#define BHI_DR                     (* (reg8 *) BHI__DR)
/* Port Number */
#define BHI_PRT_NUM                (* (reg8 *) BHI__PRT) 
/* Connect to Analog Globals */                                                  
#define BHI_AG                     (* (reg8 *) BHI__AG)                       
/* Analog MUX bux enable */
#define BHI_AMUX                   (* (reg8 *) BHI__AMUX) 
/* Bidirectional Enable */                                                        
#define BHI_BIE                    (* (reg8 *) BHI__BIE)
/* Bit-mask for Aliased Register Access */
#define BHI_BIT_MASK               (* (reg8 *) BHI__BIT_MASK)
/* Bypass Enable */
#define BHI_BYP                    (* (reg8 *) BHI__BYP)
/* Port wide control signals */                                                   
#define BHI_CTL                    (* (reg8 *) BHI__CTL)
/* Drive Modes */
#define BHI_DM0                    (* (reg8 *) BHI__DM0) 
#define BHI_DM1                    (* (reg8 *) BHI__DM1)
#define BHI_DM2                    (* (reg8 *) BHI__DM2) 
/* Input Buffer Disable Override */
#define BHI_INP_DIS                (* (reg8 *) BHI__INP_DIS)
/* LCD Common or Segment Drive */
#define BHI_LCD_COM_SEG            (* (reg8 *) BHI__LCD_COM_SEG)
/* Enable Segment LCD */
#define BHI_LCD_EN                 (* (reg8 *) BHI__LCD_EN)
/* Slew Rate Control */
#define BHI_SLW                    (* (reg8 *) BHI__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define BHI_PRTDSI__CAPS_SEL       (* (reg8 *) BHI__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define BHI_PRTDSI__DBL_SYNC_IN    (* (reg8 *) BHI__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define BHI_PRTDSI__OE_SEL0        (* (reg8 *) BHI__PRTDSI__OE_SEL0) 
#define BHI_PRTDSI__OE_SEL1        (* (reg8 *) BHI__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define BHI_PRTDSI__OUT_SEL0       (* (reg8 *) BHI__PRTDSI__OUT_SEL0) 
#define BHI_PRTDSI__OUT_SEL1       (* (reg8 *) BHI__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define BHI_PRTDSI__SYNC_OUT       (* (reg8 *) BHI__PRTDSI__SYNC_OUT) 


#if defined(BHI__INTSTAT)  /* Interrupt Registers */

    #define BHI_INTSTAT                (* (reg8 *) BHI__INTSTAT)
    #define BHI_SNAP                   (* (reg8 *) BHI__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins BHI_H */


/* [] END OF FILE */
