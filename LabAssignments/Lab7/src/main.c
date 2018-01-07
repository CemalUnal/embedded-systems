#include "..//tm4c123gh6pm.h"
#include "TExaS.h"

void EnableInterrupts(void);
void WaitForInterrupt(void);

void LED_Init(void);

void SysTick_Init(unsigned long period);
void SysTick_Handler(void);

void ADC0_Init(void);
unsigned long ADC0_In(void);

unsigned long ADCvalue;

unsigned int counter = 0;
unsigned int i = 0;

unsigned int angle = 0;
unsigned int num_of_full_brightness_leds = 0;
unsigned int brightness_percent = 0;
unsigned int on_percentage = 0;

unsigned long pwm_on_duration = 0;

unsigned long leds[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20};

int main(void) {
	
	ADC0_Init();
	LED_Init();

	SysTick_Init(160);
  
	EnableInterrupts();
	
	while(1){
		WaitForInterrupt();
	}
}

/*
	Initializes PORTB0-5 pins as output
	Parameter: None
	Returns: None
*/
void LED_Init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // 1) activate clock for Port B
	delay = SYSCTL_RCGC2_R;	// allow time for clock to start
	GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFFFF0F)+0x00000000;// 3) regular GPIO
	GPIO_PORTB_AMSEL_R &= ~0x3F; // 4) disable analog function on PB0-5
	GPIO_PORTB_DIR_R |= 0x3F;	// 5) set direction to output
	GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular port function
	GPIO_PORTB_DEN_R |= 0x3F; // 7) enable digital port
}

/*
	Initializes SysTick Timer
	Parameter: Clock cycle to wait
	Returns: None
*/
void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0; // disable SysTick during setup
	NVIC_ST_RELOAD_R = period - 1; // reload value
	NVIC_ST_CURRENT_R = 0; // any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000;// priority 1          
	NVIC_ST_CTRL_R = 0x07; // enable SysTick with core clock and interrupts
}

/*
	Handles SysTick Timer interrupts
	Decides how many LEDs will be turned on
	Decides the brightness values of not fully lighted LEDs
	Parameter: None
	Returns: None
*/
void SysTick_Handler(void) {
	counter++;
	// At each 2 ms
	if(counter == 200) {
		// Current value of potentiometer
		ADCvalue = ADC0_In();
		
		// Angle value of the current rotation
		angle = (ADCvalue * 270) / 4095;

		// How many LEDs will be turned on
		// Since the result value is rounded, we do not want it.
		// Thats why this if statement is here.
		if(angle > 265)
			num_of_full_brightness_leds = 6;
		else
			num_of_full_brightness_leds = angle / 45;

		// Brightness value of LEDs that are not fully lighted. 
		brightness_percent = (angle % 45);
		
		// Duration of ON
		pwm_on_duration = ((angle % 45) * 200) / 45;
	
		counter = 0;
	}

	if(pwm_on_duration > counter) {
		// Turn on LEDs
		for(i = 0; i < num_of_full_brightness_leds; i++) {
			GPIO_PORTB_DATA_R |= leds[i];
		}
	} else { // Turn off LEDs
		GPIO_PORTB_DATA_R &= ~leds[num_of_full_brightness_leds - 1];
	}
}

/*
	Initialization function to set up the ADC
	SS3 triggering event: software trigger
	SS3 1st sample source:  channel 1
	SS3 interrupts: enabled but not promoted to controller
	Parameter: None
	Returns: None
*/
void ADC0_Init(void){ 
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000010; // 1) activate clock for Port E
	delay = SYSCTL_RCGC2_R; //	allow time for clock to stabilize
	GPIO_PORTE_DIR_R &= ~0x04; // 2) make PE2 input
	GPIO_PORTE_AFSEL_R |= 0x04; // 3) enable alternate function on PE2
	GPIO_PORTE_DEN_R &= ~0x04; // 4) disable digital I/O on PE2
	GPIO_PORTE_AMSEL_R |= 0x04; // 5) enable analog function on PE2
	SYSCTL_RCGC0_R |= 0x00010000; // 6) activate ADC0
	delay = SYSCTL_RCGC2_R;
	SYSCTL_RCGC0_R &= ~0x00000300; // 7) configure for 125K
	ADC0_SSPRI_R = 0x0123; // 8) Sequencer 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008; // 9) disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000; // 10) seq3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+1;	//? set channel Ain9 (PE2)
	ADC0_SSCTL3_R = 0x0006; // 12) no TS0 D0, yes IE0 END0
	ADC0_ACTSS_R |= 0x0008; // 13) enable sample sequencer 3
}

/*
	Busy-wait Analog to digital conversion
	Parameter: None
	Returns: 12-bit result of ADC conversion
*/
unsigned long ADC0_In(void){
	unsigned long result;
  ADC0_PSSI_R = 0x0008; // 1) initiate SS3
	while((ADC0_RIS_R&0x08)==0){}; // 2) wait for conversion done
	result = ADC0_SSFIFO3_R&0xFFF; // 3) read result
	ADC0_ISC_R = 0x0008; // 4) acknowledge completion
	return result;
}
