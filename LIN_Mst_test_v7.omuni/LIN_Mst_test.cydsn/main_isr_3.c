/*
robocon 2014 手動機　メインマスタープログラム
version : 5
ToDo


tatsuya
*/
#include <project.h>
#include <stdio.h>

#include "ps2_controller.h"
#include <math.h>

// etc define
#define MOTOR_DATA  0x1
#define AIR_DATA    0x2
#define RAIL_DATA   0x4
#define K_DATA      0x8

// lin define
#define LIN_RX_EMPTY    	0x01
#define LIN_RX_COMPLETE 	0x02
#define LIN_RX_SEND     	0x04
#define LIN_RX_SUM_ERR  	0x08
#define LIN_RX_TIMEOUT_ERR  0x10
#define LIN_TX_SEND 	1

// lin method
uint16 LIN_Master_id_send(const uint8 id);
uint8 LIN_Master_GetArray(const uint8 id,const uint8 length,const uint8* buffer);
uint8 LIN_Master_ReadRxStatus(void);
uint8 LIN_Master_PutArray(const uint8 id,const uint8 length,const uint8* const buffer);
uint8 LIN_Master_ReadTxStatus(void);

// lin value
uint8 lin_id_flag;
uint8 lin_tx_flag = 0;
uint8 lin_rx_flag = 1;
uint8 lin_rx_sync_flag;
uint16 rx_sum;
uint8 rx_length;
uint8* rx_buffer;
uint8 rx_i;
uint8 i;

uint8 Rxbuffer[10] = {0};
uint8 line_data[2][10];// R:0 L:1
uint8 line_m[2]; // R:0 L:1 
uint16 rori;
union Airdata
{
	uint8 Trans;
	struct
	{
		uint8 TBD : 2;		// 未定
		uint8 blanco1 : 1;	// 上体起こし　中心からみて1番目
		uint8 free : 1;
		uint8 incline : 1;	// 傾斜
		uint8 roll : 1;
		uint8 blanco3 : 1;	// 中心からみて3番目
		uint8 blanco2 : 1;	// 中心からみて2番目
	} Bit;
};
union Raildata
{
	uint8 Trans;
	struct
	{
		uint8 underRail : 6;
		uint8 seesawPush : 1;
		uint8 seesawUp : 1;
	} Bit;
};

typedef struct
{
	uint8 Select;
	union Airdata Air;
	//uint8 Air;
	union Raildata Rail;
	uint8 K[3];
} LinData;

float R_speed = 0.1;
typedef struct
{
	int8 x;
	int8 y;
	int8 m;
	int16 a;
	int16 b;
	int16 c;
	int16 max;
} Speed;

/*
モーターバランス計算
*/

void calcSpeed(Speed * const sp)
{
	const float root3 = pow(3, 0.5);
	const float coef = ((float)sp->max / (128 * (1 + R_speed + root3)));
	const float y_speed = (root3 * (float)sp->y);
	const float m_speed = (R_speed * (float)sp->m);
	
	const float tmpa = (-2 * (float)sp->x) + m_speed;
	const float tmpb = (float)sp->x - y_speed + m_speed;
	const float tmpc = (float)sp->x + y_speed + m_speed;
	
	sp->a = floor(coef * tmpa + 0.5);
	sp->b = floor(coef * tmpb + 0.5);
	sp->c = -floor(coef * tmpc + 0.5);
	
	return;
}
// 左右アナログパッドから足回り速度(x,y,m)を決定
// ppsdata --> omuni
// r   : 旋回比   0 ~ 1
// mag : 速度倍率 0 ~ 1 
void PS2OmuniControl(PS2Controller * ppsData,Speed * omuni,float r,float mag)
{
	if(!PS2_Analog_Flag() || PS2_Timeout_Flag())
	{
		ppsData->ANALOG_LX = 0;
		ppsData->ANALOG_LY = 0;
		ppsData->ANALOG_RX = 0;
		ppsData->ANALOG_RY = 0;
	}
	
	// コントラーラ信号整形
	if((ppsData->ANALOG_LX != 0) || (ppsData->ANALOG_LY != 0))
	{
		if (ppsData->ANALOG_LX > (127 + 40))
			omuni->x = -(ppsData->ANALOG_LX - 128);
		else if (ppsData->ANALOG_LX < (127 - 40))
			omuni->x = (127 - ppsData->ANALOG_LX);
		else
			omuni->x = 0;
		
		if (ppsData->ANALOG_LY > (127 + 40))
			omuni->y = (ppsData->ANALOG_LY - 128);
		else if (ppsData->ANALOG_LY < (127 - 40))
			omuni->y = -(127 - ppsData->ANALOG_LY);
		else
			omuni->y = 0;
	}
	if((ppsData->ANALOG_RX != 0) || (ppsData->ANALOG_RY != 0))
	{
		if (ppsData->ANALOG_RX > (127 + 40))
		{
			omuni->m = -(ppsData->ANALOG_RX - 128);
			R_speed = -((float)omuni->m/128);
		}
		else if (ppsData->ANALOG_RX < (127 - 40))
		{
			omuni->m = 127 - ppsData->ANALOG_RX;
			R_speed = ((float)omuni->m/128);
		}
		else
		{
			omuni->m = 0;
			R_speed = 0;
		}
	}
	if(mag <= 1)
	{
		omuni->x *= mag;
		omuni->y *= mag;
	}
	if(r <= 1)
	{
		R_speed *= r;
	}
}

