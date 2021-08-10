#include "stm32f4xx.h"                  
#include "system_timetick.h"

#define ADC_BUFFSIZE 2
#define RXBUFF 16
#define TXBUFF 19

struct GPIO
{
	uint8_t mode, setOut1, setOut2, in1Result, in2Result;
} pin_B;

struct ADCStruct
{
	double adc_1, adc_2, adc_3, adc_4;
	uint8_t adcChar_1[3], adcChar_2[3],adcChar_3[3],adcChar_4[3];
}adc;

struct DACStruct
{
	double dac_1, dac_2;
	uint8_t dacChar_1[3],dacChar_2[3];
}dac;


void init_main(void);
double convertChar2Double(uint8_t*charConv);
void convertDouble2Char(double x, uint8_t*charDes);
void GPIOactive(uint8_t mode, uint8_t setOut1, uint8_t setOut2);
void convert_Buffer(uint8_t*tx_buff, struct GPIO * pin, struct ADCStruct * adcs);
void charCopy(uint8_t*buffer_des, uint8_t*buffer_source, uint16_t num_bytes );

uint16_t ADC1_Data[ADC_BUFFSIZE], ADC2_Data[ADC_BUFFSIZE];
uint8_t  rxbuff[RXBUFF], txbuff[TXBUFF], check[3], check_flag = 0;
uint16_t DAC_value_1 = 0, DAC_value_2 = 0;
uint8_t je= 0xFF;


int main(void)
{
	init_main();
	ADC_SoftwareStartConv(ADC1);
	ADC_SoftwareStartConv(ADC2);
	SysTick_Config(SystemCoreClock/200);
  DAC_DualSoftwareTriggerCmd(ENABLE);
	
	while(1)
	{
		if(tick_flag)
		{
			tick_flag = 0;
			GPIOactive(pin_B.mode,pin_B.setOut1, pin_B.setOut2);
			DAC_SetDualChannelData(DAC_Align_12b_R, (uint16_t)(dac.dac_2*0x0FFF/3.3), (uint16_t)(dac.dac_1*0x0FFF/3.3));
			DAC_value_1 = DAC_GetDataOutputValue(DAC_Channel_1);
			DAC_value_2 = DAC_GetDataOutputValue(DAC_Channel_2);
			if(check_flag == 1)
				{	convert_Buffer(txbuff, &pin_B, &adc);
					DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
					DMA1_Stream4->NDTR = TXBUFF;
					DMA_Cmd(DMA1_Stream4, ENABLE);
					check_flag = 0;
				}		
			DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
			DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);
		}
	}
}

//=================================================================================

double convertChar2Double(uint8_t*charConv)
{
	double out;
	if (charConv[1] == '.')
	{
		out = (double)(charConv[0] - 0x30) + (double)(charConv[2]-0x30)/10;
	}
	else
		out = 0;
	return out;
}

void convertDouble2Char(double x, uint8_t*charDes)
{
	charDes[0] = (uint8_t)x + 0x30;
	charDes[1] = '.';
	charDes[2] = (uint8_t)((x - (uint8_t)x)*10) + 0x30;
}

