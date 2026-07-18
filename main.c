//    Sequence Generator and Evaluator using PSoC

#include "cyhal_gpio.h"
#include "cyhal_system.h"
#if defined (CY_USING_HAL)
#include "cyhal.h"
#endif
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_syslib.h"
#include "cycfg_pins.h"
#include "cy_gpio.h"

// Defining Pins
#define D2_PORT    P10_0_PORT
#define D2_PIN     P10_0_NUM

#define D3_PORT    P10_1_PORT
#define D3_PIN     P10_1_NUM

#define D4_PORT    P10_2_PORT
#define D4_PIN     P10_2_NUM

#define D5_PORT    P10_3_PORT     
#define D5_PIN     P10_3_NUM

#define D6_PORT    P10_4_PORT
#define D6_PIN     P10_4_NUM

#define D7_PORT    P10_5_PORT
#define D7_PIN     P10_5_NUM

#define D0_PORT    P5_2_PORT
#define D0_PIN     P5_2_NUM

#define D1_PORT    P5_0_PORT
#define D1_PIN     P5_0_NUM

#define RST_PORT   P5_1_PORT
#define RST_PIN    P5_1_NUM

#define CS_PORT    P6_4_PORT
#define CS_PIN     P6_4_NUM

#define DC_PORT    P6_5_PORT
#define DC_PIN     P6_5_NUM

#define WR_PORT    P5_3_PORT
#define WR_PIN     P5_3_NUM

// LCD Resolution 
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

//Bus Test
#define BUS_TEST_ENABLED 1

// ---------------- GPIO Helper Functions ----------------
static inline void gpio_set(GPIO_PRT_Type *port, uint32_t pin, uint32_t value)
{
    Cy_GPIO_Write(port, pin, value);
}
static inline void gpio_set_high(GPIO_PRT_Type *port, uint32_t pin)
{
    Cy_GPIO_Set(port, pin);
}
static inline void gpio_set_low(GPIO_PRT_Type *port, uint32_t pin)
{
    Cy_GPIO_Clr(port, pin);
}

// ---------------- Write 8-bit Bus ----------------
static void tft_write_bus(uint8_t d)
{
    gpio_set(D0_PORT, D0_PIN, (d >> 0) & 1);
    gpio_set(D1_PORT, D1_PIN, (d >> 1) & 1);
    gpio_set(D2_PORT, D2_PIN, (d >> 2) & 1);
    gpio_set(D3_PORT, D3_PIN, (d >> 3) & 1);
    gpio_set(D4_PORT, D4_PIN, (d >> 4) & 1);
    gpio_set(D5_PORT, D5_PIN, (d >> 5) & 1);
    gpio_set(D6_PORT, D6_PIN, (d >> 6) & 1);
    gpio_set(D7_PORT, D7_PIN, (d >> 7) & 1);

    gpio_set_low(WR_PORT, WR_PIN);
    // Increase WR pulse to 5us for hard-to-latch modules / noisy wiring
    Cy_SysLib_DelayUs(5);   
    gpio_set_high(WR_PORT, WR_PIN);
}

// ---------------- Write Command ----------------
static void tft_cmd(uint8_t cmd)
{
    gpio_set_low(CS_PORT, CS_PIN);
    gpio_set_low(DC_PORT, DC_PIN);
    tft_write_bus(cmd);
    gpio_set_high(CS_PORT, CS_PIN);
}

// ---------------- Write Data ----------------
static void tft_data(uint8_t dat)
{
    gpio_set_low(CS_PORT, CS_PIN);
    gpio_set_high(DC_PORT, DC_PIN);
    tft_write_bus(dat);
    gpio_set_high(CS_PORT, CS_PIN);
}

static void tft_data16(uint16_t val)
{
    tft_data((val >> 8) & 0xFF);
    tft_data(val & 0xFF);
}

// ---------------- ILI9341 Init ----------------
static void tft_init(void)
{
    gpio_set_high(RST_PORT, RST_PIN);
    Cy_SysLib_Delay(10);
    gpio_set_low(RST_PORT, RST_PIN);
    Cy_SysLib_Delay(20);
    gpio_set_high(RST_PORT, RST_PIN);
    Cy_SysLib_Delay(150);

    tft_cmd(0x01);//Software Reset
    Cy_SysLib_Delay(120);

    tft_cmd(0x28);//Display OFF

    tft_cmd(0x36);//Memory Access Control
    tft_data(0xE8); //invert

    tft_cmd(0x3A);//Pixel Format Set
    tft_data(0x55);//16bit/pixel

    tft_cmd(0x11);//Sleep Out
    Cy_SysLib_Delay(120);

    tft_cmd(0x29);//Display ON
}

