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
void send_data(uint8_t data, uint8_t ind);
void InitTimer0(void);
void Bin2Dec(uint16_t data);

volatile uint16_t cnt = 0;
volatile uint8_t switch_state = 0;
volatile uint8_t bcd_buffer[] = {0, 0, 0, 0};

int main(void)
{
	InitPorts();
	InitTimer0();
	EIMSK |= (1 << INT0); //��������� ���������� INT0
	EICRA |= (1 << ISC01); //��������� INT0 �� ���������� �� �����
	sei(); //���������� ���������� ���- �������
	
	while(1)
	{
		if (switch_state == 0)
		{
			Bin2Dec(cnt);
			if (cnt < 9999)
			{
				cnt++;
			}
			else
			{
				cnt = 0;
			}
			_delay_ms(100);
		}
	}
}

ISR(TIMER0_COMPA_vect)
{
	send_data(bcd_buffer[3], 0);
	send_data(bcd_buffer[2], 1);
	send_data(bcd_buffer[1], 2);
	send_data(bcd_buffer[0], 3);
}

ISR(INT0_vect)
{
	if (switch_state == 0)
	{
		switch_state = 1;
	}
	else
	{
		switch_state = 0;
		cnt = 0;
	}
}

void InitPorts(void) //��������� ������������� ������
{
	DDRB = 0xFF; //���� ���� B �������� �� �����
	DDRC |= 0x0F; //���� 0-3 � ����� � ������������ �� �����
	PORTC |= 0x0F; //�� �������� ���� ����� � �������� 1
	DDRD &= ~(1 << 2);
	PORTD |= (1 << 2);
}

void InitTimer0(void) //��������� ������������� �������
{
	TCCR0A |= (1 << WGM01); //����� ������ ������ �TC
	TCCR0B |= (1 << CS02 | 1 << CS00); //��������� ������������ �� 1024
	TCNT0 = 0x00; //��������� �������� �����
	OCR0A = 16; //������ ����� ���������� ����������� ����� � OCR0A � ��� ���������� ������� ������
	TIMSK0 |= (1 << OCIE0A); //�������� ����������, ���� ������� �������� �� ��������� OCR0A
}

void Bin2Dec(uint16_t data) //� ������ bcd_buffer[] ������������ �������� ���������� cnt
{
	bcd_buffer[3]=(uint8_t)(data / 1000);
	data = data - bcd_buffer[3] * 1000;
	bcd_buffer[2] = (uint8_t)(data / 100);
	data = data - bcd_buffer[2] * 100;
	bcd_buffer[1] = (uint8_t)(data / 10);
	data = data - bcd_buffer[1] * 10;
	bcd_buffer[0] = (uint8_t)(data);
}

void send_data(uint8_t data, uint8_t ind)
{
	PORTC = 0x0F & ~(1 << ind);
	PORTB = segments[data];
	_delay_ms(5);
	PORTB = 0;
	PORTC = 0x0F;
}


