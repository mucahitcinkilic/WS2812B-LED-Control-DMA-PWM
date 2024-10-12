#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TIMER_1,
  TIMER_2,
  TIMER_3,
  TIMER_4,
  TIMER_5,
} TIMER_t;

typedef enum {
  PWM_N,
  PWM_R
}pwm_Mode;
    

void TIM1_Init(unsigned int period);
void TIM1_OC_Init(void);
void TIM1_DMA_Init(uint16_t * MemoryAdr);
void Enable_DMA_PWM(void);
void Disable_DMA_PWM(void);

extern volatile int DMA_TransferCount;
extern uint16_t aSRC_Buffer[634];

void NVIC_Config(void);
  
#ifdef __cplusplus
}
#endif

#endif