// forward declaration so set_window can call it before the function is defined
static void tft_write_bytes(const uint8_t *buf, size_t len, bool is_data);

// ---------------- Window Set ----------------
static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t col[4];
    col[0] = (x0 >> 8) & 0xFF;
    col[1] = x0 & 0xFF;
    col[2] = (x1 >> 8) & 0xFF;
    col[3] = x1 & 0xFF;
    tft_cmd(0x2A);//Column Address Set
    tft_write_bytes(col, 4, true);

    uint8_t page[4];
    page[0] = (y0 >> 8) & 0xFF;
    page[1] = y0 & 0xFF;
    page[2] = (y1 >> 8) & 0xFF;
    page[3] = y1 & 0xFF;
    tft_cmd(0x2B);//Page Address Set
    tft_write_bytes(page, 4, true);

    tft_cmd(0x2C);//Memory Write
}

// ---------------- Drawing ----------------
static void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    set_window(x, y, x+w-1, y+h-1);

    uint32_t p = w * h;
    while (p--)
        tft_data16(color);
}

static void fill_screen(uint16_t color)
{
    fill_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, color);
}

static inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) |
           ((g & 0xFC) << 3) |
           (b >> 3);
}

// --------------- GPIO Init ----------------

// ------------------------------------------------------------------
// Fast streaming helpers (hold CS low while sending many data bytes)
// ------------------------------------------------------------------
static inline void tft_start(bool is_data)
{
    gpio_set_low(CS_PORT, CS_PIN);
    if (is_data) gpio_set_high(DC_PORT, DC_PIN);
    else          gpio_set_low(DC_PORT, DC_PIN);
}

static inline void tft_end(void)
{
    gpio_set_high(CS_PORT, CS_PIN);
}

// Send multiple bytes while keeping CS low. is_data selects DC line state.
static void tft_write_bytes(const uint8_t *buf, size_t len, bool is_data)
{
    tft_start(is_data);
    for (size_t i = 0; i < len; ++i) {
        tft_write_bus(buf[i]);
    }
    tft_end();
}

// Optimized 16-bit data writer that sends two bytes with CS held low

