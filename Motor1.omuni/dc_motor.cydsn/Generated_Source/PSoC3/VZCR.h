/*******************************************************************************
* File Name: VZCR.h  
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

#if !defined(CY_PINS_VZCR_H) /* Pins VZCR_H */
#define CY_PINS_VZCR_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "VZCR_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    VZCR_Write(uint8 value) ;
void    VZCR_SetDriveMode(uint8 mode) ;
uint8   VZCR_ReadDataReg(void) ;
uint8   VZCR_Read(void) ;
uint8   VZCR_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define VZCR_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define VZCR_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define VZCR_DM_RES_UP          PIN_DM_RES_UP
#define VZCR_DM_RES_DWN         PIN_DM_RES_DWN
#define VZCR_DM_OD_LO           PIN_DM_OD_LO
#define VZCR_DM_OD_HI           PIN_DM_OD_HI
#define VZCR_DM_STRONG          PIN_DM_STRONG
#define VZCR_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define VZCR_MASK               VZCR__MASK
#define VZCR_SHIFT              VZCR__SHIFT
#define VZCR_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define VZCR_PS                     (* (reg8 *) VZCR__PS)
/* Data Register */
#define VZCR_DR                     (* (reg8 *) VZCR__DR)
/* Port Number */
#define VZCR_PRT_NUM                (* (reg8 *) VZCR__PRT) 
/* Connect to Analog Globals */                                                  
#define VZCR_AG                     (* (reg8 *) VZCR__AG)                       
/* Analog MUX bux enable */
#define VZCR_AMUX                   (* (reg8 *) VZCR__AMUX) 
/* Bidirectional Enable */                                                        
#define VZCR_BIE                    (* (reg8 *) VZCR__BIE)
/* Bit-mask for Aliased Register Access */
#define VZCR_BIT_MASK               (* (reg8 *) VZCR__BIT_MASK)
/* Bypass Enable */
#define VZCR_BYP                    (* (reg8 *) VZCR__BYP)
/* Port wide control signals */                                                   
#define VZCR_CTL                    (* (reg8 *) VZCR__CTL)
/* Drive Modes */
#define VZCR_DM0                    (* (reg8 *) VZCR__DM0) 
#define VZCR_DM1                    (* (reg8 *) VZCR__DM1)
#define VZCR_DM2                    (* (reg8 *) VZCR__DM2) 
/* Input Buffer Disable Override */
#define VZCR_INP_DIS                (* (reg8 *) VZCR__INP_DIS)
/* LCD Common or Segment Drive */
#define VZCR_LCD_COM_SEG            (* (reg8 *) VZCR__LCD_COM_SEG)
/* Enable Segment LCD */
#define VZCR_LCD_EN                 (* (reg8 *) VZCR__LCD_EN)
/* Slew Rate Control */
#define VZCR_SLW                    (* (reg8 *) VZCR__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define VZCR_PRTDSI__CAPS_SEL       (* (reg8 *) VZCR__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define VZCR_PRTDSI__DBL_SYNC_IN    (* (reg8 *) VZCR__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define VZCR_PRTDSI__OE_SEL0        (* (reg8 *) VZCR__PRTDSI__OE_SEL0) 
#define VZCR_PRTDSI__OE_SEL1        (* (reg8 *) VZCR__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define VZCR_PRTDSI__OUT_SEL0       (* (reg8 *) VZCR__PRTDSI__OUT_SEL0) 
#define VZCR_PRTDSI__OUT_SEL1       (* (reg8 *) VZCR__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define VZCR_PRTDSI__SYNC_OUT       (* (reg8 *) VZCR__PRTDSI__SYNC_OUT) 


#if defined(VZCR__INTSTAT)  /* Interrupt Registers */

    #define VZCR_INTSTAT                (* (reg8 *) VZCR__INTSTAT)
    #define VZCR_SNAP                   (* (reg8 *) VZCR__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins VZCR_H */


/* [] END OF FILE */
