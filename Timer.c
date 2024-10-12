#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "io.h"
#include "system.h"
#include "timer.h"

// Function to initialize TIM1 with a specific period
void TIM1_Init(unsigned int period){
  
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  
  /* Enable TIM1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  /* Time Base configuration */
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = period;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
}

// Function to initialize TIM1 Output Compare in PWM mode on Channel 3
void TIM1_OC_Init(void){
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  /* Channel 3 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OC3Init(TIM1, &TIM_OCInitStructure);

  /* Enable preload feature */
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
}

// Function to initialize DMA for TIM1 to control PWM signals
void TIM1_DMA_Init(uint16_t * MemoryAdr)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;
  
  /* Enable GPIOA and GPIOB clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

  /* GPIOA Configuration: Channel 3 as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_TIM1);

  /* GPIOB Configuration: Channel 3N as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_TIM1);
  
  /* Enable DMA clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  
  /* De-initialize DMA Stream */
  DMA_DeInit(DMA1_Stream6); // Recommended to deinitialize before configuration

  /* DMA configuration for TIM1 Channel 3 */
  DMA_InitStructure.DMA_Channel = DMA_Channel_6;  // TIM1 Channel 3 uses DMA Channel 6
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(TIM1->CCR3); // Address of TIM1 Channel 3 CCR register
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)MemoryAdr;  // Memory address of the data to be transferred
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;  // Data transfer from memory to peripheral
  DMA_InitStructure.DMA_BufferSize = 634;  // Size of the buffer
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  // Peripheral address remains constant
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  // Memory address will be incremented after each transfer
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  // Data size for peripheral
  DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_HalfWord;  // Data size for memory
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // Normal mode
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;  // Set DMA priority to high
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  // Disable FIFO mode
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  
  DMA_Init(DMA2_Stream6, &DMA_InitStructure);  // Initialize DMA Stream 6
}

// Function to enable DMA and PWM signal generation
void Enable_DMA_PWM(void){
  // Enable DMA
  DMA_Cmd(DMA2_Stream6, ENABLE);

  // Enable TIM1 Update DMA Request 
  TIM_DMACmd(TIM1, TIM_DMA_CC3, ENABLE);

  // Enable TIM1 main output 
  TIM_CtrlPWMOutputs(TIM1, ENABLE);

  // Enable TIM1 counter
  TIM_Cmd(TIM1, ENABLE);
}

// Function to disable DMA and PWM signal generation
void Disable_DMA_PWM(void){
  DMA_Cmd(DMA2_Stream6, DISABLE); // Disable DMA
  
  // Disable TIM1 Update DMA Request 
  TIM_DMACmd(TIM1, TIM_DMA_CC3, DISABLE);
  
  // Disable TIM1 main output 
  TIM_CtrlPWMOutputs(TIM1, DISABLE);
  
  // Disable TIM1 counter
  TIM_Cmd(TIM1, DISABLE);
}

// Function to configure NVIC for DMA interrupt handling
void NVIC_Config(void) {
  /////////////////////////////////////////////////
  // DMA transfer complete interrupt should be activated
  // To enter ISR, F and E flags must be set. F will be set when transfer completes
  DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);  // Clear interrupt flag (F = 0)
  DMA_ITConfig(DMA2_Stream6, DMA_IT_TC, ENABLE);      // Enable interrupt (E = 1)
  
  NVIC_SetPriority(DMA2_Stream6_IRQn, 1);  // Set priority of the interrupt
  NVIC_EnableIRQ(DMA2_Stream6_IRQn);  // Enable the interrupt request
}

uint16_t aSRC_Buffer[634];  // Buffer for storing data to be transferred via DMA
volatile int DMA_TransferCount = 0;  // Counter to track DMA transfers

// DMA interrupt handler
void DMA2_Stream6_IRQHandler(void) {
    // Check if the interrupt was triggered by transfer completion
    if (DMA_GetITStatus(DMA2_Stream6, DMA_IT_TCIF6)) {
        // Transfer complete
        
        ++DMA_TransferCount;  // Increment transfer count
        
        Disable_DMA_PWM();  // Disable DMA and PWM after transfer is complete
        
        // Clear interrupt flags
        DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);  
        DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6);
    }
}