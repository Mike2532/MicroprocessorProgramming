#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t segments[] = {
	0b00111111,
	0b00000110,
	0b01011011,
	0b01001111,
	0b01100110,
	0b01101101,
	0b01111101,
	0b00000111,
	0b01111111,
	0b01101111
};

void InitPorts(void);
void send_data(uint8_t data, uint8_t ind);
void InitTimer0(void);
void Bin2Dec(uint16_t data);
void InitTimer1(void);
void StartTimer1(void);
void StopTimer1(void);
void SendData(uint8_t data);
void DisplayData(uint16_t data);

volatile uint16_t cnt = 0;
volatile uint16_t switch_state = 0;
volatile uint8_t bcd_buffer[] = {0, 0, 0, 0};

int main(void)
{
	InitPorts();
	InitSPI();
	InitTimer1();
	EIMSK |= (1 << INT0); //разрешение прерывания INT0
	EICRA |= (1 << ISC01); //запуск INT0 по заднему фронту
	sei(); //глобальное разрешение прерываний
	
	PORTB &= ~(1 << PB0); //OE = low(active)
	DisplayData(0);
	
	while(1)
	{}
}

ISR (TIMER1_COMPA_vect) 
{
	DisplayData(cnt);
	if (cnt < 9999)
		cnt++;
	else
		cnt = 0;
}

ISR (INT0_vect) //прерывание по нажатию кнопки
{
	if (switch_state == 0)
	{
		switch_state = 1;
		StartTimer1();
	}
	else
	{
		StopTimer1();
		DisplayData(cnt);
		switch_state = 0;
		cnt = 0;
	}
}

void InitPorts(void) 
{
	DDRB = (1 << PB0 | 1 << PB1); //в порте B пины 0 и 1 работают на выход
	DDRD &= ~(1 << PD2); //INT0 на вход
	PORTD |= (1 << PD2); //поддяжка на INT0
}

void InitTimer1(void)
{
	TCCR1A = 0;
	TCCR1B = (1 << CS11 | 1 << CS10 | 1 << WGM12); //тактовая частота F_CPU/64, режим работы СТС 
	TCNT1 = 0;
	OCR1A = 15624; //до этого числа считаем
}

void StartTimer1(void)
{
	TCNT1 = 0; //обнуление счетчика
	TIMSK1 |= (1 << OCIE1A); //разрешаем прерывание 
}

void StopTimer1(void)
{
	TIMSK1 &= ~(1 << OCIE1A); //запрещаем прерывание
}

void Bin2Dec(uint16_t data) //в массив bcd_buffer[] записывается значение переменной cnt
{
	bcd_buffer[3] = (uint8_t)(data / 1000);
	data = data % 1000;
	bcd_buffer[2] = (uint8_t)(data / 100);
	data = data % 100;
	bcd_buffer[1] = (uint8_t)(data / 10);
	data = data % 10;
	bcd_buffer[0] = (uint8_t)(data);
}

void SendData(uint8_t data)
{
	SPDR = data; //в регистр передачи данных записываем число, после чего сразу начинается передача данных
	while (!(SPSR & (1 << SPIF))); //остаемся в этом участке кода до тех пор, пока передача данных не закончится
}


void DisplayData(uint16_t data)
{
	Bin2Dec(data);
	PORTB &= ~(1 << PB1); //clk_out = 0
	SendData(segments[bcd_buffer[0]]);
	SendData(segments[bcd_buffer[1]]);
	SendData(segments[bcd_buffer[2]]);
	SendData(segments[bcd_buffer[3]]);
	PORTB |= (1 << PB1); //clk_out = 1
}

void InitSPI(void)
{
	DDRB |= (1 << PB3 | 1 << PB5); //настраиваем пины 3 и 5 порта В на выход
	SPSR |= (1 << SPI2X); //снижает скорость работы spi в два раза
	SPCR |= (1 << SPE | 1 << MSTR); //включение SPI, режим Master
	PORTB &= ~(1 << PB3 | 1 << PB5); //подаем нули на выходные пины
}