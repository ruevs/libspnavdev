#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>
#include "spnavdev.h"

#ifdef __FreeBSD__
#define DEFDEV	"/dev/ttyu0"
#elif defined(__sgi__)
#define DEFDEV	"/dev/ttyd1"
#else
#define DEFDEV	"/dev/ttyS0"
#endif

static void sighandler(int s);

static struct spndev *dev;
static int quit;

int main(int argc, char **argv)
{
	int fd;
	fd_set rdset;
	union spndev_event ev;
	const char *s;
	int led=0;

	signal(SIGINT, sighandler);

	if(!(dev = spndev_open(argv[1]))) {
		fprintf(stderr, "Failed to open 6dof device %s\n", argv[1] ? argv[1] : "");
		return 1;
	}
	fd = spndev_fd(dev);

	printf("Monitoring device, ctrl-c to quit\n");

	while(!quit) {
		FD_ZERO(&rdset);
		FD_SET(fd, &rdset);

		if(select(fd + 1, &rdset, 0, 0, 0) > 0) {
			if(FD_ISSET(fd, &rdset)) {
				if(spndev_process(dev, &ev)) {
					switch(ev.type) {
					case SPNDEV_MOTION:
						printf("motion: T[%+6d %+6d %+6d]  R[%+6d %+6d %+6d]\n",
								ev.mot.v[0], ev.mot.v[1], ev.mot.v[2], ev.mot.v[3],
								ev.mot.v[4], ev.mot.v[5]);
						break;

					case SPNDEV_BUTTON:
						if((s = spndev_button_name(dev, ev.bn.num))) {
							printf("button %d (\"%s\") ", ev.bn.num, s);
						} else {
							printf("button %d ", ev.bn.num);
						}
						puts(ev.bn.press ? "pressed" : "released");

						if (ev.bn.press) {
							spndev_set_led(dev, led);
							led = !led;
						}

						break;

					default:
						break;
					}
				}
			}
		}

	}

	spndev_close(dev);
	return 0;
}

static void sighandler(int s)
{
	quit = 1;
}