void GPIOactive(uint8_t mode, uint8_t setOut1, uint8_t setOut2)
{
	switch(mode)
	{
		case 0: // all in
			GPIOB->MODER = 0;
			pin_B.in1Result = (uint8_t)(GPIOB->IDR & 0x00FF);
			pin_B.in2Result = (uint8_t)((GPIOB->IDR & 0xFF00)>>8);
			GPIO_ResetBits(GPIOD, GPIO_Pin_0|GPIO_Pin_2);
			GPIO_SetBits(GPIOD, GPIO_Pin_1|GPIO_Pin_3);
			break;
		case 1: // in1 out2
			GPIOB->MODER = 0x55550000;
			pin_B.in1Result = (uint8_t)(GPIOB->IDR & 0x00FF);
			pin_B.in2Result = 0;
			GPIOB->ODR =  ((uint16_t)setOut2)<<8; 
			GPIO_ResetBits(GPIOD, GPIO_Pin_0|GPIO_Pin_3);
			GPIO_SetBits(GPIOD, GPIO_Pin_1|GPIO_Pin_2);
			break;
		case 2: // in2 out1
			GPIOB->MODER = 0x00005555;
		  pin_B.in1Result = 0;
			pin_B.in2Result = (uint8_t)((GPIOB->IDR & 0xFF00)>>8);
			GPIOB->ODR =  (uint16_t)setOut1;
			GPIO_ResetBits(GPIOD, GPIO_Pin_1|GPIO_Pin_2);
			GPIO_SetBits(GPIOD, GPIO_Pin_0|GPIO_Pin_3);		
			break;
		case 3: // all out
			GPIOB->MODER = 0x55555555;
			pin_B.in1Result = 0;
		  pin_B.in2Result = 0;
			GPIOB->ODR = (uint16_t)((((uint16_t)setOut2) << 8)|((uint16_t)setOut1));
			GPIO_ResetBits(GPIOD, GPIO_Pin_1|GPIO_Pin_3);
			GPIO_SetBits(GPIOD, GPIO_Pin_0|GPIO_Pin_2);		
			break;
		default:
			break;
	}
}

void DMA1_Stream2_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);
	if ((rxbuff[0] == 0x02) && (rxbuff[15] == 0x03))
	{
		pin_B.mode = rxbuff[1];
		pin_B.setOut1 = rxbuff[2]|(rxbuff[3]<<7);
		pin_B.setOut2 = rxbuff[4]|(rxbuff[5]<<7);
		charCopy(dac.dacChar_1, &rxbuff[6],3);
		charCopy(dac.dacChar_2, &rxbuff[9],3);
		charCopy(check, &rxbuff[12],3);
		dac.dac_1 = convertChar2Double(dac.dacChar_1);
		dac.dac_2 = convertChar2Double(dac.dacChar_2);
		if((check[0] == 'a') && (check[1] == 'c') && (check[2] == 'k')) check_flag = 1;
		else check_flag = 0;
	}
}

void DMA2_Stream0_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC);
	adc.adc_1 = (double)ADC1_Data[0]*3.3/0x0FFF;
	adc.adc_2 = (double)ADC1_Data[1]*3.3/0x0FFF;
	convertDouble2Char(adc.adc_1, adc.adcChar_1);
	convertDouble2Char(adc.adc_2, adc.adcChar_2);
	DMA2_Stream0->NDTR = ADC_BUFFSIZE;
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, DISABLE);	
}

void DMA2_Stream3_IRQHandler(void)
{
	DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TC);
	adc.adc_3 = (double)ADC2_Data[0]*3.3/0x0FFF;
	adc.adc_4 = (double)ADC2_Data[1]*3.3/0x0FFF;
	convertDouble2Char(adc.adc_3, adc.adcChar_3);
	convertDouble2Char(adc.adc_4, adc.adcChar_4);
	DMA2_Stream3->NDTR = ADC_BUFFSIZE;
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, DISABLE);
}

void convert_Buffer(uint8_t*tx_buff, struct GPIO * pin, struct ADCStruct * adcs)
{
	tx_buff[0] = 0x02;
	tx_buff[17] = '\r';
	tx_buff[18] = '\n';
	if(pin->in1Result > 127)
		{
			tx_buff[1] = pin->in1Result & 127;
			tx_buff[2] = 1;
		}
	else
		{
			tx_buff[1] = pin->in1Result;
			tx_buff[2] = 0;			
		}
	if(pin->in2Result > 127)
		{
			tx_buff[3] = pin->in2Result & 127;
			tx_buff[4] = 1;
		}
	else
		{
			tx_buff[3] = pin->in2Result;
			tx_buff[4] = 0;			
		}

	charCopy(&tx_buff[5], adcs->adcChar_1, 3);
	charCopy(&tx_buff[8], adcs->adcChar_2, 3);
	charCopy(&tx_buff[11], adcs->adcChar_3, 3);
	charCopy(&tx_buff[14], adcs->adcChar_4, 3);
}

