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

/* In order to use the library, you need to define SDA_PIN1, SCL_PIN1,
 * SDA_PORT1 and SCL_PORT1 before including this file.  Have a look at
 * http://www.arduino.cc/en/Reference/PortManipulation for finding out
 * which values to use. For example, if you use digital pin 3 for
 * SDA and digital pin 13 for SCL you have to use the following
 * definitions: 
 * #define SDA_PIN1 3 
 * #define SDA_PORT1 PORTB 
 * #define SCL_PIN1 5
 * #define SCL_PORT1 PORTB
 *
 * You can also define the following constants (see also below):
 * - I2C_CPUFREQ1, when changing CPU clock frequency dynamically
 * - I2C_FASTMODE1 = 1 meaning that the I2C bus allows speeds up to 400 kHz
 * - I2C_SLOWMODE1 = 1 meaning that the I2C bus will allow only up to 25 kHz 
 * - I2C_NOINTERRUPT1 = 1 in order to prohibit interrupts while 
 *   communicating (see below). This can be useful if you use the library 
 *   for communicationg with SMbus devices, which have timeouts.
 *   Note, however, that interrupts are disabledfrom issuing a start condition
 *   until issuing a stop condition. So use this option with care!
 * - I2C_TIMEOUT1 = 0..10000 mssec in order to return from the I2C functions
 *   in case of a I2C bus lockup (i.e., SCL constantly low). 0 means no timeout
 */

/* Changelog:
 * Version 1.1: 
 * - removed I2C_CLOCK_STRETCHING
 * - added I2C_TIMEOUT1 time in msec (0..10000) until timeout or 0 if no timeout
 * - changed i2c_init1 to return true iff both SDA and SCL are high
 * - changed interrupt disabling so that the previous IRQ state is retored
 * Version 1.0: basic functionality
 */
#include <avr/io.h>
#include <Arduino.h>

#ifndef _SOFTI2C1_H
#define _SOFTI2C1_H   1

// Init function. Needs to be called once in the beginning.
// Returns false if SDA or SCL are low, which probably means 
// a I2C bus lockup or that the lines are not pulled up.
boolean __attribute__ ((noinline)) i2c_init1(void);

// Start transfer function: <addr> is the 8-bit I2C address (including the R/W
// bit). 
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_start1(uint8_t addr); 

// Similar to start function, but wait for an ACK! Be careful, this can 
// result in an infinite loop!
void  __attribute__ ((noinline)) i2c_start_wait1(uint8_t addr);

// Repeated start function: After having claimed the bus with a start condition,
// you can address another or the same chip again without an intervening 
// stop condition.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_rep_start1(uint8_t addr);

// Issue a stop condition, freeing the bus.
void __attribute__ ((noinline)) i2c_stop1(void) asm("ass_i2c_stop1");

// Write one byte to the slave chip that had been addressed
// by the previous start call. <value> is the byte to be sent.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_write1(uint8_t value) asm("ass_i2c_write1");

// Read one byte. If <last> is true, we send a NAK after having received 
// the byte in order to terminate the read sequence. 
uint8_t __attribute__ ((noinline)) i2c_read1(bool last);

// You can set I2C_CPUFREQ1 independently of F_CPU1 if you 
// change the CPU frequency on the fly. If do not define it,
// it will use the value of F_CPU1
#ifndef I2C_CPUFREQ1
#define I2C_CPUFREQ1 F_CPU1
#endif

// If I2C_FASTMODE1 is set to 1, then the highest possible frequency below 400kHz
// is selected. Be aware that not all slave chips may be able to deal with that!
#ifndef I2C_FASTMODE1
#define I2C_FASTMODE1 0
#endif

// If I2C_FASTMODE1 is not defined or defined to be 0, then you can set
// I2C_SLOWMODE1 to 1. In this case, the I2C frequency will not be higher 
// than 25KHz. This could be useful for problematic buses.
#ifndef I2C_SLOWMODE1
#define I2C_SLOWMODE1 0
#endif

// if I2C_NOINTERRUPT1 is 1, then the I2C routines are not interruptable.
// This is most probably only necessary if you are using a 1MHz system clock,
// you are communicating with a SMBus device, and you want to avoid timeouts.
// Be aware that the interrupt bit is enabled after each call. So the
// I2C functions should not be called in interrupt routines or critical regions.
#ifndef I2C_NOINTERRUPT1
#define I2C_NOINTERRUPT1 0
#endif

