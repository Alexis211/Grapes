#include "monitor.h"
#include "sys.h"

static int cursor_x = 0, cursor_y = 0;
static uint16_t *video_memory = (uint16_t*)0xE00B8000;

static void move_cursor() {
	uint16_t cursor_location = cursor_y * 80 + cursor_x;
	outb(0x3D4, 14);	//Sending high cursor byte
	outb(0x3D5, cursor_location >> 8);
	outb(0x3D4, 15);	//Sending high cursor byte
	outb(0x3D5, cursor_location);
}

static void scroll() {
	uint8_t attribute_byte = (0 /* black */ << 4) | (7/* white */ & 0x0F);
	uint16_t blank = (attribute_byte << 8) | 0x20;

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

void monitor_put(char c) {
	uint8_t fg = 7;	//White
	uint8_t bg = 0; //Black

	uint16_t attribute = (fg & 0x0F) | (bg << 4);
	attribute = attribute << 8;

	if (c == 0x08 && cursor_x) {	//Backspace
		cursor_x--;
	} else if (c == 0x09) {	//Tab
		cursor_x = (cursor_x + 8) & ~(8 - 1);
	} else if (c == '\r') {	//Carriage return
		cursor_x = 0;
	} else if (c == '\n') {	//New line
		cursor_x = 0;
		cursor_y++;
	} else if (c >= ' ') {	//Any printable character
		video_memory[cursor_y * 80 + cursor_x] = c | attribute;
		cursor_x++;
	}
	if (cursor_x >= 80) {
		cursor_x = 0;
		cursor_y++;
	}

	scroll();
	move_cursor();
}

void monitor_clear() {
	uint8_t attribute_byte = (0 /* black */ << 4) | (15 /* white */ & 0x0F);
	uint16_t blank = (attribute_byte << 8) | 0x20;

	int i;

	for (i = 0; i < 80*25; i++) {
		video_memory[i] = blank;
	}

	cursor_x = 0; cursor_y = 0;
	move_cursor();
}

void monitor_write(char *s) {
	while (*s) {
		monitor_put(*(s++));
	}
}

void monitor_writeHex(uint32_t v) {
	int i;

	monitor_put('0'); monitor_put('x');
	char hexdigits[] = "0123456789abcdef";

	for (i = 0; i < 8; i++) {
		monitor_put(hexdigits[v >> 28]);
		v = v << 4;
	}
}
