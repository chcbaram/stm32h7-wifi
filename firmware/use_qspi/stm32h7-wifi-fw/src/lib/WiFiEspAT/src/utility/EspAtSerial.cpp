/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "EspAtSerial.h"
// #include "wiring.h"
#include "hw.h"

EspAtSerial esp_serial;

static uint8_t uart_ch = HW_UART_CH_ESP;
static bool dtrState = false;
static bool rtsState = false;


void EspAtSerial::begin(void)
{
  uartOpen(uart_ch, 115200);    
}

void EspAtSerial::begin(uint32_t baud)
{
  uartOpen(uart_ch, baud);
}

void EspAtSerial::begin(uint32_t baud, uint8_t /* config */)
{
  uartOpen(uart_ch, baud);
}

void EspAtSerial::end()
{
}

int EspAtSerial::availableForWrite()
{
  return 1024;
}

size_t EspAtSerial::write(uint8_t ch)
{
  // Just write single-byte buffer.
  return write(&ch, 1);
}

size_t EspAtSerial::write(const uint8_t *buffer, size_t size)
{
  size_t rest = size;

  uartWrite(uart_ch, (uint8_t *)buffer, size);

  return size - rest;
}

int EspAtSerial::available(void)
{
  return uartAvailable(uart_ch);
}

int EspAtSerial::read(void)
{
  int ch;

  ch = uartRead(uart_ch);
  return ch;
}

// size_t EspAtSerial::readBytes(char *buffer, size_t length)
// {
//   uint16_t read;
//   auto rest = static_cast<uint16_t>(length);
//   _startMillis = millis();
//   do {
//     read = MIN(uartAvailable(HW_UART_CH_ESP), rest);
//     for (int i=0; i<read; i++)
//     {
//       buffer[i] = uartRead(HW_UART_CH_ESP);
//     }
//     rest -= read;
//     buffer += read;
//     if (rest == 0) {
//       return length;
//     }
//   } while (millis() - _startMillis < _timeout);
//   return length - rest;
// }

// size_t EspAtSerial::readBytesUntil(char terminator, char *buffer, size_t length)
// {
//   uint16_t read;
//   auto rest = static_cast<uint16_t>(length);
//   _startMillis = millis();
//   do {
//     bool found = CDC_ReceiveQueue_ReadUntil(&ReceiveQueue, static_cast<uint8_t>(terminator),
//                                             reinterpret_cast<uint8_t *>(buffer), rest, &read);
//     CDC_resume_receive();
//     rest -= read;
//     buffer += read;
//     if (found) {
//       return length - rest;
//     }
//     if (rest == 0) {
//       return length;
//     }
//   } while (millis() - _startMillis < _timeout);
//   return length - rest;
// }

int EspAtSerial::peek(void)
{
  return 0;
}

void EspAtSerial::flush(void)
{
  uartFlush(uart_ch);
}

uint32_t EspAtSerial::baud()
{
  return uartGetBaud(uart_ch);
}

uint8_t EspAtSerial::stopbits()
{
  return ONE_STOP_BIT;
}

uint8_t EspAtSerial::paritytype()
{
  return NO_PARITY;
}

uint8_t EspAtSerial::numbits()
{
  return 8;
}

void EspAtSerial::dtr(bool enable)
{
  dtrState = enable;
}

bool EspAtSerial::dtr(void)
{
  return dtrState;
}

bool EspAtSerial::rts(void)
{
  return rtsState;
}

EspAtSerial::operator bool()
{
  return dtrState;
}