// ------------------------------------------------------------------
// 5x7 font (ASCII 32..127). Each glyph is 5 bytes (columns), LSB at top.
// Source: classic 5x7 font used in many small displays.
// ------------------------------------------------------------------
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // 32 ' '
    {0x00,0x00,0x5F,0x00,0x00}, // 33 '!'
    {0x00,0x07,0x00,0x07,0x00}, // 34 '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // 35 '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // 36 '$'
    {0x23,0x13,0x08,0x64,0x62}, // 37 '%'
    {0x36,0x49,0x55,0x22,0x50}, // 38 '&'
    {0x00,0x05,0x03,0x00,0x00}, // 39 '\''
    {0x00,0x1C,0x22,0x41,0x00}, // 40 '('
    {0x00,0x41,0x22,0x1C,0x00}, // 41 ')'
    {0x14,0x08,0x3E,0x08,0x14}, // 42 '*'
    {0x08,0x08,0x3E,0x08,0x08}, // 43 '+'
    {0x00,0x50,0x30,0x00,0x00}, // 44 ','
    {0x08,0x08,0x08,0x08,0x08}, // 45 '-'
    {0x00,0x60,0x60,0x00,0x00}, // 46 '.'
    {0x20,0x10,0x08,0x04,0x02}, // 47 '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // 48 '0'
    {0x00,0x42,0x7F,0x40,0x00}, // 49 '1'
    {0x42,0x61,0x51,0x49,0x46}, // 50 '2'
    {0x21,0x41,0x45,0x4B,0x31}, // 51 '3'
    {0x18,0x14,0x12,0x7F,0x10}, // 52 '4'
    {0x27,0x45,0x45,0x45,0x39}, // 53 '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // 54 '6'
    {0x01,0x71,0x09,0x05,0x03}, // 55 '7'
    {0x36,0x49,0x49,0x49,0x36}, // 56 '8'
    {0x06,0x49,0x49,0x29,0x1E}, // 57 '9'
    {0x00,0x36,0x36,0x00,0x00}, // 58 ':'
    {0x00,0x56,0x36,0x00,0x00}, // 59 ';'
    {0x08,0x14,0x22,0x41,0x00}, // 60 '<'
    {0x14,0x14,0x14,0x14,0x14}, // 61 '='
    {0x00,0x41,0x22,0x14,0x08}, // 62 '>'
    {0x02,0x01,0x51,0x09,0x06}, // 63 '?'
    {0x32,0x49,0x79,0x41,0x3E}, // 64 '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 65 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 66 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 67 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 68 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 69 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 70 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 71 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 72 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 73 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 74 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 75 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 76 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 77 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 78 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 79 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 80 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 81 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 82 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 83 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 84 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 85 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 86 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 87 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 88 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 89 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 90 'Z'
    {0x00,0x7F,0x41,0x41,0x00}, // 91 '['
    {0x02,0x04,0x08,0x10,0x20}, // 92 '\\'
    {0x00,0x41,0x41,0x7F,0x00}, // 93 ']'
    {0x04,0x02,0x01,0x02,0x04}, // 94 '^'
    {0x40,0x40,0x40,0x40,0x40}, // 95 '_'
    {0x00,0x01,0x02,0x04,0x00}, // 96 '`'
    {0x20,0x54,0x54,0x54,0x78}, // 97 'a'
    {0x7F,0x48,0x44,0x44,0x38}, // 98 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 99 'c'
    {0x38,0x44,0x44,0x48,0x7F}, //100 'd'
    {0x38,0x54,0x54,0x54,0x18}, //101 'e'
    {0x08,0x7E,0x09,0x01,0x02}, //102 'f'
    {0x0C,0x52,0x52,0x52,0x3E}, //103 'g'
    {0x7F,0x08,0x04,0x04,0x78}, //104 'h'
    {0x00,0x44,0x7D,0x40,0x00}, //105 'i'
    {0x20,0x40,0x44,0x3D,0x00}, //106 'j'
    {0x7F,0x10,0x28,0x44,0x00}, //107 'k'
    {0x00,0x41,0x7F,0x40,0x00}, //108 'l'
    {0x7C,0x04,0x18,0x04,0x78}, //109 'm'
    {0x7C,0x08,0x04,0x04,0x78}, //110 'n'
    {0x38,0x44,0x44,0x44,0x38}, //111 'o'
    {0x7C,0x14,0x14,0x14,0x08}, //112 'p'
    {0x08,0x14,0x14,0x18,0x7C}, //113 'q'
    {0x7C,0x08,0x04,0x04,0x08}, //114 'r'
    {0x48,0x54,0x54,0x54,0x20}, //115 's'
    {0x04,0x3F,0x44,0x40,0x20}, //116 't'
    {0x3C,0x40,0x40,0x20,0x7C}, //117 'u'
    {0x1C,0x20,0x40,0x20,0x1C}, //118 'v'
    {0x3C,0x40,0x30,0x40,0x3C}, //119 'w'
    {0x44,0x28,0x10,0x28,0x44}, //120 'x'
    {0x0C,0x50,0x50,0x50,0x3C}, //121 'y'
    {0x44,0x64,0x54,0x4C,0x44}, //122 'z'
    {0x00,0x08,0x36,0x41,0x00}, //123 '{'
    {0x00,0x00,0x7F,0x00,0x00}, //124 '|'
    {0x00,0x41,0x36,0x08,0x00}, //125 '}'
    {0x08,0x08,0x2A,0x1C,0x08}, //126 '~'
    {0x00,0x00,0x00,0x00,0x00}  //127 DEL (blank)
};

// Draw a single character at (x,y) with foreground/back color and integer scale
static void draw_char(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, double_t scale)
{
    if ((uint8_t)c < 32 || (uint8_t)c > 127) c = '?';
    const uint8_t *g = font5x7[(uint8_t)c - 32];
    uint16_t w = 6 * scale; // 5 columns + 1 spacing
    uint16_t h = 8 * scale; // 8 rows

    set_window(x, y, x + w - 1, y + h - 1);

    uint8_t rowBuf[/*max width*/ 32]; // will hold pairs of bytes, max w*2; w<=6*scale, scale small
    for (uint16_t yoff = 0; yoff < h; ++yoff) {
        uint16_t glyph_row = yoff / scale; // 0..7
        uint16_t idx = 0;
        for (uint16_t xoff = 0; xoff < w; ++xoff) {
            uint16_t glyph_col = xoff / scale; // 0..5 (5 means spacing)
            uint16_t color = bg;
            if (glyph_col < 5) {
                uint8_t colByte = g[glyph_col];
                bool pixel = (colByte >> glyph_row) & 1;
                color = pixel ? fg : bg;
            }
            // append high byte, low byte
            if (idx + 2 <= sizeof(rowBuf)) {
                rowBuf[idx++] = (color >> 8) & 0xFF;
                rowBuf[idx++] = color & 0xFF;
            }
        }
        // send this row
        tft_write_bytes(rowBuf, idx, true);
    }
}