// PS2コントローラからデータを受け取り整形
// 足回り、pidパラメータ、空気、レール
void PS2ControlMachine(Speed * omuni,LinData* lindata)
{
	static PS2Controller psData;
	static uint8 start_flag = 0;
	static uint8 air_flag = 0;
	static uint8 rail_flag = 0;
	static uint8 up_flag = 0;
	static uint8 down_flag = 0;
	
	psData = PS2_Controller_get();
		
	if(!PS2_Analog_Flag() || PS2_Timeout_Flag())
	{
		psData.ANALOG_LX = 0;
		psData.ANALOG_LY = 0;
		psData.ANALOG_RX = 0;
		psData.ANALOG_RY = 0;
	}
	
	// コントラーラ信号整形
	if((psData.ANALOG_LX != 0) || (psData.ANALOG_LY != 0))
	{
		if (psData.ANALOG_LX > (127 + 40))
			omuni->x = -(psData.ANALOG_LX - 128);
		else if (psData.ANALOG_LX < (127 - 40))
			omuni->x = (127 - psData.ANALOG_LX);
		else
			omuni->x = 0;
		
		if (psData.ANALOG_LY > (127 + 40))
			omuni->y = (psData.ANALOG_LY - 128);
		else if (psData.ANALOG_LY < (127 - 40))
			omuni->y = -(127 - psData.ANALOG_LY);
		else
			omuni->y = 0;
	}
	if((psData.ANALOG_RX != 0) || (psData.ANALOG_RY != 0))
	{
		if (psData.ANALOG_RX > (127 + 40))
		{
			omuni->m = -(psData.ANALOG_RX - 128);
			R_speed = -((float)omuni->m/128);
		}
		else if (psData.ANALOG_RX < (127 - 40))
		{
			omuni->m = 127 - psData.ANALOG_RX;
			R_speed = ((float)omuni->m/128);
		}
		else
		{
			omuni->m = 0;
			R_speed = 0;
		}
	}
    //kp,ki,kd値送信
    if(psData.START)
    {
        if(!start_flag)
        {
            lindata->Select |= K_DATA;
            start_flag = 1;
        }
    }
    else
    {
        start_flag = 0;
    }
	// 空気データ整形
	if(psData.TRIANGLE)
	{
		if(!(air_flag & 0x01))
		{
			if(lindata->Air.Bit.blanco1)
			{
				lindata->Air.Bit.blanco1 = 0;
			}
			else
			{
				lindata->Air.Bit.blanco1 = 1;
			}
			air_flag |= 0x01;
		}
	}
	else
	{
		air_flag &= 0xfe;
	}
	if(psData.CROSS)
	{
		if(!(air_flag & 0x02))
		{
			if(lindata->Air.Bit.blanco2)
			{
				lindata->Air.Bit.blanco2 = 0;
			}
			else
			{
				lindata->Air.Bit.blanco2 = 1;
			}
			air_flag |= 0x02;
		}
	}
	else
	{
		air_flag &= 0xfd;
	}
	if(psData.CIRCLE)
	{
		if(!(air_flag & 0x04))
		{
			if(lindata->Air.Bit.blanco3)
			{
				lindata->Air.Bit.blanco3 = 0;
			}
			else
			{
				lindata->Air.Bit.blanco3 = 1;
			}
			air_flag |= 0x04;
		}
	}
	else
	{
		air_flag &= 0xfb;
	}
	if(psData.SQUARE)
	{
		if(!(air_flag & 0x08))
		{
			if(lindata->Air.Bit.incline)
			{
				lindata->Air.Bit.incline = 0;
			}
			else
			{
				lindata->Air.Bit.incline = 1;
			}
			air_flag |= 0x08;
		}
	}
	else
	{
		air_flag &= 0xf7;
	}
	if(psData.R1)
	{
		if(!(air_flag & 0x10))
		{
			if(lindata->Air.Bit.roll)
			{
				lindata->Air.Bit.roll = 0;
			}
			else
			{
				lindata->Air.Bit.roll = 1;
			}
			air_flag |= 0x10;
		}
	}
	else
	{
		air_flag &= 0xef;
	}
	if(psData.R2)
	{
		if(!(air_flag & 0x20))
		{
			if(lindata->Air.Bit.free)
			{
				lindata->Air.Bit.free = 0;
			}
			else
			{
				lindata->Air.Bit.free = 1;
			}
			air_flag |= 0x20;
		}
	}
	else
	{
		air_flag &= 0xdf;
	}
	// レールデータ整形
	if(psData.L1)
	{
		if(!(air_flag & 0x80))
		{
			if(lindata->Rail.Bit.seesawUp)
			{
				lindata->Rail.Bit.seesawUp = 0;
			}
			else
			{
				lindata->Rail.Bit.seesawUp = 1;
			}
			air_flag |= 0x80;
		}
	}
	else
	{
		air_flag &= ~0x80;
	}
	if(psData.L2)
	{
		if(!(air_flag & 0x40))
		{
			if(lindata->Rail.Bit.seesawPush)
			{
				lindata->Rail.Bit.seesawPush = 0;
			}
			else
			{
				lindata->Rail.Bit.seesawPush = 1;
			}
			air_flag |= 0x40;
		}
	}
	else
	{
		air_flag &= ~0x40;
	}
	
	// レールデータ整形
	
	if(psData.UP)
		lindata->Rail.Bit.underRail = 16;
	else if(psData.DOWN)
		lindata->Rail.Bit.underRail = 2;
	else
		lindata->Rail.Bit.underRail = 8;
	/*
	if(psData.UP)
	{
		if(up_flag)
		{
			up_flag = 0;
			if(lindata->Rail.Bit.underRail < 16)
			{
				lindata->Rail.Bit.underRail += 2;
			}
			else
			{
				lindata->Rail.Bit.underRail = 16;
			}
		}
	}
	else
	{
		up_flag = 1;
	}
	if(psData.DOWN)
	{
		if(down_flag)
		{
			down_flag = 0;
			if(lindata->Rail.Bit.underRail > 2)
			{
				lindata->Rail.Bit.underRail -= 2;
			}
			else
			{
				lindata->Rail.Bit.underRail = 2;
			}
		}
	}
	else
	{
		down_flag = 1;
	}
	*/
}

