#include <reg51.h>
#define KEYPAD_NO_NEW_DATA '-'
static unsigned char Last_valid_Key_G = KEYPAD_NO_NEW_DATA;
unsigned char pKey; // stored inside it the pressed key. 
unsigned char position=0; // shows which digits will appear in the 7 segement.  
unsigned char IN[]= {'0','0','0','0'}; // array of digits in entered by user.
unsigned char arr[10]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x98}; // array contains the values to display numbers 0-9 on the 7-segment LED display.
unsigned char reset = 0;//controls push button
unsigned char highVal= 0x00; // stores the upper 2 hex char of the calculated freq.
unsigned char lowVal = 0x00; // stores the lower 2 hex char of the calculated freq.
unsigned char counter=0; // needed for timers with time more than 71ms.
unsigned char temp=0; // needed for timers with time more than 71ms. decremented till zero and refilled again with counter.

// pin assignment. 
sbit LED = P3^7;
sbit Tr1 = P0^0;
sbit Tr2 = P0^1;
sbit Tr3 = P0^2;
sbit Tr4 = P0^3;
sbit c3 =P1^7;
sbit c2 =P1^6;
sbit c1 =P1^5;
sbit c0 =P1^4;
sbit r3 =P1^3;
sbit r2 =P1^2;
sbit r1 =P1^1;
sbit r0 =P1^0;

// debounce function used to get some delay after keypad button click.
void debounce()
{
	   unsigned char i = 100;
		 unsigned char j = 1000;
		 for(;i>0;i--)
		 for(;j>0;j--);		 
}	

// KeyPad_Scan used to detect the pressed value.
char KeyPad_Scan(void)
{
	static char Old_Key;
	char Key = KEYPAD_NO_NEW_DATA; //-
	
	//if row zerois pressed check which column is pressed to get the exact letter.
	r0=0;
  if(c3 ==0) {Key= 'A';}
	else if(c2==0) {Key= '3';}
	else if(c1==0) {Key= '2';}
	else if(c0==0) {Key= '1';}
	r0=1;
	
	r1=0;
  if(c3 ==0) {Key= 'B';}
	else if(c2==0) {Key= '6';}
	else if(c1==0) {Key= '5';}
	else if(c0==0) {Key= '4';}
	r1=1;
	
	r2=0;
  if(c3 ==0) {Key= 'C';}
	else if(c2==0) {Key= '9';}
	else if(c1==0) {Key= '8';}
	else if(c0==0) {Key= '7';}
	r2=1;
	
	r3=0;
  if(c3 ==0) {Key= 'D';}
	else if(c2==0) {Key= '#';}
	else if(c1==0) {Key= '0';}
	else if(c0==0) {Key= '*';}
	r3=1;
	
	// used to get delay after each press 
	debounce();
	
	// check the validity of the pressed key.
	if(Key == KEYPAD_NO_NEW_DATA)
	{
    Old_Key = KEYPAD_NO_NEW_DATA;
		Last_valid_Key_G = KEYPAD_NO_NEW_DATA;
		return 0;
	}
  
	// check that the key is not as the last pressed key and assign the global variable pkey.
	if(Key == Old_Key)
	{
    if(Key !=Last_valid_Key_G)
		{
			pKey=Key;
			Last_valid_Key_G = Key;
			return 1;
		}
	}
  Old_Key = Key;
  return 0;	
}

// delay to produce square wave with given time.
void Timer1_freqGenration (void) interrupt 3
{  
  TH1=highVal;
	TL1=lowVal;
	if (temp==0)
	{LED = ~LED;
		temp=counter;
	}
	else	temp--;
}

// function to calculate the frequency generator.
void freqGenerator(void ){
	unsigned short int delay; 
	float delayTime;
	unsigned short int freq;
	//float adjust;
	freq=(IN[0]-'0')*1000+(IN[1]-'0')*100+(IN[2]-'0')*10+(IN[3]-'0'); // get the frequency the user entered
	// calculate the overhead    
	if (freq < 1500)
		delayTime = 0.985;
		else if (freq < 2500)
		delayTime = 0.94;
		else if (freq<3500)
		delayTime = 0.89;
		else if (freq<4500)
		delayTime = 0.87;
		else if (freq<5500)
		delayTime = 0.85;
		else if (freq<7500)
		delayTime = 0.79;
		else if (freq<9500)
		delayTime = 0.75;
	  else
		delayTime = 0.72;

	 delayTime = (delayTime/(freq*2));	// delay = Ts/2
	if (delayTime <= 0.071){ //maximum limit at TH0 = 00 and TL0 = 00
		delayTime = delayTime/(0.000001085); 
		delayTime = 65536 - delayTime; // get the numbers of cycles.
		delay = (int)delayTime;
		lowVal = delay & 0xff;  // lower 8 bits
		highVal = delay >> 8;   // higher 8 bits
		counter=0; 
		temp=0;
	}
	else
		{
			// if the time needed more than 71ms divide the time needed by 10ms and assign it to counter and temp
			counter = (int)(delayTime / 0.01);
			// assign the lowVal and highVal to 10ms if we need 50ms timer then counter equals 5 and loop over 10ms 5 times
			lowVal = 0x0FF; 
			highVal = 0x0DB;
			temp=counter;	
	}
}

// called when the user presses the push button. 
void keyPressed (void) interrupt 0
{
	if (reset==0)
	{
	// Calculate the frequency entered by the user.
	freqGenerator();
	// Start the timer 
	TR1=1;
	reset = 1;
	}
	else
	{
	TR1 = 0;
  TF1 = 0;	
	LED = 1;	
	IN[0]='0';
	IN[1]='0';
	IN[2]='0';
  IN[3]='0';
	reset = 0;
	}
	debounce();//?
}	

// function for timer 0 to display on the 7 segment.  
void display() interrupt 1
{
	// digits array used to declare Tr1,Tr2,Tr3 and Tr4 with 1 to be closed.
	 unsigned char digits[] = {1,1,1,1};
	// timer for 4.4ms refreshing screen time.	
	  TL0=0x00;
	  TH0=0xF0;
	// open only 1 Tr to display a digit. 
		digits[position] = 0;
		Tr1=digits[0];
		Tr2=digits[1];
		Tr3=digits[2];
		Tr4=digits[3];
	// display on port2.
		P2 = arr[IN[position]-'0'];
		position++;					
		if (position == 4){
				position = 0;
		}	
}

void main()
{ 
	SP=0x30;
	// Declare the timer rgister.
	TMOD =  (TMOD & 0xF0) | 0x11;		// timer 0 in mode 1 (16 bit counter) and timer 1 in mode 1.
	//  declare timer0 registers
	IT0=1;
	ET0=1;
	TL0=0x00;
  TH0=0x0F0;
	// declare int0 registers.
	EX0=1;
	EA=1;
	// start timer0 
	TR0=1;
  TH1=highVal;
	TL1=lowVal;
	ET1=1;
	EA=1;
	// declare all rows by zero.
  r0 = 1;
	r1 = 1;
	r2 = 1;
	r3 = 1;

	while(1)
		{	
	// call keypad to detect if user pressed any letter		
		if(KeyPad_Scan() == 1)
		{
			// shift the 7 segments letters on click.
			IN[0]=IN[1];
			IN[1]=IN[2];
			IN[2]=IN[3];
      IN[3]=pKey;	
		}	
	}	
}

