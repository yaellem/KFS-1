#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
 
/* Check if the compiler thinks you are targeting the wrong operating system. */
//#if defined(__linux__)
//#error "You are not using a cross-compiler, you will most certainly run into trouble"
//#endif


/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void set_cursor(int offset);
int get_cursor();

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define VGA_CTRL_REGISTER 	0x3d4
#define VGA_DATA_REGISTER 	0x3d5
#define VGA_OFFSET_LOW 		0x0f
#define VGA_OFFSET_HIGH 	0x0e


// Copies value from DX :-> port ==> AL :-> result . READS THE PORT
unsigned char port_byte_in(unsigned short port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

// Copies value from AL :-> data ==> DX :-> port . WRITES THE PORT
void port_byte_out(unsigned short port, unsigned char data) {
    __asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}


void set_cursor(int offset) {
   offset /= 2;
/* The index is already a position in a uint16_t * array */
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char) (offset >> 8));
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    port_byte_out(VGA_DATA_REGISTER, (unsigned char) (offset & 0xff));
}

int get_cursor() {
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
    int offset = port_byte_in(VGA_DATA_REGISTER) << 8;
    port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
    offset += port_byte_in(VGA_DATA_REGISTER);
	offset *= 2;
/* The index is already a position in a uint16_t * array */
	return offset;
}


static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}
 

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
	return (uint16_t) uc | (uint16_t) color << 8;
}
 
size_t strlen(const char* str) 
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}
 
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
 
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;
 
void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) 
{
	terminal_color = color;
}
 
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}
// void terminal_putchar(char c) 
int terminal_putchar(char c) 
{
	/* Newline management */
	if (c == '\n') {
		terminal_column = 0;
		if (++terminal_row == VGA_HEIGHT)
			terminal_row = 0;
	} else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			terminal_column = 0;
		/* Scrolling management */
			if (++terminal_row == VGA_HEIGHT)
				terminal_row = 0;
		}
	}
	
	/* Cursor management */
	return terminal_row * VGA_WIDTH + terminal_column;
}
 
void terminal_write(const char* data, size_t size, int cursor_offset) 
{
	size_t i = 0;
	
	for (i = 0; i < size; i++) {
		cursor_offset = terminal_putchar(data[i]);
	}
	set_cursor(cursor_offset * 2);
}


void terminal_writestring(const char* data) 
{
	int cursor_offset = 0;

	terminal_write(data, strlenk(data), cursor_offset);
}
 
void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();
 
	/* Newline support is left as an exercise. */
	// terminal_writestring("Hello, kernel World!\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\ntotooookokoko\n");
	terminal_writestring("Hello, kernel World!\n1\n2\n3\n4\n5\n6\n7\n8\n9\nA\nB\nC\nD\nE\nF\n10\n11\n12\n13\n14\n15\n16\n17\n18\n01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	// terminal_writestring("Hello, kernel World!\nOui ca va merci");
}
