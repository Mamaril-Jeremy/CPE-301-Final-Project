//Author: Jeremy Mamaril and Brandon (Ri) Yu

#include <LiquidCrystal.h>
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

LiquidCrystal lcd(13, 14, 15, 16, 17, 18); //creates lcd object - figure out pins

enum States {
  DISABLED,
  IDLE, 
  ERROR,
  RUNNING,
};

void setup(){
  U0init(9600);
  
  adc_init(); //initializes the ADC
  
  lcd.begin(16, 2); //starts the lcd
}


void loop(){
  //Water level monitoring

  //Lcd display
  

  //Clock monitoring

  //Temperature 

  //Motor

  //state changes
  
}

void writeToLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
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

//Start of ADC functions
void adc_init()
{
  // setup the A register
  *my_ADCSRA |= 0b10000000; // set bit   7 to 1 to enable the ADC
  *my_ADCSRA &= 0b10111111; // clear bit 5 to 0 to disable the ADC trigger mode
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to disable the ADC interrupt
  *my_ADCSRA &= 0b11011111; // clear bit 5 to 0 to set prescaler selection to slow reading
  // setup the B register
  *my_ADCSRB &= 0b11110111; // clear bit 3 to 0 to reset the channel and gain bits
  *my_ADCSRB &= 0b11111000; // clear bit 2-0 to 0 to set free running mode
  // setup the MUX Register
  *my_ADMUX  &= 0b01111111; // clear bit 7 to 0 for AVCC analog reference
  *my_ADMUX  |= 0b01000000; // set bit   6 to 1 for AVCC analog reference
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11011111; // clear bit 5 to 0 for right adjust result
  *my_ADMUX  &= 0b11100000; // clear bit 4-0 to 0 to reset the channel and gain bits
}

unsigned int adc_read(unsigned char adc_channel_num)
{
  *my_ADMUX = (0x40 | adc_channel_num); 

  // Enable the ADC and set the prescaler to divide by 128 for a 125kHz ADC clock
  *my_ADCSRA = 0x87; // 0x87 sets the ADEN, ADSC, and ADPS2-ADPS0 bits

  // Wait for the conversion to complete
  while ((*my_ADCSRA) & (1 << ADSC)) {}

  // Read the ADC result from the data registers
  int adcValue = *my_ADC_DATA;

  // Disable the ADC
  *my_ADCSRA = 0x00; // 0x00 clears the ADEN bit

  return adcValue;
}
//End of ADC functions