// 半自動操縦(ライントレース)
void PS2AutoMachine(Speed * omuni,LinData* lindata)
{
	const double line_kp = 0.4;
	int8 line_left = line_data[0][0]-64;
	int8 line_right = line_data[1][0]-64;
	int16 line_roll = (int)((line_right - line_left)*1.2);
	int16 line_center = -(line_right + line_left);
	static PS2Controller psData;
	psData = PS2_Controller_get();
	R_speed = 1;
	
	if(psData.RIGHT)
	{
		omuni->x = -127;
		omuni->y = 0;
		omuni->m = 64;
	}
	else if(psData.LEFT)
	{
		/*
		omuni->x = 127;
		omuni->y = 0;
		omuni->m = -64;
		*/
		omuni->x = 0;
		omuni->y = 0;
		omuni->m = -125;
	}
	else
	{
		if(((line_center <= 10)&&(line_center >= -10)&&(line_roll <= 10)&&(line_roll >= -10)) 
			|| ((line_data[0][0] == 0) || (line_data[1][0] == 0)))
		{
			omuni->x = 0;
			omuni->y = 0;
			omuni->m = 0;
		}
		else
		{
			omuni->x = 0;
			//omuni->m = 0;
			// m
			if(line_roll > 127)
				omuni->m = 127;
			else if(line_roll < -127)
				omuni->m = -127;
			else
				omuni->m = (int)line_roll;
			// y
			if(line_center*line_kp > 127)
				omuni->y = 127;
			else if(line_center*line_kp < -127)
				omuni->y = -127;
			else
			{
				omuni->y = (int)line_center*line_kp;
			}
		}
	}
}

