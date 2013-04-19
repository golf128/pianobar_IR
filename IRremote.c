#include "IRremote.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "wiringPi.h"
////////////////////////////////////////////////////////////////////////
struct IRrecv
{
  uint8_t recvpin;           // pin for IR data from detector
  uint8_t rcvstate;          // state machine
  unsigned int timer;     // state timer, counts 50uS ticks.
  unsigned int rawbuf[RAWBUF]; // raw data
  uint8_t rawlen;         // counter of entries in rawbuf
} ;
struct IRrecv remote;
////////////////////////////////////////////////////////////////////////
struct decode_results {
  int decode_type; // NEC, SONY, RC5, UNKNOWN
  int panasonicAddress; // This is only used for decoding Panasonic data
  uint32_t value; // Decoded value
  int bits; // Number of bits in decoded value
  int *rawbuf; // Raw intervals in .5 us ticks
  int rawlen; // Number of records in rawbuf.
};
struct decode_results results;
// initialization
void enableIRIn(int recvpin) {
  remote.rcvstate = STATE_IDLE;
  remote.rawlen = 0;
  remote.recvpin = recvpin;
  // set pin modes
  piThreadCreate (timeThread) ;
  pinMode(recvpin, INPUT);
}
uint32_t get_result(){
return results.value ;

}
int MATCH_MARK(int measured_ticks, int desired_us) {
  return measured_ticks >= TICKS_LOW(desired_us + MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us + MARK_EXCESS);
}


static PI_THREAD (timerThread)
{
// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
  piHiPri (50) ;
  for(;;)
{
  delayMicroseconds(50);
  uint8_t irdata = (uint8_t)digitalRead(remote.recvpin);
  remote.timer++; // One more 50us tick
  if (remote.rawlen >= RAWBUF)
  {
    // Buffer overflow
    remote.rcvstate = STATE_STOP;
  }
  switch(remote.rcvstate)
  {
	case STATE_IDLE: // In the middle of a gap
		if (irdata == MARK)
		{
		  if (remote.timer < GAP_TICKS)
		  {
			// Not big enough to be a gap.
			remote.timer = 0;
		  }
		  else
		  {
			// gap just ended, record duration and start recording transmission
			remote.rawlen = 0;
			remote.rawbuf[remote->rawlen++] = remote->timer;
			remote.timer = 0;
			remote.rcvstate = STATE_MARK;
		  }
		}
		break;
	  case STATE_MARK: // timing MARK
		if (irdata == SPACE)
		{   // MARK ended, record time
		  remote.rawbuf[remote.rawlen++] = remote.timer;
		  remote.timer = 0;
		  remote.rcvstate = STATE_SPACE;
		}
		break;
	  case STATE_SPACE: // timing SPACE
		if (irdata == MARK)
		{ // SPACE just ended, record it
		  remote.rawbuf[remote.rawlen++] = remote.timer;
		  remote.timer = 0;
		  remote.rcvstate = STATE_MARK;
		}
		else
		{ // SPACE
		  if (remote.timer > GAP_TICKS)
			{
				// big SPACE, indicates gap between codes
				// Mark current code as ready for processing
				// Switch to STOP
				// Don't reset timer; keep counting space width
				remote.rcvstate = STATE_STOP;
			}
		}
		break;
	  case STATE_STOP: // waiting, measuring gap
		if (irdata == MARK) { // reset gap timer
		  remote.timer = 0;
		}
		break;
  }
  }
}

void resume() {
  remote.rcvstate = STATE_IDLE;
  remote.rawlen = 0;
}

int decode() {
  results.rawbuf = rawbuf;
  results.rawlen = rawlen;
  if (remote.rcvstate != STATE_STOP) {
    return ERR;
  }
  if (decodeNEC(results)) {
    return DECODED;
  }
  if (decodeSony(results)) {
    return DECODED;
  }
  if (decodeSanyo(results)) {
    return DECODED;
  }
  if (decodeMitsubishi(results)) {
    return DECODED;
  }
  if (decodeRC5(results)) {
    return DECODED;
  }
  if (decodeRC6(results)) {
    return DECODED;
  }
    if (decodePanasonic(results)) {
        return DECODED;
    }
    if (decodeJVC(results)) {
        return DECODED;
    }
  resume();
  return ERR;
}

