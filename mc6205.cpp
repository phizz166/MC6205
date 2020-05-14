/*
 Name:		MC6205.cpp
 Created:	5/8/2018 8:53:34 AM
 Author:	silvan
 Editor:	http://www.visualmicro.com
*/

#include "MC6205.h"


MC6205::MC6205(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4,
  uint8_t d5, uint8_t d6, uint8_t d7, uint8_t wr, uint8_t ads, uint8_t rst) {

  //construct the pins
  _d0 = d0;
  _d1 = d1;
  _d2 = d2;
  _d3 = d3;
  _d4 = d4;
  _d5 = d5;
  _d6 = d6;
  _d7 = d7;
  _wr = wr;
  _ads = ads;
  _rst = rst;

  //set as outputs
  pinMode(_d0, OUTPUT);
  pinMode(_d1, OUTPUT);
  pinMode(_d2, OUTPUT);
  pinMode(_d3, OUTPUT);
  pinMode(_d4, OUTPUT);
  pinMode(_d5, OUTPUT);
  pinMode(_d6, OUTPUT);
  pinMode(_d7, OUTPUT);
  pinMode(_wr, OUTPUT);
  pinMode(_ads, OUTPUT);
  pinMode(_rst, OUTPUT);

  //fill the page buffer with space characters
  for (int i = 0; i < 160; i++) {
    _page[i] = 0x5F;
  }
}

MC6205::MC6205(uint8_t sr_data, uint8_t sr_clock, uint8_t sr_latch,
  uint8_t wr, uint8_t ads, uint8_t rst) {

  //enable shift register mode
  _use_sr = 1;

  //construct the pins
  _sr_data = sr_data;
  _sr_clock = sr_clock;
  _sr_latch = sr_latch;
  _wr = wr;
  _ads = ads;
  _rst = rst;

  //set as outputs
  pinMode(_sr_data, OUTPUT);
  pinMode(_sr_clock, OUTPUT);
  pinMode(_sr_latch, OUTPUT);
  pinMode(_wr, OUTPUT);
  pinMode(_ads, OUTPUT);
  pinMode(_rst, OUTPUT);

  for (int i = 0; i < 160; i++) {
    _page[i] = 0x5F;
  }

}


uint8_t MC6205::begin() {
  _cursor = 0x0;  //cursor at position 0
  _cursor_stat = 1;  //cursor on

  cls();  //clear any existing data
}

//clear the screen
//note: this is a blocking function!
//maybe rewrite later or maybe don't be arsed
uint8_t MC6205::cls() {
  rst(0);
  delay(20);  //needs about this long to fully clear
  rst(1);
  return 1;
}

uint8_t MC6205::clrmem() {
  for (int i = 0; i < 160; i++) {
    _page[i] = 0x0;
  }
  return 1;
}

uint8_t MC6205::cursorOn() {
  _cursor_stat = 1;
  setData(_cursor);
  ads(0);
  ads(1);
}

uint8_t MC6205::cursorOff() {
  _cursor_stat = 0;
  setData(0xA1);
  ads(0);
  ads(1);
}

//sets cursor by display address
uint8_t MC6205::setCursor(uint8_t address) {
  _cursor = address;
  setData(_cursor);
  ads(0);
  ads(1);
}

//sets cursor by row and column
uint8_t MC6205::setCursor(uint8_t row, uint8_t col) {
  _cursor = (row * 16) + col;
}

uint8_t MC6205::storeOn() {
  _store = 1;
}

uint8_t MC6205::storeOff() {
  _store = 0;
}

uint8_t MC6205::recallPage() {
  for (int i = 0; i < 160; i++) {
    writeChar(i, _page[i]);
  }
  return 0;

}


//prints a string at the current cursor location
uint8_t MC6205::print(String s, uint16_t d) {
  for (int i = 0; i < s.length(); i++) {
    uint8_t ch = s[i];
    //bounds checking and error handling
    //shift lowercase ascii to UPPERCASE
    if ((ch > 96) && (ch < 126)) {
      ch = ch - 32;
    }
    //now, if we fit in the ASCII range, remap to the weird USSR character table
    if ((ch >= 32) && (ch < 95)) {
      ch = (95 - ch) + 32;
    }
    //if it doesn't fit, it was probably unprintable (or russian)
    //TODO: add another handler here for cyrillic characters
    else {
      ch = 35; //replace unknown characters with #
    }
    writeChar(_cursor, ch);  //write characters to the cursor location
    if (_store) {
      _page[_cursor] = ch;  //update the character page
    }
    _cursor++;
    //wrap screen
    if (_cursor >= 160) {
      _cursor = 0;
    }
    delay(d);
  }
  writeChar(_cursor, 0x5F);
  if (!_cursor_stat) {  //disable cursor afterwards if we had it turned off
    writeChar(0xA1, 0x00);
  }
}