// LINでデータ送信
void LinSendData(Speed * omuni,LinData* lindata)
{
	static uint8 air_data_old;
	static uint8 rail_data_old;
	static uint8 rori_flag;
	uint8 put_data[8];
	// 空気フラグ立て
	if(air_data_old != lindata->Air.Trans)
	{
		lindata->Select |= AIR_DATA;
	}
	air_data_old = lindata->Air.Trans;
	
	// レール&昇降フラグ
	if(rail_data_old != lindata->Rail.Trans)
	{
		// ダメなパターンなのん
		if(lindata->Rail.Bit.seesawPush && lindata->Rail.Bit.seesawUp)
		{
			// 前のデータを採用するのん
			lindata->Rail.Trans = rail_data_old;
		}
		
		lindata->Select |= RAIL_DATA;
	}
	rail_data_old = lindata->Rail.Trans;
	
	// LIN送信
	if(rori_flag == 1)
	{
		if(LIN_Master_ReadTxStatus() != LIN_TX_SEND)
		{
			rori_flag = 0;
		}
	}
	else if(rori_flag == 0)
	{
        if(lindata->Select & K_DATA)
        {
            LIN_Master_PutArray(4, 3, lindata->K);
			lindata->Select |= MOTOR_DATA;
			lindata->Select &= ~K_DATA;
        }
		else if(lindata->Select & AIR_DATA)
		{
			//air_data = 128;
			LIN_Master_PutArray(2, 1, &(lindata->Air.Trans));
			lindata->Select |= MOTOR_DATA;
			lindata->Select &= ~AIR_DATA;
		}
		else if(lindata->Select & RAIL_DATA)
		{
			// 下位6ビット：1~24　上位2ビット motor x2
			LIN_Master_PutArray(5, 1, &lindata->Rail.Trans);
			lindata->Select |= MOTOR_DATA;
			lindata->Select &= ~RAIL_DATA;
		}
		else if((lindata->Select & MOTOR_DATA))
		{
			//omuni.x = (int8)(x_sum/sumcount);
			//omuni.y = (int8)(y_sum/sumcount);
			calcSpeed(omuni);
			put_data[1] = 0xff & (omuni->a >> 8);
			put_data[0] = 0xff & omuni->a;
			put_data[3] = 0xff & (omuni->b >> 8);
			put_data[2] = 0xff & omuni->b;
			put_data[5] = 0xff & (omuni->c >> 8);
			put_data[4] = 0xff & omuni->c;
			LIN_Master_PutArray(0,6,put_data);
			//x_sum = 0;
			//y_sum = 0;
			//sumcount = 0;
		}
		
		rori_flag = 1;
	}
}



/*
CY_ISR(isr_1_isr)
{
	//LIN_Master_GetArray(0,8,Rxbuffer);
}
*/
CY_ISR(uart_tx_isr)
{
	if(LIN_ReadTxStatus() & LIN_TX_STS_FIFO_EMPTY)
	{
		if(lin_id_flag == 1)
		{
			lin_id_flag = 0;
		}
		if(lin_tx_flag == LIN_TX_SEND)
		{
			// 7bit分待つ
			// PSoC Slaveの仕様だと思われる。
			CyDelayUs(365);
			lin_tx_flag = 0;
		}
	}
}

