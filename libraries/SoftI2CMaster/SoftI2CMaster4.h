/* Arduino SoftI2C library. 
 *
 * This is a very fast and very light-weight software I2C-master library 
 * written in assembler. It is based on Peter Fleury's I2C software
 * library: http://homepage.hispeed.ch/peterfleury/avr-software.html
 *
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino I2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* In order to use the library, you need to define SDA_PIN4, SCL_PIN4,
 * SDA_PORT4 and SCL_PORT4 before including this file.  Have a look at
 * http://www.arduino.cc/en/Reference/PortManipulation for finding out
 * which values to use. For example, if you use digital pin 3 for
 * SDA and digital pin 13 for SCL you have to use the following
 * definitions: 
 * #define SDA_PIN4 3 
 * #define SDA_PORT4 PORTB 
 * #define SCL_PIN4 5
 * #define SCL_PORT4 PORTB
 *
 * You can also define the following constants (see also below):
 * - I2C_CPUFREQ4, when changing CPU clock frequency dynamically
 * - I2C_FASTMODE4 = 1 meaning that the I2C bus allows speeds up to 400 kHz
 * - I2C_SLOWMODE4 = 1 meaning that the I2C bus will allow only up to 25 kHz 
 * - I2C_NOINTERRUPT4 = 1 in order to prohibit interrupts while 
 *   communicating (see below). This can be useful if you use the library 
 *   for communicationg with SMbus devices, which have timeouts.
 *   Note, however, that interrupts are disabledfrom issuing a start condition
 *   until issuing a stop condition. So use this option with care!
 * - I2C_TIMEOUT4 = 0..10000 mssec in order to return from the I2C functions
 *   in case of a I2C bus lockup (i.e., SCL constantly low). 0 means no timeout
 */

/* Changelog:
 * Version 1.1: 
 * - removed I2C_CLOCK_STRETCHING
 * - added I2C_TIMEOUT4 time in msec (0..10000) until timeout or 0 if no timeout
 * - changed i2c_init4 to return true iff both SDA and SCL are high
 * - changed interrupt disabling so that the previous IRQ state is retored
 * Version 1.0: basic functionality
 */
#include <avr/io.h>
#include <Arduino.h>

#ifndef _SOFTI2C4_H
#define _SOFTI2C4_H   1

// Init function. Needs to be called once in the beginning.
// Returns false if SDA or SCL are low, which probably means 
// a I2C bus lockup or that the lines are not pulled up.
boolean __attribute__ ((noinline)) i2c_init4(void);

// Start transfer function: <addr> is the 8-bit I2C address (including the R/W
// bit). 
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_start4(uint8_t addr); 

// Similar to start function, but wait for an ACK! Be careful, this can 
// result in an infinite loop!
void  __attribute__ ((noinline)) i2c_start_wait4(uint8_t addr);

// Repeated start function: After having claimed the bus with a start condition,
// you can address another or the same chip again without an intervening 
// stop condition.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_rep_start4(uint8_t addr);

// Issue a stop condition, freeing the bus.
void __attribute__ ((noinline)) i2c_stop4(void) asm("ass_i2c_stop4");

// Write one byte to the slave chip that had been addressed
// by the previous start call. <value> is the byte to be sent.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_write4(uint8_t value) asm("ass_i2c_write4");

// Read one byte. If <last> is true, we send a NAK after having received 
// the byte in order to terminate the read sequence. 
uint8_t __attribute__ ((noinline)) i2c_read4(bool last);

// You can set I2C_CPUFREQ4 independently of F_CPU4 if you 
// change the CPU frequency on the fly. If do not define it,
// it will use the value of F_CPU4
#ifndef I2C_CPUFREQ4
#define I2C_CPUFREQ4 F_CPU4
#endif

// If I2C_FASTMODE4 is set to 1, then the highest possible frequency below 400kHz
// is selected. Be aware that not all slave chips may be able to deal with that!
#ifndef I2C_FASTMODE4
#define I2C_FASTMODE4 0
#endif

// If I2C_FASTMODE4 is not defined or defined to be 0, then you can set
// I2C_SLOWMODE4 to 1. In this case, the I2C frequency will not be higher 
// than 25KHz. This could be useful for problematic buses.
#ifndef I2C_SLOWMODE4
#define I2C_SLOWMODE4 0
#endif

