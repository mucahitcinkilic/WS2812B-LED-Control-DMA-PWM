#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "timer.h"
#include "oled.h"
#include "ws2812b.h"

extern uint16_t aSRC_Buffer[634];  // Buffer for storing RGB values for the LEDs
uint16_t uhTimerPeriod = 0;  // Variable for storing timer period
extern volatile int DMA_TransferCount;  // DMA transfer count to track DMA operations

void init(void)
{
  // I/O ports initialization
  Sys_IoInit();
  
  // System Clock Tick initialization (currently commented out)
  //Sys_TickInit();
  
  // Console initialization
  Sys_ConsoleInit();

  // LED initialization, setting its initial state to ON
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT_OD);  
}


void Task_Print(void)
{
   // Display the DMA transfer count on the OLED screen
   OLED_SetCursor(0, 0);
   printf("DMA_TransferCount=\n%8d", DMA_TransferCount);
}


void main()
{
  // Initial configurations
  init();
  
  // Set the font for OLED display
  OLED_SetFont(FNT_BIG);
 
  // Compute the value to be set in the ARR register to generate a signal frequency at 17.57 KHz
  uhTimerPeriod = (SystemCoreClock / 800000 ) - 1;
 
  // Initialize peripherals for WS2812B LEDs control
  TIM1_DMA_Init(aSRC_Buffer);  // Initialize DMA for TIM1 with the LED buffer
  NVIC_Config();  // Configure interrupts
  TIM1_Init(uhTimerPeriod);  // Initialize TIM1 with the calculated period
  TIM1_OC_Init();  // Initialize TIM1 Output Compare for PWM control
  Enable_DMA_PWM();  // Enable DMA for PWM signal generation

  // Task loop
  while (1) 
  {
    // Sequentially turn on the LEDs with different RGB values
    sequentialLedOn(aSRC_Buffer, 100, 255, 0, 0);  // Red
    sequentialLedOn(aSRC_Buffer, 100, 0, 255, 0);  // Green
    sequentialLedOn(aSRC_Buffer, 100, 0, 0, 255);  // Blue
    sequentialLedOn(aSRC_Buffer, 100, 255, 255, 0);  // Yellow
    sequentialLedOn(aSRC_Buffer, 100, 0, 255, 255);  // Cyan
    sequentialLedOn(aSRC_Buffer, 100, 255, 0, 255);  // Magenta
    sequentialLedOn(aSRC_Buffer, 100, 255, 255, 255);  // White
    
    // Print the DMA transfer count
    Task_Print();
  }
}