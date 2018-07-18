#include <msp430.h>

#define PWM        BIT2
#define ADC        BIT1

#define BUTTON      BIT7
#define X		       BIT4
#define Y 		BIT3

#define NUM_POINTS	63
#define SINE_POINTS 10
#define TRACE_DELAY 100000 
#define ARRAY_SIZE 100


/*

Desired functionality:

Button press: button 1 for sine wave, button two for square wave

	- sine wave: iterate through a predetermined array of numbers, which make a sine wave
	- (2nd iteration- variable resistor controlled period)
	- square wave: use pwm to generate square wave form
	- (2nd iteration- variablt resistor controlled duty cycle)
 ** Third wave, if I finish too quickly/ this is too trivial
 	- joystick drawing. interesting level max (button press plus variable resistor controlled)
 	
I fried p2.6

*/

void Triangle(void);
void Square(void);
void Sine(void);
unsigned char sine_points[NUM_POINTS] = {32,35,38,41,44,47,49,52,
						54,56,58,60,61,62,63,63,
						63,63,62,61,60,59,57,55,
						53,51,48,45,42,39,36,33,
						30,27,24,21,18,15,12,10,
						8,6,4,3,2,1,0,0,
						0,0,1,2,3,5,7,9,
						11,14,16,19,22,25,28};
unsigned char triangle_points[NUM_POINTS] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
unsigned char x_points[ARRAY_SIZE] = {2};
unsigned char y_points[ARRAY_SIZE] = {2};

unsigned int loopingx = 0;
unsigned int loopingy = 0;

void main(void)
{ 
   WDTCTL = WDTPW + WDTHOLD;
   ADC10CTL0 = ADC10SHT_2 + ADC10ON; 	    
   ADC10CTL1 = INCH_1;                       
   ADC10AE0 |= ADC;                         
  
   BCSCTL1 = CALBC1_1MHZ;            
   DCOCTL = CALDCO_1MHZ;
   
   P1SEL |= PWM; 
   P1DIR |= PWM + BIT6; 
   P1DIR &= ~BUTTON;
   P1REN |= BUTTON;
   
   P1IE |= BUTTON;
   P1IES = 0;
   P1IFG = 0;
  
  
   P2DIR = 0xBF;
   P2DIR &= ~BUTTON;
   P2SEL &= ~BUTTON;
   P2REN |= BUTTON;
   P2OUT = 0;
  
   P2IE |= BUTTON;
   P2IES = 0;
   P2IFG =0;
    
   // Enable interrupts
   __bis_SR_register(GIE); 
  
}

void Triangle(void)
{
	CCR0 = 1000;
	CCR1 = 500; 
	CCTL1 = OUTMOD_7;
	TACTL = TASSEL_2 + MC_1;	
	
	while (1) 
	{	
		ADC10CTL0 |= ENC + ADC10SC;       
   		while (ADC10CTL1 &ADC10BUSY); 
   		
   		float value = (float)ADC10MEM;
   		float bit = (float)1023;
   		float ratio = (float)value/bit;
   		
		unsigned char t;
		for (t = 0; t < SINE_POINTS; t++)
		{
			float XR = triangle_points[t];
			CCR1 = XR*10;
			float delay = (float)50*ratio;
			while(delay > 0)
			{
				__no_operation();
				delay--;
			}
			//__delay_cycles(100000);
		}
		
		unsigned char ti = SINE_POINTS-1;
		for (t = ti; t > 0; t--)
		{
			float YR = triangle_points[t];
			CCR1 = YR*10;
			float delay = (float)50*ratio;
			while(delay > 0)
			{
				__no_operation();
				delay--;
			}
			//__delay_cycles(100000);
		}	
	}
	
}

void Square(void)
{
	while (1)
	{ 
		ADC10CTL0 |= ENC + ADC10SC;       
   		while (ADC10CTL1 &ADC10BUSY); 
   		
   		float value = (float)ADC10MEM;
   		float bit = (float)1023;
   		float ratio = (float)value/bit;
   		
		unsigned char t = 0;
		for (t = 0; t < NUM_POINTS; t++)
		{	
			P1OUT |= BIT6;
			float delay = (float)50*ratio;
			while(delay > 0)
			{
				__no_operation();
				delay--;
			} 
		//	__delay_cycles(TRACE_DELAY);
		}
		unsigned char i;
		for (i = 0; i < NUM_POINTS; i++)
		{	
			P1OUT = 0;
			float delay = (float)50*ratio;
			while(delay > 0)
			{
				__no_operation();
				delay--;
			} 
		}
	}
	
}

