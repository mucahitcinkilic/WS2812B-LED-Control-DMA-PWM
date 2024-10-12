#ifndef __WS2812B_H
#define __WS2812B_H

#ifdef __cplusplus
extern "C" {
#endif
  
void ws2812b_Set_Leds(uint16_t *dataArray, unsigned char red, unsigned char green, unsigned char blue, unsigned char led_no);
void ws2812b_Set_RGB_Values(uint16_t *dataArray,unsigned char red,unsigned char green,unsigned char blue);
void sequentialLedOn(uint16_t *dataArray,unsigned long delay,unsigned char red,unsigned char green,unsigned char blue);


 
#ifdef __cplusplus
}
#endif

#endif