// I2C_TIMEOUT1 can be set to a value between 1 and 10000.
// If it is defined and nonzero, it leads to a timeout if the
// SCL is low longer than I2C_TIMEOUT1 milliseconds, i.e., max timeout is 10 sec
#ifndef I2C_TIMEOUT1
#define I2C_TIMEOUT1 0
#else 
#if I2C_TIMEOUT1 > 10000
#error I2C_TIMEOUT1 is too large
#endif
#endif

#define I2C_TIMEOUT1_DELAY_LOOPS (I2C_CPUFREQ1/1000UL)*I2C_TIMEOUT1/4000UL
#if I2C_TIMEOUT1_DELAY_LOOPS < 1
#define I2C_MAX_STRETCH 1
#else
#if I2C_TIMEOUT1_DELAY_LOOPS > 60000UL
#define I2C_MAX_STRETCH 60000UL
#else
#define I2C_MAX_STRETCH I2C_TIMEOUT1_DELAY_LOOPS
#endif
#endif

#if I2C_FASTMODE1
#define I2C_DELAY_COUNTER1 (((I2C_CPUFREQ1/400000L)/2-19)/3)
#else
#if I2C_SLOWMODE1
#define I2C_DELAY_COUNTER1 (((I2C_CPUFREQ1/25000L)/2-19)/3)
#else
#define I2C_DELAY_COUNTER1 (((I2C_CPUFREQ1/100000L)/2-19)/3)
#endif
#endif

// Table of I2C bus speed in kbit/sec:
// CPU clock:           1MHz   2MHz    4MHz   8MHz   16MHz   20MHz
// Fast I2C mode          40     80     150    300     400     400
// Standard I2C mode      40     80     100    100     100     100
// Slow I2C mode          25     25      25     25      25      25     

// constants for reading & writing
#define I2C_READ1    1
#define I2C_WRITE1   0

// map the IO register back into the IO address space
#define SDA_DDR1       	(_SFR_IO_ADDR(SDA_PORT1) - 1)
#define SCL_DDR1       	(_SFR_IO_ADDR(SCL_PORT1) - 1)
#define SDA_OUT1       	_SFR_IO_ADDR(SDA_PORT1)
#define SCL_OUT1       	_SFR_IO_ADDR(SCL_PORT1)
#define SDA_IN1		(_SFR_IO_ADDR(SDA_PORT1) - 2)
#define SCL_IN1		(_SFR_IO_ADDR(SCL_PORT1) - 2)

#ifndef __tmp_reg__
#define __tmp_reg__ 0
#endif

 
// Internal delay functions.
void __attribute__ ((noinline)) i2c_delay_half1(void) asm("ass_i2c_delay_half1");
void __attribute__ ((noinline)) i2c_wait_scl_high1(void) asm("ass_i2c_wait_scl_high1");

void  i2c_delay_half1(void)
{ // function call 3 cycles => 3C
#if I2C_DELAY_COUNTER1 < 1
  __asm__ __volatile__ (" ret");
  // 7 cycles for call and return
#else
  __asm__ __volatile__ 
    (
     " ldi      r25, %[DELAY]           ;load delay constant   ;; 4C \n\t"
     "_Lidelay1: \n\t"
     " dec r25                          ;decrement counter     ;; 4C+xC \n\t"
     " brne _Lidelay1                                           ;;5C+(x-1)2C+xC\n\t"
     " ret                                                     ;; 9C+(x-1)2C+xC = 7C+xC" 
     : : [DELAY] "M" I2C_DELAY_COUNTER1 : "r25");
  // 7 cycles + 3 times x cycles
#endif
}

