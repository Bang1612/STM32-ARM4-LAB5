/*
 * clock.c
 *
 *  Created on: Nov 30, 2024
 *      Author: MY PC
 */

#include "clock.h"
#include <ctype.h>
int counter=0;
int try_count=0;
int temp=0;
int flag=0;
 uint8_t cal_hour;
 uint8_t cal_min;
 uint8_t cal_sec;
 uint8_t cal_date;
 uint8_t cal_day;
 uint8_t cal_month;
 uint8_t cal_year;

 uint8_t al_hours;
 uint8_t al_min;
 uint8_t al_sec;
char* update_value[6] = {"Seconds", "Minutes", "Hours", "Days", "Months", "Years"};
char* update_request[6] = {"Seconds?", "Minutes?", "Hours?", "Days?", "Months?", "Years?"};

void displayTime(){
	lcd_ShowIntNum(70, 100, ds3231_hours, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(110, 100, ds3231_min, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(150, 100, ds3231_sec, 2, GREEN, BLACK, 24);
	lcd_ShowIntNum(20, 130, ds3231_day, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(70, 130, ds3231_date, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(110, 130, ds3231_month, 2, YELLOW, BLACK, 24);
	lcd_ShowIntNum(150, 130, ds3231_year, 2, YELLOW, BLACK, 24);
}

void ChangeValue(int val){
	switch(val){
	case 0:
		cal_sec++;
		if(cal_sec > 59) cal_sec=0;
		break;
	case 1:
		cal_min++;
		if(cal_min > 59) cal_min=0;
		break;
	case 2:
		cal_hour++;
		if(cal_hour > 23) cal_hour =0;
		break;
	case 3:
		cal_day++;
		cal_date++;
		if(cal_day >28){
			if(cal_month ==2){
				if(cal_year%4 !=0){
					cal_day=1;
				}
				else{
					if(cal_day >29) cal_day=0;
				}
			}
		}
		else if(cal_month==4 || cal_month==6 || cal_month==9 || cal_month==11){
			if(cal_day > 30) cal_day=0;
		}
		else{
			if(cal_day>31) cal_day=0;
		}
		if(cal_date>8) cal_date=2;

		break;
	case 4:
		cal_month++;
		if(cal_month>12) cal_month=1;
		break;
	case 5:
		cal_year++;
		break;
	default:
		break;
	}
}

void ApplyChange(){
		ds3231_Write(ADDRESS_SEC, cal_sec);
		ds3231_Write(ADDRESS_MIN, cal_min);
		ds3231_Write(ADDRESS_HOUR, cal_hour);
		ds3231_Write(ADDRESS_DAY, cal_day);
		ds3231_Write(ADDRESS_DATE, cal_date);
		ds3231_Write(ADDRESS_MONTH, cal_month);
		ds3231_Write(ADDRESS_YEAR, cal_year);
}

void ChangeAlarm(){
	switch (counter) {
	case 0:
		al_sec++;
		break;
	case 1:
		al_min++;
		break;
	case 2:
		al_hours++;
		break;
	default:
		break;
	}
}

void fsm(uint16_t status){
	switch(status){
	case NORMAL:
		ds3231_ReadTime();
		displayTime();
		lcd_StrCenter(110, 20, "NORMAL", BLUE, YELLOW, 16, 0);
		if (ds3231_hours == al_hours && ds3231_min == al_min
				&& ds3231_sec == al_sec) {
			flag=1;
		}
		if(flag){
			lcd_StrCenter(110, 20, "ALARM", RED, YELLOW, 16, 0);
		}
		if(button_count[0] || button_count[14]){
			flag=0;
		}
		break;
	case CALIBRATE:
		flag=0;
		char title[50] ="Updating ";
		strcat(title, update_value[counter]);
		strcat(title,"...");
		if (button_count[3] == 1) {
			ChangeValue(counter);
		}

		if (button_count[3] == 40) {
			setTimer3(200);
			ChangeValue(counter);
		}
		if (flag_timer3 && button_count[3] > 40) {
			setTimer3(200);
			ChangeValue(counter);
		}



		if(button_count[12] != 0){

			counter++;
			if(counter==5) {
				ApplyChange();
				counter =0;
				status=SET_ALARM;
			}

		}
		lcd_StrCenter(110, 20, title, BLUE, YELLOW, 16, 0);
		break;
	case SET_ALARM:
		if (button_count[3] == 1) {
			ChangeAlarm(counter);
		}

		if (button_count[3] == 40) {
			setTimer3(200);
			ChangeAlarm(counter);
		}
		if (flag_timer3 && button_count[3] > 40) {
			setTimer3(200);
			ChangeAlarm(counter);
		}
		if(button_count[12] != 0){
			counter++;
			if(counter>2) counter =0;
		}
		lcd_StrCenter(110, 20, "SET ALARM", BLUE, YELLOW, 16, 0);
		break;
	case UART_UPDATING:
		if(!UpdateTime(counter)){
			status=NORMAL;
			counter=0;
		}
		else {
			counter++;
		}
		break;
	default:
		break;
	}
}

int UpdateTime(int counter){
	uart_Rs232SendString(update_request[counter]);
	uint8_t response[10];
	while (try_count < 4) {
		if (HAL_UART_Receive(&huart1, response, sizeof(response), 10000)
				== HAL_OK) {
			uint8_t data = atoi((char*) response);
			switch (counter) {
			case 0:
				while (data < 0 || data > 59 || !isdigit(data)) {
					if (data < 0 || data > 59 || !isdigit(data))
						uart_Rs232SendString("Number invalid");
					HAL_UART_Receive(&huart1, response, sizeof(response),
							10000);
					data = atoi((char*) response);
				}
				cal_sec=data;
				uart_Rs232SendString(update_value[counter]);
				uart_Rs232SendString(" changed to ");
				uart_Rs232SendNum(data);
				uart_Rs232SendString("\n");
				return 1;
				break;
			case 1:
				while (data < 0 || data > 59 || !isdigit(data)) {
					if (data < 0 || data > 59 || !isdigit(data))
						uart_Rs232SendString("Number invalid");
					HAL_UART_Receive(&huart1, response, sizeof(response),
							10000);
					data = atoi((char*) response);
				}
				cal_min=data;
				uart_Rs232SendString(update_value[counter]);
				uart_Rs232SendString(" changed to ");
				uart_Rs232SendNum(data);
				uart_Rs232SendString("\n");
				return 1;
				break;
				case 2:
				while (data < 0 || data > 23 || !isdigit(data)) {
					if (data < 0 || data > 23 || !isdigit(data))
						uart_Rs232SendString("Number invalid");
					HAL_UART_Receive(&huart1, response, sizeof(response),
							10000);
					data = atoi((char*) response);
				}
				cal_hour=data;
				uart_Rs232SendString(update_value[counter]);
				uart_Rs232SendString(" changed to ");
				uart_Rs232SendNum(data);
				uart_Rs232SendString("\n");
				return 1;
				break;
			case 3:
				while (data < 0 || data > 31 || !isdigit(data)) {
					if (data < 0 || data > 31 || !isdigit(data))
						uart_Rs232SendString("Number invalid");
					HAL_UART_Receive(&huart1, response, sizeof(response),
							10000);
					data = atoi((char*) response);
				}
				cal_day=data;
				uart_Rs232SendString(update_value[counter]);
				uart_Rs232SendString(" changed to ");
				uart_Rs232SendNum(data);
				uart_Rs232SendString("\n");
				return 1;
				break;
			case 4:
				while (data < 0 || data > 12 || !isdigit(data)) {
					if (data < 0 || data > 12 || !isdigit(data))
						uart_Rs232SendString("Number invalid");
					HAL_UART_Receive(&huart1, response, sizeof(response),
							10000);
					data = atoi((char*) response);
				}
				cal_month=data;
				uart_Rs232SendString(update_value[counter]);
				uart_Rs232SendString(" changed to ");
				uart_Rs232SendNum(data);
				uart_Rs232SendString("\n");
				return 1;
				break;
			case 5:
				while (data <0 || !isdigit(data)) {
					if (data < 0 || !isdigit(data))
						uart_Rs232SendString("Number invalid");
					HAL_UART_Receive(&huart1, response, sizeof(response),
							10000);
					data = atoi((char*) response);
				}
				cal_year=data;
				uart_Rs232SendString(update_value[counter]);
				uart_Rs232SendString(" changed to ");
				uart_Rs232SendNum(data);
				uart_Rs232SendString("\n");
				ApplyChange();
				return 0;
				break;
			default:
			break;
		}
			break;
	}
	try_count++;
}

if(try_count ==4) {
	lcd_StrCenter(150, 110, "TIME OUT", RED, YELLOW, 12, 0);
	return 0;
}

}


