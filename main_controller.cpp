#include"../12f683.h"
#include"../defs.h"

char is_ambient_sensor_active();
char is_user_sensor_active();
char delay_routine(void(*f)(void));
void init_delay_routine(int value);
unsigned int i = 0, j = 0, max_v;
unsigned short address = 0;
unsigned short* ptr;

enum States { RED, BLINK_RED, BLUE, GREEN, BLINK_BLUE, NONE };

enum States state = NONE;

#define ON_TIME 120

void user_sensor_inactive_counter() {
	if (IS_GP4_ON) {
		++i;
	}
}

char is_ambient_sensor_active() {
	return IS_GP5_ON ? 1 : 0;
}

char is_user_sensor_active() {
	return IS_GP4_ON ? 0 : 1;
}

void next_state()
{
	if (state == RED)
	{
		if (is_ambient_sensor_active())
		{
			state = BLUE;
			return;
		}
		if (is_user_sensor_active())
		{
			state = BLINK_RED;
			return;
		}

	}
	else if (state == BLINK_RED)
	{
		if (is_ambient_sensor_active())
		{
			state = GREEN;
			return;
		}
		if (is_user_sensor_active())
		{
			state = RED;
			return;
		}
	}
	else if (state == BLUE)
	{
		if (is_user_sensor_active())
		{
			state = GREEN;
			return;
		}
		if (is_ambient_sensor_active())
		{
			state = RED;
			return;
		}
	}
	else if (state == GREEN)
	{

		if (i <= (max_v - 5))
		{
			state = GREEN;
			return;
		}
		if (is_user_sensor_active())
		{
			state = BLUE;
			return;
		}
	}
	else
	{
		if (is_ambient_sensor_active() && is_user_sensor_active())
			state = RED;
		else if (is_ambient_sensor_active() && is_user_sensor_active())
			state = BLINK_RED;
		else if (is_ambient_sensor_active() && is_user_sensor_active())
			state = BLUE;
		else if (is_ambient_sensor_active() && is_user_sensor_active())
			state = GREEN;
		else
			state = BLINK_BLUE;

		//Delay_ms(5000);
		return;
	}
}

void none_function() {
	OFF_GP0;
	OFF_GP1;
	OFF_GP2;
}

void blink_blue_function() {

	OFF_GP2;
	OFF_GP1;
	TOGGLE_GP0;
}

void red_function() {
	ON_GP1;
	OFF_GP2;
	OFF_GP0;
}

void blue_function() {

	ON_GP0;
	OFF_GP1;
	OFF_GP2;
}

void green_function() {
	ON_GP2;
	OFF_GP0;
	OFF_GP1;
	i = 0;
	init_delay_routine(ON_TIME);;
	while (!delay_routine(user_sensor_inactive_counter));
}

void blink_red_function() {
	TOGGLE_GP1;
	OFF_GP0;
	OFF_GP2;
	Delay_ms(500);
}

void init_delay_routine(int value)
{
	j = value;
}

char delay_routine(void(*f)(void)) {
	while (j > 0) {
		f();
		Delay_ms(500);
		--j;
		return 0;
	}
	return 1;
}

void systemInit() {

	TRISIO = BIT4_ON | BIT5_ON;
	CMCON0 = BIT0_ON | BIT1_ON | BIT2_ON;
	ADCON0 = ZERO;                // Internal ADC OFF
	ANSEL = ZERO;        // All Analog selections pins are assigned as digital I/O
	OPTION_REG = BIT3_ON | BIT2_ON | BIT1_ON | BIT0_ON; // Pre-Scaler for WDT 1:128
	WDTCON = BIT4_ON | BIT2_ON | BIT1_ON | BIT0_ON;
	GPIO = ZERO;
}

void load_limits() {
	address = 0;
	ptr = (unsigned short*)&max_v;
	*ptr = EEPROM_Read(address);
	*(ptr + 1) = EEPROM_Read(address + 1);
}

void run_functions() {
	switch (state)
	{
	case RED:
		red_function();
		break;
	case BLINK_RED:
		blink_red_function();
		break;
	case BLUE:
		blue_function();
		break;
	case GREEN:
		green_function();
		break;
	case BLINK_BLUE:
	default:
		init_delay_routine(20);
		while (!delay_routine(none_function));
		break;
	}
	asm CLRWDT;
	next_state();
}

void set_limits() {
	ON_GP2;
	OFF_GP0;
	OFF_GP1;
	i = 0;
	ptr = (unsigned short*)&i;
	init_delay_routine(ON_TIME);;
	while (!delay_routine(user_sensor_inactive_counter));

	if (i == 0) {
		init_delay_routine(40);
		while (!delay_routine(blink_red_function));
	}

	init_delay_routine(i);
	while (!delay_routine(blink_blue_function))Delay_ms(500);

	EEPROM_Write(address, *ptr);
	i >>= 8;
	EEPROM_Write(address + 1, *ptr);
	address += 2;


	init_delay_routine(40);
	while (!delay_routine(none_function));
}

void main() {

	systemInit();
	load_limits();
	init_delay_routine(20);
	while (!delay_routine(blink_blue_function));

	while (1)
	{
		if (IS_GP3_ON)
			run_functions();
		else
			set_limits();
		asm CLRWDT;
	}
}