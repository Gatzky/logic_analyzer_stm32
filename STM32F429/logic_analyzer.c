#include "logic_analyzer.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "usart.h"

//TYPES
enum mode {
	MODE_STOP = 0U,										//no action is performed
	MODE_CATCHING_FIRST_DATA = 1U,		//stm try to find first change of sample (0->1 or 1->0)
	MODE_COLLECT_DATA = 2U,						//collecting data to buffer
	MODE_SEND_DATA = 3U								//send data from buffer by uart
};

//DEFINES
#define GPIO_PORT GPIOE
#define GPIO_PIN GPIO_PIN_0
#define PRESCALE_VALUE 90
#define PERIOD_VALUE 1
#define BUFFER_LENGTH 250
#define BUFFER_ELEMENT_LENGTH 255

//GLOBAL VARIABLE
uint8_t buffer[BUFFER_LENGTH];
uint16_t countBufferValue = 0;
uint16_t countBufferQuanity = 1;
enum mode logicA_mode = MODE_STOP;
uint8_t prev_sample = 2;
uint8_t temp_sample = 2;

static TIM_HandleTypeDef s_TimerInstance = { 
	.Instance = TIM2
};

//
//Function to initialize GPIO
void logicA_InitializeGPIO(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// GPIO Ports Clock Enable
	if(GPIO_PORT == GPIOA)
		__GPIOA_CLK_ENABLE();
	else if (GPIO_PORT == GPIOB)
		__GPIOB_CLK_ENABLE();
	else if (GPIO_PORT == GPIOC)
		__GPIOC_CLK_ENABLE();
	else if (GPIO_PORT == GPIOD)
		__GPIOD_CLK_ENABLE();
	else if (GPIO_PORT == GPIOE)
		__GPIOE_CLK_ENABLE();
	else if (GPIO_PORT == GPIOF)
		__GPIOF_CLK_ENABLE();
	else if (GPIO_PORT == GPIOG)
		__GPIOG_CLK_ENABLE();
	else if (GPIO_PORT == GPIOH)
		__GPIOH_CLK_ENABLE();
	
	//Configure PIN
	GPIO_InitStruct.Pin = GPIO_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // digital Input
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIO_PORT, &GPIO_InitStruct); 
}
//
//Function to initialize timer
void logicA_InitializeTimer(void){
	__TIM2_CLK_ENABLE();
	s_TimerInstance.Init.Prescaler = PRESCALE_VALUE;
	s_TimerInstance.Init.CounterMode = TIM_COUNTERMODE_UP;
	s_TimerInstance.Init.Period = PERIOD_VALUE;
	s_TimerInstance.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	s_TimerInstance.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&s_TimerInstance);
	HAL_TIM_Base_Start_IT(&s_TimerInstance);
}

//
//Configures EXTI Line2 (connected to PG2 pin) in interrupt mode
static void logicA_InitializeEXTI2(void){
  GPIO_InitTypeDef  GPIO_InitStructure;
        
  // Enable GPIOG clock
  __GPIOG_CLK_ENABLE();
  
  // Configure PG2 pin as input with EXTI interrupt on the falling edge and pull-up
  GPIO_InitStructure.Speed = GPIO_SPEED_LOW;
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
 
        
  // Enable and set EXTI Line2 Interrupt to the lowest priority
  HAL_NVIC_SetPriority(EXTI2_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
}
//
//Function to initialize logic analyzer (GPIO & Timer)
void logicA_Initialize(void){
	logicA_InitializeGPIO();
	logicA_InitializeTimer();
	logicA_InitializeEXTI2();
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
}
//
//Interupt Handler
void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&s_TimerInstance);
}
 
//
//This function handles External line 2 interrupt request.
void EXTI2_IRQHandler(void){
  // Check if EXTI line interrupt was detected
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)  {
    // Clear the interrupt (has to be done for EXTI)
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
		//Change mode
		logicA_mode = MODE_COLLECT_DATA;
  }
}
//
//This function add sample to buffer
void logicA_AddToBuffer(uint8_t bit){
	if(buffer[countBufferQuanity] == 0){													// If this buffer element is new...
		buffer[countBufferValue] = bit;															// ... overwrite value by actual bit
		buffer[countBufferQuanity]++;																// and increment quality value
	}
	else{
		if(buffer[countBufferValue] == bit){												// If bit and value is the same
			buffer[countBufferQuanity]++;															// Just increment quanity
			if(buffer[countBufferQuanity] >= BUFFER_ELEMENT_LENGTH){	// If buffer element is full
				countBufferValue += 2;																	// Move foward, new element will be desrcibed in next iteration by first if of this function
				countBufferQuanity += 2;
				if(countBufferQuanity > BUFFER_LENGTH){									// If buffer is full
					logicA_mode = MODE_SEND_DATA;
				}
			}
		}
		if(buffer[countBufferValue] != bit){												//If actual bit is diffrent from new bit
			countBufferValue += 2;																		//Go to next element
			countBufferQuanity += 2;
			if(countBufferQuanity > BUFFER_LENGTH){
				logicA_mode = MODE_SEND_DATA;
			}
			else{
				buffer[countBufferValue] = bit;													//If buffer is not full write data here
				buffer[countBufferQuanity]++;
			}
		}
	}
}
//
//Interupt Handler Support Function
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (logicA_mode == MODE_CATCHING_FIRST_DATA){
		prev_sample = temp_sample;
		temp_sample = HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN);
		if ((prev_sample =! temp_sample) && (prev_sample != 2) && (temp_sample != 2)){
			logicA_AddToBuffer(temp_sample);
			//buffer[0] = temp_sample;
			//countBufferQuanity = 1;
			logicA_mode = MODE_COLLECT_DATA;
		}
	}
	else if (logicA_mode == MODE_COLLECT_DATA){
		temp_sample = HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN);
		logicA_AddToBuffer(temp_sample);
		
		/*if (countBufferQuanity < BUFFER_LENGTH){
			buffer[countBufferQuanity] = HAL_GPIO_ReadPin(GPIO_PORT, GPIO_PIN);
		}
		else{
			countBufferQuanity = 0;
			logicA_mode = MODE_SEND_DATA;
		}
		countBufferQuanity++;*/
	}
}
//
//Function to sending data by uart
void logicA_SendData(void){
	uint16_t i = BUFFER_LENGTH;
	//USART_WriteData(&i, sizeof(i));										//send size of buffer to synchro with Python
	for (i = 0; i < BUFFER_LENGTH; i++){
		USART_WriteData(&buffer[i], sizeof(buffer[i]));
	}
}
//
//Function to clear buffer
void logicA_ClearBuffer(void){
	uint16_t i = 0;
	for (i = 0; i < BUFFER_LENGTH; i++){
		buffer[i] = 0;
	}
}

//
//Main function of library
void logicA_function(void){
	
	logicA_mode = MODE_CATCHING_FIRST_DATA; 
	//HAL_NVIC_DisableIRQ(EXTI2_IRQn);
	
	//Catching first data & collecting data
	logicA_ClearBuffer();
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
	while(logicA_mode == MODE_CATCHING_FIRST_DATA);
	while(logicA_mode == MODE_COLLECT_DATA);
	
	//Sending data
	HAL_NVIC_DisableIRQ(TIM2_IRQn);
	logicA_SendData();		
	logicA_mode = MODE_STOP;
	//HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	while(logicA_mode == MODE_STOP);
}