// if I2C_NOINTERRUPT4 is 1, then the I2C routines are not interruptable.
// This is most probably only necessary if you are using a 1MHz system clock,
// you are communicating with a SMBus device, and you want to avoid timeouts.
// Be aware that the interrupt bit is enabled after each call. So the
// I2C functions should not be called in interrupt routines or critical regions.
#ifndef I2C_NOINTERRUPT4
#define I2C_NOINTERRUPT4 0
#endif

// I2C_TIMEOUT4 can be set to a value between 1 and 10000.
// If it is defined and nonzero, it leads to a timeout if the
// SCL is low longer than I2C_TIMEOUT4 milliseconds, i.e., max timeout is 10 sec
#ifndef I2C_TIMEOUT4
#define I2C_TIMEOUT4 0
#else 
#if I2C_TIMEOUT4 > 10000
#error I2C_TIMEOUT4 is too large
#endif
#endif

#define I2C_TIMEOUT4_DELAY_LOOPS (I2C_CPUFREQ4/1000UL)*I2C_TIMEOUT4/4000UL
#if I2C_TIMEOUT4_DELAY_LOOPS < 1
#define I2C_MAX_STRETCH 1
#else
#if I2C_TIMEOUT4_DELAY_LOOPS > 60000UL
#define I2C_MAX_STRETCH 60000UL
#else
#define I2C_MAX_STRETCH I2C_TIMEOUT4_DELAY_LOOPS
#endif
#endif

#if I2C_FASTMODE4
#define I2C_DELAY_COUNTER4 (((I2C_CPUFREQ4/400000L)/2-19)/3)
#else
#if I2C_SLOWMODE4
#define I2C_DELAY_COUNTER4 (((I2C_CPUFREQ4/25000L)/2-19)/3)
#else
#define I2C_DELAY_COUNTER4 (((I2C_CPUFREQ4/100000L)/2-19)/3)
#endif
#endif

// Table of I2C bus speed in kbit/sec:
// CPU clock:           1MHz   2MHz    4MHz   8MHz   16MHz   20MHz
// Fast I2C mode          40     80     150    300     400     400
// Standard I2C mode      40     80     100    100     100     100
// Slow I2C mode          25     25      25     25      25      25     

// constants for reading & writing
#define I2C_READ4    1
#define I2C_WRITE4   0

// map the IO register back into the IO address space
#define SDA_DDR4       	(_SFR_IO_ADDR(SDA_PORT4) - 1)
#define SCL_DD4       	(_SFR_IO_ADDR(SCL_PORT4) - 1)
#define SDA_OUT4       	_SFR_IO_ADDR(SDA_PORT4)
#define SCL_OUT4       	_SFR_IO_ADDR(SCL_PORT4)
#define SDA_IN4		(_SFR_IO_ADDR(SDA_PORT4) - 2)
#define SCL_IN4		(_SFR_IO_ADDR(SCL_PORT4) - 2)

#ifndef __tmp_reg__
#define __tmp_reg__ 0
#endif

 
// Internal delay functions.
void __attribute__ ((noinline)) i2c_delay_half4(void) asm("ass_i2c_delay_half4");
void __attribute__ ((noinline)) i2c_wait_scl_high4(void) asm("ass_i2c_wait_scl_high4");

void  i2c_delay_half4(void)
{ // function call 3 cycles => 3C
#if I2C_DELAY_COUNTER4 < 1
  __asm__ __volatile__ (" ret");
  // 7 cycles for call and return
#else
  __asm__ __volatile__ 
    (
     " ldi      r25, %[DELAY]           ;load delay constant   ;; 4C \n\t"
     "_Lidelay4: \n\t"
     " dec r25                          ;decrement counter     ;; 4C+xC \n\t"
     " brne _Lidelay4                                           ;;5C+(x-1)2C+xC\n\t"
     " ret                                                     ;; 9C+(x-1)2C+xC = 7C+xC" 
     : : [DELAY] "M" I2C_DELAY_COUNTER4 : "r25");
  // 7 cycles + 3 times x cycles
#endif
}

