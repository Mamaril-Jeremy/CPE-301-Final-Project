#include <LiquidCrystal.h>
#include <RTClib.h>
#include <DHT.h>
#include <Stepper.h>

#define RDA 0x80
#define TBE 0x20  

volatile unsigned char *portDDRB = (unsigned char *) 0x24; //Yellow LED
volatile unsigned char *portB = (unsigned char *) 0x25;

volatile unsigned char *portDDRB = (unsigned char *) 0x24; //Green LED
volatile unsigned char *portB = (unsigned char *) 0x25;

volatile unsigned char *portDDRB = (unsigned char *) 0x24; //Red LED
volatile unsigned char *portB = (unsigned char *) 0x25;

volatile unsigned char *portDDRB = (unsigned char *) 0x24; //Blue LED
volatile unsigned char *portB = (unsigned char *) 0x25;

//UART registers
volatile unsigned char *myUCSR0A = (unsigned char *)0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *)0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *)0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *) 0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *)0x00C6;

//ADC registers
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;


enum States {
  DISABLED,
  IDLE, 
  ERROR,
  RUNNING,
  START
};

void setup(){
  U0init(9600);
  
  
}


void loop(){
  
  
}

//Start of UART functions
void U0init(unsigned long U0baud)
{
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  tbaud = (FCPU / 16 / U0baud - 1);
  // Same as (FCPU / (16 * U0baud)) - 1;
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0  = tbaud;
}

unsigned char U0kbhit()
{
  return (*myUCSR0A & RDA);
}

unsigned char U0getchar()
{
  return *myUDR0;
}

void U0putchar(unsigned char U0pdata)
{
  while((*myUCSR0A &= TBE) == 0){};
  *myUDR0 = U0pdata;
}
//End of UART functions