void i2c_wait_scl_high1(void)
{
#if I2C_TIMEOUT1 <= 0
  __asm__ __volatile__ 
    ("_Li2c_wait_stretch: \n\t"
     " sbis	%[SCLIN],%[SCLPIN1]	;wait for SCL high \n\t" 
     " rjmp	_Li2c_wait_stretch \n\t"
     " cln                              ;signal: no timeout \n\t"
     " ret "
     : : [SCLIN] "I" (SCL_IN1), [SCLPIN1] "I" (SCL_PIN1));
#else
  __asm__ __volatile__ 
    ( " ldi     r27, %[HISTRETCH]       ;load delay counter \n\t"
      " ldi     r26, %[LOSTRETCH] \n\t"
      "_Lwait_stretch1: \n\t"
      " clr     __tmp_reg__             ;do next loop 255 times \n\t"
      "_Lwait_stretch1_inner_loop: \n\t"
      " rcall   _Lcheck_scl_level1       ;call check function   ;; 12C \n\t"
      " brpl    _Lstretch1_done          ;done if N=0           ;; +1 = 13C\n\t"
      " dec     __tmp_reg__             ;dec inner loop counter;; +1 = 14C\n\t"
      " brne    _Lwait_stretch1_inner_loop                      ;; +2 = 16C\n\t"
      " sbiw    r26,1                   ;dec outer loop counter \n\t"
      " brne    _Lwait_stretch1          ;continue with outer loop \n\t"
      " sen                             ;timeout -> set N-bit=1 \n\t"
      " rjmp _Lwait_return1              ;and return with N=1\n\t"
      "_Lstretch1_done:                  ;SCL=1 sensed \n\t"            
      " cln                             ;OK -> clear N-bit \n\t"
      " rjmp _Lwait_return1              ; and return with N=0 \n\t"

      "_Lcheck_scl_level1:                                      ;; call = 3C\n\t"
      " cln                                                    ;; +1C = 4C \n\t"
      " sbic	%[SCLIN],%[SCLPIN1]      ;skip if SCL still low ;; +2C = 6C \n\t"
      " rjmp    _Lscl_high1                                     ;; +0C = 6C \n\t"
      " sen                                                    ;; +1 = 7C\n\t "
      "_Lscl_high1: "
      " nop                                                    ;; +1C = 8C \n\t"
      " ret                             ;return N-Bit=1 if low ;; +4 = 12C\n\t"

      "_Lwait_return1:"
      : : [SCLIN] "I" (SCL_IN1), [SCLPIN1] "I" (SCL_PIN1), 
	[HISTRETCH] "M" (I2C_MAX_STRETCH>>8), 
	[LOSTRETCH] "M" (I2C_MAX_STRETCH&0xFF)
      : "r26", "r27");
#endif
}


boolean i2c_init1(void)
{
  __asm__ __volatile__ 
    (" cbi      %[SDADDR1],%[SDAPIN1]     ;release SDA \n\t" 
     " cbi      %[SCLDDR1],%[SCLPIN1]     ;release SCL \n\t" 
     " cbi      %[SDAOUT1],%[SDAPIN1]     ;clear SDA output value \n\t" 
     " cbi      %[SCLOUT1],%[SCLPIN1]     ;clear SCL output value \n\t" 
     " clr      r24                     ;set return value to false \n\t"
     " clr      r25                     ;set return value to false \n\t"
     " sbis     %[SDAIN1],%[SDAPIN1]      ;check for SDA high\n\t"
     " ret                              ;if low return with false \n\t"  
     " sbis     %[SCLIN],%[SCLPIN1]      ;check for SCL high \n\t"
     " ret                              ;if low return with false \n\t" 
     " ldi      r24,1                   ;set return value to true \n\t"
     " ret "
     : :
       [SCLDDR1] "I"  (SCL_DDR1), [SCLPIN1] "I" (SCL_PIN1), 
       [SCLIN] "I" (SCL_IN1), [SCLOUT1] "I" (SCL_OUT1),
       [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1), 
       [SDAIN1] "I" (SDA_IN1), [SDAOUT1] "I" (SDA_OUT1)); 
  return true;
}

bool  i2c_start1(uint8_t addr)
{
  __asm__ __volatile__ 
    (
#if I2C_NOINTERRUPT1
     " cli                              ;clear IRQ bit \n\t"
#endif
     " sbis     %[SCLIN],%[SCLPIN1]      ;check for clock stretching slave\n\t"
     " rcall    ass_i2c_wait_scl_high1   ;wait until SCL=H\n\t" 
     " sbi      %[SDADDR1],%[SDAPIN1]     ;force SDA low  \n\t" 
     " rcall    ass_i2c_delay_half1      ;wait T/2 \n\t"
     " rcall    ass_i2c_write1           ;now write address \n\t"
     " ret"
     : : [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1),
       [SCLIN] "I" (SCL_IN1),[SCLPIN1] "I" (SCL_PIN1)); 
  return true; // we never return here!
}

