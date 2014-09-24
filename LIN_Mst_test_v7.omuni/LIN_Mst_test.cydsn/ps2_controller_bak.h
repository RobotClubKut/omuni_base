
#if !defined(PS2_CONTROLLER_H)
#define PS2_CONTROLLER_H
	
#include <project.h>

/* PSコントローラーボタン定義 */
	
#define up         ((PS_data[0]&0x10)==0x10)
#define down       ((PS_data[0]&0x40)==0x40)
#define right      ((PS_data[0]&0x20)==0x20)
#define left       ((PS_data[0]&0x80)==0x80)

#define triangle   ((PS_data[1]&0x10)==0x10)
#define cross      ((PS_data[1]&0x40)==0x40)
#define circle     ((PS_data[1]&0x20)==0x20)
#define square     ((PS_data[1]&0x80)==0x80)

#define l1         ((PS_data[1]&0x04)==0x04)
#define r1	       ((PS_data[1]&0x08)==0x08)
#define l2	       ((PS_data[1]&0x01)==0x01)
#define r2	       ((PS_data[1]&0x02)==0x02)
#define l3	       ((PS_data[0]&0x02)==0x02)	//L3, R3はアナログスティックの押しこみボタン
#define r3	       ((PS_data[0]&0x04)==0x04)

#define start	   ((PS_data[0]&0x08)==0x08)
#define select     ((PS_data[0]&0x01)==0x01)

#define analog_r_x   PS_data[2]   // 左0 < 中心127 < 右255 
#define analog_r_y   PS_data[3]   // 上0 < 中心127 < 下255

#define analog_l_x   PS_data[4]   // 左0 < 中心127 < 右255 
#define analog_l_y   PS_data[5]   // 上0 < 中心127 < 下255	

uint8 PS_Controller_get(uint8* SwitchRecv);

#endif
