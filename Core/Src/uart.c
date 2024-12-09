/*
 * uart.c
 *
 *  Created on: Sep 26, 2023
 *      Author: HaHuyen
 */
#include "uart.h"

#define BUFFER_SIZE 128

uint8_t receive_buffer1 = 0;
uint8_t msg[100];

// Ring buffer structure

uint8_t RingBuffer[BUFFER_SIZE];
volatile uint16_t head=0;
volatile uint16_t pointer=0;

uint8_t dataReady = 0;
//uint8_t Full=0;

// Ring buffer instance
//head = 0;
//pointer = 0;
//Check if full
//bool BufferFull(){
//	return Full;
//}

// Function to add data to the buffer
void BufferWrite(uint8_t data) {

    RingBuffer[head] = data;
    head = (head + 1) % BUFFER_SIZE;
}
void BufferReading(){
	while(pointer != head){
		uint8_t data = RingBuffer[pointer];
		pointer=(pointer+1) % BUFFER_SIZE;
		uart_Rs232SendNum(data);
		uart_Rs232SendString("	");
	}
	dataReady=0;
}
void uart_init_rs232(){
	HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
}

void uart_Rs232SendString(uint8_t* str){
	HAL_UART_Transmit(&huart1, (void*)msg, sprintf((void*)msg,"%s",str), 10);
}

void uart_Rs232SendBytes(uint8_t* bytes, uint16_t size){
	HAL_UART_Transmit(&huart1, bytes, size, 10);
}

void uart_Rs232SendNum(uint32_t num){
	if(num == 0){
		uart_Rs232SendString("0");
		return;
	}
    uint8_t num_flag = 0;
    int i;
	if(num < 0) uart_Rs232SendString("-");
    for(i = 10; i > 0; i--)
    {
        if((num / mypow(10, i-1)) != 0)
        {
            num_flag = 1;
            sprintf((void*)msg,"%d",num/mypow(10, i-1));
            uart_Rs232SendString(msg);
        }
        else
        {
            if(num_flag != 0)
            	uart_Rs232SendString("0");
        }
        num %= mypow(10, i-1);
    }
}

void uart_Rs232SendNumPercent(uint32_t num)
{
	sprintf((void*)msg,"%ld",num/100);
    uart_Rs232SendString(msg);
    uart_Rs232SendString(".");
    sprintf((void*)msg,"%ld",num%100);
    uart_Rs232SendString(msg);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART1){
		// rs232 isr
		// Store received data in ring buffer
		BufferWrite (receive_buffer1);
		dataReady = 1; // Set flag to indicate new data



		// turn on the receice interrupt
		HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
	}
}




