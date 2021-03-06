/*******************************************************************************
* File Name: lin_timer_isr.c  
* Version 1.70
*
*  Description:
*   API for controlling the state of an interrupt.
*
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include <cydevice_trm.h>
#include <CyLib.h>
#include <lin_timer_isr.h>

#if !defined(lin_timer_isr__REMOVED) /* Check for removal by optimization */

/*******************************************************************************
*  Place your includes, defines and code here 
********************************************************************************/
/* `#START lin_timer_isr_intc` */

/* `#END` */


/*******************************************************************************
* Function Name: lin_timer_isr_Start
********************************************************************************
*
* Summary:
*  Set up the interrupt and enable it.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_Start(void) 
{
    /* For all we know the interrupt is active. */
    lin_timer_isr_Disable();

    /* Set the ISR to point to the lin_timer_isr Interrupt. */
    lin_timer_isr_SetVector(&lin_timer_isr_Interrupt);

    /* Set the priority. */
    lin_timer_isr_SetPriority((uint8)lin_timer_isr_INTC_PRIOR_NUMBER);

    /* Enable it. */
    lin_timer_isr_Enable();
}


/*******************************************************************************
* Function Name: lin_timer_isr_StartEx
********************************************************************************
*
* Summary:
*  Set up the interrupt and enable it.
*
* Parameters:  
*   address: Address of the ISR to set in the interrupt vector table.
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_StartEx(cyisraddress address) 
{
    /* For all we know the interrupt is active. */
    lin_timer_isr_Disable();

    /* Set the ISR to point to the lin_timer_isr Interrupt. */
    lin_timer_isr_SetVector(address);

    /* Set the priority. */
    lin_timer_isr_SetPriority((uint8)lin_timer_isr_INTC_PRIOR_NUMBER);

    /* Enable it. */
    lin_timer_isr_Enable();
}


/*******************************************************************************
* Function Name: lin_timer_isr_Stop
********************************************************************************
*
* Summary:
*   Disables and removes the interrupt.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_Stop(void) 
{
    /* Disable this interrupt. */
    lin_timer_isr_Disable();
}


/*******************************************************************************
* Function Name: lin_timer_isr_Interrupt
********************************************************************************
* Summary:
*   The default Interrupt Service Routine for lin_timer_isr.
*
*   Add custom code between the coments to keep the next version of this file
*   from over writting your code.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(lin_timer_isr_Interrupt)
{
    /*  Place your Interrupt code here. */
    /* `#START lin_timer_isr_Interrupt` */

    /* `#END` */

    /* PSoC3 ES1, ES2 RTC ISR PATCH  */ 
    #if(CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC3)
        #if((CYDEV_CHIP_REVISION_USED <= CYDEV_CHIP_REVISION_3A_ES2) && (lin_timer_isr__ES2_PATCH ))      
            lin_timer_isr_ISR_PATCH();
        #endif /* CYDEV_CHIP_REVISION_USED */
    #endif /* (CYDEV_CHIP_FAMILY_USED == CYDEV_CHIP_FAMILY_PSOC3) */
}


/*******************************************************************************
* Function Name: lin_timer_isr_SetVector
********************************************************************************
*
* Summary:
*   Change the ISR vector for the Interrupt. Note calling lin_timer_isr_Start
*   will override any effect this method would have had. To set the vector 
*   before the component has been started use lin_timer_isr_StartEx instead.
*
* Parameters:
*   address: Address of the ISR to set in the interrupt vector table.
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_SetVector(cyisraddress address) 
{
    CY_SET_REG16(lin_timer_isr_INTC_VECTOR, (uint16) address);
}


/*******************************************************************************
* Function Name: lin_timer_isr_GetVector
********************************************************************************
*
* Summary:
*   Gets the "address" of the current ISR vector for the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   Address of the ISR in the interrupt vector table.
*
*******************************************************************************/
cyisraddress lin_timer_isr_GetVector(void) 
{
    return (cyisraddress) CY_GET_REG16(lin_timer_isr_INTC_VECTOR);
}


/*******************************************************************************
* Function Name: lin_timer_isr_SetPriority
********************************************************************************
*
* Summary:
*   Sets the Priority of the Interrupt. Note calling lin_timer_isr_Start
*   or lin_timer_isr_StartEx will override any effect this method would 
*   have had. This method should only be called after lin_timer_isr_Start or 
*   lin_timer_isr_StartEx has been called. To set the initial
*   priority for the component use the cydwr file in the tool.
*
* Parameters:
*   priority: Priority of the interrupt. 0 - 7, 0 being the highest.
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_SetPriority(uint8 priority) 
{
    *lin_timer_isr_INTC_PRIOR = priority << 5;
}


/*******************************************************************************
* Function Name: lin_timer_isr_GetPriority
********************************************************************************
*
* Summary:
*   Gets the Priority of the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   Priority of the interrupt. 0 - 7, 0 being the highest.
*
*******************************************************************************/
uint8 lin_timer_isr_GetPriority(void) 
{
    uint8 priority;


    priority = *lin_timer_isr_INTC_PRIOR >> 5;

    return priority;
}


/*******************************************************************************
* Function Name: lin_timer_isr_Enable
********************************************************************************
*
* Summary:
*   Enables the interrupt.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_Enable(void) 
{
    /* Enable the general interrupt. */
    *lin_timer_isr_INTC_SET_EN = lin_timer_isr__INTC_MASK;
}


/*******************************************************************************
* Function Name: lin_timer_isr_GetState
********************************************************************************
*
* Summary:
*   Gets the state (enabled, disabled) of the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   1 if enabled, 0 if disabled.
*
*******************************************************************************/
uint8 lin_timer_isr_GetState(void) 
{
    /* Get the state of the general interrupt. */
    return ((*lin_timer_isr_INTC_SET_EN & (uint8)lin_timer_isr__INTC_MASK) != 0u) ? 1u:0u;
}


/*******************************************************************************
* Function Name: lin_timer_isr_Disable
********************************************************************************
*
* Summary:
*   Disables the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_Disable(void) 
{
    /* Disable the general interrupt. */
    *lin_timer_isr_INTC_CLR_EN = lin_timer_isr__INTC_MASK;
}


/*******************************************************************************
* Function Name: lin_timer_isr_SetPending
********************************************************************************
*
* Summary:
*   Causes the Interrupt to enter the pending state, a software method of
*   generating the interrupt.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_SetPending(void) 
{
    *lin_timer_isr_INTC_SET_PD = lin_timer_isr__INTC_MASK;
}


/*******************************************************************************
* Function Name: lin_timer_isr_ClearPending
********************************************************************************
*
* Summary:
*   Clears a pending interrupt.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void lin_timer_isr_ClearPending(void) 
{
    *lin_timer_isr_INTC_CLR_PD = lin_timer_isr__INTC_MASK;
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
