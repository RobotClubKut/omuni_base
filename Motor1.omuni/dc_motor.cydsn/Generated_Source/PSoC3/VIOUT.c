/*******************************************************************************
* File Name: VIOUT.c  
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
#include "VIOUT.h"


/*******************************************************************************
* Function Name: VIOUT_Write
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
void VIOUT_Write(uint8 value) 
{
    uint8 staticBits = (VIOUT_DR & (uint8)(~VIOUT_MASK));
    VIOUT_DR = staticBits | ((uint8)(value << VIOUT_SHIFT) & VIOUT_MASK);
}


/*******************************************************************************
* Function Name: VIOUT_SetDriveMode
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
void VIOUT_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(VIOUT_0, mode);
}


/*******************************************************************************
* Function Name: VIOUT_Read
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
*  Macro VIOUT_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 VIOUT_Read(void) 
{
    return (VIOUT_PS & VIOUT_MASK) >> VIOUT_SHIFT;
}


/*******************************************************************************
* Function Name: VIOUT_ReadDataReg
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
uint8 VIOUT_ReadDataReg(void) 
{
    return (VIOUT_DR & VIOUT_MASK) >> VIOUT_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(VIOUT_INTSTAT) 

    /*******************************************************************************
    * Function Name: VIOUT_ClearInterrupt
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
    uint8 VIOUT_ClearInterrupt(void) 
    {
        return (VIOUT_INTSTAT & VIOUT_MASK) >> VIOUT_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
