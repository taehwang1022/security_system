#include "LPC17xx.h"
#include "Board_LED.h"
#include "LPC17xx_uart.h"

void interrupt_config(void);
void Timer2Start(void);
void Timer3Start(void);
void Timer2Stop(void);
void Timer3Stop(void);
void INT0_On(void);
void UART0_On(void);
void UART1_On(void);
void myprint(char *str);
void reset_box(void);
void return_start(void);
void UART0_IRQHandler(void);
void return_input_adress(void);
void system_down(void);
void myprint_char(char* str);



int32_t receive_text1;
int32_t receive_text0;
int text_box[512];
int text_length;
int counter2;
int counter3;
int wrong_time_count=0;

//flag
int start_flag;
int adress_flag;
int secret_flag;
int password_flag;
int Timer_Stop_flag;
int password_check_flag;
int block_start_flag;
int adress_correct_flag;
int password_correct_flag;

int admin_flag;
int door_close_flag;
int door_open_flag ;
int admin_check_flag;
int Timer3_Stop_flag;

int press_s_flag=1;
int system_lock_flag=0;
int send_secret_flag=0;
int send_address_flag =0;
int second_X_flag;

int adress_incorrect_flag;
int password_incorrect_flag;
int door_open_msg_flag;
int admin_msg_flag;
int adress_time_over_msg_flag=0;
int pasX_msg_block_flag;
// Main function
int main (void) {

	SystemCoreClockUpdate();
	
	LED_Initialize();
	
	interrupt_config();
	
	UART0_On();
	UART1_On();
	while(1){

		if(press_s_flag ==1){
				myprint("\n\r--------------------Press s to start--------------------\n\r");
				press_s_flag=0;
		}
		
		if(start_flag==1&&block_start_flag==0){
				LPC_UART0 -> THR = 'S';
				myprint("\r\nPlease enter your home address :");
				Timer3Start();
				start_flag =0;
				adress_flag =1;
				block_start_flag =1;
		}
		
		if(secret_flag ==1&& adress_correct_flag ==1){
				myprint("\r\nPlease enter your password within 5 seconds :");
				secret_flag=0;
				password_flag=1;
				adress_correct_flag=0;
				Timer3Stop();
				Timer2Start();		
			}
		
		if(Timer_Stop_flag ==1){
			myprint("\r\nTime Over");
			Timer2Stop();
			return_input_adress();
			Timer_Stop_flag=0;
		}
		
		if(Timer3_Stop_flag ==1){
			Timer3Stop();
			Timer3_Stop_flag=0;
			return_start();	
		}
		
		if(adress_time_over_msg_flag==1){
			myprint("\r\nAddress entry time has expired");
			adress_time_over_msg_flag=0;
		}
		
		if(password_check_flag ==1){
			Timer2Stop();
			password_check_flag =0;
		}
		
		if(door_close_flag ==1){
			LED_Off(1);
			myprint("\r\n----------------------[door close]----------------------\r\n");
			door_close_flag=0;
			return_start();
		}
		
		if(admin_flag ==1){
			myprint("\r\nSystem maintenance is underway. Please wait\r\n");
			admin_flag = 0;
			system_down();
		}
		
		if(adress_incorrect_flag==1){
				myprint("\r\nThe address is incorrect \r\n");
				adress_incorrect_flag=0;
		}
		
		if(password_incorrect_flag==1&&pasX_msg_block_flag==0){
				myprint("\r\nThe password is incorrect ");
				password_incorrect_flag=0;
				return_input_adress();
		}
		
		if(door_open_msg_flag==1){
			myprint("\r\n---------------[Welcome the door is open]---------------\r\n");
			door_open_msg_flag=0;
			password_correct_flag =1;
			LED_On(1);
			door_open_flag=1;
		}
		
		if(admin_msg_flag==1&&system_lock_flag==0){
				myprint("\r\nSystem maintenance is underway. Please wait\r\n");
				admin_msg_flag=0;
		}
		
	}
	
}

