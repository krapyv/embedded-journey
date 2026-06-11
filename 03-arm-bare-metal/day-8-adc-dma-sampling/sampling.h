#ifndef SAMPLING_H
#define SAMPLING_H

#include "stm32f411.h"

/* Plain integer tokens for the preprocessor */

/* Sampling time selection*/
#define ADC_SMPR_3_CYCLES 0x0U
#define ADC_SMPR_15_CYCLES 0x1U
#define ADC_SMPR_28_CYCLES 0x2U
#define ADC_SMPR_56_CYCLES 0x3U
#define ADC_SMPR_84_CYCLES 0x4U
#define ADC_SMPR_112_CYCLES 0x5U
#define ADC_SMPR_144_CYCLES 0x6U
#define ADC_SMPR_480_CYCLES 0x7U

/* ADC prescaler */
#define ADC_PSC_2 0x0U
#define ADC_PSC_4 0x1U
#define ADC_PSC_6 0x2U
#define ADC_PSC_8 0x3U

#define HARDWARE_PORT_A 0U
#define HARDWARE_PORT_B 1U

#define TARGET_SAMP_PORT HARDWARE_PORT_A
#define TARGET_ADC_PIN 0U
#define TARGET_ADC_CHANNEL 0U
#define ADC_SAMPLE_CYCLES ADC_SMPR_15_CYCLES
#define ADC_PRESCALER ADC_PSC_6

#define ADC_BUFFER_SIZE 1000U

extern volatile uint16_t adc_raw_buffer[ADC_BUFFER_SIZE];
extern volatile uint32_t overrun_count;
extern volatile uint8_t dma_half_a_ready;
extern volatile uint8_t dma_half_b_ready;

void clocks_init(void);
void gpio_init(void);
void tim_init(void);
void dma_init(void);
void nvic_init(void);
void adc_init(void);
void uart_init(void);
void pipeline_enabling(void);
#endif