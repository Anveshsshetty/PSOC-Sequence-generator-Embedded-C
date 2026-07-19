<div align="center">

# 🚀 Sequence Generator - Embedded C

### Generalized Sequence Generator using Infineon PSoC™ OPTIGA™ Development Kit

![Language](https://img.shields.io/badge/Language-C-blue?style=for-the-badge&logo=c)
![Platform](https://img.shields.io/badge/Platform-Infineon%20PSoC-orange?style=for-the-badge)
![IDE](https://img.shields.io/badge/IDE-ModusToolbox-green?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

</div>

---

# 📖 Project Overview

This project is an **Embedded C** application developed on the **Infineon PSoC™ OPTIGA™ Development Kit**.

The system interfaces a **4×4 Matrix Keypad** with an **ILI9341 TFT Display** to generate and display numeric sequences based on a recurrence relation.

The original assignment required generating **3 output values from 3 input values**. This project extends that requirement by implementing a **generalized algorithm**, allowing the user to specify:

- 📌 Number of input elements (**N**)
- 📌 Number of output elements to generate

This makes the solution reusable for different sequence lengths without modifying the code.

---

# ✨ Features

- ⌨️ 4×4 Matrix Keypad Interface
- 🖥️ ILI9341 TFT Display Interface
- ⚡ Embedded C Firmware
- 🔄 Dynamic Sequence Generation
- 📈 User-defined Input Size
- 📊 User-defined Output Size
- 🚀 Real-time Display
- ♻️ Generalized Sequence Algorithm

---

# 🎯 Problem Statement

The original problem required generating the next sequence values using the recurrence relation:

```text
P(n) = P(n−2) + P(n−3)
```

where:

- Input Sequence = 3 values
- Output Sequence = 3 values

This implementation generalizes the solution by allowing the user to choose both the input size and the number of generated output values.

---

# 🛠️ Hardware Used

- Infineon PSoC™ OPTIGA™ Development Kit
- 4×4 Matrix Keypad
- ILI9341 TFT Display

---

# 💻 Software Used

- ModusToolbox™
- Visual Studio Code
- Embedded C

---

# 📂 Repository Structure

```text
Sequence-generator-Embedded-c
│
├── bsps/
├── deps/
├── images/
├── libs/
├── mtb_shared/
├── docs/
│   ├── INSTALLATION.md
│   ├── DEVELOPER_GUIDE.md
│   ├── USER_GUIDE.md
│   └── Project_Report.pdf
│
├── main.c
├── Makefile
├── openocd.tcl
├── project_info.json
├── README.md
├── LICENSE
└── .gitignore
```

---

# 📷 Project Images

## Block Diagram

```text
images/Block_diagram.png
```

## Hardware Setup

```text
images/Setup.png
```

---

## TFT Display Output

> Add a photo of the TFT display showing the generated sequence.

```text
images/valid.jpeg
```

```text
images/invalid.jpeg
```
---

# 🚀 Getting Started

Clone the repository:

```bash
git clone https://github.com/Anveshsshetty/PSOC-Sequence-generator-Embedded-C.git
```

Open the project in **ModusToolbox™** and build it.

For detailed setup instructions, refer to the installation guide below.

---

# 📚 Documentation

Detailed documentation is available in the **docs** folder.

| 📄 Document | Description |
|------------|-------------|
| 🚀 [INSTALLATION.md](docs/INSTALLATION.md) | Complete setup guide, project import, build, and programming instructions |
| 🧠 [DEVELOPER_GUIDE.md](docs/DEVELOPER_GUIDE.md) | Detailed explanation of the source code, program flow, and sequence generation algorithm |
| 📖 [USER_GUIDE.md](docs/USER_GUIDE.md) | Step-by-step guide to operating the application |
| 📑 [Project_Report.pdf](docs/project_report.pdf) | Complete project report |

---

# 🌱 Future Improvements

- Touchscreen-based input
- Additional sequence generation algorithms
- EEPROM storage
- UART communication
- Improved graphical user interface

---

# 🤝 Contributing

Contributions, suggestions, and improvements are welcome. Feel free to fork the repository and submit a pull request.

---

# 👨‍💻 Author

**Anvesh S Shetty**

Electronics & Communication Engineering

---

# 📜 License

This project is licensed under the **MIT License**.

---

<div align="center">

### ⭐ If you found this project useful, please consider giving it a Star!

</div>