void charCopy(uint8_t*buffer_des, uint8_t*buffer_source, uint16_t num_bytes )
{
	for(int i = 0; i< num_bytes; i++)
	{
		buffer_des[i] = buffer_source[i];
	}
}


//========================================================================================

/*ADC: 12 bits - pin: PA0, PA1 (AI) - PA2, PA3 (DAC Results)
	DAC: 12 bits - pin: PA4, PA5(AO)
	IO: PORT B (Pin PB0 -> PB15)
	UART4 :
					baudrate: 115200
					data: 8 bits
					stop bit: 1
					parity: 0
					Pin: PC10(TX) - PC11(RX)
	FRAME:
				 ------ ---------------  ------------------  --------------------  -------------------  --------------------- ---------------- -----
		RX: | 0x02 | Mode (1 byte) | Set Out 1 (1 byte) | Set Out 2 (1 byte) | DAC 1 Set (3 bytes) | DAC 2 Set (3 bytes) | Check (3 bytes) |0x03 | 					(14 bytes)
				 ------ ---------------  ------------------  --------------------  -------------------  --------------------- ---------------- -----
			   ------ ---------------------- ---------------------- ----------------- ----------------- ------------------------ ------------------------ ---- ----
		TX: | 0x02 | In 1 Result (1 byte) | In 2 Result (1 byte) | ADC 1 (3 bytes) | ADC 2 (3 bytes) | DAC 1 Result (3 bytes) | DAC 2 Result (3 bytes) | \r | \n | (17 bytes)
				 ------ ---------------------- ---------------------- ----------------- ----------------- ------------------------ ------------------------ ---- ----
					
*/

void init_main(void)
{
	GPIO_InitTypeDef 				GPIO_InitStructure; 
	USART_InitTypeDef 			USART_InitStructure;  
	DMA_InitTypeDef   			DMA_InitStructure;
  NVIC_InitTypeDef  			NVIC_InitStructure;
	ADC_InitTypeDef					ADC_InitStructure;
	ADC_CommonInitTypeDef		ADC_CommonInitStructure;
	DAC_InitTypeDef					DAC_InitStructure;
	
	/*Cap xung*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	
	//ADC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1| GPIO_Pin_2 | GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init (GPIOA, &GPIO_InitStructure);

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
	
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2;
	ADC_Init(ADC1, &ADC_InitStructure);		ADC_Init(ADC2, &ADC_InitStructure);
		
	ADC_Cmd(ADC1, ENABLE);
	ADC_Cmd(ADC2, ENABLE);
		
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 2, ADC_SampleTime_3Cycles);
		
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
		
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC1_Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = ADC_BUFFSIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC);
		
	ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
		
	DMA_InitStructure.DMA_Channel = DMA_Channel_1;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC2->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC2_Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = ADC_BUFFSIZE;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream3, ENABLE);
	ADC_DMACmd(ADC2, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);
	DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TC);
	
	//DAC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);	
	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_Cmd(DAC_Channel_2, ENABLE);	
	
	//UART 4
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10|GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);
	
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UART4, &USART_InitStructure);

  USART_Cmd(UART4, ENABLE);

  USART_DMACmd(UART4, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
	
	//RX
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)rxbuff;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = RXBUFF;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream2, &DMA_InitStructure);
  DMA_Cmd(DMA1_Stream2, ENABLE);
	
	DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);
	
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  DMA_ITConfig(DMA1_Stream2, DMA_IT_TC, ENABLE);
	
	//TX
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR; 
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)txbuff; 
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; 
  DMA_InitStructure.DMA_BufferSize = TXBUFF; 
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; 
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; 
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; 
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;        
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull; 
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single; 
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream4, &DMA_InitStructure);
//DMA_Cmd(DMA1_Stream4, ENABLE);
	
	
	//GPIO
	GPIOB->MODER = 0;
	GPIOB->OTYPER = 0;
	GPIOB->PUPDR = 0xAAAAAAAA;
	GPIOB->OSPEEDR = 0xAAAAAAAA;
	
	//74LS373
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