// IRQ Handler
void TIMER2_IRQHandler (void) {
			LPC_TIM2->IR = 0x00000001; //MR0 interrupt flag
			LPC_TIM2->TC = 0;
			LPC_TIM2->PC = 0;
			LPC_TIM2->TCR = 0x00000001;
			counter2++;
			if(counter2 ==500){
				Timer_Stop_flag =1;
				counter2 =0;
			}
			
}
void TIMER3_IRQHandler (void) {
			LPC_TIM3->IR = 0x00000001; //MR0 interrupt flag
			LPC_TIM3->TC = 0;
			LPC_TIM3->PC = 0;
			LPC_TIM3->TCR = 0x00000001;
			counter3++;
			if(counter3 ==1000){
				adress_time_over_msg_flag=1;
				Timer3_Stop_flag =1;
				counter3 =0;
			}
			
}
void UART0_IRQHandler(void){
			uint32_t status = LPC_UART0 -> IIR;
			if((status&0x04) == 0x04){
				receive_text0 = LPC_UART0 -> RBR;
				
				if(receive_text0 =='X'&&second_X_flag==0){
					Timer3_Stop_flag=1;
					adress_incorrect_flag=1;
				}
				
				if(receive_text0 =='O'&&secret_flag ==1){
					adress_correct_flag =1;
					second_X_flag=1;
					
				}
				
				if(receive_text0 =='X'&&admin_check_flag==1){
					admin_check_flag=0;
					password_incorrect_flag=1;
				}
				
				if(receive_text0 =='O'&&admin_check_flag ==1){
					door_open_msg_flag=1;
					admin_check_flag =0;
				}
				
				if(receive_text0 =='C'&&door_open_flag ==1){
					door_close_flag=1;
					door_open_flag =0;
				}
				
				if(receive_text0 =='L'){
					system_down();
					system_lock_flag =1;
				}
				
				if(receive_text0 =='A'){
					if(system_lock_flag==0){
					admin_msg_flag=1;
					}
					
					system_down();
				}
				
				if(receive_text0 =='R'&&system_lock_flag==0){
					return_start();
				}
				
				if(receive_text0 =='U'&&system_lock_flag==1){
					system_lock_flag=0;
				}
				
			}
}
void UART1_IRQHandler(void){
		uint32_t status = LPC_UART1 -> IIR;
	
		if((status&0x04) == 0x04){
				receive_text1 = LPC_UART1 -> RBR;
			if(receive_text1<'A' || receive_text1 > 'Z'){
				if(receive_text1  == 's'&& press_s_flag==0&&block_start_flag==0){
					start_flag =1;
				}
				if(adress_flag==1&&start_flag==0){
						if(receive_text1 == 13){
							LPC_UART0 -> THR = 'E';
							adress_flag =0;
							secret_flag=1;
						}
						else{
							LPC_UART1 -> THR = receive_text1;
							LPC_UART0 -> THR = receive_text1;
						}
						
				}
				if(password_flag==1){
					if(receive_text1 == 13){
						LPC_UART0 -> THR = 'E';
						password_flag =0;
						password_check_flag =1;
						admin_check_flag=1;
					}
					else{
					LPC_UART1 -> THR = receive_text1;
					LPC_UART0 -> THR = receive_text1;
					}
				}
			}
		}
		else if ( (status & 0x02) == 0x02 ) {
			if (text_length > 0){
				int i;
				LPC_UART1 -> THR = text_box[0];
        for (i = 0; i < text_length - 1; i++){
					text_box[i] = text_box[i + 1];
        }
        text_length--;
			}
		}
}

//funtion
void system_down(){
	password_flag=0;
	start_flag=0;
	secret_flag=0;
	password_check_flag =0;
	block_start_flag=1;
	Timer3Stop();
	Timer2Stop();
}

void return_start(void){
	LPC_UART0 -> THR = 'T';
	password_incorrect_flag=0;
	start_flag=0;
	adress_flag=0;
	secret_flag=0;
	password_flag=0;
	Timer_Stop_flag=0;
	password_check_flag=0;
	block_start_flag =0;
	adress_correct_flag=0;
	password_correct_flag=0;

	admin_flag=0;
	door_close_flag=0;
	door_open_flag=0;
	admin_check_flag=0;
	Timer3_Stop_flag=0;

	press_s_flag=1;
	system_lock_flag=0;
	send_secret_flag=0;
	send_address_flag =0;
	second_X_flag=0;

	adress_incorrect_flag=0;
	password_incorrect_flag=0;
	door_open_msg_flag=0;
	admin_msg_flag=0;
	adress_time_over_msg_flag=0;
	pasX_msg_block_flag=0;
	wrong_time_count=0;
}

void return_input_adress(void){
	wrong_time_count++;
	if(wrong_time_count ==3){
		LPC_UART0 -> THR = 'L';
		myprint("\n\rPassword error occurs 3 times. Contact the management office.\r\n");
		pasX_msg_block_flag=1;
		system_lock_flag=1;
		system_down();
		wrong_time_count=0;
	}
	else{
		adress_correct_flag=1;
		secret_flag=1;
		Timer2Stop();
	}
	
}
void myprint(char* str){
	int i = 0;
	while (str[i] != '\0'){
		text_box[text_length] = str[i];
		text_length++;
		i++;
	}
	LPC_UART1 -> THR=0;
}
//register setting
void interrupt_config (void) {	
	NVIC_ClearPendingIRQ (TIMER2_IRQn);
  NVIC_EnableIRQ (TIMER2_IRQn);
	NVIC ->ICPR[0] |= (1<<4);
	NVIC ->ISER[0] |= (1<<4);
}

