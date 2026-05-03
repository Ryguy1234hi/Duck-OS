#include <stdint.h>

__attribute__((section(".multiboot")))
unsigned int multiboot_header[] = {
    0x1BADB002, // magic
    0x00,       // flags
    -(0x1BADB002) // checksum
};
#define VIDEO_MEMORY 0xB8000
#define KEYBOARD_DATA_PORT 0x60

volatile char* video = (volatile char*)VIDEO_MEMORY;

int cursor = 0;

char input_buffer[128];
int buffer_index = 0;

/* =========================
   LOW LEVEL PORT READ
========================= */
static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

/* =========================
   SCREEN OUTPUT
========================= */
void putchar(char c) {
    if (c == '\n') {
        cursor += (80 - (cursor / 2) % 80) * 2;
        return;
    }

    video[cursor++] = c;
    video[cursor++] = 0x07;
}

void print(const char* str) {
    for (int i = 0; str[i]; i++) {
        putchar(str[i]);
    }
}

void clear() {
    for (int i = 0; i < 80 * 25 * 2; i++) {
        video[i] = 0;
    }
    cursor = 0;
}

/* =========================
   STRING FUNCTIONS
========================= */
int strcmp(const char* a, const char* b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

/* =========================
   COMMAND SYSTEM
========================= */
void run_command(const char* cmd) {
    if (strcmp(cmd, "help")) {
        print("\nCommands: help, clear, echo\n");
    }
    else if (strcmp(cmd, "clear")) {
        clear();
    }
    else if (strcmp(cmd, "echo")) {
        print("\nHello from kernel!\n");
    }
    else {
        print("\nUnknown command\n");
    }
}

/* =========================
   KEYBOARD SCAN CODE MAP
========================= */
char scancode_to_ascii(unsigned char sc) {
    switch (sc) {
        case 0x1E: return 'a';
        case 0x30: return 'b';
        case 0x2E: return 'c';
        case 0x20: return 'd';
        case 0x12: return 'e';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x17: return 'i';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x32: return 'm';
        case 0x31: return 'n';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x10: return 'q';
        case 0x13: return 'r';
        case 0x1F: return 's';
        case 0x14: return 't';
        case 0x16: return 'u';
        case 0x2F: return 'v';
        case 0x11: return 'w';
        case 0x2D: return 'x';
        case 0x15: return 'y';
        case 0x2C: return 'z';

        case 0x39: return ' ';   // SPACE
        case 0x1C: return '\n';  // ENTER

        default: return 0;
    }
}

/* =========================
   KEYBOARD HANDLER
========================= */
void keyboard_handler() {
    unsigned char sc = inb(KEYBOARD_DATA_PORT);

    // BACKSPACE
    if (sc == 0x0E) {
        if (buffer_index > 0) {
            buffer_index--;

            input_buffer[buffer_index] = '\0';

            cursor -= 2;
            putchar(' ');
            cursor -= 2;
        }
        return;
    }

    char c = scancode_to_ascii(sc);
    if (c == 0) return;

    if (c == '\n') {
        input_buffer[buffer_index] = '\0';

        print("\n");
        run_command(input_buffer);

        buffer_index = 0;
        print("\n> ");
    }
    else {
        input_buffer[buffer_index++] = c;
        putchar(c);
    }
}

/* =========================
   KERNEL ENTRY
========================= */
void kernel_main() {
    clear();

    print("Duck!OS\n");
    print("Type 'help'\n\n> ");

    while (1) {
        keyboard_handler();
    }
}
