/*******************************************************************************
* File Name: VIOUT.h  
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

#if !defined(CY_PINS_VIOUT_H) /* Pins VIOUT_H */
#define CY_PINS_VIOUT_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "VIOUT_aliases.h"


/***************************************
*        Function Prototypes             
***************************************/    

void    VIOUT_Write(uint8 value) ;
void    VIOUT_SetDriveMode(uint8 mode) ;
uint8   VIOUT_ReadDataReg(void) ;
uint8   VIOUT_Read(void) ;
uint8   VIOUT_ClearInterrupt(void) ;


/***************************************
*           API Constants        
***************************************/

/* Drive Modes */
#define VIOUT_DM_ALG_HIZ         PIN_DM_ALG_HIZ
#define VIOUT_DM_DIG_HIZ         PIN_DM_DIG_HIZ
#define VIOUT_DM_RES_UP          PIN_DM_RES_UP
#define VIOUT_DM_RES_DWN         PIN_DM_RES_DWN
#define VIOUT_DM_OD_LO           PIN_DM_OD_LO
#define VIOUT_DM_OD_HI           PIN_DM_OD_HI
#define VIOUT_DM_STRONG          PIN_DM_STRONG
#define VIOUT_DM_RES_UPDWN       PIN_DM_RES_UPDWN

/* Digital Port Constants */
#define VIOUT_MASK               VIOUT__MASK
#define VIOUT_SHIFT              VIOUT__SHIFT
#define VIOUT_WIDTH              1u


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define VIOUT_PS                     (* (reg8 *) VIOUT__PS)
/* Data Register */
#define VIOUT_DR                     (* (reg8 *) VIOUT__DR)
/* Port Number */
#define VIOUT_PRT_NUM                (* (reg8 *) VIOUT__PRT) 
/* Connect to Analog Globals */                                                  
#define VIOUT_AG                     (* (reg8 *) VIOUT__AG)                       
/* Analog MUX bux enable */
#define VIOUT_AMUX                   (* (reg8 *) VIOUT__AMUX) 
/* Bidirectional Enable */                                                        
#define VIOUT_BIE                    (* (reg8 *) VIOUT__BIE)
/* Bit-mask for Aliased Register Access */
#define VIOUT_BIT_MASK               (* (reg8 *) VIOUT__BIT_MASK)
/* Bypass Enable */
#define VIOUT_BYP                    (* (reg8 *) VIOUT__BYP)
/* Port wide control signals */                                                   
#define VIOUT_CTL                    (* (reg8 *) VIOUT__CTL)
/* Drive Modes */
#define VIOUT_DM0                    (* (reg8 *) VIOUT__DM0) 
#define VIOUT_DM1                    (* (reg8 *) VIOUT__DM1)
#define VIOUT_DM2                    (* (reg8 *) VIOUT__DM2) 
/* Input Buffer Disable Override */
#define VIOUT_INP_DIS                (* (reg8 *) VIOUT__INP_DIS)
/* LCD Common or Segment Drive */
#define VIOUT_LCD_COM_SEG            (* (reg8 *) VIOUT__LCD_COM_SEG)
/* Enable Segment LCD */
#define VIOUT_LCD_EN                 (* (reg8 *) VIOUT__LCD_EN)
/* Slew Rate Control */
#define VIOUT_SLW                    (* (reg8 *) VIOUT__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define VIOUT_PRTDSI__CAPS_SEL       (* (reg8 *) VIOUT__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define VIOUT_PRTDSI__DBL_SYNC_IN    (* (reg8 *) VIOUT__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define VIOUT_PRTDSI__OE_SEL0        (* (reg8 *) VIOUT__PRTDSI__OE_SEL0) 
#define VIOUT_PRTDSI__OE_SEL1        (* (reg8 *) VIOUT__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define VIOUT_PRTDSI__OUT_SEL0       (* (reg8 *) VIOUT__PRTDSI__OUT_SEL0) 
#define VIOUT_PRTDSI__OUT_SEL1       (* (reg8 *) VIOUT__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define VIOUT_PRTDSI__SYNC_OUT       (* (reg8 *) VIOUT__PRTDSI__SYNC_OUT) 


#if defined(VIOUT__INTSTAT)  /* Interrupt Registers */

    #define VIOUT_INTSTAT                (* (reg8 *) VIOUT__INTSTAT)
    #define VIOUT_SNAP                   (* (reg8 *) VIOUT__SNAP)

#endif /* Interrupt Registers */

#endif /* End Pins VIOUT_H */


/* [] END OF FILE */