// Draw a null-terminated string starting at (x,y)
static void draw_string(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg, double_t scale)
{
    uint16_t cursor_x = x;
    uint16_t cursor_y = y;
    const uint16_t char_w = 6 * scale; // 5 columns + 1 spacing
    const uint16_t char_h = 8 * scale;

    while (*s) {
        // If drawing this character would run off the right edge, wrap to next line
        if ((uint32_t)cursor_x + char_w > (uint32_t)TFT_WIDTH) {
            cursor_x = x;
            cursor_y += char_h + 1; // move down one line (+1 pixel spacing)
            // If no vertical space left, stop drawing
            if ((uint32_t)cursor_y + char_h > (uint32_t)TFT_HEIGHT) {
                break;
            }
        }
        draw_char(cursor_x, cursor_y, *s, fg, bg, scale);
        cursor_x += char_w;
        ++s;
    }
}

static void gpio_init_for_tft(void)
{
    Cy_GPIO_Pin_FastInit(D0_PORT, D0_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D1_PORT, D1_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D2_PORT, D2_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D3_PORT, D3_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D4_PORT, D4_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D5_PORT, D5_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D6_PORT, D6_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(D7_PORT, D7_PIN, CY_GPIO_DM_STRONG, 0, HSIOM_SEL_GPIO);

    Cy_GPIO_Pin_FastInit(CS_PORT,  CS_PIN,  CY_GPIO_DM_STRONG, 1, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(DC_PORT,  DC_PIN,  CY_GPIO_DM_STRONG, 1, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(WR_PORT,  WR_PIN,  CY_GPIO_DM_STRONG, 1, HSIOM_SEL_GPIO);
    Cy_GPIO_Pin_FastInit(RST_PORT, RST_PIN, CY_GPIO_DM_STRONG, 1, HSIOM_SEL_GPIO);
}

static void gpio_init_for_key(void)
{
    cyhal_gpio_init(P9_4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_PULLUP, 0);
    cyhal_gpio_init(P9_0, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, 1);
    cyhal_gpio_init(P9_7, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_PULLUP, 0);
    cyhal_gpio_init(P9_1, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, 1);
    cyhal_gpio_init(P8_4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_PULLUP, 0);
    cyhal_gpio_init(P9_2, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, 1);
    cyhal_gpio_init(P6_0, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_PULLUP, 0);
    cyhal_gpio_init(P9_3, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_PULLUP, 1);
}
// Keypad
int keypad(void)
{
    for (;;) 
    {
        // Row 0 active
        cyhal_gpio_write(P9_4, 0);
        cyhal_gpio_write(P9_7, 1);
        cyhal_gpio_write(P8_4, 1);
        cyhal_gpio_write(P6_0, 1);
        if (cyhal_gpio_read(P9_0) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_0) == 0) //Make sure that the key is released
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 0;
        }
        if (cyhal_gpio_read(P9_1) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_1) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 1;
        }
        if (cyhal_gpio_read(P9_2) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_2) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 2;
        }
        if (cyhal_gpio_read(P9_3) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_3) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 3;
        }
        // Row 1 active
        cyhal_gpio_write(P9_4, 1);
        cyhal_gpio_write(P9_7, 0);
        cyhal_gpio_write(P8_4, 1);
        cyhal_gpio_write(P6_0, 1);
        if (cyhal_gpio_read(P9_0) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_0) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 4;
        }
        if (cyhal_gpio_read(P9_1) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_1) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 5;
        }
        if (cyhal_gpio_read(P9_2) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_2) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 6;
        }
        if (cyhal_gpio_read(P9_3) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_3) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 7;
        }
        // Row 2 active
        cyhal_gpio_write(P9_4, 1);
        cyhal_gpio_write(P9_7, 1);
        cyhal_gpio_write(P8_4, 0);
        cyhal_gpio_write(P6_0, 1);
        if (cyhal_gpio_read(P9_0) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_0) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 8;
        }
        if (cyhal_gpio_read(P9_1) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_1) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 9;
        }
        if (cyhal_gpio_read(P9_2) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_2) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 10;
        }
        if (cyhal_gpio_read(P9_3) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_3) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 11;
        }
        // Row 3 active
        cyhal_gpio_write(P9_4, 1);
        cyhal_gpio_write(P9_7, 1);
        cyhal_gpio_write(P8_4, 1);
        cyhal_gpio_write(P6_0, 0);
        if (cyhal_gpio_read(P9_0) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_0) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 12;
        }
        if (cyhal_gpio_read(P9_1) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_1) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 13;
        }
        if (cyhal_gpio_read(P9_2) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_2) == 0) cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 14;
        }
        if (cyhal_gpio_read(P9_3) == 0) 
        {
            cyhal_system_delay_ms(20);
            while (cyhal_gpio_read(P9_3) == 0) 
                cyhal_system_delay_ms(10);
            cyhal_system_delay_ms(20);
            return 15;
        }
        // Small pause to reduce CPU usage between scans
        cyhal_system_delay_ms(5);
    }
}

