# 🧑‍💻 Developer Guide

Welcome to the developer documentation for the **Sequence Generator - Embedded C** project.

This document explains the internal working of the project, including its architecture, program flow, source code organization, and sequence generation algorithm. It is intended for developers who want to understand, modify, or extend the application.

---

# 📑 Table of Contents

- Project Architecture
- Program Workflow
- Source Code Organization
- Hardware Interfaces
- Sequence Generation Algorithm
- Application Flow
- Important Variables
- Future Enhancements

---

# 🏗️ Project Architecture

The application consists of four major modules.

```
                   +----------------------+
                   |      main()          |
                   +----------+-----------+
                              |
          +-------------------+-------------------+
          |                   |                   |
          |                   |                   |
          ▼                   ▼                   ▼
   TFT Display         Keypad Driver     Sequence Generator
 Initialization        User Input          Algorithm
          |                   |                   |
          +-------------------+-------------------+
                              |
                              ▼
                    Display Generated Output
```

Each module performs a specific task, making the program easier to understand and maintain.

---

# ⚙️ Program Workflow

The firmware follows the sequence shown below.

```
Start
   │
   ▼
Initialize Board
   │
   ▼
Initialize TFT Display
   │
   ▼
Display Welcome Screen
   │
   ▼
Read Number of Input Elements
   │
   ▼
Read Initial Sequence
   │
   ▼
Read Number of Output Elements
   │
   ▼
Generate New Sequence
   │
   ▼
Display Results
   │
   ▼
Wait for User Input
```

The program continuously interacts with the user through the keypad and updates the TFT display in real time.

---

# 📂 Source Code Organization

The project is primarily implemented in **main.c**, which contains the application logic.

The code can be divided into the following sections:

| Section | Description |
|---------|-------------|
| Library Includes | Required header files |
| Global Variables | Stores sequence values and user input |
| Display Functions | Controls the TFT display |
| Keypad Functions | Reads keypad input |
| Sequence Logic | Generates the output sequence |
| Main Function | Controls overall program execution |

---

# 📚 Libraries Used

The application makes use of the following libraries provided by ModusToolbox™ and the display driver.

| Library | Purpose |
|----------|---------|
| cyhal | Hardware Abstraction Layer |
| cybsp | Board Support Package |
| mtb_ili9341 | TFT Display Driver |
| mtb_st7789 / graphics libraries (if applicable) | Graphics support |
| stdio.h | Input and Output |
| stdlib.h | Standard utilities |
| string.h | String handling |

> **Note:** The exact libraries may vary depending on the BSP and display middleware used in your project.

---

# 🖥️ TFT Display Module

The TFT display is responsible for providing the graphical user interface.

It is used to display:

- Welcome screen
- User prompts
- Number of input elements
- Input sequence
- Generated sequence
- Status messages

The display is updated dynamically whenever the user enters data through the keypad.

---

# ⌨️ Keypad Interface

The project uses a **4×4 Matrix Keypad** for user interaction.

The keypad allows the user to:

- Enter the number of input elements
- Enter sequence values
- Specify the number of output values
- Confirm entries

The keypad scanning routine continuously monitors the key states and converts the detected key press into the corresponding character or number.

---

# 🔢 Sequence Generation

The sequence is generated using the recurrence relation:

```
P(n) = P(n−2) + P(n−3)
```

Unlike the original assignment, the implementation is generalized.

Instead of restricting the user to three input values, the firmware accepts **N input values** and generates the requested number of additional sequence elements.

This enhancement allows the application to support sequences of varying lengths without modifying the source code.

---

# 📋 Important Variables

The application maintains several variables to store user inputs and generated values.

Typical data maintained by the program includes:

| Variable | Purpose |
|----------|---------|
| Input Count | Number of initial elements entered by the user |
| Output Count | Number of sequence values to generate |
| Sequence Array | Stores both input and generated values |
| Current Index | Tracks the current sequence position |
| Key Value | Stores keypad input |

