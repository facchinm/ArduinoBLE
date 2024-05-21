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

#if defined(ARDUINO_SILABS)

#include "HCISilabsTransport.h"
#include "sl_string.h"

static RingBufferN<258> arduino_buf;
buf_t buf = { 0 };

extern "C" int strcasecmp(char const *a, char const *b) {
  return sl_strcasecmp(a, b);
}

/**************************************************************************//**
 * Transmit function
 *
 * Transmits len bytes of data from adaptation layer through Uart interface.
 *
 * @param[out] len Message lenght
 * @param[out] data Message data
 *
 * @note After transmit the reception is automatically started.
 *****************************************************************************/
void sl_ncp_host_com_write(uint32_t len, uint8_t *data)
{
  for (int i = 0; i < len; i++) {
    arduino_buf.store_char(data[i]);
  }
}

/**************************************************************************//**
 * Receive function
 *
 * Copies received data from Uart interface to adaptation layer
 *
 * @param[out] len Message lenght
 * @param[out] data Message data
 *
 * @return Received message length
 *****************************************************************************/
int32_t sl_ncp_host_com_read(uint32_t len, uint8_t *data)
{
  (void)data;
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // Check if there is data in the buffer from Uart
  if (len <= buf.len) {
    // Copy data to adaptation layer
    memcpy((void *)data, (void *)buf.buf, (size_t)len);
    buf.len -= len;
    memmove((void *)buf.buf, (void *)&buf.buf[len], buf.len);
  } else {
    len = -1;
  }
  CORE_EXIT_ATOMIC();
  return len;
}

/**************************************************************************//**
 * Gives back already received message length.
 *
 * This function checks if data arrived from Uart interface. This way the calls
 * can be non blocking.
 *
 * @param[out] len Message lenght
 * @param[out] data Message data
 *
 * @return Buffer length
 *****************************************************************************/
int32_t sl_ncp_host_com_peek(void)
{
  return buf.len;
}


HCISilabsTransportClass::HCISilabsTransportClass()
{
}

HCISilabsTransportClass::~HCISilabsTransportClass()
{
}

int HCISilabsTransportClass::begin()
{
  buf.len = 0;
  // Register communication interface functions in adaptation layer
  sl_status_t sc = sl_bt_api_initialize_nonblock(sl_ncp_host_com_write,
                                                 sl_ncp_host_com_read,
                                                 sl_ncp_host_com_peek);
  if (sc == SL_STATUS_OK) {
    return 1;
  } else {
    return 0;
  }

}

void HCISilabsTransportClass::end()
{
}

void HCISilabsTransportClass::wait(unsigned long timeout)
{
  while (!available()) {
    delay(10);
  }
}

int HCISilabsTransportClass::available()
{
  return arduino_buf.available();
}

// never called
int HCISilabsTransportClass::peek()
{
  return arduino_buf.peek();
}

int HCISilabsTransportClass::read()
{
  return arduino_buf.read_char();
}

size_t HCISilabsTransportClass::write(const uint8_t* data, size_t len)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_ATOMIC();
  // command fits into command buffer; otherwise discard it
  if (len <= (sizeof(buf.buf) - buf.len)) {
    memcpy((void *)&buf.buf[buf.len], (void *)data, (size_t)len);
    buf.len += len;
  }
  CORE_EXIT_ATOMIC();
}

HCISilabsTransportClass HCISilabsTransport;

HCITransportInterface& HCITransport = HCISilabsTransport;

#endif
