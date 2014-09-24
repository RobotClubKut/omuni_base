/*******************************************************************************
* File Name: VZCR.c  
* Version 1.90
*
* Description:
*  This file contains API to enable firmware control of a Pins component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "VZCR.h"


/*******************************************************************************
* Function Name: VZCR_Write
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
void VZCR_Write(uint8 value) 
{
    uint8 staticBits = (VZCR_DR & (uint8)(~VZCR_MASK));
    VZCR_DR = staticBits | ((uint8)(value << VZCR_SHIFT) & VZCR_MASK);
}


/*******************************************************************************
* Function Name: VZCR_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to this drive mode.
*
* Return: 
*  None
*
*******************************************************************************/
void VZCR_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(VZCR_0, mode);
}


/*******************************************************************************
* Function Name: VZCR_Read
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
*  Macro VZCR_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 VZCR_Read(void) 
{
    return (VZCR_PS & VZCR_MASK) >> VZCR_SHIFT;
}


/*******************************************************************************
* Function Name: VZCR_ReadDataReg
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
uint8 VZCR_ReadDataReg(void) 
{
    return (VZCR_DR & VZCR_MASK) >> VZCR_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(VZCR_INTSTAT) 

    /*******************************************************************************
    * Function Name: VZCR_ClearInterrupt
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
    uint8 VZCR_ClearInterrupt(void) 
    {
        return (VZCR_INTSTAT & VZCR_MASK) >> VZCR_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