CY_ISR(uart_rx_isr)
{
	if(lin_rx_flag & LIN_RX_SEND)
	{
		if(rx_i < rx_length)
		{
			rx_buffer[rx_i] = LIN_GetByte() & 0xff;
			
			if(lin_rx_sync_flag == 0)
			{
				/*
				if(rx_buffer[rx_i] == 0x55)
				{
					return;
				}
				*/
				lin_rx_sync_flag = 1;
				return;
			}
			
			else if(lin_rx_sync_flag == 1)
			{
				/*
				if(rx_buffer[rx_i] == rx_sum)
				{
					return;
				}
				*/
				lin_rx_sync_flag = 2;
				return;
			}
			
			rx_sum += rx_buffer[rx_i];
			rx_i++;
		}
		else
		{
			LIN_Timer_Stop();
			rx_sum = 0xff & (~( ((rx_sum >> 8) & 0xf) + (rx_sum & 0xff)));
			if((rx_sum & 0xff) == (LIN_GetByte() & 0xff))
			{
				lin_rx_flag &= ~LIN_RX_SEND;
				lin_rx_flag &= ~LIN_RX_SUM_ERR;
				lin_rx_flag &= ~LIN_RX_TIMEOUT_ERR;
				lin_rx_flag |= LIN_RX_COMPLETE;
				lin_rx_flag |= LIN_RX_EMPTY;
			}
			else
			{
				lin_rx_flag &= ~LIN_RX_SEND;
				lin_rx_flag &= ~LIN_RX_COMPLETE;
				lin_rx_flag |= LIN_RX_EMPTY;
				lin_rx_flag |= LIN_RX_SUM_ERR;
			}
		}
	}
}

CY_ISR(lin_timeout_isr)
{
	//char str[40];
	if(lin_rx_flag & LIN_RX_SEND)
	{
		/*
		while(1)
		{
			if(UART_2_ReadTxStatus() & UART_2_TX_STS_FIFO_EMPTY)
			{
				sprintf(str,"err %x \n",(int)lin_rx_flag);
				UART_2_PutString(str);
			}
		}
		*/
		LIN_Timer_WriteCounter(255);
		LIN_Timer_Stop();
		lin_rx_flag &= ~LIN_RX_SEND;
		lin_rx_flag &= ~LIN_RX_COMPLETE;
		lin_rx_flag |= LIN_RX_EMPTY;
		lin_rx_flag |= LIN_RX_TIMEOUT_ERR;
		/*
		LIN_Stop();
		LIN_Start();
		LIN_EnableRxInt();
		LIN_EnableTxInt();
		LIN_ClearRxBuffer();
		LIN_ClearTxBuffer();
		lin_rx_flag = LIN_RX_EMPTY;
		lin_id_flag = 0;
		lin_tx_flag = 0;
		lin_rx_sync_flag = 0;
		rx_sum = 0;
		rx_length = 0;
		rx_buffer = 0;
		rx_i = 0;
		*/
		/*
		sprintf(str,"err\n");
		UART_2_PutString(str);
		*/
	}
}

// Line Scan
CY_ISR(line_isr)
{
	static uint8 line_flag;
	static int line_i;
	uint16 line_get; 
	
	do
	{
		line_get = Line_GetByte();
		
		if(((line_get & 0xff) == 0x82) && (line_flag == 0))
		{
			line_flag = 1;
			line_i = 0;
		}
		else if(((line_get & 0xff) == 0x83) && (line_flag == 1))
		{
			line_flag = 2;
			line_data[0][line_i] = '\0';
			line_m[0] = line_i;
			line_i = 0;
		}
		else if(((line_get & 0xff) == 0x84) && (line_flag == 2))
		{
			line_m[1] = line_i;
			line_flag = 0;
			line_data[1][line_i] = '\0';
		}
		else if(line_flag == 1)
		{
			line_data[0][line_i] = (uint8)(line_get & 0xff);
			line_i++;
		}
		else if(line_flag == 2)
		{
			line_data[1][line_i] = (uint8)(line_get & 0xff);
			line_i++;
		}
	}while(((line_get >> 8) & 0xff) & Line_RX_STS_FIFO_NOTEMPTY);
	return;
}

