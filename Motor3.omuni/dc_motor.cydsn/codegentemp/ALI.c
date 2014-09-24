/*******************************************************************************
* File Name: ALI.c  
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
#include "ALI.h"


/*******************************************************************************
* Function Name: ALI_Write
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
void ALI_Write(uint8 value) 
{
    uint8 staticBits = (ALI_DR & (uint8)(~ALI_MASK));
    ALI_DR = staticBits | ((uint8)(value << ALI_SHIFT) & ALI_MASK);
}


/*******************************************************************************
* Function Name: ALI_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to one of the following drive modes.
*
*  ALI_DM_STRONG     Strong Drive 
*  ALI_DM_OD_HI      Open Drain, Drives High 
*  ALI_DM_OD_LO      Open Drain, Drives Low 
*  ALI_DM_RES_UP     Resistive Pull Up 
*  ALI_DM_RES_DWN    Resistive Pull Down 
*  ALI_DM_RES_UPDWN  Resistive Pull Up/Down 
*  ALI_DM_DIG_HIZ    High Impedance Digital 
*  ALI_DM_ALG_HIZ    High Impedance Analog 
*
* Return: 
*  None
*
*******************************************************************************/
void ALI_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(ALI_0, mode);
}


/*******************************************************************************
* Function Name: ALI_Read
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
*  Macro ALI_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 ALI_Read(void) 
{
    return (ALI_PS & ALI_MASK) >> ALI_SHIFT;
}


/*******************************************************************************
* Function Name: ALI_ReadDataReg
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
uint8 ALI_ReadDataReg(void) 
{
    return (ALI_DR & ALI_MASK) >> ALI_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(ALI_INTSTAT) 

    /*******************************************************************************
    * Function Name: ALI_ClearInterrupt
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
    uint8 ALI_ClearInterrupt(void) 
    {
        return (ALI_INTSTAT & ALI_MASK) >> ALI_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