void i2c_wait_scl_high4(void)
{
#if I2C_TIMEOUT4 <= 0
  __asm__ __volatile__ 
    ("_Li2c_wait_stretch: \n\t"
     " sbis	%[SCLIN],%[SCLPIN4]	;wait for SCL high \n\t" 
     " rjmp	_Li2c_wait_stretch \n\t"
     " cln                              ;signal: no timeout \n\t"
     " ret "
     : : [SCLIN] "I" (SCL_IN4), [SCLPIN4] "I" (SCL_PIN4));
#else
  __asm__ __volatile__ 
    ( " ldi     r27, %[HISTRETCH]       ;load delay counter \n\t"
      " ldi     r26, %[LOSTRETCH] \n\t"
      "_Lwait_stretch4: \n\t"
      " clr     __tmp_reg__             ;do next loop 255 times \n\t"
      "_Lwait_stretch4_inner_loop: \n\t"
      " rcall   _Lcheck_scl_level4       ;call check function   ;; 12C \n\t"
      " brpl    _Lstretch4_done          ;done if N=0           ;; +1 = 13C\n\t"
      " dec     __tmp_reg__             ;dec inner loop counter;; +1 = 14C\n\t"
      " brne    _Lwait_stretch4_inner_loop                      ;; +2 = 16C\n\t"
      " sbiw    r26,1                   ;dec outer loop counter \n\t"
      " brne    _Lwait_stretch4          ;continue with outer loop \n\t"
      " sen                             ;timeout -> set N-bit=1 \n\t"
      " rjmp _Lwait_return4              ;and return with N=1\n\t"
      "_Lstretch4_done:                  ;SCL=1 sensed \n\t"            
      " cln                             ;OK -> clear N-bit \n\t"
      " rjmp _Lwait_return4              ; and return with N=0 \n\t"

      "_Lcheck_scl_level4:                                      ;; call = 3C\n\t"
      " cln                                                    ;; +1C = 4C \n\t"
      " sbic	%[SCLIN],%[SCLPIN4]      ;skip if SCL still low ;; +2C = 6C \n\t"
      " rjmp    _Lscl_high4                                     ;; +0C = 6C \n\t"
      " sen                                                    ;; +1 = 7C\n\t "
      "_Lscl_high4: "
      " nop                                                    ;; +1C = 8C \n\t"
      " ret                             ;return N-Bit=1 if low ;; +4 = 12C\n\t"

      "_Lwait_return4:"
      : : [SCLIN] "I" (SCL_IN4), [SCLPIN4] "I" (SCL_PIN4), 
	[HISTRETCH] "M" (I2C_MAX_STRETCH>>8), 
	[LOSTRETCH] "M" (I2C_MAX_STRETCH&0xFF)
      : "r26", "r27");
#endif
}


boolean i2c_init4(void)
{
  __asm__ __volatile__ 
    (" cbi      %[SDADDR4],%[SDAPIN4]     ;release SDA \n\t" 
     " cbi      %[SCLDDR4],%[SCLPIN4]     ;release SCL \n\t" 
     " cbi      %[SDAOUT4],%[SDAPIN4]     ;clear SDA output value \n\t" 
     " cbi      %[SCLOUT4],%[SCLPIN4]     ;clear SCL output value \n\t" 
     " clr      r24                     ;set return value to false \n\t"
     " clr      r25                     ;set return value to false \n\t"
     " sbis     %[SDAIN4],%[SDAPIN4]      ;check for SDA high\n\t"
     " ret                              ;if low return with false \n\t"  
     " sbis     %[SCLIN],%[SCLPIN4]      ;check for SCL high \n\t"
     " ret                              ;if low return with false \n\t" 
     " ldi      r24,1                   ;set return value to true \n\t"
     " ret "
     : :
       [SCLDDR4] "I"  (SCL_DD4), [SCLPIN4] "I" (SCL_PIN4), 
       [SCLIN] "I" (SCL_IN4), [SCLOUT4] "I" (SCL_OUT4),
       [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4), 
       [SDAIN4] "I" (SDA_IN4), [SDAOUT4] "I" (SDA_OUT4)); 
  return true;
}

