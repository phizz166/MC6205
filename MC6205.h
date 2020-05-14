/*
 Name:	    MC6205.h
 Created:	5/8/2018 8:53:34 AM
 Author:	silvan
 Editor:	http://www.visualmicro.com
*/

#ifndef _MC6205_h
#define _MC6205_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class MC6205 {
public:
  //this version for direct parallel output (11 pins)
  MC6205(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
    uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
    uint8_t wr, uint8_t ads, uint8_t rst);

  //this version to use a 74HC595-type shift register (6 pins)
  MC6205(uint8_t sr_data, uint8_t sr_clock, uint8_t sr_latch,
    uint8_t wr, uint8_t ads, uint8_t rst);

  uint8_t begin();

  //direct pin setters
  uint8_t rst(uint8_t level);
  uint8_t ads(uint8_t level);
  uint8_t wr(uint8_t level);

  //clear screen
  uint8_t cls();

  uint8_t clrmem();

  //cursor controls
  uint8_t cursorOn();
  uint8_t cursorOff();
  uint8_t setCursor(uint8_t address);
  uint8_t setCursor(uint8_t row, uint8_t col);

  uint8_t storeOn();
  uint8_t storeOff();
  uint8_t recallPage();

  //print strings
  uint8_t print(String s, uint16_t d = 0);
  uint8_t printToPage(String s);

  //write individual characters by address
  uint8_t writeChar(uint8_t address, uint8_t data);
  uint8_t setData(uint8_t value);

  uint8_t printRussian(String s, uint16_t d = 0);

  uint8_t deromanizer(uint8_t s);


private:

  //a page to store the screen state, for recall and deletions and stuff
  uint8_t _page[160]; 

  //pins
  uint8_t _d0;
  uint8_t _d1;
  uint8_t _d2;
  uint8_t _d3;
  uint8_t _d4;
  uint8_t _d5;
  uint8_t _d6;
  uint8_t _d7;
  uint8_t _wr;
  uint8_t _ads;
  uint8_t _rst;
  uint8_t _sr_data;
  uint8_t _sr_clock;
  uint8_t _sr_latch;

  uint8_t _use_sr = 0;
  uint8_t _store = 0;

  uint8_t _cursor;  //cursor location, 0-159
  uint8_t _cursor_stat; //cursor status

  uint16_t _delay_char;
  uint32_t _timer_char;

  //array that maps this alphabet to the weird MC6205 character table
  //russian alphabet:  А Б В Г Д Е Ж З
  //                   И Й К Л М Н О П
  //                   Р С Т У Ф Х Ц Ч
  //                   Ш Щ Ъ Ы Ь Э Ю Я  //32 characters
  uint8_t cyrillic[32] = {
    0x1E, 0x1D, 0x08, 0x18, 0x1B, 0x1A, 0x09, 0x05,
    0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0F,
    0x0D, 0x0C, 0x0B, 0x0A, 0x19, 0x17, 0x1C, 0x01,
    0x04, 0x02, 0x07, 0x06, 0x07, 0x03, 0x1F, 0x0E };

};
#endif