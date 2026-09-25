#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "spnavdev.h"

static struct spndev *dev;
static int quit = 0;

void usercallback(union spndev_event ev, void* uptr) {
	static int led = 0;
	const char* s;
	struct spndev* dev = (struct spndev*)uptr;

	switch (ev.type) {
	case SPNDEV_MOTION:
		printf("motion: T[%+6d %+6d %+6d]  R[%+6d %+6d %+6d]\n",
			ev.mot.v[0], ev.mot.v[1], ev.mot.v[2], ev.mot.v[3],
			ev.mot.v[4], ev.mot.v[5]);
		break;

	case SPNDEV_BUTTON:
		if ((s = spndev_button_name(dev, ev.bn.num))) {
			printf("button %d (\"%s\") ", ev.bn.num, s);
		}
		else {
			printf("button %d ", ev.bn.num);
		}
		puts(ev.bn.press ? "pressed" : "released");

		if (ev.bn.press) {
			spndev_set_led(dev, led);
			spndev_write_lcd(dev, 0xAA);
			spndev_set_lcd_bl(dev, led);
			led = !led;
		}

		break;

	default:
		break;
	}
}

extern int test()
{
	if(!(dev = spndev_open(0))) {
		fprintf(stderr, "Failed to open 6dof device\n");
		return 1;
	}

	spndev_set_userptr(dev, (void*)dev);
	spndev_set_usercallback(dev, usercallback);

	printf("Monitoring device, ctrl-c to quit\n");

	return 0;
}

int main(int argc, char** argv) {
	printf("Press the button to test\n");
	return 0;
}
