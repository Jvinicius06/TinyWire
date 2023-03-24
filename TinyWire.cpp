/*****************************************************************************
 *
 *
 * File              TinyWire.cpp
 * Date              Saturday, 10/29/17
 * Composed by 		lucullus
 * Modified by Benoit3 on 18/5/2019 to add multibyte send
 *
 *  **** See TinyWire.h for Credits and Usage information ****
 *
 *
 *  This library is free software; you can redistribute it and/or modify it under the
 *  terms of the GNU General Public License as published by the Free Software
 *  Foundation; either version 2.1 of the License, or any later version.
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 *  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 ******************************************************************************/

#ifndef tiny_twi_cpp
#define tiny_twi_cpp

#include "TinyWire.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "twi.h"

template <int N>
void (*TinyTwi<N>::user_onRequest)(uint8_t);
template <int N>
void (*TinyTwi<N>::user_onReceive)(uint8_t, int);

template <int N>
uint8_t TinyTwi<N>::slave_addr[N];

template <int N>
TinyTwi<N>::TinyTwi() {
}

/*---------------------------------------------------
 Initialize USI and library for master function
-----------------------------------------------------*/
template <int N>
void TinyTwi<N>::end() {
    Twi_end();
}

template <int N>
void TinyTwi<N>::begin() {
    master_mode = true;
    Twi_attachSlaveTxEvent(onRequestService);
    Twi_attachSlaveRxEvent(onReceiveService);
    Twi_master_init();
}

/*---------------------------------------------------
 Initialize USI and library for slave function
 Parameter: uint8_t slave address
-----------------------------------------------------*/
template <int N>
void TinyTwi<N>::begin(uint8_t id, uint8_t I2C_SLAVE_ADDR) {
    master_mode = false;
    slave_addr[id] = I2C_SLAVE_ADDR;
    Twi_attachSlaveTxEvent(onRequestService);
    Twi_attachSlaveRxEvent(onReceiveService);
    Twi_attachCheckAddress(onCheckAddressService);
    Twi_slave_init();
}

template <int N>
uint8_t TinyTwi<N>::send(uint8_t data) {
    if (master_mode || temp_master_mode) {
        return Twi_master_send(data);
    } else {
        return Twi_slave_send(data);
    }
}

template <int N>
uint8_t TinyTwi<N>::send(uint8_t *data, uint8_t length) {
    if (master_mode || temp_master_mode) {
        // TODO
        // return Twi_master_send(data,length);
        return 0;
    } else {
        return Twi_slave_send(data, length);
    }
}

template <int N>
uint8_t TinyTwi<N>::write(uint8_t data) {
    return send(data);
}

template <int N>
uint8_t TinyTwi<N>::read() {
    return Twi_receive();
}

template <int N>
uint8_t TinyTwi<N>::receive() {
    return read();
}

template <int N>
uint8_t TinyTwi<N>::available() {
    return Twi_available();
}

template <int N>
void TinyTwi<N>::beginTransmission(uint8_t slaveAddr) {
    Twi_master_beginTransmission(slaveAddr);
}

template <int N>
uint8_t TinyTwi<N>::endTransmission() {
    uint8_t temp;

    if (!master_mode) {
        temp_master_mode = true;
        Twi_master_init();
    }

    temp = Twi_master_endTransmission();
    if (temp_master_mode) {
        temp_master_mode = false;
        begin(slave_addr);
    }
    return temp;
}

template <int N>
uint8_t TinyTwi<N>::requestFrom(uint8_t slaveAddr, uint8_t numBytes) {
    uint8_t temp;
    if (!master_mode) {
        temp_master_mode = true;
        Twi_master_init();
    }
    temp = Twi_master_requestFrom(slaveAddr, numBytes);
    if (temp_master_mode) {
        temp_master_mode = false;
        begin(slave_addr);
    }
    return temp;
}

// behind the scenes function that is called when data is received
template <int N>
void TinyTwi<N>::onReceiveService(uint8_t address, int numBytes) {
    // don't bother if user hasn't registered a callback
    if (!user_onReceive) {
        return;
    }
    // alert user program
    user_onReceive(address, numBytes);
}

// behind the scenes function that is called when data is requested
template <int N>
void TinyTwi<N>::onRequestService(uint8_t address) {
    // don't bother if user hasn't registered a callback
    if (!user_onRequest) {
        return;
    }
    // alert user program
    user_onRequest(address);
}

template <int N>
bool TinyTwi<N>::onCheckAddressService(int address) {
    for (int i = 0; i < N; i++) {
        if (slave_addr[i] == address) {
            return true;
        }
    }
    return false;
}

template <int N>
void TinyTwi<N>::onReceive(void (*function)(uint8_t, int)) {
    user_onReceive = function;
}

template <int N>
void TinyTwi<N>::onRequest(void (*function)(void)) {
    user_onRequest = function;
}

/*-----------------------------------------------------
 Preinstantiate TinyWire Object
-------------------------------------------------------*/
TinyTwi<NUMBEROFSLAVE> TinyWire = TinyTwi<NUMBEROFSLAVE>();

#endif