The sequence array grows logically as new elements are generated and displayed.

---

# 🔄 Application Flow Summary

1. Hardware is initialized.
2. TFT display is configured.
3. Welcome screen is displayed.
4. User enters the number of input elements.
5. User enters the initial sequence.
6. User specifies the number of output elements.
7. The sequence generation algorithm computes new values.
8. Generated values are displayed on the TFT screen.
9. The application waits for the next user interaction.

---
# 🧑‍💻 Code Explanation

---

# Header Files

```c
#include "cyhal_gpio.h"
#include "cyhal_system.h"
#include "cyhal.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_syslib.h"
#include "cycfg_pins.h"
#include "cy_gpio.h"
```

## Explanation

The project uses the **Infineon PSoC Hardware Abstraction Layer (HAL)** together with the **Peripheral Driver Library (PDL)**.

Each header serves a different purpose.

| Header | Purpose |
|---------|----------|
| cyhal_gpio.h | GPIO configuration and control |
| cyhal_system.h | System initialization |
| cyhal.h | Hardware Abstraction Layer |
| cy_pdl.h | Peripheral Driver Library |
| cybsp.h | Board Support Package |
| cy_syslib.h | Delay functions and utilities |
| cycfg_pins.h | Pin configuration generated by ModusToolbox |
| cy_gpio.h | Direct GPIO register operations |

These libraries allow the firmware to communicate directly with the hardware.

---

# Pin Definitions

```c
#define D0_PORT P5_2_PORT
#define D0_PIN P5_2_NUM
```

Similar definitions exist for

```
D0-D7

WR

CS

DC

RST
```

## Explanation

The TFT display communicates through an **8-bit parallel interface**.

Each data line is mapped to one GPIO pin.

```
PSoC GPIO

↓

D0

↓

LCD Bit0
```

```
PSoC GPIO

↓

D7

↓

LCD Bit7
```

Control pins are also defined.

| Pin | Purpose |
|------|----------|
| WR | Write Strobe |
| CS | Chip Select |
| DC | Data/Command Select |
| RST | Hardware Reset |

Using macros makes the program portable and easier to maintain.

---

# TFT Resolution

```c
#define TFT_WIDTH 320
#define TFT_HEIGHT 240
```

## Explanation

These constants define the display resolution.

```
Width

320 pixels

Height

240 pixels
```

These values are later used for

- Screen clearing
- Rectangle drawing
- Boundary checking

---

# GPIO Helper Functions

```c
static inline void gpio_set(...)
```

```c
gpio_set_high(...)
```

```c
gpio_set_low(...)
```

## Why these functions?

Instead of repeatedly writing

```c
Cy_GPIO_Write(...)
```

throughout the program,

the code wraps it inside helper functions.

Example

Instead of

```c
Cy_GPIO_Write(port,pin,1);
```

you simply write

```c
gpio_set_high(port,pin);
```

Advantages

- Cleaner code

- Better readability

- Easy maintenance

- Reduced repetition

---

# Writing Data to the TFT

One of the most important functions is

```c
static void tft_write_bus(uint8_t d)
```

```c
gpio_set(D0_PORT,D0_PIN,(d>>0)&1);

gpio_set(D1_PORT,D1_PIN,(d>>1)&1);

...

gpio_set(D7_PORT,D7_PIN,(d>>7)&1);
```

## Explanation

The TFT display receives **8 bits simultaneously**.

Suppose

```text
d = 173
```

Binary

```
10101101
```

The function separates every bit.

```
Bit7 → D7

Bit6 → D6

Bit5 → D5

...

Bit0 → D0
```

Each GPIO pin is then driven HIGH or LOW.

This effectively sends one byte to the LCD.

---

# Write Strobe

After placing the data on the bus,

the code performs

```c
gpio_set_low(WR_PORT,WR_PIN);

Cy_SysLib_DelayUs(5);

gpio_set_high(WR_PORT,WR_PIN);
```

## Explanation

The LCD reads data only when

```
WR

HIGH

↓

LOW

↓

HIGH
```

