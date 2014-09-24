/*******************************************************************************
* File Name: AHI.c  
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
#include "AHI.h"


/*******************************************************************************
* Function Name: AHI_Write
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
void AHI_Write(uint8 value) 
{
    uint8 staticBits = (AHI_DR & (uint8)(~AHI_MASK));
    AHI_DR = staticBits | ((uint8)(value << AHI_SHIFT) & AHI_MASK);
}


/*******************************************************************************
* Function Name: AHI_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to one of the following drive modes.
*
*  AHI_DM_STRONG     Strong Drive 
*  AHI_DM_OD_HI      Open Drain, Drives High 
*  AHI_DM_OD_LO      Open Drain, Drives Low 
*  AHI_DM_RES_UP     Resistive Pull Up 
*  AHI_DM_RES_DWN    Resistive Pull Down 
*  AHI_DM_RES_UPDWN  Resistive Pull Up/Down 
*  AHI_DM_DIG_HIZ    High Impedance Digital 
*  AHI_DM_ALG_HIZ    High Impedance Analog 
*
* Return: 
*  None
*
*******************************************************************************/
void AHI_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(AHI_0, mode);
}


/*******************************************************************************
* Function Name: AHI_Read
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
*  Macro AHI_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 AHI_Read(void) 
{
    return (AHI_PS & AHI_MASK) >> AHI_SHIFT;
}


/*******************************************************************************
* Function Name: AHI_ReadDataReg
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
uint8 AHI_ReadDataReg(void) 
{
    return (AHI_DR & AHI_MASK) >> AHI_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(AHI_INTSTAT) 

    /*******************************************************************************
    * Function Name: AHI_ClearInterrupt
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
    uint8 AHI_ClearInterrupt(void) 
    {
        return (AHI_INTSTAT & AHI_MASK) >> AHI_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