bool  i2c_start4(uint8_t addr)
{
  __asm__ __volatile__ 
    (
#if I2C_NOINTERRUPT4
     " cli                              ;clear IRQ bit \n\t"
#endif
     " sbis     %[SCLIN],%[SCLPIN4]      ;check for clock stretching slave\n\t"
     " rcall    ass_i2c_wait_scl_high4   ;wait until SCL=H\n\t" 
     " sbi      %[SDADDR4],%[SDAPIN4]     ;force SDA low  \n\t" 
     " rcall    ass_i2c_delay_half4      ;wait T/2 \n\t"
     " rcall    ass_i2c_write4           ;now write address \n\t"
     " ret"
     : : [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4),
       [SCLIN] "I" (SCL_IN4),[SCLPIN4] "I" (SCL_PIN4)); 
  return true; // we never return here!
}

bool  i2c_rep_start4(uint8_t addr)
{
  __asm__ __volatile__ 

    (
#if I2C_NOINTERRUPT4
     " cli \n\t"
#endif
     " sbi	%[SCLDDR4],%[SCLPIN4]	;force SCL low \n\t" 
     " rcall 	ass_i2c_delay_half4	;delay  T/2 \n\t" 
     " cbi	%[SDADDR4],%[SDAPIN4]	;release SDA \n\t" 
     " rcall	ass_i2c_delay_half4	;delay T/2 \n\t" 
     " cbi	%[SCLDDR4],%[SCLPIN4]	;release SCL \n\t" 
     " rcall 	ass_i2c_delay_half4	;delay  T/2 \n\t" 
     " sbis     %[SCLIN],%[SCLPIN4]      ;check for clock stretching slave\n\t"
     " rcall    ass_i2c_wait_scl_high4   ;wait until SCL=H\n\t" 
     " sbi 	%[SDADDR4],%[SDAPIN4]	;force SDA low \n\t" 
     " rcall 	ass_i2c_delay_half4	;delay	T/2 \n\t" 
     " rcall    ass_i2c_write4       \n\t"
     " ret"
     : : [SCLDDR4] "I"  (SCL_DD4), [SCLPIN4] "I" (SCL_PIN4),[SCLIN] "I" (SCL_IN4),
         [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4)); 
  return true; // just to fool the compiler
}

void  i2c_start_wait4(uint8_t addr)
{
 __asm__ __volatile__ 
   (
    " push	r24                     ;save original parameter \n\t"
    "_Li2c_start_wait4: \n\t"
    " pop       r24                     ;restore original parameter\n\t"
    " push      r24                     ;and save again \n\t"
#if I2C_NOINTERRUPT4
    " cli                               ;disable interrupts \n\t"
#endif
    " sbis     %[SCLIN],%[SCLPIN4]      ;check for clock stretching slave\n\t"
    " rcall    ass_i2c_wait_scl_high4   ;wait until SCL=H\n\t" 
    " sbi 	%[SDADDR4],%[SDAPIN4]	;force SDA low \n\t" 
    " rcall 	ass_i2c_delay_half4	;delay T/2 \n\t" 
    " rcall 	ass_i2c_write4	        ;write address \n\t" 
    " tst	r24		        ;if device not busy -> done \n\t" 
    " brne	_Li2c_start_wait4_done \n\t" 
    " rcall	ass_i2c_stop4	        ;terminate write & enable IRQ \n\t" 
    " rjmp	_Li2c_start_wait1	;device busy, poll ack again \n\t" 
    "_Li2c_start_wait4_done: \n\t"
    " pop       __tmp_reg__             ;pop off orig argument \n\t"
    " ret "
    : : [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4),
      [SCLIN] "I" (SCL_IN4),[SCLPIN4] "I" (SCL_PIN4)); 
}

void  i2c_stop4(void)
{
  __asm__ __volatile__ 
    (
     " sbi      %[SCLDDR4],%[SCLPIN4]     ;force SCL low \n\t" 
     " sbi      %[SDADDR4],%[SDAPIN4]     ;force SDA low \n\t" 
     " rcall    ass_i2c_delay_half4      ;T/2 delay \n\t"
     " cbi      %[SCLDDR4],%[SCLPIN4]     ;release SCL \n\t" 
     " rcall    ass_i2c_delay_half4      ;T/2 delay \n\t"
     " sbis     %[SCLIN],%[SCLPIN4]      ;check for clock stretching slave\n\t"
     " rcall    ass_i2c_wait_scl_high4   ;wait until SCL=H\n\t" 
     " cbi      %[SDADDR4],%[SDAPIN4]     ;release SDA \n\t" 
     " rcall    ass_i2c_delay_half4 \n\t"
#if I2C_NOINTERRUPT4
     " sei                              ;enable interrupts again!\n\t"
#endif
     : : [SCLDDR4] "I"  (SCL_DD4), [SCLPIN4] "I" (SCL_PIN4), [SCLIN] "I" (SCL_IN4),
         [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4)); 
}

