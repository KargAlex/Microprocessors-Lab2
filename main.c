#include "platform.h"
#include <stdio.h>
#include <stdint.h>
#include "uart.h"
#include <string.h>
#include "queue.h"
#include "leds.h"
#include "delay.h"
#include "timer.h"
#include "gpio.h"

// === Constants ===
#define BUFF_SIZE 64
#define BUTTON_PIN P_SW
#define BUTTON 13

Queue rx_queue; // Queue for storing received characters

// === Global State ===
uint8_t input_length = 0;
uint8_t current_digit_index = 0;
volatile uint8_t digit_ready = 0;
volatile uint8_t new_input_received = 0;
volatile uint8_t input_active = 0;
volatile uint8_t button_press_count = 0;

// Led logic
volatile uint8_t led_locked = 0;
volatile uint8_t timer = 0;
volatile uint8_t blink_count = 0;
volatile uint8_t led_status = 0;


char input_staging[BUFF_SIZE]; // separate buffer for typing
int staging_index = 0;         // index into staging

void handle_uart_input(void) {
    uint8_t rx_char;
    static uint8_t input_prompt_shown = 0;

    while (queue_dequeue(&rx_queue, &rx_char)) {
        // Backspace (0x7F)
        if (rx_char == 0x7F) {
            if (staging_index > 0) {
                staging_index--;
                uart_print("\b \b");  // Erase last character on terminal
            }
        }

        // Enter
        else if (rx_char == '\r') {
            input_staging[staging_index] = '\0';
            new_input_received = 1;
            staging_index = 0;
            input_prompt_shown = 0; // reset for next input
            uart_print("\r\n"); // move to new line
        }

        // Normal character
        else if (staging_index < BUFF_SIZE - 1) {
            // If we're starting to type, cancel previous analysis
            if (input_active) {
                input_active = 0;
                timer_disable();
                input_prompt_shown = 0; // reset for prompt display
                staging_index = 0; // reset current buffer
								memset(input_staging, 0, sizeof(input_staging)); 	// Clear the buffer 
            }

            if (!input_prompt_shown) {
                uart_print("\nInput: ");
                input_prompt_shown = 1;
            }

            input_staging[staging_index++] = rx_char;

            // Echo character to screen
            char s[2] = {rx_char, '\0'};
            uart_print(s);
        }
    }
}


// Interrupt Service Routine for UART receive
void uart_rx_isr(uint8_t rx) {
	// Check if the received character is a printable ASCII character
	if (rx >= 0x0 && rx <= 0x7F ) {
		// Store the received character
		queue_enqueue(&rx_queue, rx);
	}
}

void button_callback(int num) {
    
	if(num == BUTTON) {
		led_locked = !led_locked;
		button_press_count++;

    if (led_locked)
        uart_print("Interrupt: Button pressed. LED locked. Count = ");
    else
        uart_print("Interrupt: Button pressed. LED unlocked. Count = ");

		char str[12];
		sprintf(str, "%d\r\n", button_press_count);
		uart_print(str);
	}
}


// === Digit Handler ===
void process_digit(char digit_char)
{
    int digit = digit_char - '0';
    char msg[32];
    sprintf(msg, "Digit %d -> ", digit);
	
    if (led_locked) {
        strcat(msg, "Skipped LED action\r\n");
        uart_print(msg);
        return;
    }

    if (digit % 2 == 0) {
        strcat(msg, "Blink LED\r\n");
				//call_led_callback_twice()
        blink_count = 2;
    } else {
        strcat(msg, "Toggle LED\r\n");
				//call_led_callback_once
        blink_count = 1;
    }

    uart_print(msg);
}


void timer_callback(void){
	timer++;
	
	if (timer % 2 == 0) {
		if(blink_count > 0) {
			blink_count--;
			if (!led_locked) {
				led_status = !led_status;
				leds_set(led_status, 0,0);
			}
		}
	}
		
	if (timer % 5 == 0) {
		digit_ready = 1;
		timer = 0;
	}
}


int main() {
		
	
	// Initialize the receive queue and UART
	queue_init(&rx_queue, 64);
	uart_init(115200);
	uart_set_rx_callback(uart_rx_isr); 
	uart_enable(); 
	
	// Initiate leds
	leds_init();
	
	// Initialize timer
	timer_set_callback(timer_callback);
	timer_init(100000);  // 100ms
	timer_disable();
	
	// Initialize button
	gpio_set_mode(BUTTON_PIN, PullDown);            // Configure as pulldown input
	gpio_set_trigger(BUTTON_PIN, Rising);           // Interrupt on rising edge
	gpio_set_callback(BUTTON_PIN, button_callback); 
	
	NVIC_SetPriority(EXTI15_10_IRQn, 0);
	NVIC_SetPriority(USART2_IRQn, 1);
	
	__enable_irq(); // Enable interrupts
	
	uart_print("\r\n");// Print newline
	
	while (1) {
		
    handle_uart_input();   // Checks and updates input_buffer, sets new_input_received

    if (new_input_received) {
			input_length = strlen(input_staging);
			current_digit_index = 0;
			digit_ready = 0;
			new_input_received = 0;
			input_active = 1;


			timer_enable();  // ?? restart processing
	}

    if (input_active && digit_ready) {
        digit_ready = 0;

        if (current_digit_index < input_length) {
            char digit = input_staging[current_digit_index++];
            if (digit >= '0' && digit <= '9') {
                process_digit(digit);
            } else if (digit == '-') {
                current_digit_index = 0; // loop
            }
        } else {
            input_active = 0;
            uart_print("End of sequence. Waiting for new number...\r\n");
        }
    }

	}
		
		
	
}