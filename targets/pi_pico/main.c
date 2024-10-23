/**
 * Based on https://github.com/hextreeio/rp2350-security-playground-demo/blob/main/main.c
 * Modified by David Oswald <d.f.oswald@bham.ac.uk>
 **/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/watchdog.h"

#define OUTER_LOOP_CNT 200
#define INNER_LOOP_CNT 200

#define TRIGGER_ACTIVE() {}
#define TRIGGER_RELEASE() {}

const uint LED_PIN = 25;

int main() 
{
	volatile register uint32_t i, j;
	volatile register uint32_t cnt;
	
	stdio_init_all();
	sleep_ms(2000);
	printf("Welcome to the glitch target v0.1...\n");

	watchdog_enable(100, 1);
	
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);
	
	while (1)
    {	
		watchdog_update();
		cnt = 0;

		for(int k = 0; k < 100; k++)
		{
			watchdog_update();
			gpio_put(LED_PIN, 1);
			sleep_ms(1);
		}

		for(int k = 0; k < 100; k++)
		{
			watchdog_update();
			gpio_put(LED_PIN, 0);
			sleep_ms(1);
		}

		TRIGGER_ACTIVE();
		for (i = 0; i < OUTER_LOOP_CNT; i++)
		{
			watchdog_update();
			for (j = 0; j < INNER_LOOP_CNT; j++)
			{
				cnt++;
			}
		}
		TRIGGER_RELEASE();

		const bool glitch_detected = i != OUTER_LOOP_CNT || j != INNER_LOOP_CNT
			|| cnt != (OUTER_LOOP_CNT * INNER_LOOP_CNT);

		// Check for glitch
		if (glitch_detected)
		{
			// X indicates successful glitch
			putchar_raw('X');
		}
		else
		{
			// N indicates regular execution
			putchar_raw('N');
		}
		watchdog_update();
	}

    return 0;
}

