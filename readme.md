    113522053 蔡尚融 kjmse716

[於HackMD中閱讀](https://hackmd.io/@kjmse716/B1oVjJyzye)
# Writing a Simple Linux Driver and Recompiling the Kernel

## Setting up enviroment

```
uname -r
sudo apt install build-essential linux-headers-$(uname -r)
```

![image](https://hackmd.io/_uploads/ryrq01Jzye.png)

### kernel module
kernel module
![image](https://hackmd.io/_uploads/ryiWBGyGke.png)


將此kernel module放置於之後欲編譯的linux kernel source code folder的`/driver/lab1`資料夾下


## MakeFile:

### 關於obj-y 和 obj-m:

在 Linux kernel 的 Makefile 中，obj-y 和 obj-m 是用來指定哪些文件應該被編譯進內核或作為模組編譯的。這兩者有著不同的用途和行為：

#### obj-y
用途：`obj-y` 用來指定那些應該被編譯進內核映像（vmlinux）的文件。

行為：當在 Makefile 中使用 obj-y 時，這些文件會被編譯並鏈接到內核映像中，成為內核的一部分。這些文件中的代碼在內核啟動時就會被加載和執行。

範例：
```
makefile
obj-y += foo.o
```
在這個範例中，foo.o 會被編譯並鏈接到內核映像中。

#### obj-m
用途：`obj-m` 用來指定那些應該被編譯成內核模組（kernel modules）的文件。

行為：當在 Makefile 中使用 `obj-m` 時，這些文件會被編譯成可載入的模組（Loadable Kernel Module).ko 文件，這些模組可以在內核運行時動態加載和卸載。這些文件中的代碼不會在內核啟動時加載，可以根據需要動態加載。
```
makefile
obj-m += bar.o
```
在這個範例中，bar.o 會被編譯成一個內核模組，可以在內核運行時動態加載。

### 設定Makefile檔案
lab1資料夾下的Makefile檔案
![image](https://hackmd.io/_uploads/HJ4rDV1Mkx.png)








### 編譯kernel module lab1

使用obj-m方式的module不需要重新make整個linux kernel.



`make -C /path/to/kernel/source M=$(pwd) modules`
-C：指定要切換到 kernel source 資料夾，然後執行編譯。
M=$(pwd)：告訴 kernel build system 在當前目錄下找到 Makefile 並編譯模組。
modules：指定只編譯 kernel modules。

使用:
`make -C /usr/src/linux-6.11.7 M=$(pwd) modules`


會出現以下錯誤

![image](https://hackmd.io/_uploads/ByeEcmmkfJx.png)

由於系統目前kernel版本的`6.8.0-40`。這裡誤使用到新下載的kernel source code`6.11.7`版本(實驗至此還沒有make更新kernel)，且先前安裝必要套件使用linux-headers-$(uname -r)，安裝的linux-headers也是`6.8.0-40`，故發生此錯誤。

用6.8.0-40也就是當前版本的kernel headers來做為source就可以解決。

改用以下指令`sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules`或`sudo make -C /usr/src/linux-headers-6.8.0-40-generic M=$(pwd) modules`


![image](https://hackmd.io/_uploads/rJCB3Xyfke.png)
![image](https://hackmd.io/_uploads/Bkr3qN1zkx.png)

![image](https://hackmd.io/_uploads/S1MEgHJz1g.png)



### Loading and unloading the module.

#### `insmod`加載模組


![image](https://hackmd.io/_uploads/Sy_ro4JM1x.png)

dmesg
![image](https://hackmd.io/_uploads/rkSqiEyfJl.png)


#### `rmmod`解除加載模組
![image](https://hackmd.io/_uploads/HJ5uhNkMye.png)
![image](https://hackmd.io/_uploads/r1Qh3EyGye.png)




### 改寫module為build-in part of Linux kernel

#### 前置工作
clone Linux source code:
使用git clone https://github.com/torvalds/linux.git


由於原本就已將module是置於欲編譯的kernel source code folder中進行實作，故不需要移動，但須先使用`sudo make clean`來清除剛才make輸出檔案。
![image](https://hackmd.io/_uploads/B1BogS1M1x.png)


#### 修改Makefile

修改lab1資料夾下的Makefile
將原本的 `obj-m` 替換成 `obj-y`讓系統將此module作為kernel build-in而非可載入的模組編譯
![image](https://hackmd.io/_uploads/rJtf-rJGJe.png)

修改driver資料夾下的Makefile檔案
![image](https://hackmd.io/_uploads/H1ZeIGJzyg.png)


#### 編譯kernel
![image](https://hackmd.io/_uploads/S1DuBByzyx.png)

![image](https://hackmd.io/_uploads/S1e6SrJzJe.png)


```
sudo make -j7&&
sudo make modules_install -j7&&
sudo make install -j7&&
sudo update-grub&&
reboot
```

module after compile
![image](https://hackmd.io/_uploads/SJukjBlGkl.png)
可以看到沒有生成.ko檔案

同時這些檔案也可以在`Computer/lib/modules/6.11.7/build/lab1`中看到
![image](https://hackmd.io/_uploads/Sk6rlPgGkl.png)


重新啟動後查詢
![image](https://hackmd.io/_uploads/S1_MqSgM1e.png)

也可以透過modprobe 手動載入build-in module lab1，
若嘗試移除build-in module，會觸發FATAL error.
![image](https://hackmd.io/_uploads/B1DwY9xGkl.png)




# 使用MSP430fr4133讀取內建感測器溫度並寫入FRAM



使用CCS Composer Studio Theia IDE

:::spoiler 程式碼
```c=1

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



```
:::





## 程式碼介紹

程式除了`main()` function外主要包含了五個function:


### `init(void)`
設定ADC 類比數位轉換器、初始化按鈕設定以及LED設定。

#### 開發版功能設定
```c=24
WDTCTL = WDTPW | WDTHOLD;  // 停用看門狗
PM5CTL0 &= ~LOCKLPM5;      // 停用高阻抗
```
看門狗透過計數負責偵測程式異常，如果一段時間內沒有重至看門狗，看門狗就會認為系統發生故障，並讓開發版reset重新啟動程式。

高阻抗(就像是一種省電模式，可能會影響模組的運作)。

#### ADC 設定


![image](https://hackmd.io/_uploads/H1CBeJoMyl.png)

ADC設定參考於ti MSP430FR413x Demo - ADC, Sample A12 Temp.

```c=53
    // Configure ADC - Pulse sample mode; ADCSC trigger
    ADCCTL0 |= ADCSHT_8 | ADCON;  // ADC ON, 設定ADC sample-and-hold time為1000b = 256 ADCCLK cycles
    ADCCTL1 |= ADCSHP;  // s/w trig, single ch/conv, MODOSC
    ADCCTL2 |= ADCRES;  // 10-bit conversion results
    ADCMCTL0 |= ADCSREF_1 | ADCINCH_12;  // 設定ADC input 為ch A12 => 內建的temperture sensor

    // Configure reference
    PMMCTL0_H = PMMPW_H;  // Unlock the PMM registers
    PMMCTL2 |= INTREFEN | TSENSOREN;  // Enable internal reference and temperature sensor
    __delay_cycles(400);  // Delay for reference settling
```





#### `ADCCTL0 |= ADCSHT_8 | ADCON;`  
`ADCSHT_8`設定ADC Figure 21-1中的Sample& Hold元件要對欲採樣訊號採樣的cycle長度。  

![image](https://hackmd.io/_uploads/rySfNDhfJx.png)
![image](https://hackmd.io/_uploads/ByfU0LjGye.png)
![image](https://hackmd.io/_uploads/SJS0XvhMJg.png)
#### `ADCCTL1 |= ADCSHP;`  

![image](https://hackmd.io/_uploads/S1gLEDhGke.png)
![image](https://hackmd.io/_uploads/SJTLVD3fJl.png)

#### `ADCCTL2 |= ADCRES;`  
設定ADC的轉換精度。可以看到若經度設定越高conversion時間將越長。
![image](https://hackmd.io/_uploads/HkmdEwhGJe.png)
![image](https://hackmd.io/_uploads/S1Oc4w3fJx.png)
#### `ADCMCTL0 |= ADCSREF_1 | ADCINCH_12;`  
`ADCSREF_1`設定Reference 正負參考電壓來源，分別為內建的參考電壓源 (VREF)與地電位(AVSS)。
`ADCINCH_12`設定ADC轉換的訊號來源為A12(ch12)也就是內建的temperture sensor頻道。
![image](https://hackmd.io/_uploads/rkwaEDhzyl.png)
![image](https://hackmd.io/_uploads/Sy51Swhfyg.png)


#### 按鈕設定
```c=64
//設定按鈕
P1DIR &=~BIT2;
P1REN |= BIT2; 
P1DIR |= BIT0;
```
`P1DIR`控制P1.x中每個腳位分別為輸入(0)還是輸出模式(1)。  
> 在此將BIT2設定為代表P1.2也就是內建的其中一顆按鈕的針腳設定為輸入模式。  
![image](https://hackmd.io/_uploads/BJmRnD2fkg.png)

`P1REN`開啟或關閉P1.x中每個腳位上拉電阻(用於將沒有輸入信號的針腳拉高至高電位)。  

> 這裡將BIT2上拉式電阻開啟，也就是當按鈕沒被按下時P1OUT 中的BIT2會是1，被按下後會變成1。  
![image](https://hackmd.io/_uploads/rkQk6PhM1l.png)


### readTemperature(void)
於此funciton中實作了進行溫度讀取並存入FRAM的功能。
可以看到溫度由`ADCMEM0`被讀至`temp`中，並在轉換成`IntDegC`後被存入`*FRAM_write_ptr`。
```c=97
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
```


#### `ADCCTL0 |= ADCENC | ADCSC;`  
`ADCENC`將ADC轉換器設為Enable。  
`ADCSC`開始進行ADC轉換。  

![image](https://hackmd.io/_uploads/rySfNDhfJx.png)
![image](https://hackmd.io/_uploads/H1t4Tw3G1l.png)

#### `temp = ADCMEM0;`
先將tempture sensor讀取的溫度由ADCMEM0中讀出。  

![image](https://hackmd.io/_uploads/BJpC6Dnzke.png)

#### `IntDegC = (temp - CALADC_15V_30C) * (85 - 30) / (CALADC_15V_85C - CALADC_15V_30C) + 30;`  
進行溫度轉換

#### `FRAM_write_ptr = (float *)FRAM_TEST_START;`  
建立一個float pointer指向定義的FRAM記憶體位置`FRAM_TEST_START`(0x1800)。


#### `validation_ptr = (float *)FRAM_TEMP_VALIDATION;`
建立一個float pointer指向定義的FRAM 溫度可用性驗證位置。

#### `SYSCFG0 &= ~DFWP;`、`SYSCFG0 |= DFWP; `
於資料寫入前與寫入後分別開啟與關閉FRAM的寫入保護。
![image](https://hackmd.io/_uploads/rJC6Av2Gyl.png)
![image](https://hackmd.io/_uploads/SkLCCPnfJl.png)

#### `*FRAM_write_ptr = IntDegC;`
將資料寫入FRAM中的指定位置。

#### `*validation_ptr = 716;`
由於正確的寫入了溫度資料，故將validation位置的數值改為代表valid 716(自定義的數值)。



### float readFromFram(void)
實作由Fram中讀取數據

```c=112
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

```
會由FRAM中FRAM_TEST_START位至讀出float並進行回傳，但若validation位置儲存的資料不等於設定的float 716，則會判斷為FRAM中的溫度數據不可用，回傳0。


### int main(void)
```c=71
int main(void)
{
    init();
    initLCD();  // 初始化 LCD


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
```

首先呼叫`init();`進行ADC類比數位轉換器的設定。
接下來呼叫`initLCD();`初始化LCD設定。
進入主迴圈`while`。

```c=79
if(!(P1IN & BIT2)){
        readTemperature();
        __no_operation();
    }
```
此部分負責偵測只有按鈕被按下時(P1IN的BIT2為0)才會呼叫readTemperature()來讀取內建的溫度感測器並存入FRAM。

```c=83
stored_temperture = readFromFram();
displayFloat(stored_temperture);  // 顯示浮點數
if(stored_temperture>=30){
    P1OUT |=BIT0;
    __no_operation();
}else{
    P1OUT &=~BIT0;
    __no_operation();
}

```
迴圈中的此部分會不斷透過readFromFram() funcition來自指定的`0x1800`位置讀取數值，並透過displayFloat()來將這個由FRAM中讀取的溫度數值顯示到LCD上，下方的if、else會檢查如果儲存的溫度數值大於30度則使LEDP1.0(紅燈)亮起表示溫度異常。

透過將溫度存入FRAM，再使用FRAM讀出的資料來偵測是否過熱，可以讓系統實現在斷電復原後可以判斷斷電前最後一次讀取的溫度數值是否過熱。



### initLCD(void)
主要參考至ti LCD msp430fr413x_LCDE_01 Demo中的設定。

###  displayFloat(float num)
將傳入的float顯示至LCD螢幕上。
```c=158
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
```

首先將float拆解成十位數、個位數與小數第一位分別存成int t、t2、t3。
再分別將三個數字顯示至LCD上。







# Demo

## 基本功能測試
2-1、2-2:
設計按下button p1.2時才會執行readTemperature() function，由內建temperture sensor讀取溫度並存至FRAM，而LCD則會由FRAM讀取溫度數值並進行顯示。


{%youtube cK-jDgWYM7Q %}

https://youtube.com/shorts/cK-jDgWYM7Q?feature=share

驗證讀取的溫度被立即存入FRAM

{%youtube WM3f_6Re-A4 %}
https://youtube.com/shorts/WM3f_6Re-A4

## 讀取時驗證FRAM中的溫度是否是可用的功能Demo(補充)
由於前面Demo尚未加上由FRAM讀取溫度時驗證可用性的功能，故另外進行Demo。
將0x1824作為validation數字的FRAM存放位置，在啟動時只有當validation數字=716時才會讀取並顯示至LCD上，若啟動時validation數字儲存的數字非716則會當成FRAM中並沒有儲存可用的溫度數據。

於main() function中新增了測試程式碼:
```c=70
#ifdef Test_Validation    
validation_ptr = (float *)FRAM_TEMP_VALIDATION;
SYSCFG0 &= ~DFWP;
*validation_ptr = 123;
SYSCFG0 |= DFWP;
#endif  
```
若使用者有進行`#define Test_Validation `，系統會在啟動時將validation數字改為123，如此斷電重啟時就不會讀取FRAM中儲存的不可用溫度數值。
{%youtube fKfczVbrQJ8 %}
https://youtu.be/fKfczVbrQJ8
 
 
## FRAM讀取應用、斷電重啟測試
測試透過FRAM中紀錄的溫度數值判斷是否過熱的功能。
測試斷電重啟並透過讀取FRAM中的數值繼續工作(感應斷電前最後紀錄的溫度是否過熱)。


{%youtube 53QD02jaVkU%}
https://youtu.be/53QD02jaVkU













# reference:
https://blog.csdn.net/zhangbijun1230/article/details/81253871


:::spoiler 其他一些學習資料
# ADC Core的運作、功能、目的以及原理


ADC Core，也就是類比數位轉換器核心，用於將連續變化的類比訊號轉換成離散的數位訊號的電子元件或電路。

![image](https://hackmd.io/_uploads/H1CBeJoMyl.png)

ADCINChx:
這是ADC的輸入通道，用於連接各種類比訊號源，如感測器、電位器等。
x代表輸入緩衝器的選擇。


VEREF-, VREF+:
這是ADC的參考電壓，用於定義ADC的量測範圍。
VREF-通常接地，VREF+則可以接內建的參考電壓源或外部電壓源。

Sync訊號 就像是ADC的啟動按鈕，告訴ADC什麼時候開始工作。
ADCDIVx訊號 就像是ADC的工作節奏，影響ADC轉換的速度和精度




21.2.6 Conversion Result

 For all conversion modes, the conversion result is accessible by reading the ADCMEM0 register. When a
 conversion result is written to ADCMEM0, the ADCIFG0 interrupt flag is set.
 
 
  21.2.7.1 Single-Channel Single-Conversion Mode
 A single channel selected by ADCINCHx is sampled and converted once. The ADC result is written to
 ADCMEM0.
 ![image](https://hackmd.io/_uploads/HJ7uilsMJe.png)





:::


