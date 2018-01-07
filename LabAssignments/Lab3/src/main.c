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

#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))

void PortE_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010;     // 1) activate clock for Port E
	delay = SYSCTL_RCGC2_R;           // allow time for clock to start
	GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
	GPIO_PORTE_CR_R = 0x0E;           // allow changes to PE1-3
	GPIO_PORTE_AMSEL_R = 0x00;        // 3) disable analog on PE
	GPIO_PORTE_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PE4-0
	GPIO_PORTE_DIR_R = 0x0E;          // 5) PE0 in, PF3-1 out
	GPIO_PORTE_AFSEL_R = 0x00;        // 6) disable alt funct on PE7-0
	GPIO_PORTE_PUR_R = 0x00;
	GPIO_PORTE_DEN_R = 0x1F;          // 7) enable digital I/O on PE4-0
}

unsigned long In;

typedef enum{
	false,
	true
}bool;

bool flag = true;

unsigned int counter = 0;
unsigned long color_values[] = {0x02,0x04,0x08};
unsigned int color_index = 0;

void SysTick_Init(void){

  NVIC_ST_CTRL_R = 0;
  NVIC_ST_CTRL_R = 0x00000005;

}

void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  				// number of counts to wait
  NVIC_ST_CURRENT_R = 0;       				// any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ 	// wait for count flag
  }
}

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****

// PE0, PB0, or PA2 connected to positive logic momentary switch using 10k ohm pull down resistor
// PE1, PB1, or PA3 connected to positive logic LED through 470 ohm current limiting resistor
// To avoid damaging your hardware, ensure that your circuits match the schematic
// shown in Lab8_artist.sch (PCB Artist schematic file) or 
// Lab8_artist.pdf (compatible with many various readers like Adobe Acrobat).

void StraightDelay(unsigned int deliminator){
	int i;
	for(i=0; i<deliminator*10; i++){	
		NVIC_ST_RELOAD_R = 800000-1;  // number of counts to wait
		NVIC_ST_CURRENT_R = 0;
		while((NVIC_ST_CTRL_R&0x00010000)==0){
			In = GPIO_PORTE_DATA_R & 0x01;
			if(In == 0x01 && flag){
				color_index = (color_index + 2)%3;
				flag = !flag;
			}
		}
	}
}

void ReverseDelay(unsigned int deliminator){
	int i;
	for(i=0; i<deliminator*10; i++){	
		NVIC_ST_RELOAD_R = 800000-1;  // number of counts to wait
		NVIC_ST_CURRENT_R = 0;
		while((NVIC_ST_CTRL_R&0x00010000)==0){
			In = GPIO_PORTE_DATA_R & 0x01;
			if(In == 0x01 && !flag){
				color_index = (color_index + 1)%3;
				flag = !flag;
			}
		}
	}
}

int main(void){ 
//**********************************************************************
// The following version tests input on PE0 and output on PE1
//**********************************************************************
//  TExaS_Init(SW_PIN_PE0, LED_PIN_PE1, ScopeOn);  // activate grader and set system clock to 80 MHz
	PortE_Init();
	SysTick_Init();
	EnableInterrupts();           // enable interrupts for the grader
	
	while(1){
		while(flag){											// as long as the flag is true
			GPIO_PORTE_DATA_R = color_values[color_index];		// LED will light color value with respect to its index
			StraightDelay(color_index+1);						// each color produces specific delay time
			color_index = (color_index + 1)%3;
		}
		while(!flag){											// if the flag is not true any more
			color_index = (color_index + 2)%3;					// it means the board will revolves in reverse order
			GPIO_PORTE_DATA_R = color_values[color_index];
			ReverseDelay(color_index+1);
		}
	}
  
}