bool i2c_write4(uint8_t value)
{
  __asm__ __volatile__ 
    (
     " sec                              ;set carry flag \n\t"
     " rol      r24                     ;shift in carry and shift out MSB \n\t"
     " rjmp _Li2c_write4_first \n\t"
     "_Li2c_write4_bit:\n\t"
     " lsl      r24                     ;left shift into carry ;; 1C\n\t"
     "_Li2c_write4_first:\n\t"
     " breq     _Li2c_get_ack4           ;jump if TXreg is empty;; +1 = 2C \n\t"
     " sbi      %[SCLDDR4],%[SCLPIN4]     ;force SCL low         ;; +2 = 4C \n\t"
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
     " brcc     _Li2c_write4_low                                ;;+1/+2=5/6C\n\t"
     " nop                                                     ;; +1 = 7C \n\t"
     " cbi %[SDADDR4],%[SDAPIN4]	        ;release SDA           ;; +2 = 9C \n\t"
     " rjmp      _Li2c_write4_high                              ;; +2 = 11C \n\t"
     "_Li2c_write4_low: \n\t"
     " sbi	%[SDADDR4],%[SDAPIN4]	;force SDA low         ;; +2 = 9C \n\t"
     " rjmp	_Li2c_write4_high                               ;;+2 = 11C \n\t"
     "_Li2c_write4_high: \n\t"
#if I2C_DELAY_COUNTER4 >= 1
     " rcall 	ass_i2c_delay_half4	;delay T/2             ;;+X = 11C+X\n\t"
#endif
     " cbi	%[SCLDDR4],%[SCLPIN4]	;release SCL           ;;+2 = 13C+X\n\t"
     " cln                              ;clear N-bit           ;;+1 = 14C+X\n\t"
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
     " sbis	%[SCLIN],%[SCLPIN4]	;check for SCL high    ;;+2 = 16C+X\n\t"
     " rcall    ass_i2c_wait_scl_high4 \n\t"
     " brpl     _Ldelay_scl_high4                              ;;+2 = 18C+X\n\t"
     "_Li2c_write4_return_false: \n\t"
     " clr      r24                     ; return false because of timeout \n\t"
     " rjmp     _Li2c_write4_return \n\t"
     "_Ldelay_scl_high4: \n\t"
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2             ;;+X= 18C+2X\n\t"
#endif
     " rjmp	_Li2c_write4_bit \n\t"
     "              ;; +2 = 20C +2X for one bit-loop \n\t"
     "_Li2c_get_ack4: \n\t"
     " sbi	%[SCLDDR4],%[SCLPIN4]	;force SCL low ;; +2 = 5C \n\t"
     " nop \n\t"
     " nop \n\t"
     " cbi	%[SDADDR4],%[SDAPIN4]	;release SDA ;;+2 = 7C \n\t"
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2 ;; +X = 7C+X \n\t"
#endif
     " clr	r25                                            ;; 17C+2X \n\t"
     " clr	r24		        ;return 0              ;; 14C + X \n\t"
     " cbi	%[SCLDDR4],%[SCLPIN4]	;release SCL ;; +2 = 9C+X\n\t"
     "_Li2c_ack_wait4: \n\t"
     " cln                              ; clear N-bit          ;; 10C + X\n\t" 
     " nop \n\t"
     " sbis	%[SCLIN],%[SCLPIN4]	;wait SCL high         ;; 12C + X \n\t"
     " rcall    ass_i2c_wait_scl_high4 \n\t"
     " brmi     _Li2c_write4_return_false                       ;; 13C + X \n\t "
     " sbis	%[SDAIN4],%[SDAPIN4]      ;if SDA hi -> return 0 ;; 15C + X \n\t"
     " ldi	r24,1                   ;return true           ;; 16C + X \n\t"
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2             ;; 16C + 2X \n\t"
#endif
     "_Li2c_write4_return: \n\t"
     " nop \n\t "
     " nop \n\t "
     " sbi	%[SCLDDR4],%[SCLPIN4]	;force SCL low so SCL=H is short\n\t"
     " ret \n\t"
     "              ;; + 4 = 17C + 2X for acknowldge bit"
     ::
      [SCLDDR4] "I"  (SCL_DD4), [SCLPIN4] "I" (SCL_PIN4), [SCLIN] "I" (SCL_IN4),
      [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4), [SDAIN4] "I" (SDA_IN4)); 
  return true; // fooling the compiler
}