int main()
{
	enum MODE_STATUS{
	    NOT_STATUS,
	    SEESAW,
	    BLANCO,
	    PAULWALK,
	    JUNGLEGYM
	};
	char str[30];
	uint8 modeSelect = NOT_STATUS;
	uint8 modeCount = 0;
	uint8 circle_flag;
	uint8 cross_flag;
	
	// ps2 controller data
	PS2Controller psData;
	
	LinData lindata;
	
	// speed
	Speed omuni;
	omuni.max = 3000;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    LIN_Start();
	UART_2_Start();
	PS2_Start();
	Line_Start();
	
	uart_tx_isr_StartEx(uart_tx_isr);
	uart_rx_isr_StartEx(uart_rx_isr);
	lin_timer_isr_StartEx(lin_timeout_isr);
	isr_line_StartEx(line_isr);
//	isr_1_StartEx(isr_1_isr);
	UART_2_EnableTxInt();
	LIN_EnableRxInt();
	LIN_EnableTxInt();
	Line_EnableRxInt();
	
	//LIN_Master_PutArray(2,8,Txbuffer);
	
	lindata.Select = MOTOR_DATA;
	// PID data
	// 0.01*x
	lindata.K[0] = 30;  // p
	lindata.K[1] = 30;  // i
	lindata.K[2] = 0;   // d
	lindata.Rail.Bit.underRail = 8;
	
    // analog押すまで待機
    while (!PS2_Analog_Flag());
	
    // startボタンを押すまで待機
    while (!PS2_Controller_get().START);
    
	for(;;)
	{
		// ps2 get
		psData = PS2_Controller_get();
		PS2ControlMachine(&omuni,&lindata);
		LinSendData(&omuni,&lindata);
		
		if(psData.SELECT && psData.TRIANGLE)
			modeSelect = JUNGLEGYM;
		if(psData.SELECT && psData.CIRCLE)
			modeSelect = BLANCO;
		if(psData.SELECT && psData.CROSS)
			modeSelect = SEESAW;
		if(psData.SELECT && psData.SQUARE)
			modeSelect = PAULWALK;
		/*
		if(modeSelect == JUNGLEGYM)
		{
			
		}
		*/
		if(modeSelect == BLANCO)
		{
			// 本当はこの前に子供をブランコに乗せる動きがあるのん
			// init
			lindata.Air.Bit.blanco1 = 1;
			lindata.Air.Bit.blanco2 = 0;
			lindata.Air.Bit.blanco3 = 1;
			lindata.Air.Bit.roll = 0;
			lindata.Air.Bit.incline = 0;
			lindata.Rail.Bit.seesawPush = 0;
			lindata.Rail.Bit.seesawUp = 0;
			//lindata.Rail.Bit.underRail = 24;
			modeCount = 0;
			circle_flag = 0;
			cross_flag  = 0;
			LinSendData(&omuni,&lindata);
			while(1)
			{
				psData = PS2_Controller_get();
				// モードカウントを上げる
				if(psData.CIRCLE)
				{
					if(circle_flag)
					{
						if(modeCount < 9)
							modeCount++;
						circle_flag = 0;
					}
				}
				else
				{
					circle_flag = 1;
				}
				// モードカウントを下げる
				if(psData.CROSS)
				{
					if(cross_flag)
					{
						if(modeCount > 0)
							modeCount--;
						cross_flag = 0;
					}
				}
				else
				{
					cross_flag = 1;
				}
				// モードクリア
				if(psData.START)
				{
					modeSelect = NOT_STATUS;
					break;
				}
				// 自由に動いてブランコをつかむ
				if(modeCount == 0)
				{
					lindata.Air.Bit.blanco1 = 1;
					lindata.Air.Bit.blanco2 = 0;
					lindata.Air.Bit.blanco3 = 1;
					// 旋回比:1 速度倍率:0.3
					PS2OmuniControl(&psData,&omuni,1,0.3);
				}
				// ブランコを引っ掛ける
				else if(modeCount == 1)
				{
					lindata.Air.Bit.blanco1 = 1;
					lindata.Air.Bit.blanco2 = 1;
					lindata.Air.Bit.blanco3 = 1;
					//lindata.Rail.Bit.underRail = 24;	
				}
				// ブランコを引く
				else if(modeCount == 2)
				{
					lindata.Air.Bit.blanco1 = 0;
					lindata.Air.Bit.blanco2 = 1;
					lindata.Air.Bit.blanco3 = 1;
					//lindata.Rail.Bit.underRail = 5;
					PS2OmuniControl(&psData,&omuni,1,0.3);
				}
				// 子供受け取り
				else if(modeCount == 3)
				{
					lindata.Air.Bit.blanco1 = 1;
					lindata.Air.Bit.blanco2 = 0;
					lindata.Air.Bit.blanco3 = 0;
					//lindata.Rail.Bit.underRail = 5;	
					PS2OmuniControl(&psData,&omuni,1,0.3);
				}
				else
				{
					modeSelect = NOT_STATUS;
					break;
				}
				LinSendData(&omuni,&lindata);
			}
		}
		
		if(modeSelect == SEESAW)
		{
			// init
			lindata.Air.Bit.blanco1 = 1;
			lindata.Air.Bit.blanco2 = 0;
			lindata.Air.Bit.blanco3 = 0;
			lindata.Air.Bit.roll = 0;
			lindata.Air.Bit.incline = 0;
			lindata.Rail.Bit.seesawPush = 0;
			lindata.Rail.Bit.seesawUp = 0;
			//lindata.Rail.Bit.underRail = 16;
			LinSendData(&omuni,&lindata);
			modeCount = 0;
			circle_flag = 0;
			cross_flag  = 0;
			LinSendData(&omuni,&lindata);
			//CyDelay(1000);
			//while()
			// 手動モード
			//while()
			// 半自動モード
			while(1)
			{
				psData = PS2_Controller_get();
				//PS2AutoMachine(&omuni,&lindata);
				PS2ControlMachine(&omuni,&lindata);
				// モードカウントを上げる
				if(psData.CIRCLE)
				{
					if(circle_flag)
					{
						if(modeCount < 10)
							modeCount++;
						circle_flag = 0;
					}
				}
				else
				{
					circle_flag = 1;
				}
				// モードカウントを下げる
				if(psData.CROSS)
				{
					if(cross_flag)
					{
						if(modeCount > 0)
							modeCount--;
						cross_flag = 0;
					}
				}
				else
				{
					cross_flag = 1;
				}
				// モードクリア
				if(psData.START)
				{
					modeSelect = NOT_STATUS;
					break;
				}
				// 自由に動くのん
				if(modeCount == 0)
				{
					PS2OmuniControl(&psData,&omuni,1,0.3);
				}
				// 子供ロボットを置くのん(未実装)
				// 白線をなぞるように動くのん
				else if(modeCount == 1)
				{
					lindata.Air.Bit.blanco1 = 1;
					lindata.Air.Bit.blanco2 = 0;
					lindata.Air.Bit.blanco3 = 0;
					lindata.Rail.Bit.seesawPush = 0;
					lindata.Rail.Bit.seesawUp = 0;
					PS2AutoMachine(&omuni,&lindata);
				}
				// 1ラップ目降下
				else if(modeCount == 2)
					lindata.Rail.Bit.seesawUp = 1;
				// 1ラップ目上昇
				else if(modeCount == 3)
					lindata.Rail.Bit.seesawUp = 0;
				// 2ラップ目降下
				else if(modeCount == 4)
					lindata.Rail.Bit.seesawUp = 1;
				// 2ラップ目上昇
				else if(modeCount == 5)
					lindata.Rail.Bit.seesawUp = 0;
				// 3ラップ目降下
				else if(modeCount == 6)
					lindata.Rail.Bit.seesawUp = 1;
				// 3ラップ目上昇
				else if(modeCount == 7)
					lindata.Rail.Bit.seesawUp = 0;
				// 白線をなぞるように動くのん
				else if(modeCount == 8)
				{
					lindata.Air.Bit.blanco1 = 1;
					lindata.Air.Bit.blanco2 = 0;
					lindata.Air.Bit.blanco3 = 0;
					lindata.Rail.Bit.seesawPush = 0;
					lindata.Rail.Bit.seesawUp = 0;
					PS2AutoMachine(&omuni,&lindata);
				}
				// 子供を回収するのん(未実装)
				// モード終了
				else
				{
					modeSelect = NOT_STATUS;
					break;
				}
				LinSendData(&omuni,&lindata);
			}
		}
		/*
		if(modeSelect == PAULWALK)
		{
			
		}
		*/
		if(UART_2_ReadTxStatus() & UART_2_TX_STS_FIFO_EMPTY)
		{
			//sprintf(str,"%4d,%4d,%4d,%5d,%5d,%5d\n",(int16)omuni.x,(int)omuni.y,(int)omuni.m,(int16)omuni.a,(int16)omuni.b,(int16)omuni.c);
			
			sprintf(str,"%4d\n",(int)lindata.Air.Trans);
			//sprintf(str,"L:%2x,%2x R:%2x,%2x\n",(int)psData.ANALOG_LX,(int)psData.ANALOG_LY,(int)psData.ANALOG_RX,(int)psData.ANALOG_RY);
			//sprintf(str,"R:%4x,%4x  L:%4x,%4x\n",(int)line_data[0][0],(int)line_data[0][1],(int)line_data[1][0],(int)line_data[1][1]);
			//sprintf(str,"Rail = %d\n",(int)lindata.Rail);
			UART_2_PutString(str);
		}
    }
}

