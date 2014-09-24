/*******************************************************************************
* File Name: BHI.c  
* Version 2.0
*
* Description:
*  This file contains API to enable firmware control of a Pins component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "BHI.h"


/*******************************************************************************
* Function Name: BHI_Write
********************************************************************************
*
* Summary:
*  Assign a new value to the digital port's data output register.  
*
* Parameters:  
*  prtValue:  The value to be assigned to the Digital Port. 
*
* Return: 
*  None 
*  
*******************************************************************************/
void BHI_Write(uint8 value) 
{
    uint8 staticBits = (BHI_DR & (uint8)(~BHI_MASK));
    BHI_DR = staticBits | ((uint8)(value << BHI_SHIFT) & BHI_MASK);
}


/*******************************************************************************
* Function Name: BHI_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to one of the following drive modes.
*
*  BHI_DM_STRONG     Strong Drive 
*  BHI_DM_OD_HI      Open Drain, Drives High 
*  BHI_DM_OD_LO      Open Drain, Drives Low 
*  BHI_DM_RES_UP     Resistive Pull Up 
*  BHI_DM_RES_DWN    Resistive Pull Down 
*  BHI_DM_RES_UPDWN  Resistive Pull Up/Down 
*  BHI_DM_DIG_HIZ    High Impedance Digital 
*  BHI_DM_ALG_HIZ    High Impedance Analog 
*
* Return: 
*  None
*
*******************************************************************************/
void BHI_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(BHI_0, mode);
}


/*******************************************************************************
* Function Name: BHI_Read
********************************************************************************
*
* Summary:
*  Read the current value on the pins of the Digital Port in right justified 
*  form.
*
* Parameters:  
*  None 
*
* Return: 
*  Returns the current value of the Digital Port as a right justified number
*  
* Note:
*  Macro BHI_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 BHI_Read(void) 
{
    return (BHI_PS & BHI_MASK) >> BHI_SHIFT;
}


/*******************************************************************************
* Function Name: BHI_ReadDataReg
********************************************************************************
*
* Summary:
*  Read the current value assigned to a Digital Port's data output register
*
* Parameters:  
*  None 
*
* Return: 
*  Returns the current value assigned to the Digital Port's data output register
*  
*******************************************************************************/
uint8 BHI_ReadDataReg(void) 
{
    return (BHI_DR & BHI_MASK) >> BHI_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(BHI_INTSTAT) 

    /*******************************************************************************
    * Function Name: BHI_ClearInterrupt
    ********************************************************************************
    *
    * Summary:
    *  Clears any active interrupts attached to port and returns the value of the 
    *  interrupt status register.
    *
    * Parameters:  
    *  None 
    *
    * Return: 
    *  Returns the value of the interrupt status register
    *  
    *******************************************************************************/
    uint8 BHI_ClearInterrupt(void) 
    {
        return (BHI_INTSTAT & BHI_MASK) >> BHI_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
