#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t segments[] = {
	0b00111111, //0
	0b00000110, //1
	0b01011011, //2
	0b01001111, //3
	0b01100110, //4
	0b01101101, //5
	0b01111101, //6
	0b00000111, //7
	0b01111111, //8
	0b01101111 //9
};

void InitPorts(void);
void InitTimer1(void);
void Bin2Dec(uint16_t data);
void SendData(uint8_t data);
void DisplayData(uint16_t data);
void InitSPI(void);
void InitADC(void);
void InitUSART(void);
void SendString(char* buffer);
void SendChar(char symbol);

volatile uint8_t bcd_buffer[] = {0, 0, 0, 0};
volatile uint16_t display_val = 0;

int main(void)
{
	InitPorts();
	InitSPI();
	InitTimer1();
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC01);
	InitADC();
	InitUSART();
	sei();
	
	PORTB &= ~(1 << PB0);
	DisplayData(0);
	SendString("Hello\r\n");
	while(1)
	{
		DisplayData(display_val);
	}
}

ISR (TIMER1_COMPB_vect) {}
ISR (INT0_vect) 
{
	SendString("Value = "); 
	SendChar(0x30 + bcd_buffer[3]);
	SendChar(0x30 + bcd_buffer[2]);
	SendChar(0x30 + bcd_buffer[1]);
	SendChar(0x30 + bcd_buffer[0]);
	SendString("\r\n");
}

ISR (ADC_vect) //прерывание срабатывает когда завершено преобразование АЦП
{
	display_val = ADC; //запись преобразованного значения в переменную
}

ISR(USART_RX_vect)
{
	if(UDR0 == 0x20)
		SendString("Roger that\r\n");
}

void InitPorts(void)
{
	DDRB = (1 << PB0 | 1 << PB1 | 1 << PB3 | 1 << PB5);
	DDRD = 0;
	PORTD |= (1 << PD2);
}

void InitTimer1(void)
{
	TCCR1A = 0;
	TCCR1B = (1 << CS11 | 1 << CS10 | 1 << WGM12);
	TCNT1 = 0;
	TIMSK1 |= (1 << OCIE1B);
	OCR1A = 15262;
	OCR1B = 15262;
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
	SPDR = data;
	while(!(SPSR & (1 << SPIF)));
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

void InitADC(void)
{
	ADMUX = (1 << MUX0); //выбор входного канала ADC1
	ADCSRB = (1 << ADTS2 | 1 << ADTS0); //срабатывание компоратора В таймера1
	ADCSRA = (1 << ADEN | 1 << ADATE | 1 << ADIE); //запуск АЦП, автоматический перезапуск АЦП, разрешение прерывания АЦП
}

void InitUSART(void) //функция для настройки USART
{
	UCSR0B = (1 << RXEN0 | 1 << TXEN0); // включение приемника и передатчика
	UCSR0C = (1 << UCSZ01 | 1 << UCSZ00); //определение размера посылки: 8 бит
	UBRR0H = 0; 
	UBRR0L = 0x0C; //скорость передачи 4800бод
}

void SendChar(char symbol)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = symbol;
}

void SendString(char* buffer)
{
	while (*buffer != 0)
	SendChar(*buffer++);
}