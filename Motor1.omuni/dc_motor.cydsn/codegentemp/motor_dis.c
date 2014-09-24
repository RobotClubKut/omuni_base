/*******************************************************************************
* File Name: motor_dis.c  
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
#include "motor_dis.h"


/*******************************************************************************
* Function Name: motor_dis_Write
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
void motor_dis_Write(uint8 value) 
{
    uint8 staticBits = (motor_dis_DR & (uint8)(~motor_dis_MASK));
    motor_dis_DR = staticBits | ((uint8)(value << motor_dis_SHIFT) & motor_dis_MASK);
}


/*******************************************************************************
* Function Name: motor_dis_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to one of the following drive modes.
*
*  motor_dis_DM_STRONG     Strong Drive 
*  motor_dis_DM_OD_HI      Open Drain, Drives High 
*  motor_dis_DM_OD_LO      Open Drain, Drives Low 
*  motor_dis_DM_RES_UP     Resistive Pull Up 
*  motor_dis_DM_RES_DWN    Resistive Pull Down 
*  motor_dis_DM_RES_UPDWN  Resistive Pull Up/Down 
*  motor_dis_DM_DIG_HIZ    High Impedance Digital 
*  motor_dis_DM_ALG_HIZ    High Impedance Analog 
*
* Return: 
*  None
*
*******************************************************************************/
void motor_dis_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(motor_dis_0, mode);
}


/*******************************************************************************
* Function Name: motor_dis_Read
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
*  Macro motor_dis_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 motor_dis_Read(void) 
{
    return (motor_dis_PS & motor_dis_MASK) >> motor_dis_SHIFT;
}


/*******************************************************************************
* Function Name: motor_dis_ReadDataReg
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
uint8 motor_dis_ReadDataReg(void) 
{
    return (motor_dis_DR & motor_dis_MASK) >> motor_dis_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(motor_dis_INTSTAT) 

    /*******************************************************************************
    * Function Name: motor_dis_ClearInterrupt
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
    uint8 motor_dis_ClearInterrupt(void) 
    {
        return (motor_dis_INTSTAT & motor_dis_MASK) >> motor_dis_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