This pulse tells the display

> "The byte on the data bus is valid."

The 5 µs delay ensures reliable operation even with slower LCD modules.

---

# Sending Commands

```c
static void tft_cmd(uint8_t cmd)
```

```c
gpio_set_low(CS_PORT,CS_PIN);

gpio_set_low(DC_PORT,DC_PIN);

tft_write_bus(cmd);

gpio_set_high(CS_PORT,CS_PIN);
```

## Explanation

The TFT controller distinguishes between

```
Commands

and

Data
```

To send a command,

the firmware sets

```
DC = LOW
```

Examples of commands

```
0x01

Software Reset

0x11

Sleep Out

0x29

Display ON
```

---

# Sending Data

```c
static void tft_data(uint8_t dat)
```

Unlike commands,

this function sets

```
DC = HIGH
```

before transmitting the byte.

Data represents

- Colors

- Coordinates

- Characters

- Images

rather than instructions.

---

# Writing 16-bit Data

```c
static void tft_data16(uint16_t value)
{
    tft_data((value>>8)&0xFF);

    tft_data(value&0xFF);
}
```

## Explanation

The display operates using

```
RGB565

16-bit colour
```

Example

```
1111100000000000
```

Red

↓

Upper Byte

↓

Lower Byte

The function automatically splits the colour into two bytes before sending it to the LCD.

---

# TFT Initialization

One of the most important functions is

```c
static void tft_init()
```

The initialization sequence follows the ILI9341 datasheet.

### Step 1

Hardware Reset

```c
gpio_set_high(RST_PORT,RST_PIN);

...

gpio_set_low(RST_PORT,RST_PIN);
```

This resets the LCD controller.

---

### Step 2

Software Reset

```c
tft_cmd(0x01);
```

The display returns to its default configuration.

---

### Step 3

Display OFF

```c
tft_cmd(0x28);
```

The screen is disabled while configuration registers are updated.

---

### Step 4

Memory Access Control

```c
tft_cmd(0x36);

tft_data(0xE8);
```

This configures

- Display orientation
- RGB order
- Row/column increment
- Screen inversion

---

### Step 5

Pixel Format

```c
tft_cmd(0x3A);

tft_data(0x55);
```

The display is configured for

```
16-bit RGB565
```

which provides

```
65,536 colours
```

---

### Step 6

Sleep Out

```c
tft_cmd(0x11);
```

The LCD exits sleep mode and becomes operational.

---

### Step 7

Display ON

```c
tft_cmd(0x29);
```

Finally,

the display begins accepting drawing commands.

The TFT is now fully initialized and ready to display graphics and text.

---
# 🎨 Graphics Engine

After initializing the TFT display, the application provides a set of graphics functions that allow text and shapes to be drawn on the screen.

Instead of relying on an external graphics library, the project implements its own graphics engine.

The graphics module consists of

- Display window selection
- Rectangle drawing
- Screen filling
- RGB565 colour conversion
- Bitmap font
- Character rendering
- String rendering

---

# 🖼 Setting the Drawing Window

```c
static void set_window(uint16_t x0,
                       uint16_t y0,
                       uint16_t x1,
                       uint16_t y1)
```

## Purpose

ILI9341 does not allow pixels to be written at arbitrary locations directly.

Instead,

the firmware first defines a rectangular drawing area called the **Window**.

After that,

every pixel transmitted is automatically written inside this window.

---

### Column Address

```c
tft_cmd(0x2A);
```

The command **0x2A**

tells the LCD

```
Set Column Address
```

The program then sends

```
Starting X Coordinate

Ending X Coordinate
```

---

### Page Address

```c
tft_cmd(0x2B);
```

This command selects

```
Starting Row

Ending Row
```

---

### Memory Write

```c
tft_cmd(0x2C);
```

Once the window is configured,

the display enters

```
Memory Write Mode
```

Every pixel received afterwards is written sequentially into the selected area.

---

# 🟩 Drawing a Rectangle