bool  i2c_rep_start1(uint8_t addr)
{
  __asm__ __volatile__ 

    (
#if I2C_NOINTERRUPT1
     " cli \n\t"
#endif
     " sbi	%[SCLDDR1],%[SCLPIN1]	;force SCL low \n\t" 
     " rcall 	ass_i2c_delay_half1	;delay  T/2 \n\t" 
     " cbi	%[SDADDR1],%[SDAPIN1]	;release SDA \n\t" 
     " rcall	ass_i2c_delay_half1	;delay T/2 \n\t" 
     " cbi	%[SCLDDR1],%[SCLPIN1]	;release SCL \n\t" 
     " rcall 	ass_i2c_delay_half1	;delay  T/2 \n\t" 
     " sbis     %[SCLIN],%[SCLPIN1]      ;check for clock stretching slave\n\t"
     " rcall    ass_i2c_wait_scl_high1   ;wait until SCL=H\n\t" 
     " sbi 	%[SDADDR1],%[SDAPIN1]	;force SDA low \n\t" 
     " rcall 	ass_i2c_delay_half1	;delay	T/2 \n\t" 
     " rcall    ass_i2c_write1       \n\t"
     " ret"
     : : [SCLDDR1] "I"  (SCL_DDR1), [SCLPIN1] "I" (SCL_PIN1),[SCLIN] "I" (SCL_IN1),
         [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1)); 
  return true; // just to fool the compiler
}

void  i2c_start_wait1(uint8_t addr)
{
 __asm__ __volatile__ 
   (
    " push	r24                     ;save original parameter \n\t"
    "_Li2c_start_wait1: \n\t"
    " pop       r24                     ;restore original parameter\n\t"
    " push      r24                     ;and save again \n\t"
#if I2C_NOINTERRUPT1
    " cli                               ;disable interrupts \n\t"
#endif
    " sbis     %[SCLIN],%[SCLPIN1]      ;check for clock stretching slave\n\t"
    " rcall    ass_i2c_wait_scl_high1   ;wait until SCL=H\n\t" 
    " sbi 	%[SDADDR1],%[SDAPIN1]	;force SDA low \n\t" 
    " rcall 	ass_i2c_delay_half1	;delay T/2 \n\t" 
    " rcall 	ass_i2c_write1	        ;write address \n\t" 
    " tst	r24		        ;if device not busy -> done \n\t" 
    " brne	_Li2c_start_wait1_done \n\t" 
    " rcall	ass_i2c_stop1	        ;terminate write & enable IRQ \n\t" 
    " rjmp	_Li2c_start_wait1	;device busy, poll ack again \n\t" 
    "_Li2c_start_wait1_done: \n\t"
    " pop       __tmp_reg__             ;pop off orig argument \n\t"
    " ret "
    : : [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1),
      [SCLIN] "I" (SCL_IN1),[SCLPIN1] "I" (SCL_PIN1)); 
}

void  i2c_stop1(void)
{
  __asm__ __volatile__ 
    (
     " sbi      %[SCLDDR1],%[SCLPIN1]     ;force SCL low \n\t" 
     " sbi      %[SDADDR1],%[SDAPIN1]     ;force SDA low \n\t" 
     " rcall    ass_i2c_delay_half1      ;T/2 delay \n\t"
     " cbi      %[SCLDDR1],%[SCLPIN1]     ;release SCL \n\t" 
     " rcall    ass_i2c_delay_half1      ;T/2 delay \n\t"
     " sbis     %[SCLIN],%[SCLPIN1]      ;check for clock stretching slave\n\t"
     " rcall    ass_i2c_wait_scl_high1   ;wait until SCL=H\n\t" 
     " cbi      %[SDADDR1],%[SDAPIN1]     ;release SDA \n\t" 
     " rcall    ass_i2c_delay_half1 \n\t"
#if I2C_NOINTERRUPT1
     " sei                              ;enable interrupts again!\n\t"
#endif
     : : [SCLDDR1] "I"  (SCL_DDR1), [SCLPIN1] "I" (SCL_PIN1), [SCLIN] "I" (SCL_IN1),
         [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1)); 
}

