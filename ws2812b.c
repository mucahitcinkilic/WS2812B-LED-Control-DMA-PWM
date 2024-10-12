#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "io.h"
#include "system.h"
#include "timer.h"
#include "ws2812b.h"

#define LED_BITS 24  // Number of bits per LED (for GRB values)
#define LED_COUNT 16  // Number of LEDs
#define RESET_PERIOD 250  // Reset period in array elements
#define TOTAL_ELEMENTS 634  // Total elements including reset period

typedef enum{
    SendZero,  // Command to send a '0' bit
    SendOne,   // Command to send a '1' bit
    Reset,     // Command to send the reset signal
} Ws2812BitCmd;

/*
Timing for ws2812b_v2:
t0H = 300nS, ±80nS
t0L = 790nS, ±210nS

T1H = 790nS, ±210nS
T1L = 300nS, ±80nS

Reset signal duration >280µS
*/

// Function to set a specific LED to the given RGB values
void ws2812b_Set_Leds(uint16_t *dataArray, unsigned char red, unsigned char green, unsigned char blue, unsigned char led_no){
   uint16_t uhTimerPeriod2 = 104;
    
    uint16_t ValueDuties[3] = {
        (uint16_t)(((uint32_t)26 * uhTimerPeriod2) / 100),  // 0 bit duty cycle (~36%)
        (uint16_t)(((uint32_t)75 * uhTimerPeriod2) / 100),  // 1 bit duty cycle (~64%)
        (uint16_t)(0),  // Reset signal
    };
    
    // Initialize all data array values to '0' except the reset period
    for (int i = 0; i < (LED_BITS * LED_COUNT); ++i) {
        dataArray[i] = ValueDuties[SendZero];
    }
    
    int start_index = led_no * LED_BITS;
    
    // Insert the GRB data values corresponding to the specific LED
    for (int i = 0; i < 8; i++) {
        dataArray[start_index + i] = (green & (1 << (7 - i))) ? ValueDuties[SendOne] : ValueDuties[SendZero];
    }

    for (int i = 0; i < 8; i++) {
        dataArray[start_index + 8 + i] = (red & (1 << (7 - i))) ? ValueDuties[SendOne] : ValueDuties[SendZero];
    }

    for (int i = 0; i < 8; i++) {
        dataArray[start_index + 16 + i] = (blue & (1 << (7 - i))) ? ValueDuties[SendOne] : ValueDuties[SendZero];
    }
    
    // Set the reset period values at the end of the data array
    for (int i = 384; i < 634; ++i)
        dataArray[i] = ValueDuties[Reset];
}

// Function to set RGB values for all LEDs
// The sending order is GRB
void ws2812b_Set_RGB_Values(uint16_t *dataArray, unsigned char red, unsigned char green, unsigned char blue){
   uint16_t uhTimerPeriod2 = 104;
    
    uint16_t ValueDuties[3] = {
        (uint16_t)(((uint32_t)26 * uhTimerPeriod2) / 100),  // 0 bit duty cycle (~36%)
        (uint16_t)(((uint32_t)75 * uhTimerPeriod2) / 100),  // 1 bit duty cycle (~64%)
        (uint16_t)(0),  // Reset signal
    };
    
    // Convert each color component (GRB) into PWM signals
    for (int j = 0; j < 384; j += 24) {
        // Green color bits
        for (int i = 0; i < 8; i++) {
            dataArray[i + j] = (green & (1 << (7 - i))) ? ValueDuties[SendOne] : ValueDuties[SendZero];
        }

        // Red color bits
        for (int i = 0; i < 8; i++) {
            dataArray[i + j + 8] = (red & (1 << (7 - i))) ? ValueDuties[SendOne] : ValueDuties[SendZero];
        }

        // Blue color bits
        for (int i = 0; i < 8; i++) {
            dataArray[i + j + 16] = (blue & (1 << (7 - i))) ? ValueDuties[SendOne] : ValueDuties[SendZero];
        }
    }
    
    // Set the reset period values at the end of the data array
    for (int i = 384; i < 634; ++i)
        dataArray[i] = ValueDuties[Reset];
}

// Function to sequentially turn on LEDs with specified RGB values and delays
void sequentialLedOn(uint16_t *dataArray, unsigned long delay, unsigned char red, unsigned char green, unsigned char blue){
    for (int i = 0; i < LED_COUNT; ++i) {
        DelayMs(delay);  // Delay between each LED activation
        ws2812b_Set_Leds(dataArray, red, green, blue, i);  // Set RGB values for the current LED
        Enable_DMA_PWM();  // Enable DMA for PWM transmission to the LEDs
    }
}