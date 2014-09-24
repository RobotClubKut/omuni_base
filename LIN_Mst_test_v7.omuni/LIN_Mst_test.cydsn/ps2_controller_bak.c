#include <project.h>
#include "ps2_controller.h"

uint8 PS_Controller_get(uint8* SwitchRecv)
{
    uint16 buff;
    uint16 udata;
    uint16 temp;
    uint8 i = 0;
    uint16 timeout_t = 0;
    PS2_ClearRxBuffer();
    for(i = 0;i < 12;i++)
    {
        //if(i >= 8)
        //{
        //    return 1;
        //}
        timeout_t = 0;
        while(PS2_GetRxBufferSize() <= 8)
        {
            if(timeout_t < 10000)
            {
                timeout_t++;
            }
            else
            {
                return 2;
            }
            CyDelayUs(1);
        }
        if((PS2_ReadRxData() & 0x0F) == 0x0F)
        {
            break;
        }
    }
    for(i = 0;i < 12;i++)
    {
        timeout_t = 0;
        while(PS2_GetRxBufferSize() <= 8)
        {
            if(timeout_t < 10000)
            {
                timeout_t++;
            }
            else
            {
                return 3;
            }
            CyDelayUs(1);
        }
        udata = PS2_ReadRxData();
        if(!(udata & 0x08)){
            buff = udata;
            continue;
        }
        if((udata & 0x07)==(buff & 0x07)){
            temp  = buff & 0xF0;
            temp |= udata >> 4;
            SwitchRecv[udata & 0x07] = temp;
        }
        /*
        if(udata & 0x0f == 0x0f)
        {
            return 0;
        }
        */
    }
    return -1;
}