char disp[18]="Sequence Generator";
int e[10];
char out[10];
char o1[2];
char o2[2];
int k[10]={0};

int main(void)
{
    cybsp_init();
    __enable_irq();

    gpio_init_for_tft();
    gpio_init_for_key();
    Cy_SysLib_Delay(20);

    tft_init();

    fill_screen(rgb565(0,0,0));
    draw_string(30, 10,disp, rgb565(0,0,255), rgb565(0,0,0), 2.5);
    draw_string(10, 40,"Enter no of digit in input seq", rgb565(255,255,255), rgb565(0,0,0), 1.7);
    int N1 = keypad();
    if (N1 >= 0 && N1 <= 9) 
    {
        o1[0] = '0' + N1;
    } 
    else if (N1 >= 10 && N1 <= 15) 
    {
        o1[0] = 'A' + (N1 - 10);
    } 
    else 
    {
        o1[0] = '?';
    }
    o1[1] = '\0';
    draw_string(10, 60,o1, rgb565(255,255,255), rgb565(0,0,0), 1.7);
    draw_string(10, 80,"Enter no of digit in output seq", rgb565(255,255,255), rgb565(0,0,0), 1.7);
    int N2 = keypad();
    if (N2 >= 0 && N2 <= 9) 
    {
        o2[0] = '0' + N2;
    } 
    else if (N2 >= 10 && N2 <= 15) 
    {
        o2[0] = 'A' + (N2 - 10);
    } 
    else 
    {
        o2[0] = '?';
    }
    o2[1] = '\0';
    draw_string(10, 100,o2, rgb565(255,255,255), rgb565(0,0,0), 1.7);

    for(int i=0;i<N1;i++)
    {
        k[i] = keypad()+1;
    }

    for(int i=0;i<N2;i++)
    {
        if(k[i] != 0)
        {
            if ((k[i]-1) >= 0 && (k[i]-1) <= 9) 
            {
                out[i] = '0' + (k[i]-1);
            } 
            else if ((k[i]-1) >= 10 && (k[i]-1) <= 15) 
            {
                out[i] = 'A' + (k[i] - 10 -1);
            } 
            else 
            {
                out[i] = '?';
            }
            //Error condition
            if (i>=3)
            {
                e[i-3] = k[i-2]+k[i-3]-1;
                if(e[i-3] == k[i] )
                {
                    draw_string(10, 180,"Result : Valid", rgb565(255,255,255), rgb565(0,0,0), 2);
                }
                else 
                {
                    draw_string(10, 180,"Result : Invalid", rgb565(255,255,255), rgb565(0,0,0), 2);
                }
            }
        }
        else 
        {
            k[i] = k[i-2] + k[i-3]-1;
            if ((k[i]-1) >= 0 && (k[i]-1) <= 9) 
            {
                out[i] = '0' + (k[i]-1);
            } 
            else if ((k[i]-1) >= 10 && (k[i]-1) <= 15) 
            {
                out[i] = 'A' + (k[i]-10-1);
            } 
            else  
            {
                out[i] = '?';
            }
            out[N2] = '\0';
        }
    }
    draw_string(10, 120, "Output Sequence;", rgb565(0,255,0), rgb565(0,0,0), 2);
    draw_string(10, 150, out, rgb565(255,255,255), rgb565(0,0,0), 2.5);
    for (;;)
    { 
    }   
}