uint16 LIN_Master_id_send(const uint8 id)
{
	uint8 p0 = 0;
	uint8 p1 = 0;
	uint8 get_id;
	if(lin_id_flag == 0)
	{
		p0 = (id&0x01)^((id >> 1)&0x01)^((id >> 2)&0x01)^((id >> 4)&0x01);
		p1 = ~(((id >> 1)&0x01)^((id >> 3)&0x01)^((id >> 4)&0x01)^((id >> 5)&0x01));
		get_id = (id&0x3f)|((p0 << 6)&0x40)|((p1 << 7)&0x80);
		LIN_SendBreak(LIN_SEND_WAIT_REINIT);
		CyDelayUs(100);
		LIN_ClearTxBuffer();
		LIN_ClearRxBuffer();
		LIN_PutChar(0x55);
		LIN_PutChar(get_id);
		lin_id_flag = 1;
		return 0xff&get_id;
	}
	else
	{
		return 0x100;
	}
}

uint8 LIN_Master_GetArray(const uint8 id,const uint8 length,const uint8* buffer)
{
	if((lin_rx_flag & LIN_RX_EMPTY) &(lin_tx_flag == 0))
	{
		rx_sum = LIN_Master_id_send(id);
		if(rx_sum == 0x100)
		{
			lin_rx_flag |= LIN_RX_EMPTY;
			lin_rx_flag &= ~LIN_RX_SEND;
			LIN_Timer_Stop();
			return 1;
		}		
		LIN_ClearRxBuffer();
		rx_length = length;
		rx_buffer = buffer;
		rx_i = 0;
		lin_rx_sync_flag = 0;
		lin_rx_flag &= ~LIN_RX_EMPTY;
		lin_rx_flag &= ~LIN_RX_COMPLETE;
		lin_rx_flag |= LIN_RX_SEND;
		LIN_Timer_WriteCounter(255);
		LIN_Timer_Start();

	}
	else
	{
		return 1;
	}
	return 0;
}

