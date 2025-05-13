# ARM Assembly & C Project – UART-Controlled LED Processing with Button Interrupt

## Overview
This embedded systems project implements a number-processing routine on the STM32F401RE using C (with ARM CMSIS libraries). The program takes a numerical input over UART, processes each digit every 0.5 seconds using a hardware timer, and controls an onboard LED. Additionally, a hardware button can interrupt and control LED behavior dynamically.

## Requirements
- **Microcontroller:** Nucleo STM32F401RE  
- **Tools:** Keil uVision, PuTTY/Tera Term  
- **Languages:** C  

## Functionality

### 1. UART Input Handling
- Accepts dynamic user input via UART.
- Backspace and overflow are managed appropriately.
- The Enter key (`\r`) confirms the end of input.
- Inputs ending with `-` trigger an infinite repeat mode until interrupted by a new number.

### 2. Timer-Based Digit Processing
- A hardware timer triggers every **0.5 seconds (500,000 µs)**.
- Each interrupt processes **one digit at a time** from the received string:
  - **Even digit** → Blink LED (200ms ON, 200ms OFF).
  - **Odd digit** → Toggle LED state (ON → OFF or OFF → ON).
- UART logs the action taken for each digit.

### 3. Button Interrupt Handling
- **PC13 button** triggers an interrupt.
- On press:
  - Toggles LED locking mode:
    - Locked → No LED changes (actions skipped but logged).
    - Unlocked → LED actions resume normally.
  - UART prints current state and button press count.
- Button interrupt takes priority over UART input.

### 4. Real-Time Input Interruption
- If a new number is typed during an ongoing process:
  - The current analysis halts immediately.
  - Timer stops.
  - New input overwrites previous.
  - The process restarts from the beginning with the new input.

### 5. Special Repeat Mode (`-`)
- If the entered number ends with a dash (`-`), e.g., `81-`:
  - The number is processed in a **continuous loop**.
  - The loop ends only when new input is received.

### 6. UART Feedback
- Every interaction is logged over UART:
  - Digit analysis.
  - LED actions.
  - Button presses and LED lock/unlock state.
  - New input prompts or buffer warnings.

## Example UART Output

Input: 52719   
Digit 5 -> Toggle LED  
Digit 2 -> Blink LED  
Digit 7 -> Toggle LED    
Interrupt: Button pressed. LED locked. Count = 1  
Digit 1 -> Skipped LED action  
Digit 9 -> Skipped LED action  
Interrupt: Button pressed. LED unlocked. Count = 2  
End of sequence. Waiting for new number... 

Input: 81-  
Digit 8 -> Blink LED  
Digit 1 -> Toggle LED  
Digit 8 -> Blink LED  
Digit 1 -> Toggle LED  
... (New input received)  
Input: 42  
Digit 4 -> Blink LED  
Digit 2 -> Blink LED  
End of sequence. Waiting for new number... 

## Files
- `main.c` – Core logic: UART input handling, digit analysis, LED control, timer and button interrupts.
- `Drivers`
---

📌 *Developed for the "Microprocessors & Peripherals" 8th semester course, AUTh, Spring 2025.*
=======
# ARM Assembly & C Project – UART-Controlled LED Processing with Button Interrupt

## Overview
This embedded systems project implements a number-processing routine on the STM32F401RE using C (with ARM CMSIS libraries). The program takes a numerical input over UART, processes each digit every 0.5 seconds using a hardware timer, and controls an onboard LED. Additionally, a hardware button can interrupt and control LED behavior dynamically.

## Requirements
- **Microcontroller:** Nucleo STM32F401RE  
- **Tools:** Keil uVision, PuTTY/Tera Term  
- **Languages:** C  

## Functionality

### 1. UART Input Handling
- Accepts dynamic user input via UART.
- Backspace and overflow are managed appropriately.
- The Enter key (`\r`) confirms the end of input.
- Inputs ending with `-` trigger an infinite repeat mode until interrupted by a new number.

### 2. Timer-Based Digit Processing
- A hardware timer triggers every **0.5 seconds (500,000 µs)**.
- Each interrupt processes **one digit at a time** from the received string:
  - **Even digit** → Blink LED (200ms ON, 200ms OFF).
  - **Odd digit** → Toggle LED state (ON → OFF or OFF → ON).
- UART logs the action taken for each digit.

### 3. Button Interrupt Handling
- **PC13 button** triggers an interrupt.
- On press:
  - Toggles LED locking mode:
    - Locked → No LED changes (actions skipped but logged).
    - Unlocked → LED actions resume normally.
  - UART prints current state and button press count.
- Button interrupt takes priority over UART input.

### 4. Real-Time Input Interruption
- If a new number is typed during an ongoing process:
  - The current analysis halts immediately.
  - Timer stops.
  - New input overwrites previous.
  - The process restarts from the beginning with the new input.

### 5. Special Repeat Mode (`-`)
- If the entered number ends with a dash (`-`), e.g., `81-`:
  - The number is processed in a **continuous loop**.
  - The loop ends only when new input is received.

### 6. UART Feedback
- Every interaction is logged over UART:
  - Digit analysis.
  - LED actions.
  - Button presses and LED lock/unlock state.
  - New input prompts or buffer warnings.

## Example UART Output

Input: 52719   
Digit 5 -> Toggle LED  
Digit 2 -> Blink LED  
Digit 7 -> Toggle LED    
Interrupt: Button pressed. LED locked. Count = 1  
Digit 1 -> Skipped LED action  
Digit 9 -> Skipped LED action  
Interrupt: Button pressed. LED unlocked. Count = 2  
End of sequence. Waiting for new number... 

Input: 81-  
Digit 8 -> Blink LED  
Digit 1 -> Toggle LED  
Digit 8 -> Blink LED  
Digit 1 -> Toggle LED  
... (New input received)  
Input: 42  
Digit 4 -> Blink LED  
Digit 2 -> Blink LED  
End of sequence. Waiting for new number... 

## Files
- `main.c` – Core logic: UART input handling, digit analysis, LED control, timer and button interrupts.
- `Drivers`
---

📌 *Developed for the "Microprocessors & Peripherals" 8th semester course, AUTh, Spring 2025.*
>>>>>>> 70fe52944230b7245e3c254e840136e6f34fd707