//prints to the page in the background, so you can swap quickly without refreshing
//cls(), printToPage(), recallPage(), clrmem(), printToPage(), recallPage, clrmem(), etc.
//note that there is still only one cursor
uint8_t MC6205::printToPage(String s) {
  for (int i = 0; i < s.length(); i++) {
    uint8_t ch = s[i];
    //bounds checking and error handling
    //shift lowercase ascii to UPPERCASE
    if ((ch > 96) && (ch < 126)) {
      ch = ch - 32;
    }
    //now, if we fit in the ASCII range, remap to the weird USSR character table
    if ((ch >= 32) && (ch < 95)) {
      ch = (95 - ch) + 32;
    }
    //if it doesn't fit, it was probably unprintable (or russian)
    //TODO: add another handler here for cyrillic characters
    else {
      ch = 35; //replace unknown characters with #
    }
    _page[_cursor] = ch;  //update the character page
    _cursor++;
    //wrap screen
    if (_cursor >= 160) {
      _cursor = 0;
    }

  }
}


//control line access
uint8_t MC6205::rst(uint8_t level) {
  digitalWrite(_rst, level);
}
uint8_t MC6205::ads(uint8_t level) {
  digitalWrite(_ads, level);
}
uint8_t MC6205::wr(uint8_t level) {
  digitalWrite(_wr, level);
}

//actual communications sequence
uint8_t MC6205::writeChar(uint8_t address, uint8_t data) {
  ads(1);
  wr(1);
  setData(address);
  ads(0);
  ads(1);
  setData(data);
  wr(0);
  wr(1);
}

//SPI version of the comm sequence
//use an HC595 level shifter



//write a byte onto the display data lines
uint8_t MC6205::setData(uint8_t value) {

  //serial mode with 74HC595
  if (_use_sr) {
    digitalWrite(_sr_latch, LOW);  //delatch shift register
    shiftOut(_sr_data, _sr_clock, MSBFIRST, value);
    digitalWrite(_sr_latch, HIGH);
  }

  //parallel mode
  else {
    digitalWrite(_d0, value & 0x1);
    digitalWrite(_d1, value & 0x2);
    digitalWrite(_d2, value & 0x4);
    digitalWrite(_d3, value & 0x8);
    digitalWrite(_d4, value & 0x10);
    digitalWrite(_d5, value & 0x20);
    digitalWrite(_d6, value & 0x40);
    digitalWrite(_d7, value & 0x80);
  }
}


/*   GOST 16876 (1971) Romanization of cyrillic characters
 А   Б   В   Г   Д   Е   Ж   З   И   Й   К   Л   М   Н   О   П
 a   b   v   g   d   e   zh  z   i   j   k   l   m   n   o   p

 Р   С   Т   У   Ф   Х   Ц   Ч   Ш   Щ   Ъ   Ы   Ь   Э   Ю   Я
 r   s   t   u   f   kh  c   ch  sh shh  "   y   '   eh  ju  ja
 */

 //multi characters: zh, kh, ch, sh, shh, eh, ju, ja
 //so must check: z, k, c, s, e, j