void Sine(void)
{
	while (1)
	{ 
		ADC10CTL0 |= ENC + ADC10SC;       
   		while (ADC10CTL1 &ADC10BUSY); 
   		
   		float value = (float)ADC10MEM;
   		float bit = (float)1023;
   		float ratio = (float)value/bit;
   		
		unsigned char t = 0;
		for (t = 0; t < NUM_POINTS; t++)
		{	
			float XR = sine_points[t];
			P2OUT = XR;
			float delay = (float)50*ratio;
			while(delay > 0)
			{
				__no_operation();
				delay--;
			} 
		//	__delay_cycles(TRACE_DELAY);
		}
	}
}

void PlotY(float XR, volatile unsigned char array[ARRAY_SIZE], int fill)
{	
		if (P1IN & BIT3)
		{	if (XR+1 == 98)
			{	 
				unsigned int newerest = 98;
				array[fill] = newerest;
			} else {
				unsigned int new = XR + 1;
				array[fill] = new;
			}
			
		} else if (XR-1 == 0){
			loopingy = 0;
			unsigned int newer = XR - 1;
			array[fill] = newer;
		} else {
			loopingy = 0;
			unsigned int newest = 0;
			array[fill] = newest;
		}
}

void PlotX(float XR, volatile unsigned char array[ARRAY_SIZE], int fill)
{	
		if (P1IN & BIT4)
		{	
			if (XR < 98)
			{
				unsigned int new = XR + 1;
				array[fill] = new;
			} else {
				unsigned int newerest = 98;
				array[fill] = newerest;
			}
		} else if (XR-1 > 0){
			loopingx = 0;
			unsigned int newer = XR - 1;
			array[fill] = newer;
		} else {
			loopingx = 0;
			unsigned int newest = 0;
			array[fill] = newest;
		}

}

// Note: I mixed up the naming early on, and the issues snowballed. XR, and x_points correspond to the y axis, and YR and y_points to the x axis. 

void Draw(void)
{ 	
	CCR0 = 1000;
	CCR1 = 250; 
	CCTL1 = OUTMOD_7;
	TACTL = TASSEL_2 + MC_1;	
	
	while(1)
	{	
		int fill = 0;
		unsigned char t;	
		for (t = 0; t < ARRAY_SIZE; t++)
		{	
			float XR;
			float YR;
			if(loopingx == 0){
				XR = x_points[t];
			} else {
				XR = x_points[ARRAY_SIZE];
			}
			CCR1 = XR*10;
			if(loopingy == 0){
 				YR = y_points[t];
			} else {
 				YR = y_points[ARRAY_SIZE];
			}
 			P2OUT = YR;
 			
			__delay_cycles(TRACE_DELAY);
	
			if (fill < ARRAY_SIZE)
			{	
				fill++;
				PlotY(XR, x_points, fill);
				PlotX(YR, y_points, fill);
			}
		}
		loopingx = 1;
		loopingy = 1;
	}
}



  
#if defined(__TI_COMPILER_VERSION__)
#pragma vector=PORT2_VECTOR
__interrupt void port2_isr(void)
#else
  void __attribute__ ((interrupt(PORT2_VECTOR))) port2_isr (void)
#endif
{   
	P1OUT = 0;
	P2OUT = 0;
	P2IFG = 0;
	P2IE |= BUTTON;
	P2IES = 0;
	
	ADC10CTL0 |= ENC + ADC10SC;       
   	while (ADC10CTL1 &ADC10BUSY);
	
	if (ADC10MEM > 256)
	{
		Sine();
	} else {
		Draw();
	}
}

#if defined(__TI_COMPILER_VERSION__)
#pragma vector=PORT1_VECTOR
__interrupt void port1_isr(void)
#else
  void __attribute__ ((interrupt(PORT1_VECTOR))) port1_isr (void)
#endif
{ 
	P1OUT = 0;
	P2OUT = 0;
	P1IFG = 0;
	P1IE |= BUTTON;
	P1IES = 0;
	
	ADC10CTL0 |= ENC + ADC10SC;       
   	while (ADC10CTL1 &ADC10BUSY);
	
	if (ADC10MEM > 256)
	{
		Triangle();
	} else {
		Square();
	}
}