// NECs have a repeat only 4 items long
long decodeNEC() {
  long data = 0;
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results.rawbuf[offset], NEC_HDR_MARK)) {
    return ERR;
  }
  offset++;
  // Check for repeat
  if (remote.rawlen == 4 &&  MATCH_SPACE(results.rawbuf[offset], NEC_RPT_SPACE) &&   MATCH_MARK(results.rawbuf[offset+1], NEC_BIT_MARK)) {
    results.bits = 0;
    results.value = REPEAT;
    results.decode_type = NEC;
    return DECODED;
  }
  if (remote->rawlen < 2 * NEC_BITS + 4) {
    return ERR;
  }
  // Initial space
  if (!MATCH_SPACE(results.rawbuf[offset], NEC_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  for (int i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(results.rawbuf[offset], NEC_BIT_MARK)) {
      return ERR;
    }
    offset++;
    if (MATCH_SPACE(results.rawbuf[offset], NEC_ONE_SPACE)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_SPACE(results.rawbuf[offset], NEC_ZERO_SPACE)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }
  // Success
  results.bits = NEC_BITS;
  results.value = data;
  results.decode_type = NEC;
  return DECODED;
}

long decodeSony() {
  long data = 0;
  if (remote.rawlen < 2 * SONY_BITS + 2) {
    return ERR;
  }
  int offset = 0; // Dont skip first space, check its size

  // Some Sony's deliver repeats fast after first
  // unfortunately can't spot difference from of repeat from two fast clicks
  if (results.rawbuf[offset] < SONY_DOUBLE_SPACE_USECS) {
    results.bits = 0;
    results.value = REPEAT;
    results.decode_type = SANYO;
    return DECODED;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results.rawbuf[offset], SONY_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < remote.rawlen) {
    if (!MATCH_SPACE(results.rawbuf[offset], SONY_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(results.rawbuf[offset], SONY_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(results.rawbuf[offset], SONY_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results.bits = (offset - 1) / 2;
  if (results.bits < 12) {
    results.bits = 0;
    return ERR;
  }
  results.value = data;
  results.decode_type = SONY;
  return DECODED;
}

long decodeSanyo() {
  long data = 0;
  if (remote.rawlen < 2 * SANYO_BITS + 2) {
    return ERR;
  }
  int offset = 0; // Skip first space

  if (results.rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
    results.bits = 0;
    results.value = REPEAT;
    results.decode_type = SANYO;
    return DECODED;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results.rawbuf[offset], SANYO_HDR_MARK)) {
    return ERR;
  }
  offset++;

  // Skip Second Mark
  if (!MATCH_MARK(results.rawbuf[offset], SANYO_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < remote.rawlen) {
    if (!MATCH_SPACE(results.rawbuf[offset], SANYO_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(results.rawbuf[offset], SANYO_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(results.rawbuf[offset], SANYO_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results.bits = (offset - 1) / 2;
  if (results.bits < 12) {
    results.bits = 0;
    return ERR;
  }
  results.value = data;
  results.decode_type = SANYO;
  return DECODED;
}

// Looks like Sony except for timings, 48 chars of data and time/space different
long decodeMitsubishi() {
  long data = 0;
  if (remote.rawlen < 2 * MITSUBISHI_BITS + 2) {
    return ERR;
  }
  int offset = 0; // Skip first space
   offset++;

   if (!MATCH_MARK(results.rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  while (offset + 1 < remote.rawlen) {
    if (MATCH_MARK(results.rawbuf[offset], MITSUBISHI_ONE_MARK)) {
      data = (data << 1) | 1;
    }
    else if (MATCH_MARK(results.rawbuf[offset], MITSUBISHI_ZERO_MARK)) {
      data <<= 1;
    }
    else {
      return ERR;
    }
    offset++;
    if (!MATCH_SPACE(results.rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
       break;
    }
    offset++;
  }

  // Success
  results.bits = (offset - 1) / 2;
  if (results.bits < MITSUBISHI_BITS) {
    results.bits = 0;
    return ERR;
  }
  results.value = data;
  results.decode_type = MITSUBISHI;
  return DECODED;
}


// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int getRClevel( int *offset, int *used, int t1) {
  if (*offset >= results,rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  int width = result.rawbuf[*offset];
  int val = ((*offset) % 2) ? MARK : SPACE;
  int correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  int avail;
  if (MATCH(width, t1 + correction)) {
    avail = 1;
  }
  else if (MATCH(width, 2*t1 + correction)) {
    avail = 2;
  }
  else if (MATCH(width, 3*t1 + correction)) {
    avail = 3;
  }
  else {
    return -1;
  }

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
  return val;
}

long decodeRC5() {
  if (remote.rawlen < MIN_RC5_SAMPLES + 2) {
    return ERR;
  }
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  // Get start bits
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != SPACE) return ERR;
  if (getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  int nbits;
  for (nbits = 0; offset < remote.rawlen; nbits++) {
    int levelA = getRClevel(results, &offset, &used, RC5_T1);
    int levelB = getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      // 1 bit
      data = (data << 1) | 1;
    }
    else if (levelA == MARK && levelB == SPACE) {
      // zero bit
      data <<= 1;
    }
    else {
      return ERR;
    }
  }

  // Success
  results.bits = nbits;
  results.value = data;
  results.decode_type = RC5;
  return DECODED;
}

long decodeRC6() {
  if (results.rawlen < MIN_RC6_SAMPLES) {
    return ERR;
  }
  int offset = 1; // Skip first space
  // Initial mark
  if (!MATCH_MARK(results.rawbuf[offset], RC6_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (!MATCH_SPACE(results.rawbuf[offset], RC6_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  long data = 0;
  int used = 0;
  // Get start bit (1)
  if (getRClevel(results, &offset, &used, RC6_T1) != MARK) return ERR;
  if (getRClevel(results, &offset, &used, RC6_T1) != SPACE) return ERR;
  int nbits;
  for (nbits = 0; offset < results.rawlen; nbits++) {
    int levelA, levelB; // Next two levels
    levelA = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    }
    levelB = getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    }
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    }
    else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    }
    else {
      return ERR; // Error
    }
  }
  // Success
  results.bits = nbits;
  results.value = data;
  results.decode_type = RC6;
  return DECODED;
}
long decodePanasonic() {
    unsigned long long data = 0;
    int offset = 1;

    if (!MATCH_MARK(results.rawbuf[offset], PANASONIC_HDR_MARK)) {
        return ERR;
    }
    offset++;
    if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_SPACE)) {
        return ERR;
    }
    offset++;

    // decode address
    for (int i = 0; i < PANASONIC_BITS; i++) {
        if (!MATCH_MARK(results.rawbuf[offset++], PANASONIC_BIT_MARK)) {
            return ERR;
        }
        if (MATCH_SPACE(results.rawbuf[offset],PANASONIC_ONE_SPACE)) {
            data = (data << 1) | 1;
        } else if (MATCH_SPACE(results.rawbuf[offset],PANASONIC_ZERO_SPACE)) {
            data <<= 1;
        } else {
            return ERR;
        }
        offset++;
    }
    results.value = (unsigned long)data;
    results.panasonicAddress = (unsigned int)(data >> 32);
    results.decode_type = PANASONIC;
    results.bits = PANASONIC_BITS;
    return DECODED;
}
long decodeJVC() {
    long data = 0;
    int offset = 1; // Skip first space
    // Check for repeat
    if (remote.rawlen - 1 == 33 &&
        MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK) &&
        MATCH_MARK(results.rawbuf[remote.rawlen-1], JVC_BIT_MARK)) {
        results.bits = 0;
        results.value = REPEAT;
        results.decode_type = JVC;
        return DECODED;
    }
    // Initial mark
    if (!MATCH_MARK(results.rawbuf[offset], JVC_HDR_MARK)) {
        return ERR;
    }
    offset++;
    if (remote->rawlen < 2 * JVC_BITS + 1 ) {
        return ERR;
    }
    // Initial space
    if (!MATCH_SPACE(results.rawbuf[offset], JVC_HDR_SPACE)) {
        return ERR;
    }
    offset++;
    for (int i = 0; i < JVC_BITS; i++) {
        if (!MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)) {
            return ERR;
        }
        offset++;
        if (MATCH_SPACE(results.rawbuf[offset], JVC_ONE_SPACE)) {
            data = (data << 1) | 1;
        }
        else if (MATCH_SPACE(results.rawbuf[offset], JVC_ZERO_SPACE)) {
            data <<= 1;
        }
        else {
            return ERR;
        }
        offset++;
    }
    //Stop bit
    if (!MATCH_MARK(results.rawbuf[offset], JVC_BIT_MARK)){
        return ERR;
    }
    // Success
    results.bits = JVC_BITS;
    results.value = data;
    results.decode_type = JVC;
    return DECODED;
}


