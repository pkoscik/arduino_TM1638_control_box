#include "TM1630.hpp"

TM1638::TM1638(int strobe, int clock, int data)
{
  pin_strobe = strobe;
  pin_clock = clock;
  pin_data = data;
}

void TM1638::init()
{
  sendCommand(0x8f); // ENABLE
  sendCommand(0x40); // Auto address incr.
  digitalWrite(pin_strobe, LOW);
  shiftOut(pin_data, pin_clock, LSBFIRST, 0xC0);
  for (uint8_t i = 0; i < 16; i++)
  {
    shiftOut(pin_data, pin_clock, LSBFIRST, 0x00);
  }
  digitalWrite(pin_strobe, HIGH);
}

uint8_t TM1638::readButtons()
{
  uint8_t buttons = 0b00000000;

  digitalWrite(pin_strobe, LOW);
  // Enter "read key scanning data" mode and read keys
  shiftOut(pin_data, pin_clock, LSBFIRST, 0x42);
  pinMode(pin_data, INPUT);
  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t b = shiftIn(pin_data, pin_clock, LSBFIRST) << i;
    buttons |= b;
  }
  pinMode(pin_data, OUTPUT);
  digitalWrite(pin_strobe, HIGH);
  return buttons;
}

void TM1638::setLED(uint8_t position, bool value)
{
  sendCommand(0x44);
  digitalWrite(pin_strobe, LOW);
  shiftOut(pin_data, pin_clock, LSBFIRST, 0xC1 + (position << 1));
  shiftOut(pin_data, pin_clock, LSBFIRST, value);
  digitalWrite(pin_strobe, HIGH);
}

void TM1638::setChar(uint8_t position, char character)
{
  uint8_t letter7 = charToFont(character);

  sendCommand(0x44);
  digitalWrite(pin_strobe, LOW);
  shiftOut(pin_data, pin_clock, LSBFIRST, 0xC0 + (position << 1));
  shiftOut(pin_data, pin_clock, LSBFIRST, letter7);
  digitalWrite(pin_strobe, HIGH);
}

void TM1638::setString(String text, bool clean = true)
{
  // XXX: this could be optimised by sending entire
  //      string in one data transmission
  for (int i = 0; i < (clean ? 8 : text.length()); i++)
  {
    setChar(i, text[i]);
  }
}

uint8_t TM1638::charToFont(char c)
{
  return FONT[c - 32];
}

void TM1638::sendCommand(uint8_t data)
{
  digitalWrite(pin_strobe, LOW);
  shiftOut(pin_data, pin_clock, LSBFIRST, data);
  digitalWrite(pin_strobe, HIGH);
}
