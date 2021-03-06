/*******************************************************************************
* File Name: B.c  
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
#include "B.h"


/*******************************************************************************
* Function Name: B_Write
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
void B_Write(uint8 value) 
{
    uint8 staticBits = (B_DR & (uint8)(~B_MASK));
    B_DR = staticBits | ((uint8)(value << B_SHIFT) & B_MASK);
}


/*******************************************************************************
* Function Name: B_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to one of the following drive modes.
*
*  B_DM_STRONG     Strong Drive 
*  B_DM_OD_HI      Open Drain, Drives High 
*  B_DM_OD_LO      Open Drain, Drives Low 
*  B_DM_RES_UP     Resistive Pull Up 
*  B_DM_RES_DWN    Resistive Pull Down 
*  B_DM_RES_UPDWN  Resistive Pull Up/Down 
*  B_DM_DIG_HIZ    High Impedance Digital 
*  B_DM_ALG_HIZ    High Impedance Analog 
*
* Return: 
*  None
*
*******************************************************************************/
void B_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(B_0, mode);
}


/*******************************************************************************
* Function Name: B_Read
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
*  Macro B_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 B_Read(void) 
{
    return (B_PS & B_MASK) >> B_SHIFT;
}


/*******************************************************************************
* Function Name: B_ReadDataReg
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
uint8 B_ReadDataReg(void) 
{
    return (B_DR & B_MASK) >> B_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(B_INTSTAT) 

    /*******************************************************************************
    * Function Name: B_ClearInterrupt
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
    uint8 B_ClearInterrupt(void) 
    {
        return (B_INTSTAT & B_MASK) >> B_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
