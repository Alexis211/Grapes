#include "monitor.h"
#include "sys.h"

static int cursor_x = 0, cursor_y = 0;
static uint16_t *video_memory = (uint16_t*)0xE00B8000;

static uint8_t attribute = 0x07; // 0 = background = black, 7 = foreground = white

/*	For internal use only. Tells hardware to move the cursor at (cursor_x, cursor_y). */
static void move_cursor() {
	uint16_t cursor_location = cursor_y * 80 + cursor_x;
	outb(0x3D4, 14);	//Sending high cursor byte
	outb(0x3D5, cursor_location >> 8);
	outb(0x3D4, 15);	//Sending high cursor byte
	outb(0x3D5, cursor_location);
}

/*	For internal use only. Scrolls everything up one line. */
static void scroll() {
	uint16_t blank = (attribute << 8) | 0x20;

	if (cursor_y >= 25) {
		int i;
		for (i = 0; i < 80*24; i++) {
			video_memory[i] = video_memory[i+80];
		}
		for (i = 80*24; i < 80*25; i++) {
			video_memory[i] = blank;
		}
		cursor_y = 24;
	}
}

/*	Put one character on the screen. This function handles special characters \b, \t, \r and  \n. */
void monitor_put(char c) {
	if (c == '\b' && cursor_x) {	//Backspace
		cursor_x--;
	} else if (c == '\t') {	//Tab
		cursor_x = (cursor_x + 8) & ~(8 - 1);
	} else if (c == '\r') {	//Carriage return
		cursor_x = 0;
	} else if (c == '\n') {	//New line
		cursor_x = 0;
		cursor_y++;
	} else if (c >= ' ') {	//Any printable character
		video_memory[cursor_y * 80 + cursor_x] = c | (attribute << 8);
		cursor_x++;
	}
	if (cursor_x >= 80) {
		cursor_x = 0;
		cursor_y++;
	}

	scroll();
	move_cursor();
}

/* Clears the screen and moves cursor to (0,0) (top left corner) */
void monitor_clear() {
	uint16_t blank = (attribute << 8) | 0x20;

	int i;

	for (i = 0; i < 80*25; i++) {
		video_memory[i] = blank;
	}

	cursor_x = 0; cursor_y = 0;
	move_cursor();
}

/* Writes a string to the monitor */
void monitor_write(char *s) {
	while (*s) {
		monitor_put(*(s++));
	}
}

/* Writes a number in hexadecimal notation to the monitor */
void monitor_writeHex(uint32_t v) {
	int i;

	monitor_put('0'); monitor_put('x');
	char hexdigits[] = "0123456789abcdef";

	for (i = 0; i < 8; i++) {
		monitor_put(hexdigits[v >> 28]);
		v = v << 4;
	}
}

/* Writes a number in decimal notation to the monitor */
void monitor_writeDec(uint32_t v) {
	if (v == 0) {
		monitor_put('0');
		return;
	}

	char numbers[] = "0123456789";
	while (v > 0) {
		int order = 1, no = 1;
		while (v / order > 0) order *= 10;
		order /= 10;
		monitor_put(numbers[v / order]);
		v = v - (v / order * order);
		while (v / no > 0) no *= 10;
		while (no < order) {
			monitor_put('0');
			no *= 10;
		}
	}
}