```c
static void fill_rect(
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t color)
```

## Purpose

Draws a filled rectangle anywhere on the TFT display.

---

### Step 1

Select the drawing window

```c
set_window(
    x,
    y,
    x+w-1,
    y+h-1
);
```

---

### Step 2

Calculate number of pixels

```c
uint32_t p = w * h;
```

Example

Rectangle

```
40 × 20

↓

800 pixels
```

---

### Step 3

Write colour repeatedly

```c
while(p--)
{
    tft_data16(color);
}
```

Each iteration writes

```
One Pixel
```

until the entire rectangle is filled.

---

# 🖥 Clearing the Screen

```c
static void fill_screen(uint16_t color)
{
    fill_rect(
        0,
        0,
        TFT_WIDTH,
        TFT_HEIGHT,
        color
    );
}
```

## Explanation

Instead of clearing the display pixel-by-pixel,

the program simply draws

one large rectangle

covering

```
320 × 240 pixels
```

This is much simpler and more efficient.

Example

```c
fill_screen(rgb565(0,0,0));
```

Result

```
Entire Screen

↓

Black
```

---

# 🎨 RGB565 Colour Conversion

The display operates in

```
RGB565

16-bit Colour Format
```

The conversion function is

```c
static inline uint16_t rgb565(
    uint8_t r,
    uint8_t g,
    uint8_t b)
```

Implementation

```c
return ((r & 0xF8) << 8)
     | ((g & 0xFC) << 3)
     | (b >> 3);
```

---

## Why RGB565?

Normal RGB uses

```
8 bits

for

Red

Green

Blue

=

24 bits
```

ILI9341 uses

```
5 bits Red

6 bits Green

5 bits Blue

=

16 bits
```

This reduces memory usage while still providing

```
65,536 Colours
```

---

Example

```c
rgb565(255,0,0)
```

↓

Produces

```
Red
```

---

```c
rgb565(0,255,0)
```

↓

Produces

```
Green
```

---

```c
rgb565(0,0,255)
```

↓

Produces

```
Blue
```

---

# 🚀 Fast Streaming

Instead of

```
CS LOW

↓

Send Byte

↓

CS HIGH

↓

Repeat
```

your program optimizes communication.

```c
static inline void tft_start(bool is_data)
```

keeps

```
CS LOW
```

throughout the transfer.

Similarly,

```c
static inline void tft_end()
```

raises CS only once after transmission is complete.

---

This greatly improves

- TFT refresh speed
- Text rendering speed
- Rectangle drawing speed

because the LCD controller does not need to reselect the chip for every byte.

---

# 📦 Sending Multiple Bytes

```c
static void tft_write_bytes(
    const uint8_t *buf,
    size_t len,
    bool is_data)
```

The function

1. Starts communication

```c
tft_start(is_data);
```

2. Sends all bytes

```c
for(size_t i=0;i<len;i++)
{
    tft_write_bus(buf[i]);
}
```

3. Ends communication

```c
tft_end();
```

This function is mainly used by

- set_window()
- Address configuration
- TFT initialization

---

# 🔤 Bitmap Font

One of the most interesting parts of the project is

```c
static const uint8_t font5x7[][5]
```

This is a **custom bitmap font**.

Each character occupies

```
5 Bytes
```

Example

```
'A'

↓

0x7E
0x11
0x11
0x11
0x7E
```

Each byte represents one column of pixels.

```
█   █

█   █

█████

█   █

█   █
```

The table contains

```
ASCII 32

↓

ASCII 127
```

allowing the program to display letters,

numbers,

symbols,

and punctuation without using any graphics library.

---

# 💡 Advantages of Custom Font

✔ No external libraries

✔ Very small memory footprint

✔ Fast rendering

✔ Easy to scale

✔ Suitable for embedded systems

---
# 🔤 Character Rendering (`draw_char()`)

One of the most important functions in this project is `draw_char()`. It converts a character into pixels and displays it on the TFT screen using the custom 5×7 bitmap font.

