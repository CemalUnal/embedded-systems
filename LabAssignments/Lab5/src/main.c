// ***** 0. Documentation Section *****
// SwitchLEDInterface.c for Lab 8
// Runs on LM4F120/TM4C123
// Use simple programming structures in C to toggle an LED
// while a button is pressed and turn the LED on when the
// button is released.  This lab requires external hardware
// to be wired to the LaunchPad using the prototyping board.
// January 15, 2016
//      Jon Valvano and Ramesh Yerraballi

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "../Lab15_SpaceInvaders/Nokia5110.c"

#include <stdlib.h>

void EnableInterrupts(void);
void DisableInterrupts(void);
void WaitForInterrupt(void);

volatile unsigned long delay;
volatile unsigned int counter;

volatile unsigned int start_time;
volatile unsigned int barrier;

volatile unsigned int RT1,RT2;

volatile unsigned int turn;

void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
	NVIC_ST_RELOAD_R = period-1;// reload value
	NVIC_ST_CURRENT_R = 0;      // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2          
	NVIC_ST_CTRL_R = 0x07; 			// enable SysTick with core clock and interrupts
	
	// enable interrupts after all initialization is finished
}

/* After passing a phase of SysTick timer, then SysTick_Handler will be operated*/
void SysTick_Handler(void){
	counter++;															// duration of one SysTick timer iteration is 0.01 sec
																					// this counter will be increased during 0.01 sec
	if(counter == start_time*1000){
		Nokia5110_Clear();
		Nokia5110_OutString("PRESS");
		barrier++;
	}
}

void CheckReactions(void){
	if(RT1 > RT2){
		Nokia5110_Clear();
		if(RT2 > 10)
			Nokia5110_OutString("Player-2 won");
		else
			Nokia5110_OutString("Player-1 won");
	}
	else{
		Nokia5110_Clear();
		if(RT1 > 10)
			Nokia5110_OutString("Player-1 won");
		else
			Nokia5110_OutString("Player-2 won");
	}
	Nokia5110_OutString("RT1:");
	Nokia5110_OutUDec(RT1);
	Nokia5110_OutString(" ms");
	Nokia5110_OutString("RT2:");
	Nokia5110_OutUDec(RT2);
	Nokia5110_OutString(" ms");
	Nokia5110_OutString("            ");
	Nokia5110_OutString("Another game will begin!");
}

void Delay(unsigned int deliminator){
	unsigned long volatile time;
	time = deliminator*108110;
	while(time){
		time--;
	}
}

void GPIOPortE_Handler(void){ 
	
	/* if player press the button after press command */
	if(barrier > 0){
		if(GPIO_PORTE_RIS_R & 0x01){
			GPIO_PORTE_ICR_R = 0x01;
			RT1 = counter - start_time*1000;
		}
		if(GPIO_PORTE_RIS_R & 0x02){
			GPIO_PORTE_ICR_R = 0x02;
			RT2 = counter - start_time*1000;
		}
		if(RT1 != 0 && RT2 != 0){
			CheckReactions();
			turn++;
		}
	}
	/* if player press the button before press command */
	else{
		if(GPIO_PORTE_RIS_R & 0x01){
			GPIO_PORTE_ICR_R = 0x01;
			Nokia5110_Clear();
			Nokia5110_OutString("Player-2 won");
		}
		if(GPIO_PORTE_RIS_R & 0x02){
			GPIO_PORTE_ICR_R = 0x02;
			Nokia5110_Clear();
			Nokia5110_OutString("Player-1 won");
		}
		Nokia5110_OutString("False Start");
		turn++;
	}
}



void Switch_Init(void){     
  SYSCTL_RCGC2_R |= 0x00000010; 		// (a) activate clock for port E
	delay = SYSCTL_RCGC2_R;						// (b) initialize count and wait for clock
	GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
	GPIO_PORTE_CR_R = 0x03;           // allow changes to PE0													//0x01 
  GPIO_PORTE_DIR_R &= ~0x03;       	// 5) PE0 in																		//0x01
	GPIO_PORTE_AFSEL_R &= ~0x03;  		//	  disable alt funct on PE0									//0x01
  GPIO_PORTE_DEN_R |= 0x03;     		//     enable digital I/O on PE4								
  GPIO_PORTE_PCTL_R &= ~0x000000FF; //  configure PE0 as GPIO
  GPIO_PORTE_AMSEL_R &= ~0x03;  		//    disable analog functionality on PE0
  GPIO_PORTE_PUR_R |= 0x00;    			//     enable weak pull-up on PF4
  GPIO_PORTE_IS_R &= ~0x03;     		// (d) PE0 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x03;    		//     PE0 is not both edges
  GPIO_PORTE_IEV_R |= 0x03;    			//     PE0 rising edge event
  GPIO_PORTE_ICR_R = 0x03;      		// (e) clear flag0
  GPIO_PORTE_IM_R |= 0x03;      		// (f) arm interrupt on PE0
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF00)|0x00000020; // (g) priority 1
  NVIC_EN0_R = 0x00000010;      		// (h) enable interrupt 4 in NVIC
}

int main(void){ 								// running at 16 MHz
	Switch_Init();
	Nokia5110_Init();
	SysTick_Init(16000);
	
	while(1){
		RT1 = 0;
		RT2 = 0;
		counter = 0;
		turn = 0;
		barrier = 0;
		
		Nokia5110_ClearBuffer();
		Nokia5110_DisplayBuffer();
		
		Nokia5110_OutString("GET READY!");
		start_time = (rand()%10) + 1;

		while(turn == 0){
			WaitForInterrupt();
		}
		Delay(80);
	}
}