bool i2c_write1(uint8_t value)
{
  __asm__ __volatile__ 
    (
     " sec                              ;set carry flag \n\t"
     " rol      r24                     ;shift in carry and shift out MSB \n\t"
     " rjmp _Li2c_write1_first \n\t"
     "_Li2c_write1_bit:\n\t"
     " lsl      r24                     ;left shift into carry ;; 1C\n\t"
     "_Li2c_write1_first:\n\t"
     " breq     _Li2c_get_ack1           ;jump if TXreg is empty;; +1 = 2C \n\t"
     " sbi      %[SCLDDR1],%[SCLPIN1]     ;force SCL low         ;; +2 = 4C \n\t"
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
     " brcc     _Li2c_write1_low                                ;;+1/+2=5/6C\n\t"
     " nop                                                     ;; +1 = 7C \n\t"
     " cbi %[SDADDR1],%[SDAPIN1]	        ;release SDA           ;; +2 = 9C \n\t"
     " rjmp      _Li2c_write1_high                              ;; +2 = 11C \n\t"
     "_Li2c_write1_low: \n\t"
     " sbi	%[SDADDR1],%[SDAPIN1]	;force SDA low         ;; +2 = 9C \n\t"
     " rjmp	_Li2c_write1_high                               ;;+2 = 11C \n\t"
     "_Li2c_write1_high: \n\t"
#if I2C_DELAY_COUNTER1 >= 1
     " rcall 	ass_i2c_delay_half1	;delay T/2             ;;+X = 11C+X\n\t"
#endif
     " cbi	%[SCLDDR1],%[SCLPIN1]	;release SCL           ;;+2 = 13C+X\n\t"
     " cln                              ;clear N-bit           ;;+1 = 14C+X\n\t"
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
     " sbis	%[SCLIN],%[SCLPIN1]	;check for SCL high    ;;+2 = 16C+X\n\t"
     " rcall    ass_i2c_wait_scl_high1 \n\t"
     " brpl     _Ldelay_scl_high1                              ;;+2 = 18C+X\n\t"
     "_Li2c_write1_return_false: \n\t"
     " clr      r24                     ; return false because of timeout \n\t"
     " rjmp     _Li2c_write1_return \n\t"
     "_Ldelay_scl_high1: \n\t"
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2             ;;+X= 18C+2X\n\t"
#endif
     " rjmp	_Li2c_write1_bit \n\t"
     "              ;; +2 = 20C +2X for one bit-loop \n\t"
     "_Li2c_get_ack1: \n\t"
     " sbi	%[SCLDDR1],%[SCLPIN1]	;force SCL low ;; +2 = 5C \n\t"
     " nop \n\t"
     " nop \n\t"
     " cbi	%[SDADDR1],%[SDAPIN1]	;release SDA ;;+2 = 7C \n\t"
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2 ;; +X = 7C+X \n\t"
#endif
     " clr	r25                                            ;; 17C+2X \n\t"
     " clr	r24		        ;return 0              ;; 14C + X \n\t"
     " cbi	%[SCLDDR1],%[SCLPIN1]	;release SCL ;; +2 = 9C+X\n\t"
     "_Li2c_ack_wait1: \n\t"
     " cln                              ; clear N-bit          ;; 10C + X\n\t" 
     " nop \n\t"
     " sbis	%[SCLIN],%[SCLPIN1]	;wait SCL high         ;; 12C + X \n\t"
     " rcall    ass_i2c_wait_scl_high1 \n\t"
     " brmi     _Li2c_write1_return_false                       ;; 13C + X \n\t "
     " sbis	%[SDAIN1],%[SDAPIN1]      ;if SDA hi -> return 0 ;; 15C + X \n\t"
     " ldi	r24,1                   ;return true           ;; 16C + X \n\t"
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2             ;; 16C + 2X \n\t"
#endif
     "_Li2c_write1_return: \n\t"
     " nop \n\t "
     " nop \n\t "
     " sbi	%[SCLDDR1],%[SCLPIN1]	;force SCL low so SCL=H is short\n\t"
     " ret \n\t"
     "              ;; + 4 = 17C + 2X for acknowldge bit"
     ::
      [SCLDDR1] "I"  (SCL_DDR1), [SCLPIN1] "I" (SCL_PIN1), [SCLIN] "I" (SCL_IN1),
      [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1), [SDAIN1] "I" (SDA_IN1)); 
  return true; // fooling the compiler
}

