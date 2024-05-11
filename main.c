#include "stm32f4xx.h"
#include <string.h>
#include <stdio.h>											// string charlardan olusan dizi yani cumle olusturmak icin
#include <stdlib.h>

uint8_t adc_value;
uint16_t pwm_value;

int q=0;
int c=0;
/* Systick Handler ile bekleme olusturma */  

 void delay (uint32_t time)
{
	while(time)
	time--;	
}	

char str[50];  // 50 elemanli str tanimladik
uint32_t i=0, a=0;
char gelenveri[21] = {0};



void USART_Puts(USART_TypeDef* USARTx, volatile char *s) //usart için hangi usarti kullandigimizi belirttigimiz yer 
{
	while(*s) // burdaki yildiz pointer, salliyorum yukarda 20 karakterli bir sey girdik 20 kere dönsün ki tüm karakterleri atabilsin
	{
		while( USART_GetFlagStatus(USART2, USART_FLAG_TXE)== RESET); // usartx olarak tanimlanan modül sr olarak tasarlanan sr registerindaki bilgi 0x0000040 olmadigi sürece while içinde hapsolsun eger bu böyle olursa while içi 0 olur ve gerçeklesmez alt satira gecer ; ise while içindeki islem gerçeklestigi sürece islemci hicbrisey yapmaz ve içerisinin 0 olmasini bekler. bu bize usart mesgul oldugu sürece hiçbir sey yapmamasini saglatir
		USART_SendData(USARTx, *s); 
		*s++;
	}	
}

void USART2_IRQHandler(void)
{
	TIM_OCInitTypeDef TIM_OCInitStruct;
	TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_PWM1;  //Output compare or Output control mode Set pulse when CNT==CCRx
	TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;		
	TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High; // Active High, i.e 0 -> 1 starts duty cycle
	while( USART_GetFlagStatus(USART2, USART_FLAG_TXE)== RESET);
	char tmp = USART_ReceiveData(USART2);
	USART_SendData(USART2, tmp);
	gelenveri[i]=tmp;
	i++;
	
	if(tmp=='1')
	{
		for(int q=3600;q>0;q--)
		{
		TIM_OCInitStruct.TIM_Pulse=q;//CCR1_Val; Initial duty cycle
	  TIM_OC1Init(TIM4,&TIM_OCInitStruct);
	  TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);
		TIM_OCInitStruct.TIM_Pulse=q;//CCR2_Val; Initial duty cycle
	  TIM_OC2Init(TIM4,&TIM_OCInitStruct);
	  TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);
		delay(1680);	
		c=1;
			}				
	}
		
	
	if(tmp=='2')
	{
		for(int q=0;q<3601;q++)
		{
		TIM_OCInitStruct.TIM_Pulse=q;//CCR1_Val; Initial duty cycle
	  TIM_OC1Init(TIM4,&TIM_OCInitStruct);
	  TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);
		TIM_OCInitStruct.TIM_Pulse=q;//CCR2_Val; Initial duty cycle
	  TIM_OC2Init(TIM4,&TIM_OCInitStruct);
	  TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);
		c=0;
		delay(1680);		
		}
	}
}




GPIO_InitTypeDef GPIO_InitStruct;
ADC_InitTypeDef ADC_InitStruct;
ADC_CommonInitTypeDef ADC_CommonInitStruct;
TIM_TimeBaseInitTypeDef TIM_InitStruct;
TIM_OCInitTypeDef TIM_OCInitStruct;
USART_InitTypeDef  USART_InitStruct;
NVIC_InitTypeDef NVIC_InitStruct;

uint16_t Read_ADC()
{
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_56Cycles);
	ADC_SoftwareStartConv(ADC1);

	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

	return ADC_GetConversionValue(ADC1);
}

uint32_t map(long A, long B, long C, long D, long E)
{
	return (A * E) / C;
}

void config()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);  // rcc kütüphanesinden usart ve a portunun yerini bulup clock verdik

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF;   	// alnernative function (input-output-adc disinda farkli bir seyi yapacagimizi belirttik)
	GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_2 | GPIO_Pin_3; // beyaz A2 Yesil A3
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;    // veri gönderdigim push pull ve pull up olmak zorunda yoksa çalismaz
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz; //çok kritik degil
	GPIO_Init(GPIOA, &GPIO_InitStruct); // A portu out olarak ayarladik
	
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);

	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInit(&ADC_CommonInitStruct);
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_8b;
	ADC_Init(ADC1, &ADC_InitStruct);
	ADC_Cmd(ADC1, ENABLE);

	
	
	TIM_InitStruct.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_InitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStruct.TIM_Period = 19999;
	TIM_InitStruct.TIM_Prescaler = 84;
	TIM_InitStruct.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM4, &TIM_InitStruct);
	TIM_Cmd(TIM4, ENABLE);

	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	
	USART_InitStruct.USART_BaudRate 						= 115200; //verdigimiz hiz saniyede gönderilmesi gereken bit (8-10 metre tasiyacaksan hizi azaltman iyi olacaktir)
	USART_InitStruct.USART_HardwareFlowControl	= USART_HardwareFlowControl_None; // verinin kontrol edilip edilmeme durumu
	USART_InitStruct.USART_Mode								= USART_Mode_Tx | USART_Mode_Rx; // veri göndermek için bu kodu yazdik demek
	USART_InitStruct.USART_Parity              = USART_Parity_No;
	USART_InitStruct.USART_StopBits						= USART_StopBits_1;   
	USART_InitStruct.USART_WordLength					= USART_WordLength_8b;
	USART_Init(USART2,&USART_InitStruct);
	USART_Cmd(USART2, ENABLE);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE); //.Bir diger özellik ise USART bir veri alirsa kesme olusturabilsin diye RXNE kesmesini aktif ediyoruz.Bir çok kesme mevcut.
	

  NVIC_InitStruct.NVIC_IRQChannel=USART2_IRQn;
  NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
  NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
  NVIC_Init(&NVIC_InitStruct);

}


int main(void)
{
	config();
	
  while (1)
  {
		void USART2_IRQHandler();		
	  adc_value = Read_ADC();
	  pwm_value = map(adc_value, 0, 2000, 0, 19999);
	 
		if(c!=1)
		{	
	  TIM_OCInitStruct.TIM_Pulse = pwm_value;
	  TIM_OC1Init(TIM4, &TIM_OCInitStruct);
	  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
		delay(168000);
		}
	}	

}