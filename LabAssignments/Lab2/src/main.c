// main.c
// Runs on LM4F120 or TM4C123
// C2_Toggle_PF1, toggles PF1 (red LED) at 5 Hz
// Daniel Valvano, Jonathan Valvano, and Ramesh Yerraballi
// January 18, 2016

// LaunchPad built-in hardware
// SW1 left switch is negative logic PF4 on the Launchpad
// SW2 right switch is negative logic PF0 on the Launchpad
// red LED connected to PF1 on the Launchpad
// blue LED connected to PF2 on the Launchpad
// green LED connected to PF3 on the Launchpad


#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define PF4                     (*((volatile unsigned long *)0x40025040))
#define PF3                     (*((volatile unsigned long *)0x40025020))
#define PF2                     (*((volatile unsigned long *)0x40025010))
#define PF1                     (*((volatile unsigned long *)0x40025008))
#define PF0                     (*((volatile unsigned long *)0x40025004))
#define GPIO_PORTF_DR2R_R       (*((volatile unsigned long *)0x40025500))
#define GPIO_PORTF_DR4R_R       (*((volatile unsigned long *)0x40025504))
#define GPIO_PORTF_DR8R_R       (*((volatile unsigned long *)0x40025508))
#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
	
#define COLOR_SIZE 							3

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

// Color    LED(s) PortF
// dark     ---    0
// red      R--    0x02
// blue     --B    0x04
// green    -G-    0x08
// yellow   RG-    0x0A
// sky blue -GB    0x0C
// white    RGB    0x0E
// pink     R-B    0x06

unsigned long In;												// captures PF4 (SW1) port values

typedef enum{
	false,
	true
}bool;																	// boolean value descriptor is defined

bool flag = true;												// flag variable used for stating lighting order 

unsigned int counter = 0;								// counts the duration of pressing SW1 button
unsigned long color_val[] = {0x04,0x02,0x08};	// color values are stored in an array
unsigned int color_indx = 0;						// indexes of color array

/* Delay function */
void Delay(unsigned int deliminator){		// deliminator is a special coefficient that calculates delay
																				// time corresponding each color value
	unsigned long volatile time;	
	time = deliminator*108110*2;					// 108110*2 approximately correspond to 0.5 second
	while(time){
		In = GPIO_PORTF_DATA_R & 0x10;			// reads SW1 switch in each iteration of the loop
		if(In == 0x00){											// in case of pressing SW1 button
			counter++;												// counter will be increased
			if(counter > 25){									// if SW1 is pressed enough, flag value will be interchanged 
																				// for switching order of lighting 
				flag = !flag;
			}
		}
		else if(In == 0x10)									// otherwise, counter will be reset
			counter = 0;
		time--;
	}
}


int main(void){  
  PortF_Init();  												// make PF1 out (PF1 built-in LED)
	
	while(1){
    while(flag){												// as long as the flag is true
			GPIO_PORTF_DATA_R = color_val[color_indx];	// LED will light color value with respect to its index
			Delay(color_indx+1);							// each color produces specific delay time
			color_indx = (color_indx + 1)%3;
		}
    while(!flag){												// if the flag is not true any more
			color_indx = (color_indx + 2)%3;		// it means the board will revolves in reverse order
			GPIO_PORTF_DATA_R = color_val[color_indx];
			Delay(color_indx+1);
		}
	}
}
