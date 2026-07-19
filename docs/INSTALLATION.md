# 🛠️ Installation Guide

This guide explains how to set up the project, create a ModusToolbox™ application, import the source code, build the project, and run it on the **Infineon PSoC™ OPTIGA™ Development Kit**.

---

# 📋 Prerequisites

Before you begin, ensure you have the following:

### 💻 Software

- ModusToolbox™
- Visual Studio Code

### 🛠 Hardware

- Infineon PSoC™ OPTIGA™ Development Kit
- ILI9341 TFT Display
- 4×4 Matrix Keypad
- USB Cable

---

# 📥 Step 1 – Install ModusToolbox™

1. Download and install **ModusToolbox™**.
2. Launch ModusToolbox after installation.
3. Make sure the required Board Support Package (BSP) for your development kit is installed.

---

# 🚀 Step 2 – Create an Empty Application

### Open ModusToolbox™

Select

```
File
   ↓
New
   ↓
ModusToolbox™ Application
```

---

### Select the Target Board

Choose your **Infineon PSoC™ OPTIGA™ Development Kit**.

Click **Next**.

---

### Select the Application Type

Choose

```
Empty Application
```

Click **Next**.

---

### Name the Project

Example

```
Sequence-generator-Embedded-c
```

Choose the desired project location and click **Finish**.

---

# 📂 Step 3 – Import the Project Files

Open the newly created project folder.

Replace the generated files with the files from this repository.

### Copy the following folders

```
bsps/
deps/
libs/
mtb_shared/
```

### Copy the following files

```
main.c
Makefile
openocd.tcl
project_info.json
```

> **Note:** Replace the default `main.c` with the `main.c` from this repository.

---

# 🖥️ Step 4 – Open the Project in Visual Studio Code

You can open the project in two ways.

### Method 1

Inside ModusToolbox™

```
Tools
   ↓
Open in VS Code
```

### Method 2

Open Visual Studio Code manually.

Select

```
File
   ↓
Open Folder
```

Browse to your project folder and open it.

---

# 🔨 Step 5 – Build the Project

Inside ModusToolbox™, click

```
Build Application
```

Wait until the build completes successfully.

If there are no errors, the project is ready to be programmed.

---

# ⚡ Step 6 – Program the Development Kit

1. Connect the PSoC™ board using a USB cable.
2. Ensure the board is detected.
3. Click

```
Program Device
```

Wait until programming completes successfully.

---

# ▶️ Step 7 – Run the Application

After programming:

- The TFT display initializes.
- The welcome screen is displayed.
- The application prompts the user for input.

The user can:

- Enter the number of input elements.
- Enter the sequence using the keypad.
- Enter the number of output elements to generate.
- View the generated sequence on the TFT display.

---

# 📷 Expected Output

## Welcome Screen

```
Sequence Generator
```

---

## User Input

```
Enter Number of Inputs

5
```

---

## Generated Sequence

```
Generated Sequence

13
21
34
55
89
```

Replace the above text with screenshots of your actual TFT output.

---

# ❗ Troubleshooting

## Build Errors

- Verify that all project files have been copied.
- Ensure the correct BSP is installed.
- Clean and rebuild the project.

---

## Programming Errors

- Check the USB connection.
- Verify that the correct development board is connected.
- Close any application that may already be using the board.

---

## TFT Display Issues

- Verify all display connections.
- Check power supply.
- Ensure the display is connected correctly.

---

## Keypad Issues

- Verify row and column connections.
- Check GPIO pin assignments.
- Confirm the keypad is connected securely.

---

# 📚 Additional Documentation

For more information, refer to:

- 📖 [README.md](../README.md)
- 🧠 [CODE_EXPLANATION.md](CODE_EXPLANATION.md)
- 👨‍💻 [USER_GUIDE.md](USER_GUIDE.md)

---

<div align="center">

### 🎉 Installation Complete!

Your project is now ready to build, program, and run on the Infineon PSoC™ OPTIGA™ Development Kit.

</div>
