/* 
 * File:   newmain.c
 * Author: frog
 *
 * Created on November 30, 2020, 4:45 PM
 */
#define F_CPU 1000lU
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdbool.h>


#define FC_5V PA0
#define RESET PA1
#define I_BUZ PA2

#define LED_ARM PB3
#define LED_VBAT PB2

#define BUZ PB1
#define ALARM_ON() PORTB |= _BV(BUZ)
#define ALARM_OFF() PORTB |= ~_BV(BUZ)

//void delay(int i) {
//    _delay_ms(10);
//}
//
//void blink(uint8_t led) {
//    PORTB ^= _BV(led);
//    delay(5);
//    PORTB ^= _BV(led);
//}

void sleep_5V(void) {
    PCMSK0 |= (1<<PCINT0); // Pin Change Mask Register 0
    PCICR |= (1<<PCIE0); // Pin Change Interrupt Control Register
    sleep_enable();
    sei();

    sleep_cpu();
    sleep_disable();
}

static volatile bool had_reset = false;
void sleep_reset(void) {
    PCMSK0 |= (1<<PCINT1); // Pin Change Mask Register 1
    PCICR |= (1<<PCIE0); // Pin Change Interrupt Control Register
    sleep_enable();
    sei();

    sleep_cpu();
    cli();
    sleep_disable();
}

ISR(PCINT0_vect) {
    cli();
}

void wd_enable() {
    WDTCSR |= 1 << 3; // WDE Watchdog System Reset Enable
    WDTCSR |= 1 << 6; // Watchdog Interrupt enable
}

void wd_disable() {
    WDTCSR &= ~(1 << 3); // WDE Watchdog System Reset Enable
    WDTCSR &= ~(1 << 6); // Watchdog Interrupt enable
    RSTFLR &= ~(1 << 3); // Reset Flag Register
}


int main(void) {
    wd_disable();
    WDTCSR |= 0b0001; // every 32ms
            
    
    // Define Pins
    DDRB |= _BV(LED_ARM);
    DDRB |= _BV(LED_VBAT);
    DDRB = 0xFF;
    
    DDRA = 0x00;
    
    // PORTA |= _BV(FC_5V);
    
    // Blink LEDs
    // blink(LED_ARM);
    // blink(LED_VBAT);
    
    bool alarm_state = false;
    
    while (1) {
        // Sleep until FC_5V HIGH
        sleep_5V();
                
        // LED_VBAT, LED_ARM = HIGH
        PORTB |= _BV(LED_ARM);
        PORTB |= _BV(LED_VBAT);
        
        // Sleep until FC_5V LOW
        sleep_5V();
        
        // LED_VBAT = LOW
        PORTB &= ~_BV(LED_VBAT);
        
        alarm_state = true;
        
        while (alarm_state == true) {
            wd_enable();
            // ALARM = HIGH
            ALARM_ON();
            //SLEEP(RESET == HIGH) // gets interupted by WDT
            sleep_reset();
            //if(POR!=0 && TO!=0) {} // Wakeup due to RESET going high
            if (!WDRF) {
                break;
            }
            //ALARM = LOW
            ALARM_OFF();
            //SLEEP(RESET == HIGH) // gets interupted by WDT
            sleep_reset();
            // Wakeup due to RESET going high
            if (!WDRF) {
                break;
            }
        }
        ALARM_OFF(); // Ensure alarm is turned off
        alarm_state = false;
        
        // LED_ARM = LOW
        PORTB |= ~_BV(LED_ARM);
    }
}

