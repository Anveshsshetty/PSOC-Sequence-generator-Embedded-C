# 📖 User Guide

Welcome to the **Sequence Generator - Embedded C** project.

This guide explains how to use the application on the **Infineon PSoC™ OPTIGA™ Development Kit**. It covers hardware setup, user input, sequence generation, validation, and interpreting the output displayed on the TFT screen.

---

# 📑 Table of Contents

- Project Overview
- Hardware Connections
- Powering the System
- Application Workflow
- Entering Input
- Generating the Sequence
- Sequence Validation
- Understanding the Output
- Example Execution
- Troubleshooting
- Frequently Asked Questions

---

# 📖 Project Overview

The application allows the user to generate and validate numeric sequences using a **4×4 Matrix Keypad** as the input device and an **ILI9341 TFT Display** as the output interface.

The firmware provides an interactive user interface where users can:

- Enter the number of input elements.
- Enter the initial sequence.
- Specify the number of output elements.
- Generate the remaining sequence.
- Validate an existing sequence.

---

# 🛠 Hardware Connections

The project requires the following hardware:

| Component | Purpose |
|----------|----------|
| Infineon PSoC™ OPTIGA™ Development Kit | Main Controller |
| ILI9341 TFT Display | Display Output |
| 4×4 Matrix Keypad | User Input |
| USB Cable | Power & Programming |

---

# ⚡ Powering the System

1. Connect the PSoC™ board to your computer using a USB cable.
2. Program the firmware.
3. Reset the board if required.
4. Wait for the TFT display to initialize.

The welcome screen will appear automatically.

---

# 🚀 Application Workflow

The application follows the workflow below.

```
Power ON

↓

Initialize Hardware

↓

Initialize TFT Display

↓

Display Welcome Screen

↓

Enter Number of Input Elements

↓

Enter Initial Sequence

↓

Enter Number of Output Elements

↓

Generate / Validate Sequence

↓

Display Output
```

---

# ⌨️ Entering Input

## Step 1

The display prompts

```
Enter Number of Input Elements
```

Example

```
5
```

Press the required key on the keypad.

---

## Step 2

The application now asks for the initial sequence.

Example

```
1

2

3

5

8
```

Each key press is displayed on the TFT.

---

## Step 3

The display asks

```
Enter Number of Output Elements
```

Example

```
8
```

The application now has enough information to process the sequence.

---

# 🔢 Sequence Generation

After receiving the required input,

the firmware generates additional sequence values based on the implemented recurrence relation.

Generated values are displayed automatically.

Example

```
Input

1

2

3

5

8

↓

Generated

13

21

34

55
```

---

# ✅ Sequence Validation

If the complete sequence is entered,

the firmware compares each element with the expected value.

Possible results

```
✔ Valid Sequence
```

or

```
❌ Invalid Sequence
```

The corresponding message is displayed on the TFT screen.

---

# 🖥 TFT Display Output

The TFT display shows different screens during execution.

## Welcome Screen

```
Sequence Generator
```

---

## Input Screen

```
Enter Number of Input Elements
```

---

## Sequence Entry

```
Enter Input Sequence
```

---

## Output Screen

```
Generated Sequence

13

21

34

55
```

---

## Validation Result

```
Result

VALID
```

or

```
Result

INVALID
```

---

# 🧪 Example Execution

### User Input

```
Input Size

5
```

```
Sequence

1

2

3

5

8
```

```
Output Size

8
```

---

### TFT Output

```
Generated Sequence

13

21

34

55
```

---

# ❗ Troubleshooting

## TFT Display is Blank

- Verify display connections.
- Ensure the board is powered.
- Confirm the firmware was programmed successfully.

---

## Keypad Not Responding

- Check row and column connections.
- Verify the keypad wiring.
- Restart the board.

---

## Incorrect Sequence

- Ensure the correct input sequence is entered.
- Verify the number of input elements.
- Check the requested output size.

---

# ❓ Frequently Asked Questions

### Can I enter more than three input values?

**Yes.**

Unlike the original assignment, this implementation allows the user to specify the number of input elements.

---

### Can I generate any number of output values?

**Yes.**

The user specifies the required number of output elements.

---

### Does the application validate sequences?

**Yes.**

The firmware can verify whether an entered sequence follows the implemented recurrence relation.

---

### Can this project be extended?

Yes.

Possible extensions include:

- Touchscreen support
- UART communication
- EEPROM storage
- SD card logging
- Multiple sequence algorithms
- Wireless communication

---

# 📚 Additional Documentation

For more information, refer to:

- 🚀 [README.md](../README.md)
- 🛠 [INSTALLATION.md](INSTALLATION.md)
- 👨‍💻 [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md)

---

# 🎉 Congratulations!

You have successfully learned how to operate the **Sequence Generator - Embedded C** application.

Happy Coding! 🚀
