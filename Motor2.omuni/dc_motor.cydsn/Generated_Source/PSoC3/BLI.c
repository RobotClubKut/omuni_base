/*******************************************************************************
* File Name: BLI.c  
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
#include "BLI.h"


/*******************************************************************************
* Function Name: BLI_Write
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
void BLI_Write(uint8 value) 
{
    uint8 staticBits = (BLI_DR & (uint8)(~BLI_MASK));
    BLI_DR = staticBits | ((uint8)(value << BLI_SHIFT) & BLI_MASK);
}


/*******************************************************************************
* Function Name: BLI_SetDriveMode
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
void BLI_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(BLI_0, mode);
}


/*******************************************************************************
* Function Name: BLI_Read
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
*  Macro BLI_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 BLI_Read(void) 
{
    return (BLI_PS & BLI_MASK) >> BLI_SHIFT;
}


/*******************************************************************************
* Function Name: BLI_ReadDataReg
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
uint8 BLI_ReadDataReg(void) 
{
    return (BLI_DR & BLI_MASK) >> BLI_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(BLI_INTSTAT) 

    /*******************************************************************************
    * Function Name: BLI_ClearInterrupt
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
    uint8 BLI_ClearInterrupt(void) 
    {
        return (BLI_INTSTAT & BLI_MASK) >> BLI_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