## Function Prototype

```c
draw_char(
    uint16_t x,
    uint16_t y,
    char c,
    uint16_t fg,
    uint16_t bg,
    double_t scale
)
```

---

## Parameters

| Parameter | Description |
|-----------|-------------|
| `x` | Starting X coordinate |
| `y` | Starting Y coordinate |
| `c` | Character to display |
| `fg` | Text colour |
| `bg` | Background colour |
| `scale` | Character scaling factor |

---

## Step 1 – Validate the Character

```c
if ((uint8_t)c < 32 || (uint8_t)c > 127)
    c = '?';
```

### Explanation

The bitmap font only supports printable ASCII characters (32–127).

If the character is outside this range, it is replaced with `'?'`.

Example

```
Input

'A'

↓

Valid

↓

Display A
```

```
Input

ASCII 10

↓

Invalid

↓

Display ?
```

---

## Step 2 – Select the Font Bitmap

```c
const uint8_t *g = font5x7[(uint8_t)c - 32];
```

### Explanation

Each character is stored as a **5-column bitmap**.

For example,

```
'A'

↓

ASCII = 65

↓

65 − 32 = 33

↓

font5x7[33]
```

This retrieves the bitmap for the character.

---

## Step 3 – Calculate Character Size

```c
uint16_t w = 6 * scale;
uint16_t h = 8 * scale;
```

### Explanation

Each character consists of:

- **5 columns** for the glyph
- **1 column** for spacing
- **8 rows** (7 pixels + spacing)

The `scale` parameter enlarges the character.

Example

```
scale = 1

↓

6 × 8 pixels
```

```
scale = 2

↓

12 × 16 pixels
```

---

## Step 4 – Define the Drawing Area

```c
set_window(
    x,
    y,
    x + w - 1,
    y + h - 1
);
```

### Explanation

Before writing pixels, the display window is set to the exact size of the character.

This means every pixel sent afterwards automatically fills only that character area.

---

## Step 5 – Render the Character

Your code loops through every row and every column of the scaled character.

```c
for (uint16_t yoff = 0; yoff < h; ++yoff)
{
    ...
    for (uint16_t xoff = 0; xoff < w; ++xoff)
    {
        ...
    }
}
```

### Explanation

For each screen pixel:

1. Determine the corresponding font row.
2. Determine the corresponding font column.
3. Check whether the bitmap bit is set.
4. Choose foreground or background colour.
5. Send the colour to the TFT.

This effectively converts the stored bitmap into visible pixels.

---

## Scaling Technique

Your implementation enlarges characters without requiring separate font files.

```c
glyph_row = yoff / scale;
glyph_col = xoff / scale;
```

Instead of storing multiple font sizes, the same bitmap is reused.

Example

Original

```
█ █
███
█ █
```

Scale = 2

```
██  ██

██████

██  ██
```

This approach saves Flash memory while supporting different text sizes.

---

# 📝 String Rendering (`draw_string()`)

Instead of displaying one character at a time manually, the project provides `draw_string()` to print complete text.

## Function Prototype

```c
draw_string(
    uint16_t x,
    uint16_t y,
    const char *s,
    uint16_t fg,
    uint16_t bg,
    double_t scale
)
```

---

## Purpose

This function repeatedly calls `draw_char()` until the end of the string is reached.

Example

```c
draw_string(
    20,
    40,
    "Sequence Generator",
    BLUE,
    BLACK,
    2
);
```

Output

```
Sequence Generator
```

---

## Cursor Management

The function maintains two cursor positions.

```c
cursor_x
cursor_y
```

Initially,

```
cursor_x = x
cursor_y = y
```

After each character,

```c
cursor_x += char_w;
```

The cursor automatically moves to the next character position.

---

## Automatic Line Wrapping

One of the best features of your implementation is automatic line wrapping.

```c
if(cursor_x + char_w > TFT_WIDTH)
{
    cursor_x = x;
    cursor_y += char_h + 1;
}
```

### Explanation

