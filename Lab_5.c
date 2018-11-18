#include "stm32f7xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "GLCD_Config.h"                // Keil.STM32F746G-Discovery::Board Support:Graphic LCD
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include <math.h>

static void SystemClock_Config(void);
extern GLCD_FONT GLCD_Font_16x24;

signed char read_kypd(void){
	
	signed char OUTPUT;
	int INPUT,i,j;
	int tab[16] = {1,4,7,0,2,5,8,15,3,6,9,14,10,11,12,13};// reference table
	OUTPUT = -1;                                // default OUTPUT value
	GPIOI -> BSRR = 0x0F;                       // set COLs to 1
	for(i = 0; i < 4;i++){
		GPIOI -> BSRR = (1<<(16+i));            //set COLi to 0
		INPUT=((GPIOF->IDR&0x3C0)>>6);          //read ROWs from GPIOF
		
		if (INPUT != 0x0F && OUTPUT != -1){     //multiple keys pressed
				return -2;
			}
		for(j = 0; j < 4; j++){
			if(((INPUT>>j)&0x01)==0){           //a 0 is detected on ROWj
				OUTPUT = tab[i*4 +j];
			}
		}
		GPIOI -> BSRR = (1<<i);                 //set COLi back to 0
		
	}
	return OUTPUT;
	
}

int main(void)
{
	int key,prev_key,state,reset,prev_reset;
	int password[4] = {5,3,9,2};    //password
	SystemClock_Config();
	
	RCC->AHB1ENR |= (1<<5)|(1<<8);  //initialize GPIOI,GPIOF
	GPIOI->MODER |= (1<<0)|(1<<2)|(1<<4)|(1<<6);//set PI0-PI3 to OUTPUT mode
	
	GLCD_Initialize();              //initialize GLCD
	GLCD_SetFont(&GLCD_Font_16x24); //set font
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK); //set foreground color
	GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);  //set background color
	GLCD_ClearScreen();
	state,prev_key,reset = 0;
	key = -1;
	prev_reset = 1;
	
	while (1){
		key = read_kypd();          //read OUTPUT from keypad
		reset = GPIOI->IDR&(1<<11); //read reset from PI11
		if ((key != prev_key)||(reset != prev_reset)){ //check whether there is new reading
			GLCD_ClearScreen();
			prev_key = key;         //update prev_key
			prev_reset = reset;     //update prev_reset
			if ((key != -1)){       //check whether a key is pressed
                //state machine
				switch(state){
					case 0:{
						if(key == password[state]){
							state += 1;
						}else{
                            state = 0;
						}
						break;
					}
					case 1:{
						if(key == password[state]){
							state += 1;
						}else if(key == password[state-1]){
							state = 1;
						}else{
							state = 0;
						}
						break;
					}
					case 2:{
						if(key == password[state]){
							state += 1;
						}else if(key == password[state-2]){
							state = 1;
						}else{
							state = 0;
						}
						break;
					}
					case 3:{
						if(key == password[state]){
							state += 1;
						}else if(key == password[state-3]){
							state = 1;
						}else{
							state = 0;
						}
						break;
                    }
				}
            //system is unlocked
			}else if(state == 4){
				GLCD_DrawString(GLCD_SIZE_X / 2 - 50, GLCD_SIZE_Y/2, "System Unlocked");
                //if reset button is pressed, reset the system
				if (reset == 1){
					state = 0;
				}
			}
			else{
				continue;
			}
		}else{
			continue;
		}
	}
	return 0;
}

void SysTick_Handler (void)
{
    HAL_IncTick();
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}
