/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include <stdlib.h>

uint8 buffer[32],Lin_receive_flag = 0;
int16 rpm,test_rpm=3500;
int16 operation = 0;
uint16 i_data,target,t=0;
int16 det_rpm1 = 0,det_rpm2 = 0;
int16 det_operation = 0;
bit timer_flag = 0;
int16 sample_count1 =0,sample_count2 =0;
uint8 Kp=0,Ki=0,Kd=0;


void LIN_Slave_init()
{
	if(0u != l_sys_init()){
		
		CyHalt(0x00);
		
	}
	
	if(0u != l_ifc_init(LIN_1_IFC_HANDLE)){
	
		CyHalt(0x00);
		
	}
}

/////10ms立った時にフラグを上げる////////////

CY_ISR(motor_isr)
{
	
	timer_flag = 1;
	
}


int main()
{
	/////モジュールの宣言///////////////
	
	CyGlobalIntEnable; 
	PWM_A_Start();
	QuadDec_1_Start();
//	ADC_DelSig_1_Start();
	isr_2_StartEx(motor_isr);
	UART_1_Start();
	UART_1_EnableTxInt();
	LIN_Slave_init();
	
	/////メイン処理////////////////
	/////足回りの番号に書き込みするときはLINのSignalの位置を変更する。
	/////足回り1はrpmのdata1,2 足回り2はrpmのdata3,4 足回り3はrpmのdata5,6
    for(;;)
    {
		////電流センサ使ってない///////////////
//		ADC_DelSig_1_StartConvert();
//		while(ADC_DelSig_1_IsEndConversion(ADC_DelSig_1_WAIT_FOR_RESULT) == 0);
//		i_data = ADC_DelSig_1_GetResult16();
		
		/////10msたった時にフラグ開始/////////
		
		if(timer_flag == 1){
			
			/////マスターから目標値取得///////
			
			if(!(l_flg_tst_target() == 0))
			{
				
				target = l_u16_rd_target();
				Lin_receive_flag = 0;
				l_flg_clr_target();
			}
			
			else
			{
				if(Lin_receive_flag < 5)
				{
					
					Lin_receive_flag++;
					
				}
		
			}
			
			if(Lin_receive_flag > 4)
			{
					
				target = 0;
					
			}
			
			
			/////マスターからKp,Ki,Kd値取得///////
			if(l_flg_tst_PID() == 1)
			{
				
				Kp = l_u8_rd_Kp();
				Ki = l_u8_rd_Ki();
				Kd = l_u8_rd_Kd();
				
			}
			
			sample_count1 = QuadDec_1_GetCounter();
			QuadDec_1_SetCounter(0);
			
			
			/////ロリコンフィルタ（値の変化が小さいときサンプルを前の値を計算に使用）//////////////
			
			if((sample_count1 - sample_count2 <= 5) && (sample_count1 - sample_count2 >= -5)){
					
				sample_count2 = sample_count1;
			
				/////操作量を計算////////////////
				
				rpm = sample_count1*15;
				det_rpm1 = target - rpm;
				det_operation = (int16)((((det_rpm1 - det_rpm2) * 0.3) + ((double)det_rpm1 * (double)(Ki * 0.01))));
		    	operation = operation + det_operation;
				det_rpm2 = det_rpm1;
				
			}
				
			
			
			/////モータの出力制限/////////////
			
			if(operation <= -4500)
			{
				
				operation = -4500;
				
			}
			else if(operation >=4500)
			{
				
				operation = 4500;
				
			}
			
			else if((operation <= 10) && (operation >= -10)){
				
				operation = 0;
				
			}
			/////PWM出力値変更（フィードバックが出来なかったらCompareを1と2の番号を入れ替える）///////////
			
			if(operation == 0){
				
				PWM_A_WriteCompare2(5000);
				PWM_A_WriteCompare1(5000);
			
			}
			else{
			
				PWM_A_WriteCompare2(5000+(int16)operation);
				PWM_A_WriteCompare1(5000-(int16)operation);
			
			}
			//////UARTでデバック用///////////
			
			if(UART_1_ReadTxStatus() & UART_1_TX_STS_FIFO_EMPTY)
			{
				
				sprintf(buffer,"%5d , %5d , %5d , %5d\n",(int)Kp,(int)Ki,(int)Kd,target);
				UART_1_PutString(buffer);
				
			}
			
			/////timeをカウントアップ/////////////////
			/*
			t++;
			*/
			/////カウント初期化//////////////////
			/*
			if(t == 60000)
			{
				
				t = 0;
				
			}
			*/
			/////フラグ解除/////////////
			
			timer_flag =0;
			
			/////1つ前のサンプルを保存/////
			
			sample_count2 = sample_count1;
			
		}
	}
}

/* [] END OF FILE */