If a word reaches the right edge of the display,

the cursor automatically moves to the next line.

Example

```
ABCDEFGHIJKLMNOPQRSTUV...
```

becomes

```
ABCDEFGHIJKLMN

OPQRSTUVWXYZ
```

without any additional code.

---

## Preventing Screen Overflow

Your implementation also prevents text from being written beyond the bottom of the display.

```c
if(cursor_y + char_h > TFT_HEIGHT)
{
    break;
}
```

This ensures that characters are never drawn outside the valid display area.

---

# Why This Design is Efficient

Your text rendering implementation has several advantages:

- ✅ Custom bitmap font (no external graphics library)
- ✅ Supports scalable text
- ✅ Automatic line wrapping
- ✅ Boundary checking
- ✅ Low memory usage
- ✅ Optimized for embedded systems

---

# Overall Rendering Process

```
draw_string()

        │
        ▼

Read Character

        │
        ▼

draw_char()

        │
        ▼

Read Font Bitmap

        │
        ▼

Scale Character

        │
        ▼

Convert Bitmap to Pixels

        │
        ▼

Send Pixel Data to TFT

        │
        ▼

Display Character
```

---

## Summary

The combination of `draw_char()` and `draw_string()` forms a lightweight graphics engine for the ILI9341 TFT display. By using a custom 5×7 bitmap font, dynamic scaling, automatic line wrapping, and boundary checking, the project provides a compact and efficient text rendering solution suitable for resource-constrained embedded systems.
---
# ⌨️ Keypad Driver

The project uses a **4×4 matrix keypad** to receive user input. Instead of dedicating one GPIO pin to each key, the keypad is arranged in rows and columns, reducing the number of GPIO pins required.

---

## Working Principle

```
          Columns

        C0  C1  C2  C3
        │   │   │   │
R0 ─────●───●───●───●
R1 ─────●───●───●───●
R2 ─────●───●───●───●
R3 ─────●───●───●───●
```

The firmware scans the keypad by activating **one row at a time** while reading all four column inputs.

---

## Keypad Function

```c
int keypad(void)
{
    for(;;)
    {
        ...
    }
}
```

### Explanation

The infinite loop continuously scans the keypad until a key is pressed.

If no key is detected, scanning continues.

Once a key is detected, the corresponding key value is returned to the caller.

---

## Row Scanning

Example:

```c
cyhal_gpio_write(P9_4,0);
cyhal_gpio_write(P9_7,1);
cyhal_gpio_write(P8_4,1);
cyhal_gpio_write(P6_0,1);
```

### Explanation

Here,

- Row 0 is driven **LOW**
- Remaining rows are driven **HIGH**

Now only the switches connected to Row 0 can create a LOW signal on the column lines.

---

## Reading Columns

```c
if(cyhal_gpio_read(P9_0)==0)
{
    ...
    return 0;
}
```

### Explanation

If Column 0 becomes LOW,

it means

```
Row 0

+

Column 0

↓

Key 0 Pressed
```

The function immediately returns the corresponding key value.

---

## Debouncing

```c
cyhal_system_delay_ms(20);

while(cyhal_gpio_read(...)==0)
    cyhal_system_delay_ms(10);

cyhal_system_delay_ms(20);
```

### Why?

Mechanical switches do not produce a clean transition.

Instead,

```
Pressed

↓

101010101010

↓

Stable LOW
```

The delay removes this bouncing effect, ensuring only one key press is detected.

---

# 🚀 Main Application

The `main()` function controls the complete application.

```c
int main(void)
{
    ...
}
```

---

# Step 1 – Initialize Hardware

```c
cybsp_init();

__enable_irq();

gpio_init_for_tft();

gpio_init_for_key();

tft_init();
```

### Explanation

The firmware initializes

- Board Support Package
- Global interrupts
- TFT GPIO
- Keypad GPIO
- TFT display

After this step, all peripherals are ready.

---

# Step 2 – Display Welcome Screen

```c
fill_screen(rgb565(0,0,0));

draw_string(...);
```