void Timer3Start(void){
	LPC_SC->PCONP |= (1<<23);		//timer3 power/clock control bit
  LPC_SC->PCLKSEL1 |= (1<<14);
	LPC_TIM3->IR = 0xFFFFFFFF;	// MR0 interrupt flag  
	LPC_TIM3->TC = 0;
	LPC_TIM3->PC = 0;
	LPC_TIM3->PR = 0;
	LPC_TIM3->TCR = 0x00000002;  //Reset
  LPC_TIM3->CTCR = 0x00000000; //Timer mode 00
  LPC_TIM3->PR = (SystemCoreClock / 1000000)-1;   //PR=99 100->1us 100,000,000 ->1蟾
  LPC_TIM3->MR0 = 0x0000270F;	//1us
  LPC_TIM3->MCR = 0x00000005;;  // MR0 
  LPC_TIM3->TCR = 0x00000001;  //Timer enalbe
}

void Timer3Stop(void){
	LPC_TIM3->TCR = 0x00000000;
	LPC_TIM3->TC = 0;
	LPC_TIM3->PC = 0;
	counter3=0;
}

void Timer2Start(void){
	LPC_SC->PCONP |= (1<<22);		//timer2 power/clock control bit
  LPC_SC->PCLKSEL1 |= (1<<12);
	LPC_TIM2->IR = 0xFFFFFFFF;	// MR0 interrupt flag  
	LPC_TIM2->TC = 0;
	LPC_TIM2->PC = 0;
	LPC_TIM2->PR = 0;
	LPC_TIM2->TCR = 0x00000002;  //Reset
  LPC_TIM2->CTCR = 0x00000000; //Timer mode 00
  LPC_TIM2->PR = (SystemCoreClock / 1000000)-1;   //PR=99 100->1us 100,000,000 ->1蟾
  LPC_TIM2->MR0 = 0x0000270F;	//1us
  LPC_TIM2->MCR = 0x00000005;;  // MR0 
  LPC_TIM2->TCR = 0x00000001;  //Timer enalbe
}

void Timer2Stop(void){
	counter2=0;
	LPC_TIM2->TCR = 0x00000000;
	LPC_TIM2->TC = 0;
	LPC_TIM2->PC = 0;
}

void UART0_On(void){
	UART_CFG_Type UART_config_struct;

	LPC_SC->PCONP |= (3<<3);
  LPC_SC->PCLKSEL0 |= (0<<6);
  LPC_PINCON->PINSEL0 |= (1<<4);
  LPC_PINCON->PINSEL0 |= (1<<6); 
	NVIC->ICPR[0] |= (1<<5);
	NVIC->ISER[0] |= (1<<5);
	UART_config_struct.Baud_rate = 19200;
  UART_config_struct.Parity = UART_PARITY_NONE;
  UART_config_struct.Databits = UART_DATABIT_8;
  UART_config_struct.Stopbits = UART_STOPBIT_1;
  UART_Init(LPC_UART0, &UART_config_struct);   
  UART_IntConfig(LPC_UART0, UART_INTCFG_RBR, ENABLE);   
  UART_TxCmd(LPC_UART0, ENABLE);
}

void UART1_On(void){
	NVIC->ICPR[0] |= (1<<6);                                                   // UART1 Interrupt Pending Set
	NVIC->ISER[0] |= (1<<6);                                                   // UART1 Interrupt Enable
  LPC_SC->PCONP |= (1<<4);                                             // UART1 power/clock control bit
  LPC_SC->PCLKSEL0 &= ~(3<< 8);                                 // PCLKSEL[9:8]=00: PCLK_UART1 peripheral = CCLK/4
  LPC_PINCON->PINSEL4 |= (2<<0);                              // TXD1
  LPC_PINCON->PINSEL4 |= (2<<2);                              // TXD2
  LPC_UART1->LCR = 0x83;                                           //   Line Control Register: determines the format of the data character, 83 = 8'b0101_0011
  LPC_UART1->DLL = 0x36;                                         //                                                                    PCLK
  LPC_UART1->DLM = 0x00;                                                   //   UARTn_baudrate = 式式式式式式式式式式式式式式式式式式式式式式式式式式
  LPC_UART1->FDR = 0x21;                                                   //                                    16 ▼ (256 ▼ DLM + DLL) ▼ (1 + DivAddVal / MulVal)
                                                                                                            //   baudrate = 25M/(16*(256*0+54)*(1+1/2)) = 19290 bps (=19200 bps + 0.47%)
  LPC_UART1->LCR = 0x03;                                                   // divisor latch disable | break transmission disable | odd parity | parity diable | 1 stop bits | 8-bit
  LPC_UART1->IER = 0x07;                                                      //   Interrupt Enable Register
  LPC_UART1->TER = 0x80;                                                   //   Transmit Enable Register: [7]-'TXEN'
}

void check_failed(uint8_t *file,uint32_t line){
	while(1);
}
