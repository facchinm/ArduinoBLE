/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(ARDUINO_ARCH_MBED)

#include "HCIUartTransport.h"

#if defined(__has_include) &&  __has_include("WiFiNINA.h")
    #define COEXISTENCE 1
    #undef NINA_RTS
    #undef NINA_CTS
    #define NINA_RTS                    NINA_ACK
    #define NINA_CTS                    NINA_GPIO0
#endif

#if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
#ifndef COEXISTENCE
#define SerialHCI Serial2
#else
#define SerialHCI SerialNina
#endif
#elif defined(ARDUINO_SAMD_NANO_33_IOT)
#ifdef COEXISTENCE
#undef SerialHCI
#define SerialHCI Serial2
#endif
// SerialHCI is already defined in the variant
#elif defined(ARDUINO_PORTENTA_H7_M4)
// SerialHCI is already defined in the variant
#elif defined(ARDUINO_PORTENTA_H7_M7)
#define SerialHCI Serial2
#else
#error "Unsupported board selected!"
#endif

HCIUartTransportClass::HCIUartTransportClass(HardwareSerial& uart, unsigned long baudrate) :
  _uart(&uart),
  _baudrate(baudrate)
{
}

HCIUartTransportClass::~HCIUartTransportClass()
{
}

int HCIUartTransportClass::begin()
{
  _uart->begin(_baudrate);

  return 1;
}

void HCIUartTransportClass::end()
{
  _uart->end();
}

void HCIUartTransportClass::wait(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (available()) {
      break;
    }
  }
}

int HCIUartTransportClass::available()
{
  return _uart->available();
}

int HCIUartTransportClass::peek()
{
  return _uart->peek();
}

int HCIUartTransportClass::read()
{
  return _uart->read();
}

size_t HCIUartTransportClass::write(const uint8_t* data, size_t length)
{
#if defined(ARDUINO_AVR_UNO_WIFI_REV2 ) || defined(COEXISTENCE)
  // wait while the CTS pin is low
  while (digitalRead(NINA_CTS) == HIGH);
#endif

  size_t result = _uart->write(data, length);

  _uart->flush();

  return result;
}

#if defined(ARDUINO_AVR_UNO_WIFI_REV2 ) || defined(COEXISTENCE)
HCIUartTransportClass HCIUartTransport(SerialHCI, 119600);
#else
HCIUartTransportClass HCIUartTransport(SerialHCI, 912600);
#endif
HCITransportInterface& HCITransport = HCIUartTransport;

#endif