uint8_t i2c_read1(bool last)
{
  __asm__ __volatile__ 
    (
     " ldi	r23,0x01 \n\t"
     "_Li2c_read1_bit: \n\t"
     " sbi	%[SCLDDR1],%[SCLPIN1]	;force SCL low         ;; 2C \n\t" 
     " cbi	%[SDADDR1],%[SDAPIN1]	;release SDA(prev. ACK);; 4C \n\t" 
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2             ;; 4C+X \n\t" 
#endif
     " cbi	%[SCLDDR1],%[SCLPIN1]	;release SCL           ;; 6C + X \n\t" 
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2             ;; 6C + 2X \n\t" 
#endif
     " cln                              ; clear N-bit          ;; 7C + 2X \n\t"
     " nop \n\t "
     " nop \n\t "
     " nop \n\t "
     " sbis     %[SCLIN], %[SCLPIN1]     ;check for SCL high    ;; 9C +2X \n\t" 
     " rcall    ass_i2c_wait_scl_high1 \n\t"
     " brmi     _Li2c_read1_return       ;return if timeout     ;; 10C + 2X\n\t"
     " clc		  	        ;clear carry flag      ;; 11C + 2X\n\t" 
     " sbic	%[SDAIN1],%[SDAPIN1]	;if SDA is high        ;; 11C + 2X\n\t" 
     " sec			        ;set carry flag        ;; 12C + 2X\n\t" 
     " rol	r23		        ;store bit             ;; 13C + 2X\n\t" 
     " brcc	_Li2c_read1_bit	        ;while receiv reg not full \n\t"
     "                         ;; 15C + 2X for one bit loop \n\t" 
     
     "_Li2c_put_ack1: \n\t" 
     " sbi	%[SCLDDR1],%[SCLPIN1]	;force SCL low         ;; 2C \n\t" 
     " cpi	r24,0                                          ;; 3C \n\t" 
     " breq	_Li2c_put_ack1_low	;if (ack=0) ;; 5C \n\t" 
     " cbi	%[SDADDR1],%[SDAPIN1]	;release SDA \n\t" 
     " rjmp	_Li2c_put_ack1_high \n\t" 
     "_Li2c_put_ack1_low:                ;else \n\t" 
     " sbi	%[SDADDR1],%[SDAPIN1]	;force SDA low         ;; 7C \n\t" 
     "_Li2c_put_ack1_high: \n\t" 
     " nop \n\t "
     " nop \n\t "
     " nop \n\t "
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2             ;; 7C + X \n\t" 
#endif
     " cbi	%[SCLDDR1],%[SCLPIN1]	;release SCL           ;; 9C +X \n\t" 
     " cln                              ;clear N               ;; +1 = 10C\n\t"
     " nop \n\t "
     " nop \n\t "
     " sbis	%[SCLIN],%[SCLPIN1]	;wait SCL high         ;; 12C + X\n\t" 
     " rcall    ass_i2c_wait_scl_high1 \n\t"
#if I2C_DELAY_COUNTER1 >= 1
     " rcall	ass_i2c_delay_half1	;delay T/2             ;; 11C + 2X\n\t" 
#endif
     "_Li2c_read1_return: \n\t"
     " nop \n\t "
     " nop \n\t "
     "sbi	%[SCLDDR1],%[SCLPIN1]	;force SCL low so SCL=H is short\n\t"
     " mov	r24,r23                                        ;; 12C + 2X \n\t"
     " clr	r25                                            ;; 13 C + 2X\n\t"
     " ret                                                     ;; 17C + X"
     ::
      [SCLDDR1] "I"  (SCL_DDR1), [SCLPIN1] "I" (SCL_PIN1), [SCLIN] "I" (SCL_IN1),
      [SDADDR1] "I"  (SDA_DDR1), [SDAPIN1] "I" (SDA_PIN1), [SDAIN1] "I" (SDA_IN1) 
     ); 
  return ' '; // fool the compiler!
}

#endif



