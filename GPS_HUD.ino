/*
Compile size examples
+------------+---------+--------+------+
|  microchip | SU_MODE | Flash  | SRAM |
+------------+---------+--------+------+
| ATmega328p | 0:TX&RX | 7,516b | 490b |
| ATmega32u4 | 0:TX&RX |10,542b | 459b |
|   ATtiny85 | 2:RX    | 6,438b | 351b |
+------------+---------+--------+------+
*/
//ATtiny85
//                                  +-\/-+
//                             PB5 1|*   |8 VCC
// GPS Tx (Software Serial) => PB3 2|    |7 PB2 => LCD DIO
//                             PB4 3|    |6 PB1 => LCD RCLK
//                             GND 4|    |5 PB0 => LCD SCLK
//                                  +----+
//
//Arduino Nano
//               +----+=====+----+
//               |    | USB |    |
//             13| D13+-----+D12 |12
//               |3V3        D11~|11
//               |Vref       D10~|10
// LCD SCLK <= 14| A0/D14     D9~|9
// LCD RCLK <= 15| A1/D15     D8 |8
//  LCD DIO <= 16| A2/D16     D7 |7 <= GPS Tx (Software Serial)
//             17| A3/D17     D6~|6 => GPS Rx (Software Serial)
//             18| A4/D18     D5~|5
//             19| A5/D19     D4 |4
//             20| A6         D3~|3
//             21| A7         D2 |2
//               | 5V        GND |
//               | RST       RST |
//               | GND       TX1 |0
//               | Vin       RX1 |1
//               |  5V MOSI GND  |
//               |   [ ][ ][ ]   |
//               |   [*][ ][ ]   |
//               | MISO SCK RST  |
//               +---------------+
/*
* 7 segment display
a_
f| |b
g-
e| |c
d- .
*/

#include <stdint.h>

#include <LED_4Bit7Seg.h>

#ifdef __AVR_ATtiny85__
#define SU_MODE 2
#define NO_FLOATS
#define NO_SETUP
#define TIMESYNC_ONLY 0
#else
#define SU_MODE 0
#endif

#include <SoftwareUart.h>

#ifdef __AVR_ATtiny85__
SoftwareUart<> uart(2, -1);// Rx, Tx pin
#else
SoftwareUart<> uart(7, 6);// Rx, Tx pin
#endif

#include <TimeDateTools.h> // include before wwvb.h AND/OR ATtinyGPS.h
#include <ATtinyGPS.h>

ATtinyGPS gps;

LED_4Bit7Seg led;

const int8_t local_timezone[2] = { 10, 30 }; // This is your local timezone : ACDT (UTC +10:30)

uint32_t t0, t1;

bool displayTime = false;

void setup()
{
	//led.setup(DIO,RCLK,SCLK)
#ifdef __AVR_ATtiny85__
	led.setup(2, 1, 0);
#else
	led.setup(A2, A1, A0);
#endif

	// set the timezone to local time
	gps.setTimezone(local_timezone[0], local_timezone[1]); // set this to your local time e.g.

	uart.begin(9600);
#if (SU_MODE != SU_RX_ONLY)
	gps.setup(uart);
#endif

	led.setLR_2dp(gps.hh, gps.mm);

	t0 = millis();
	t1 = t0;
}

void loop()
{
	// Parse the gps data
	if (uart.available())
	{
		gps.parse(uart.read());
	}

	// update display data every 2s
	if (millis() - t0 >= 2000)
	{
		t0 = millis();
		if (displayTime)
		{
			led.setLR_2dp(gps.hh, gps.mm);
		}
		else
		{
			led.set(gps.kmph, 0);
		}

		displayTime = !displayTime;
	}

	// refresh display every 5ms
	if (millis() - t1 > 4)
	{
		t1 = millis();
		led.display();
	}
}
