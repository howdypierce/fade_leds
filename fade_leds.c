/* 
 * fade_leds.c
 *
 * Howdy Pierce, howdy@cardinalpeak.com
 * 
 * Provide a pleasing fade-in / fade-out effect for a string of WS2812
 * LEDs running on a Raspberry Pi. See the README for more
 * information.
 * 
 * This file requires the rpi_ws281x library, available at 
 *                https://github.com/jgarff/rpi_ws281x
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ws2811.h"

#define TARGET_FREQ             WS2811_TARGET_FREQ
#define GPIO_PIN                18
#define DMA                     10
#define STRIP_TYPE              WS2811_STRIP_GRB
#define LED_COUNT               5

#define STEPS 40

/* To prevent mulitple instances of fade_leds from stepping on each *
 * other's DMA, fade_leds will wait until the following file is
 * deleted until proceeding.
 */
#define SENTINEL_FILE "/tmp/fade_leds_sentinel"

ws2811_t ledstring =
{
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
            .brightness = 255,
            .strip_type = STRIP_TYPE,
        },
        [1] =
        {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};

void set_color(unsigned int c)
{
	ws2811_return_t ret;
	int i;

	for (i=0; i<LED_COUNT; i++) {
		ledstring.channel[0].leds[i] = c;
	}

	if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS) {
		fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
		return;
	}
}

unsigned int newcolor(unsigned int old, unsigned int tgt, float step)
{
	int r_old = (old & 0xFF0000) >> 16;
	int g_old = (old & 0x00FF00) >>  8;
	int b_old = (old & 0x0000FF);

	int r_tgt = (tgt & 0xFF0000) >> 16;
	int g_tgt = (tgt & 0x00FF00) >>  8;
	int b_tgt = (tgt & 0x0000FF);

	unsigned int r_new = r_old + (int)((r_tgt - r_old) * step);
	unsigned int g_new = g_old + (int)((g_tgt - g_old) * step);
	unsigned int b_new = b_old + (int)((b_tgt - b_old) * step);

	return ((r_new << 16) | (g_new << 8) | b_new);
}

int test(unsigned int old, unsigned int tgt)
{
	int i;
	unsigned int res;
	
	for (i=0; i<=10; i++) {
		res = newcolor(old, tgt, i/10.0);
		printf("0x%06x 0x%06x %0.3f 0x%06x\n", old, tgt, i/10.0, res);
	}
}

void usage(char *bn)
{
	fprintf(stderr, "Usage: %s color [color..]\n\n", bn);
	fprintf(stderr, "Where each color is an RGB hex value of the form 0xFF1100\n");
	fprintf(stderr, "The program will take one second to transition between each color pair\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	ws2811_return_t ret;
	unsigned int colors[20];
	int num_colors = 0;
	unsigned int c;
	int i = 0;
	int s;
	int fd;

	if (argc < 2)
		usage(argv[0]);

	for (i=1; i<argc; i++) {
		if (sscanf(argv[i], "%x", &c) != 1) {
			fprintf(stderr, "Could not resolve argument %s\n", argv[i]);
			usage(argv[0]);
		}
		colors[i-1] = c;
	}
	num_colors = argc - 1;

	/* atomically see if the test file exists */
	while ((fd = open(SENTINEL_FILE, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR)) < 0) {
		if (errno != EEXIST) {
			fprintf(stderr, "Error %d creating sentinel file %s: %s\n", errno,
					SENTINEL_FILE, strerror(errno));
			exit(1);
		}
		usleep(100000);
		if (i++ > 300) {
			fprintf(stderr, "Too long a wait for the sentinel file %s\n",
					SENTINEL_FILE);
			fprintf(stderr, "to disappear. If no other process is accessing the LEDs,\n");
			fprintf(stderr, "try deleting this file by hand.\n");
			exit(1);
		}
	}
	close(fd);

	if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS) {
		fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
		return 1;
	}

	for (i=1; i<num_colors; i++) {
		for (s=0; s<STEPS; s++) {
			set_color(newcolor(colors[i-1], colors[i], (float)s/STEPS));
			usleep(1000000 / STEPS);
		}
		set_color(colors[i]);
		usleep(1000000 / STEPS);
	}				

	ws2811_fini(&ledstring);

	if (unlink(SENTINEL_FILE) < 0) {
		fprintf(stderr, "Error %d removing sentinel file %s: %s\n", errno,
					SENTINEL_FILE, strerror(errno));
		exit(1);
	}

	exit(0);
}