//this takes GOST 1971 romanized cyrillic and prints it in cyrillic characters
//only accepts CAPITAL LETTERS at this point bc lazy
uint8_t MC6205::printRussian(String s, uint16_t d) {
  uint8_t ch;
  for (int i = 0; i < s.length(); i++) {
    //if it's not the beginning of a double character, pass it through
    if ((s[i] != 'Z') && (s[i] != 'K') && (s[i] != 'C') && (s[i] != 'S') && (s[i] != 'E') && (s[i] != 'J')) {
      ch = s[i];
    }
    else { //if it could be a double character, then...
      //Z
      if (s[i] == 'Z') {
        if (s[i + 1] == 'H') {
          ch = 'z';  //zh
          i++;
        }
        else {
          ch = 'Z';
        }
      }

      //K
      if (s[i] == 'K') {
        if (s[i + 1] == 'H') {
          ch = 'k'; //kh
          i++;
        }
        else {
          ch = 'K';
        }
      }

      //C
      if (s[i] == 'C') {
        if (s[i + 1] == 'H') {
          ch = 'c'; //ch
          i++;
        }
        else {
          ch = 'C';
        }
      }

      //E
      if (s[i] == 'E') {
        if (s[i + 1] == 'H') {
          ch = 'e'; //eh
          i++;
        }
        else {
          ch = 'E';
        }
      }

      //J
      if (s[i] == 'J') {
        if (s[i + 1] == 'U') {
          ch = 'u';  //ju
          i++;
        }
        else if (s[i + 1] == 'A') {
          ch = 'a';  //ja
          i++;
        }
        else {
          ch = 'J';
        }
      }

      //S
      if (s[i] == 'S') {
        if (s[i + 1] == 'H') {
          if (s[i + 2] == 'H') {
            ch = 't'; //shh
            i += 2;
          }
          else {
            ch = 's'; //sh
            i++;
          }
        }
        else {
          ch = 'S';
        }
      }
    }  //end of that SPAGETTI logic. we can now pass ch to the de-romanizer
    uint8_t ch_code;
    //code to extract anything that is not in the ascii upper or lowercase range    
    if (((ch >= 65) && (ch <= 90)) || ((ch >= 97) && (ch <= 122))) {
      ch_code = deromanizer(ch); //directly deromanize
    }
    else {
      if (ch == 91) ch_code = 0x24;
      if (ch == 92) ch_code = 0x23;
      if (ch == 93) ch_code = 0x22;
      if (ch == 94) ch_code = 0x21;
      if (ch == 95) ch_code = 0x20;
      if (ch == 96) ch_code = 0x58;  //not an apostrophe but close enough
      if (ch <= 64) {
        ch_code = (95 - ch) + 32;
      }
    }

    //when printing russian we can't send quotes or apostrophes. whatever
    if ((ch == '\'') || (ch == '\"')) ch_code = deromanizer(ch);
    writeChar(_cursor, ch_code);  //write characters to the cursor location   
    if (_store) {
      _page[_cursor] = ch;  //update the character page
    }
    _cursor++;
    delay(d);
  } //end of print loop

  writeChar(_cursor, 0x5F);  //move the cursor off the last character (write a space afterwards)
  if (!_cursor_stat) {  //disable cursor after printing if we had it turned off
    writeChar(0xA1, 0x00);
  }
}


uint8_t MC6205::deromanizer(uint8_t s) {
  uint8_t out;
  switch (s) {
  case 'A':
    out = 0x1E;
    break;
  case 'B':
    out = 0x1D;
    break;
  case 'V':
    out = 0x08;
    break;
  case 'G':
    out = 0x18;
    break;
  case 'D':
    out = 0x1B;
    break;
  case 'E':
    out = 0x1A;
    break;
  case 'z':
    out = 0x09;
    break;
  case 'Z':
    out = 0x05;
    break;
  case 'I':
    out = 0x16;
    break;
  case 'J':
    out = 0x15;
    break;
  case 'K':
    out = 0x14;
    break;
  case 'L':
    out = 0x13;
    break;
  case 'M':
    out = 0x12;
    break;
  case 'N':
    out = 0x11;
    break;
  case 'O':
    out = 0x10;
    break;
  case 'P':
    out = 0x0F;
    break;
  case 'R':
    out = 0x0D;
    break;
  case 'S':
    out = 0x0C;
    break;
  case 'T':
    out = 0x0B;
    break;
  case 'U':
    out = 0x0A;
    break;
  case 'F':
    out = 0x19;
    break;
  case 'k':
    out = 0x17;
    break;
  case 'C':
    out = 0x1C;
    break;
  case 'c':
    out = 0x01;
    break;
  case 's':
    out = 0x04;
    break;
  case 't':
    out = 0x02;
    break;
  case '\"':
    out = 0x07;
    break;
  case 'Y':
    out = 0x06;
    break;
  case '\'':
    out = 0x07;
    break;
  case 'e':
    out = 0x03;
    break;
  case 'u':
    out = 0x1F;
    break;
  case 'a':
    out = 0x0E;
    break;
  }
  return out;
}