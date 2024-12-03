
#include "driverlib.h"
#include "msp430fr4133.h"
#include <msp430.h>

#define CALADC_15V_30C  *((unsigned int *)0x1A1A)  // Temperature Sensor Calibration-30 C
#define CALADC_15V_85C  *((unsigned int *)0x1A1C)  // Temperature Sensor Calibration-85 C
#define FRAM_TEST_START 0x1800
#define FRAM_TEMP_VALIDATION 0x1824



#define pos1 4                                                 // Digit A1 - L4
#define pos2 6                                                 // Digit A2 - L6
#define pos3 8                                                 // Digit A3 - L8
#define pos4 10                                                // Digit A4 - L10
#define pos5 2                                                 // Digit A5 - L2
#define pos6 18                                                // Digit A6 - L18


float temp;
float IntDegC;
float *FRAM_write_ptr;
float stored_temperture;
float *validation_ptr;

void readTemperature(void);
float readFromFram(void);
void initLCD(void);
void displayFloat(float num);

const char digit[10] =
{
    0xFC,                                                      // "0"
    0x60,                                                      // "1"
    0xDB,                                                      // "2"
    0xF3,                                                      // "3"
    0x67,                                                      // "4"
    0xB7,                                                      // "5"
    0xBF,                                                      // "6"
    0xE4,                                                      // "7"
    0xFF,                                                      // "8"
    0xF7                                                       // "9"
};





void init(void)
{ 
    //設定讀取溫度
    WDTCTL = WDTPW | WDTHOLD;  // 停用看門狗
    PM5CTL0 &= ~LOCKLPM5;      // 停用高阻抗

    // Configure ADC - Pulse sample mode; ADCSC trigger
    ADCCTL0 |= ADCSHT_8 | ADCON;  // ADC ON, 設定ADC sample-and-hold time為1000b = 256 ADCCLK cycles
    ADCCTL1 |= ADCSHP;  // s/w trig, single ch/conv, MODOSC
    ADCCTL2 |= ADCRES;  // 10-bit conversion results
    ADCMCTL0 |= ADCSREF_1 | ADCINCH_12;  // 設定ADC input 為ch A12 => 內建的temperture sensor

    // Configure reference
    PMMCTL0_H = PMMPW_H;  // Unlock the PMM registers
    PMMCTL2 |= INTREFEN | TSENSOREN;  // Enable internal reference and temperature sensor
    __delay_cycles(400);  // Delay for reference settling

    //設定按鈕
    P1DIR &=~BIT2;
    P1REN |= BIT2; 
    P1DIR |= BIT0;
    }


int main(void)
{
    init();
    initLCD();  // 初始化 LCD

    #ifdef Test_Validation    
    validation_ptr = (float *)FRAM_TEMP_VALIDATION;
    SYSCFG0 &= ~DFWP;
    *validation_ptr = 123;
    SYSCFG0 |= DFWP;
    #endif    

        
    while (1)
    {
        if(!(P1IN & BIT2)){
            readTemperature();
            __no_operation();
        }
        stored_temperture = readFromFram();
        displayFloat(stored_temperture);  // 顯示浮點數
        if(stored_temperture>=30){
            P1OUT |=BIT0;
            __no_operation();
        }else{
            P1OUT &=~BIT0;
            __no_operation();
        }

        __delay_cycles(10);  // Delay between readings
    }
}

void readTemperature(void)
{
    ADCCTL0 |= ADCENC | ADCSC;  // Start sampling and conversion
    while (ADCCTL1 & ADCBUSY);  // Wait for conversion to complete

    temp = ADCMEM0;
    // Temperature in Celsius
    IntDegC = (temp - CALADC_15V_30C) * (85 - 30) / (CALADC_15V_85C - CALADC_15V_30C) + 30;
    FRAM_write_ptr = (float *)FRAM_TEST_START;
    validation_ptr = (float *)FRAM_TEMP_VALIDATION;
    SYSCFG0 &= ~DFWP;
    *FRAM_write_ptr = IntDegC;
    *validation_ptr = 716;
    SYSCFG0 |= DFWP;    
   

}

float readFromFram(void)
{
    validation_ptr = (float *)FRAM_TEMP_VALIDATION;
    if (*validation_ptr==716) {
        FRAM_write_ptr = (float *)FRAM_TEST_START;
        return *FRAM_write_ptr;
    }else{
        return 0;
    }  
}



void initLCD(void)
{
    // 初始化 LCD

    P4SEL0 |= BIT1 | BIT2;                                     // P4.2~P4.1: crystal pins
    do
    {
        CSCTL7 &= ~(XT1OFFG | DCOFFG);                         // Clear XT1 and DCO fault flag
        SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);                                  // Test oscillator fault flag
    CSCTL6 = (CSCTL6 & ~(XT1DRIVE_3)) | XT1DRIVE_2;            // Higher drive strength and current consumption for XT1 oscillator



    SYSCFG2 |= LCDPCTL;                                        // R13/R23/R33/LCDCAP0/LCDCAP1 pins selected

    LCDPCTL0 = 0xFFFF;
    LCDPCTL1 = 0x07FF;
    LCDPCTL2 = 0x00F0;                                         // L0~L26 & L36~L39 pins selected

    LCDCTL0 = LCDSSEL_0 | LCDDIV_7;                            // flcd ref freq is xtclk

    // LCD Operation - Mode 3, internal 3.08v, charge pump 256Hz
    LCDVCTL = LCDCPEN | LCDREFEN | VLCD_6 | (LCDCPFSEL0 | LCDCPFSEL1 | LCDCPFSEL2 | LCDCPFSEL3);

    LCDMEMCTL |= LCDCLRM;                                      // Clear LCD memory

    LCDCSSEL0 = 0x000F;                                        // Configure COMs and SEGs
    LCDCSSEL1 = 0x0000;                                        // L0, L1, L2, L3: COM pins
    LCDCSSEL2 = 0x0000;

    LCDM0 = 0x21;                                              // L0 = COM0, L1 = COM1
    LCDM1 = 0x84;                                              // L2 = COM2, L3 = COM3
}




void displayFloat(float num)
{
    int t = num/10;
    int t2 = (int)num%10;
    int t3 = (num - (int)num )*10;

    // 顯示字串到 LCD
    LCDMEM[pos1] = digit[t];
    LCDMEM[pos2] = digit[t2];
    LCDMEM[pos3] = digit[t3];
    LCDCTL0 |= LCD4MUX | LCDON;                                // Turn on LCD, 4-mux selected

    PMMCTL0_H = PMMPW_H;                                       // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF_L; 

}