uint8 LIN_Master_ReadRxStatus()
{
	uint8 flag = lin_rx_flag;
	//lin_rx_flag &= ~LIN_RX_SUM_ERR; 
	return flag;
}

uint8 LIN_Master_ReadTxStatus()
{
	return lin_tx_flag;
}

uint8 LIN_Master_PutArray(const uint8 id,const uint8 length,const uint8 *buffer)
{
	if((lin_tx_flag == 0))/* && (lin_rx_flag & LIN_RX_EMPTY))*/
	{
		uint8 i;
		uint16 sum;
		LIN_DisableTxInt();
		sum = LIN_Master_id_send(id);
		lin_tx_flag = LIN_TX_SEND;
		if(sum == 0x100)
		{
			LIN_EnableTxInt();
			return 1;
		}
		
		//LIN_ClearTxBuffer();
		for(i = 0; i < length; i++)
		{
			sum += buffer[i];
			LIN_PutChar(buffer[i]);
			//while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));	
		}
		
		sum = 0xff & (~( ((sum >> 8) & 0x7) + (sum & 0xff)));
		LIN_PutChar(sum);
		//while(!(LIN_ReadTxStatus() & LIN_TX_STS_COMPLETE));
		lin_tx_flag = LIN_TX_SEND;
		LIN_EnableTxInt();
		return 0;
	}
	else
	{
		return 1;
	}
}
