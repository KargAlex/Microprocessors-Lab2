#include "platform.h"
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include <string.h>
#include "queue.h"
#include "timer.h"
#include <stdbool.h>
#include "leds.h"
#include "delay.h"
#include "gpio.h"

#define BUFF_SIZE 128           // Max buffer length
#define TIMER_PERIOD 500000     // Timer interval in microseconds
#define BUTTON_PIN 13 					// PC13

Queue rx_queue; // Queue for storing received UART characters

// UART ISR — stores characters into a queue
void uart_rx_isr(uint8_t rx) {
    if (rx >= 0x00 && rx <= 0x7F) {
        queue_enqueue(&rx_queue, rx);
    }
}

// Global buffers and flags
char buff[BUFF_SIZE];                   // Buffer to store received number as string
volatile uint8_t currentBuffIndex = 0;  // Tracks current digit being processed
char led_msg[32];                       // UART message buffer (LED)
char button_msg[56];										// UART message buffer (button)
volatile bool input_ready = false;      // Tracks whether we're processing input (true) or waiting for new input (false)
volatile bool is_led_on = false;       
volatile uint8_t button_count = 0;
volatile bool is_button_pressed = false;
volatile bool repeat_proccess = false;	// Tracks if '-' is at end of input
volatile bool print_msg_button = false;
volatile bool print_msg_led = false;

// Timer ISR — processes one digit per interrupt
void timer_isr() {
    // If all digits have been processed, stop timer
    if (buff[currentBuffIndex] == '\0' && input_ready) {
			if (repeat_proccess) {
				currentBuffIndex = 0;
				return;
			}
			timer_disable();
			uart_print("End of sequence. Waiting for new number...\r\n");
			return;
    }

    // Convert current digit from ASCII to integer
    int digit = buff[currentBuffIndex] - '0';

    // Skip non-digit characters
    if (digit < 0 || digit > 9) {
        currentBuffIndex++;
        return;
    }

    // Handle even/odd digit behavior
    if (digit % 2) {
			if (is_button_pressed)
				sprintf(led_msg, "Digit %d -> Skipped LED action\r\n", digit);
			else {
				sprintf(led_msg, "Digit %d -> Toggle LED\r\n", digit);
				is_led_on = !is_led_on;
				if (is_led_on)
					leds_set(1, 0, 0);
				else 
					leds_set (0,0,0);
			}
    } 
		else {
			if (is_button_pressed)
				sprintf(led_msg, "Digit %d -> Skipped LED action\r\n", digit);
			else {
				sprintf(led_msg, "Digit %d -> Blink LED\r\n", digit);
				
				if (is_led_on) {	
					leds_set(0, 0, 0);
					delay_ms(200);
					leds_set(1, 0, 0);
					delay_ms(200);
					leds_set(0, 0, 0);
					
					is_led_on = false;
				}
				else {
					leds_set(1, 0, 0);
					delay_ms(200);
					leds_set(0, 0, 0);
					delay_ms(200);
					leds_set(1, 0, 0);
					
					is_led_on = true;
				}
			}
		}
			
		print_msg_led = true;
		
    currentBuffIndex++; // Move to next digit for next timer tick
		
}


// Button ISR
void button_isr(int pin_index) {
	if (pin_index	== BUTTON_PIN) {
		button_count++;
		if(is_button_pressed) {
			if (input_ready) {	// for display purposes, newline problems
				sprintf(button_msg, "Interrupt: Button pressed. LED unlocked. Count = %d\r\n", button_count);
			} 
			else {
				sprintf(button_msg, "\r\nInterrupt: Button pressed. LED unlocked. Count = %d\r\n", button_count);
			}
			is_button_pressed = false;
		}
		else {
			if (input_ready) {	// for display purposes, newline problems
				sprintf(button_msg, "Interrupt: Button pressed. LED locked. Count = %d\r\n", button_count);
			} 
			else {
				sprintf(button_msg, "\r\nInterrupt: Button pressed. LED locked. Count = %d\r\n", button_count);
			}
			is_button_pressed = true;
		}
		print_msg_button = true;
	}
	
}


int main() {
    // Init hardware modules
    leds_init();
    queue_init(&rx_queue, 128);
    uart_init(115200);
    uart_set_rx_callback(uart_rx_isr);
    uart_enable();
    timer_init(TIMER_PERIOD);
    timer_set_callback(timer_isr);
    gpio_set_mode(P_SW, PullDown);          // Button --> Pulldown
    gpio_set_trigger(P_SW, Rising);      		// Trigger on rising edge
    gpio_set_callback(P_SW, button_isr); 
	
		NVIC_SetPriority(EXTI15_10_IRQn, 0);  // Button on EXTI15_10
    NVIC_SetPriority(USART2_IRQn, 1);     
	
    __enable_irq(); // Enable global interrupts

    uint8_t rx_char = 0;
    uint32_t buff_index = 0;

    uart_print("\r\nInput:");

    while (1) {
        // Wait for input characters
        while (!queue_dequeue(&rx_queue, &rx_char)) {
					
					if (print_msg_button) {
            uart_print(button_msg);    
            print_msg_button = false;
					}
					
					if (print_msg_led) {
						uart_print(led_msg);
						print_msg_led = false;
					}
					__WFI(); // Sleep until interrupt fires
				}

        // If input is already being processed and you have new input, cancel old proccess immediately
        if (input_ready) {
					repeat_proccess = false;
					timer_disable();             		// Stop current analysis
					input_ready = false;         		// Mark as ready for new input
					uart_print("\r\nInput:");    	
					buff_index = 0;
					memset(buff, 0, sizeof(buff)); 	// Clear the buffer completely
        }

        // Handle backspace
        if (rx_char == 0x7F) {
					if (buff_index > 0) {
							buff_index--;
							uart_tx(rx_char); // Echo backspace
					}
        } else {
					// Echo character and store
					uart_tx(rx_char);
					buff[buff_index++] = (char)rx_char;
        }

        // If Enter was pressed, mark input complete and start processing
        if (rx_char == '\r') {
					if (buff[buff_index -2] == '-')
						repeat_proccess = true;			// If last character is '-', repeat proccess
					buff[buff_index - 1] = '\0';  // Replace '\r' with string terminator
					uart_print("\r\n");           
					currentBuffIndex = 0;         
					input_ready = true;           // Mark input as ready for proccessing
					timer_enable();               // Begin digit proccessing
        }

        // Buffer overflow warning
        if (buff_index >= BUFF_SIZE) {
					uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
					buff_index = 0;
        }
				
				
    }
}
