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

#define BUFF_SIZE 128           // Read buffer length
#define TIMER_PERIOD 500000     // Timer interval in microseconds

Queue rx_queue; // Queue for storing received UART characters

// UART receive interrupt handler — stores characters into a queue
void uart_rx_isr(uint8_t rx) {
    if (rx >= 0x00 && rx <= 0x7F) {
        queue_enqueue(&rx_queue, rx);
    }
}

// Global buffers and flags
char buff[BUFF_SIZE];                   // Buffer to store received number as string
uint8_t volatile currentBuffIndex = 0;  // Tracks current digit being processed
char msg[32];                           // UART message buffer
volatile bool input_ready = false;      // Flag: true when number is entered and ready for processing
volatile bool is_led_on = false;        // State of the LED

// Timer ISR — processes one digit per interrupt
void timer_isr() {
    // If all digits have been processed, stop timer
    if (buff[currentBuffIndex] == '\0' && input_ready) {
			timer_disable();
			uart_print("End of sequence. Waiting for new number...");
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
			sprintf(msg, "Digit %d -> Toggle LED\r\n", digit);
			uart_print(msg);
			is_led_on = !is_led_on;
			if (is_led_on) {
				leds_set(1, 0, 0);
			}
			else 
				leds_set (0,0,0);
    } 
		else {
			sprintf(msg, "Digit %d -> Blink LED\r\n", digit);
			uart_print(msg);
			if (is_led_on) {	
				leds_set(0, 0, 0);
				delay_ms(200);
				leds_set(1, 0, 0);
				delay_ms(200);
			}
			else {
				leds_set(1, 0, 0);
				delay_ms(200);
				leds_set(0, 0, 0);
				delay_ms(200);
			}
		}
			

    currentBuffIndex++; // Move to next digit for next timer tick
		
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
    __enable_irq(); // Enable global interrupts

    uint8_t rx_char = 0;
    uint32_t buff_index = 0;

    uart_print("\r\nInput:");

    while (1) {
        // Wait for input characters
        while (!queue_dequeue(&rx_queue, &rx_char))
            __WFI(); // Sleep until interrupt fires

        // If input is already being processed (timer active), cancel it immediately
        if (input_ready) {
					timer_disable();             		// Stop current analysis
					input_ready = false;         		// Mark as ready for new input
					currentBuffIndex = 0;        		// Reset digit index
					uart_print("\r\n\nInput:");    		// Prompt again
					buff_index = 0;              		// Reset buffer index
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
            buff[buff_index - 1] = '\0';  // Replace '\r' with string terminator
            uart_print("\r\n");           // Newline after input
            currentBuffIndex = 0;         // Reset digit processing index
            input_ready = true;           // Mark input as ready for analysis
            timer_enable();               // Begin timer-based digit processing
        }

        // Buffer overflow warning
        if (buff_index >= BUFF_SIZE) {
            uart_print("Stop trying to overflow my buffer! I resent that!\r\n");
            buff_index = 0;
        }
    }
}
