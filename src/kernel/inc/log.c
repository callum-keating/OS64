#include "drivers/serial.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

size_t lenofstr(const char *str) {
    size_t len = 0;

    while (*str) {
        str++;
        len++;
    }

    return len;
}

void strappend(char *dest, const char *str1, const char *str2) {
    size_t i = 0;

    while (*str1) {
        dest[i++] = *str1++;
    }

    while (*str2) {
        dest[i++] = *str2++;
    }

    dest[i] = '\0';
}

void handleDecimal(int value) {
    if (value == 0) {
        write_serial('0');
        return;
    }

    if (value < 0) {
        write_serial('-');

        unsigned int number = (unsigned int)(-(value + 1)) + 1;
        char buffer[10];
        size_t i = 0;

        while (number > 0) {
            buffer[i++] = '0' + (number % 10);
            number /= 10;
        }

        while (i > 0) {
            write_serial(buffer[--i]);
        }

        return;
    }

    unsigned int number = (unsigned int)value;
    char buffer[10];
    size_t i = 0;

    while (number > 0) {
        buffer[i++] = '0' + (number % 10);
        number /= 10;
    }

    while (i > 0) {
        write_serial(buffer[--i]);
    }
}

void handleUnsignedDecimal(unsigned int value) {
    if (value == 0) {
        write_serial('0');
        return;
    }

    char buffer[10];
    size_t i = 0;

    while (value > 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        write_serial(buffer[--i]);
    }
}

void handleLowerHex(unsigned int value) {
    const char *digits = "0123456789abcdef";

    if (value == 0) {
        write_serial('0');
        return;
    }

    char buffer[8];
    size_t i = 0;

    while (value > 0) {
        buffer[i++] = digits[value & 0xF];
        value >>= 4;
    }

    while (i > 0) {
        write_serial(buffer[--i]);
    }
}

void handleUpperHex(unsigned int value) {
    const char *digits = "0123456789ABCDEF";

    if (value == 0) {
        write_serial('0');
        return;
    }

    char buffer[8];
    size_t i = 0;

    while (value > 0) {
        buffer[i++] = digits[value & 0xF];
        value >>= 4;
    }

    while (i > 0) {
        write_serial(buffer[--i]);
    }
}

void handleCharacter(int value) {
    write_serial((char)value);
}

void handleString(const char *str) {
    if (str == NULL) {
        write_serial_str("(null)");
        return;
    }

    write_serial_str(str);
}

void handlePointer(const void *ptr) {
    uintptr_t value = (uintptr_t)ptr;
    const char *digits = "0123456789abcdef";

    write_serial_str("0x");

    for (int i = (int)(sizeof(uintptr_t) * 2) - 1; i >= 0; i--) {
        write_serial(digits[(value >> (i * 4)) & 0xF]);
    }
}

void handlePercent(void) {
    write_serial('%');
}

void logf(const char *str, ...) {
    va_list args;
    va_start(args, str);

    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] != '%') {
            write_serial(str[i]);
            continue;
        }

        i++;

        if (str[i] == '\0') {
            write_serial('%');
            break;
        }

        switch (str[i]) {
            case 'd':
                handleDecimal(va_arg(args, int));
                break;

            case 'x':
                handleLowerHex(va_arg(args, unsigned int));
                break;

            case 'X':
                handleUpperHex(va_arg(args, unsigned int));
                break;

            case 'u':
                handleUnsignedDecimal(va_arg(args, unsigned int));
                break;

            case 'c':
                handleCharacter(va_arg(args, int));
                break;

            case 's':
                handleString(va_arg(args, const char *));
                break;

            case 'p':
                handlePointer(va_arg(args, void *));
                break;

            case '%':
                handlePercent();
                break;

            default:
                write_serial_str("Unknown format specifier: %");
                write_serial(str[i]);
                break;
        }
    }

    va_end(args);
}