uint8_t i2c_read4(bool last)
{
  __asm__ __volatile__ 
    (
     " ldi	r23,0x01 \n\t"
     "_Li2c_read4_bit: \n\t"
     " sbi	%[SCLDDR4],%[SCLPIN4]	;force SCL low         ;; 2C \n\t" 
     " cbi	%[SDADDR4],%[SDAPIN4]	;release SDA(prev. ACK);; 4C \n\t" 
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2             ;; 4C+X \n\t" 
#endif
     " cbi	%[SCLDDR4],%[SCLPIN4]	;release SCL           ;; 6C + X \n\t" 
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2             ;; 6C + 2X \n\t" 
#endif
     " cln                              ; clear N-bit          ;; 7C + 2X \n\t"
     " nop \n\t "
     " nop \n\t "
     " nop \n\t "
     " sbis     %[SCLIN], %[SCLPIN4]     ;check for SCL high    ;; 9C +2X \n\t" 
     " rcall    ass_i2c_wait_scl_high4 \n\t"
     " brmi     _Li2c_read4_return       ;return if timeout     ;; 10C + 2X\n\t"
     " clc		  	        ;clear carry flag      ;; 11C + 2X\n\t" 
     " sbic	%[SDAIN4],%[SDAPIN4]	;if SDA is high        ;; 11C + 2X\n\t" 
     " sec			        ;set carry flag        ;; 12C + 2X\n\t" 
     " rol	r23		        ;store bit             ;; 13C + 2X\n\t" 
     " brcc	_Li2c_read4_bit	        ;while receiv reg not full \n\t"
     "                         ;; 15C + 2X for one bit loop \n\t" 
     
     "_Li2c_put_ack4: \n\t" 
     " sbi	%[SCLDDR4],%[SCLPIN4]	;force SCL low         ;; 2C \n\t" 
     " cpi	r24,0                                          ;; 3C \n\t" 
     " breq	_Li2c_put_ack4_low	;if (ack=0) ;; 5C \n\t" 
     " cbi	%[SDADDR4],%[SDAPIN4]	;release SDA \n\t" 
     " rjmp	_Li2c_put_ack4_high \n\t" 
     "_Li2c_put_ack4_low:                ;else \n\t" 
     " sbi	%[SDADDR4],%[SDAPIN4]	;force SDA low         ;; 7C \n\t" 
     "_Li2c_put_ack4_high: \n\t" 
     " nop \n\t "
     " nop \n\t "
     " nop \n\t "
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2             ;; 7C + X \n\t" 
#endif
     " cbi	%[SCLDDR4],%[SCLPIN4]	;release SCL           ;; 9C +X \n\t" 
     " cln                              ;clear N               ;; +1 = 10C\n\t"
     " nop \n\t "
     " nop \n\t "
     " sbis	%[SCLIN],%[SCLPIN4]	;wait SCL high         ;; 12C + X\n\t" 
     " rcall    ass_i2c_wait_scl_high4 \n\t"
#if I2C_DELAY_COUNTER4 >= 1
     " rcall	ass_i2c_delay_half4	;delay T/2             ;; 11C + 2X\n\t" 
#endif
     "_Li2c_read4_return: \n\t"
     " nop \n\t "
     " nop \n\t "
     "sbi	%[SCLDDR4],%[SCLPIN4]	;force SCL low so SCL=H is short\n\t"
     " mov	r24,r23                                        ;; 12C + 2X \n\t"
     " clr	r25                                            ;; 13 C + 2X\n\t"
     " ret                                                     ;; 17C + X"
     ::
      [SCLDDR4] "I"  (SCL_DD4), [SCLPIN4] "I" (SCL_PIN4), [SCLIN] "I" (SCL_IN4),
      [SDADDR4] "I"  (SDA_DDR4), [SDAPIN4] "I" (SDA_PIN4), [SDAIN4] "I" (SDA_IN4) 
     ); 
  return ' '; // fool the compiler!
}

#endif



