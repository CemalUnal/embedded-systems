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

void EnableInterrupts(void);
void DisableInterrupts(void);
void WaitForInterrupt(void);

typedef enum{
	false,
	true
}bool;

bool flag;

unsigned long color_val[] = {0x02,0x04,0x08};

long color_index;
volatile unsigned long delay;
volatile unsigned int counter;

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
	counter++;															// duration of one SysTick timer iteration is 0.05 sec
																					// this counter will be increased during 0.05 sec
	if(counter == ((color_index+2)*5)){			// if the counter reaches a threshold of corresponding index value
																					// then the LED light will be changed
		if(flag){
			color_index = (color_index + 1)%3;
		}
		else{
			color_index = (color_index + 2)%3;
		}
		GPIO_PORTE_DATA_R = color_val[color_index]; 
		counter = 0;
	}
}

void GPIOPortE_Handler(void){ // buton kontrol
	
	if(GPIO_PORTE_RIS_R & 0x01){  // PE0 touch
    GPIO_PORTE_ICR_R = 0x01;  	// acknowledge flag0
		flag = !flag;								// flag will be toggled as long as the switch will be pressed
  }
}

void Switch_Init(void){     
  SYSCTL_RCGC2_R |= 0x00000010; 		// (a) activate clock for port E
	delay = SYSCTL_RCGC2_R;						// (b) initialize count and wait for clock
	GPIO_PORTE_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port E
	GPIO_PORTE_CR_R = 0x01;           // allow changes to PE0
  GPIO_PORTE_DIR_R &= ~0x01;       	// 5) PE0 in
	GPIO_PORTE_AFSEL_R &= ~0x01;  		//     disable alt funct on PE0
  GPIO_PORTE_DEN_R |= 0x01;     		//     enable digital I/O on PE4
  GPIO_PORTE_PCTL_R &= ~0x0000000F; //  configure PE0 as GPIO
  GPIO_PORTE_AMSEL_R &= ~0x01;  		//    disable analog functionality on PE0
  GPIO_PORTE_PUR_R |= 0x00;    			//     enable weak pull-up on PF4
  GPIO_PORTE_IS_R &= ~0x01;     		// (d) PE0 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x01;    		//     PE0 is not both edges
  GPIO_PORTE_IEV_R |= 0x01;    			//     PE0 rising edge event
  GPIO_PORTE_ICR_R = 0x01;      		// (e) clear flag0
  GPIO_PORTE_IM_R |= 0x01;      		// (f) arm interrupt on PE0
  NVIC_PRI1_R = (NVIC_PRI1_R&0xFFFFFF00)|0x00000020; // (g) priority 1
  NVIC_EN0_R = 0x00000010;      		// (h) enable interrupt 4 in NVIC
}

void Led_Init(void){
	SYSCTL_RCGC2_R |= 0x00000010; // (a) activate clock for port E
	delay = SYSCTL_RCGC2_R;
  GPIO_PORTE_AMSEL_R &= ~0x0E;      // disable analog functionality on PA5
  GPIO_PORTE_PCTL_R &= ~0x0000FFF0; // configure PE1-3 as GPIO
  GPIO_PORTE_DIR_R |= 0x0E;     		// make PE1-3 out
  GPIO_PORTE_AFSEL_R &= ~0x0E;  		// disable alt funct on PE1-3
  GPIO_PORTE_DEN_R |= 0x0E;     		// enable digital I/O on PA5
  GPIO_PORTE_DATA_R &= ~0x0E;   		// make PA1-3 low
}

int main(void){ 								// running at 16 MHz
  
	color_index = -1;							// initialization of color index 
	counter = 0;									// counter that counts iteration of SysTick timer
	flag = true;									
  Led_Init();
	Switch_Init();
	
  EnableInterrupts();      			// enable after everything initialized
	SysTick_Init(800000);        	// initialize SysTick timer, every 0.05s
	
  while(1){                   	// interrupts every 1ms, 500 Hz flash
    WaitForInterrupt();
  }
}
