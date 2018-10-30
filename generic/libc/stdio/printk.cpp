/*
 *   File name: printk.cpp
 *
 *  Created on: 2017年3月18日, 下午8:47:23
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description:
 */
#include <macros.h>
#include <assert.h>
#include <types.h>
#include <stdarg.h>
#include <console.h>

#define MY_BUF_SIZE     64

extern "C" const char __hexChars[];

struct s_file
{
	char buff[MY_BUF_SIZE + 1];
	word_t pos;
};

static void my_fflush(struct s_file *file)
{
	file->buff[file->pos] = '\0';
	debugConsole.puts(file->buff);
	file->pos = 0;
}

static void my_putc(const char c, struct s_file *file)
{
	file->buff[file->pos++] = c;
	if (file->pos == MY_BUF_SIZE)
	{
		my_fflush(file);
	}
}

static word_t print_ulong(word_t val, word_t ulBase, struct s_file *file)
{
	if(ulBase != 10 && ulBase != 16)
	{
		return 0;
	}

	if(val == 0)
	{
		my_putc('0', file);
		return 1;
	}

	int digits[sizeof(unsigned long) * 2 + 3];
	word_t i = 0;
	word_t count = 0;

	while(val)
	{
		digits[i] = val % ulBase;
		++i;
		val /= ulBase;
	}

	count = i;
	for (; i > 0; --i)
	{
		my_putc(__hexChars[ digits[i-1] ], file);
	}

	return count;
}

static word_t print_ullong(u64 val, word_t ulBase, struct s_file* file)
{
	if(ulBase != 16)
	{
		return 0;
	}

	u32 upper, lower;
	u32 count = 0;
	upper = (u32)(val >> 32LLU);
	lower = (u32)val & 0xFFFFFFFF;

	if(upper > 0)
	{
		count += print_ulong(upper, ulBase, file);

		u32 mask = 0xF0000000U;
		u32 shifts = 0;
		while(!(mask & lower))
		{
			my_putc('0', file);
			++count;
			mask >>= 4;
			++shifts;
			if(shifts == 6) break;
		}
	}
	count += print_ulong(lower, ulBase, file);
	return count;
}

static int print_str(const char* str, struct s_file* file)
{
	u32 count;

	for (count = 0; *str; ++count, ++str)
	{
		my_putc(*str, file);
	}

	return count;
}

static int vprintf(const char *format, va_list ap)
{
    if (!format) {
        return 0;
    }

    unsigned int n;
    unsigned int formatting;
	struct s_file res;
	res.pos = 0;

    n = 0;
    formatting = 0;
    while (*format) {
        if (formatting) {
            switch (*format) {
            case '%':
            	my_putc('%', &res);
                n++;
                format++;
                break;

            case 'd': {
                int x = va_arg(ap, int);

                if (x < 0) {
                	my_putc('-', &res);
                    n++;
                    x = -x;
                }

                n += print_ulong(x, 10, &res);
                format++;
                break;
            }

            case 'u':
                n += print_ulong(va_arg(ap, unsigned int), 10, &res);
                format++;
                break;

            case 'x':
                n += print_ulong(va_arg(ap, unsigned int), 16, &res);
                format++;
                break;

            case 'p': {
                unsigned long p = va_arg(ap, unsigned long);
                if (p == 0) {
                    n += print_str("(nil)", &res);
                } else {
                    n += print_str("0x", &res);
                    n += print_ulong(p, 16, &res);
                }
                format++;
                break;
            }

            case 's':
                n += print_str(va_arg(ap, char *), &res);
                format++;
                break;

            case 'l':
                format++;
                switch (*format) {
                case 'd': {
                    long x = va_arg(ap, long);

                    if (x < 0) {
                    	my_putc('-', &res);
                        n++;
                        x = -x;
                    }

                    n += print_ulong((unsigned long)x, 10, &res);
                    format++;
                }
                break;
                case 'l':
                    if (*(format + 1) == 'x') {
                        n += print_ullong(va_arg(ap, unsigned long long), 16, &res);
                    }
                    format += 2;
                    break;
                case 'u':
                    n += print_ulong(va_arg(ap, unsigned long), 10, &res);
                    format++;
                    break;
                case 'x':
                    n += print_ulong(va_arg(ap, unsigned long), 16, &res);
                    format++;
                    break;

                default:

                    return -1;
                }
                break;
            default:

                return -1;
            }

            formatting = 0;
        } else {
            switch (*format) {
            case '%':
                formatting = 1;
                format++;
                break;

            default:
                my_putc(*format, &res);
                n++;
                format++;
                break;
            }
        }
    }

    my_fflush(&res);
    return n;
}

BEG_EXT_C

int printk(const char *format, ...)
{
	int res;
	va_list args;
	va_start(args, format);
	res = vprintf(format, args);
	va_end(args);

	return res;
}

END_EXT_C