### Explanation

The screen is cleared to black.

Then

```
Sequence Generator
```

is displayed as the title.

---

# Step 3 – Read Input Size

```c
draw_string(...);

int N1 = keypad();
```

The user enters

```
Number of Input Elements
```

Example

```
5
```

The value is stored in

```
N1
```

---

# Step 4 – Display Input Size

```c
o1[0]='0'+N1;
```

### Explanation

The keypad returns an integer.

The TFT requires text.

Therefore

```
5

↓

'5'
```

is stored before displaying it.

---

# Step 5 – Read Output Size

```c
int N2 = keypad();
```

The user enters

```
Number of Output Elements
```

Example

```
8
```

This value is stored inside

```
N2
```

---

# Step 6 – Read Initial Sequence

```c
for(int i=0;i<N1;i++)
{
    k[i]=keypad()+1;
}
```

### Explanation

The loop executes **N1** times.

Each keypad value is stored in the sequence array.

Example

```
Input

1

2

3

↓

k[]

2 3 4
```

The code adds **+1** because internally it stores values one greater than the keypad return value.

---

# Step 7 – Generate / Validate Sequence

The core logic is

```c
if(k[i]!=0)
{
    ...
}
else
{
    ...
}
```

Two operating modes are supported.

---

## Validation Mode

When the user provides a value,

the firmware checks

```c
e[i-3]=k[i-2]+k[i-3]-1;
```

Then

```c
if(e[i-3]==k[i])
```

If equal

```
Result : Valid
```

Otherwise

```
Result : Invalid
```

This verifies whether the entered sequence follows the recurrence relation.

---

## Generation Mode

If no value exists,

the firmware generates one automatically.

```c
k[i]=k[i-2]+k[i-3]-1;
```

The generated value is then stored back into the sequence array.

This is the enhancement that makes your implementation more flexible than the original assignment.

---

# Step 8 – Convert Output to Characters

The generated integer values are converted to ASCII before display.

```c
out[i]='0'+(k[i]-1);
```

Example

```
7

↓

'7'
```

For values greater than 9,

the program converts them to

```
A

B

C

...

F
```

allowing hexadecimal-style display.

---

# Step 9 – Display Result

```c
draw_string(
    10,
    120,
    "Output Sequence;",
    ...
);

draw_string(
    10,
    150,
    out,
    ...
);
```

The TFT now displays the complete generated sequence.

Example

```
Output Sequence

123581321
```

---

# Step 10 – Infinite Loop

```c
for(;;)
{
}
```

### Explanation

Once execution is complete,

the firmware remains active.

The microcontroller continues running until reset or power-off.

---

# 📊 Time Complexity

Let

```
N = Number of Input Elements

M = Number of Output Elements
```

Reading Inputs

```
O(N)
```

Generating Sequence

```
O(M)
```

Overall

```
O(N + M)
```

---

# 💾 Space Complexity

The sequence is stored in a single array.

Therefore,

```
Space Complexity

O(M)
```

No dynamic memory allocation is used, making the implementation suitable for resource-constrained embedded systems.

---

# ✅ Project Highlights

- Custom 8-bit parallel ILI9341 TFT driver
- Custom 5×7 bitmap font rendering
- Scalable text rendering
- Matrix keypad scanning with software debouncing
- Generalized sequence generation
- Sequence validation
- Real-time TFT output
- Efficient embedded C implementation
- Modular graphics and hardware abstraction

---

# 📌 Conclusion

This project demonstrates the development of a complete embedded application on the **Infineon PSoC™ OPTIGA™ Development Kit**. It integrates a matrix keypad for user input and an ILI9341 TFT display for graphical output while implementing a generalized sequence generation algorithm.

Compared to the original assignment, which was limited to fixed input and output sizes, this implementation allows the user to specify the sequence length dynamically. The project also includes a custom TFT graphics engine, bitmap font rendering, and keypad driver, making it a reusable foundation for future embedded user-interface applications.
