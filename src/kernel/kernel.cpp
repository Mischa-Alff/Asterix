#include <cstddef>
#include <cstdint>
#include <multiboot.h>
#include <kernel/paging.hpp>
#include <kernel/memory.hpp>
#include <kernel/assembly.hpp>

constexpr size_t VGA_WIDTH = 80;
constexpr size_t VGA_HEIGHT = 24;

/* Hardware text mode color constants. */
enum Color
{
	Black = 0,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Brown,
	LightGray,
	LightGrey = LightGray,
	DarkGray,
	DarkGrey = DarkGray,
	LightBlue,
	LightGreen,
	LightCyan,
	LightRed,
	LightMagenta,
	LightBrown,
	White
};


size_t strlen(const char* bytes)
{
	size_t i;
	for(i=0;bytes[i]!=0 || i == 31;++i);
	return i;
}
 
class Terminal
{
public: 
	size_t row;
	size_t column;
	uint8_t color;
	uint16_t* buffer;
	 
	void initialize()
	{
		row = 0;//get_cursor_pos_y();
		column = 0;//get_cursor_pos_x();
		set_color(Color::White, Color::Black);
		buffer = (uint16_t*) 0xB8000;
	}
	 
	void set_color(Color foreground, Color background)
	{
		color = foreground | background << 4;
	}

	void putc(const char &byte)
	{
		if(byte == '\n')
		{
			++row;
			return;
		}
		else if(byte == '\r')
		{
			column = 0;
			return;
		}
		else
		{
			size_t index     = row * 80 + column;
			uint16_t vgadata = (uint16_t)byte | ((uint16_t)color<<8);
			buffer[index] = vgadata;
		}

		if(++column == 80)
		{
			column = 0;
			if(++row == 24)
				row = 0;
		}
	}
	 
	void puts(const char* bytes)
	{
		auto length = strlen(bytes);
		for(auto i=0;i<length || i == 31;++i)
			putc(bytes[i]);
	}
};

char* strrev(char* str)
{
	if(str)
	{
		char* end = str+strlen(str)-1;
		while(str < end)
		{
			*str ^= *end;
			*end ^= *str;
			*str ^= *end;
			str++;
			end--;
		}
	}
	return str;
}

char* itoa(uint64_t val, char* str, int base=10)
{
	if(val == 0)
	{
		str[0] = '0';
		str[1] = '\0';
		return str;
	}

	int rem, i{0};
	//char str[20];
	static char bases[] = "0123456789ABCDEF";
	while(val)
	{
		rem = val % base;
		str[i++] = bases[rem];
		val /= base;
	}

	str[i] = '\0';

	strrev(str);
	return str;
}

Terminal terminal;

extern "C"
void kmain(multiboot_info_t &multiboot, uint64_t magic)
{
	char buff[128];
	terminal.initialize();
	uint64_t memory = (uint64_t)multiboot.mem_upper;// + (uint64_t)multiboot.mem_lower;
	terminal.puts("Finished kernel setup!\r\n");
	terminal.puts("Multiboot magic number was 0x");
	terminal.puts(itoa(magic, buff, 16));
	terminal.puts(".\r\n");
	terminal.puts("Detected ");
	terminal.puts(itoa(memory, buff));
	terminal.puts(" KB of memory.\r\n");
}
