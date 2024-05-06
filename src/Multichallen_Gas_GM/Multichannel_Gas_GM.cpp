/*
    Multichannel_GAS_GM.cpp
    Description: A drive for Seeed Grove Multichannel gas sensor V2.0.
    2019 Copyright (c) Seeed Technology Inc.  All right reserved.
    Author: Hongtai Liu(lht856@foxmail.com)
    2019-6-18

    The MIT License (MIT)
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.1  USA
*/

#include <Arduino.h>
#ifdef SOFTWAREWIRE
    #include <SoftwareWire.h>
#else
    #include <Wire.h>
#endif

#include "Multichannel_Gas_GM.h"

template<class T>
GAS_GM<T>::GAS_GM() {

}

/**
    @description: start a instance
    @param {type}  wire(SoftwareWire or TwoWire), address(uint_8)
    @return: None
*/
template<class T>
void GAS_GM<T>::begin(T& wire, uint8_t address) {
    _Wire = &wire;
    _Wire->begin();
    GM_ADDRESS = address;
    preheated();
    isPreheated = true;
}


/**
    @description:  set the I2C address, use after begin.
    @param {type}  address(uint_8)
    @return: None
*/
template<class T>
void GAS_GM<T>::setAddress(uint8_t address) {
    GM_ADDRESS = address;
    preheated();
    isPreheated = true;
}


/**
    @description:  transmit a byte to I2C device
    @param {type}  cmd(uint_8)
    @return: None
*/
template<class T>
void GAS_GM<T>::GMWriteByte(uint8_t cmd) {
    _Wire->beginTransmission(GM_ADDRESS); // transmit to device #4
    _Wire->write(cmd);              // sends one byte
    _Wire->endTransmission();    // stop transmitting
    delay(1);
}

/**
    @description:  read 4 bytes from I2C device
    @param {type}  cmd(uint_8)
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::GMRead32() {
    uint8_t index = 0;
    uint32_t value = 0;
    _Wire->requestFrom((int)GM_ADDRESS, (int)4);
    while (_Wire->available()) { // slave may send less than requested
        uint8_t byte = _Wire->read(); // receive a byte as character
        value += byte << (8 * index);
        index++;
    }
    delay(1);
    return value;
}

/**
    @description:  warmming up the gas sensor
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
void GAS_GM<T>::preheated() {
    GMWriteByte(WARMING_UP);
    isPreheated = true;
}

/**
    @description:  disable warmming up the gas sensor
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
void GAS_GM<T>::unPreheated() {
    GMWriteByte(WARMING_DOWN);
    isPreheated = false;
}

/**
    @description:  get the adc value of GM102B
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::getGM102B() {
    if (!isPreheated) {
        preheated();
    }
    GMWriteByte(GM_102B);
    return GMRead32();
}

/**
    @description:  get the adc value of GM302B
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::getGM302B() {
    if (!isPreheated) {
        preheated();
    }
    GMWriteByte(GM_302B);
    return GMRead32();
}

#ifdef GM_402B
/**
    @description:  get the adc value of GM402B
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::getGM402B() {
    if (!isPreheated) {
        preheated();
    }
    GMWriteByte(GM_402B);
    return GMRead32();
}
#endif

/**
    @description:  get the adc value of GM502B
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::getGM502B() {
    if (!isPreheated) {
        preheated();
    }
    GMWriteByte(GM_502B);
    return GMRead32();
}

/**
    @description:  get the adc value of GM702B
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::getGM702B() {
    if (!isPreheated) {
        preheated();
    }
    GMWriteByte(GM_702B);
    return GMRead32();
}

#ifdef GM_802B
/**
    @description:  get the adc value of GM802B
    @param {type}:  None
    @return: uint32_t
*/
template<class T>
uint32_t GAS_GM<T>::getGM802B() {
    if (!isPreheated) {
        preheated();
    }
    GMWriteByte(GM_802B);
    return GMRead32();
}
#endif

/**
    @description:  change the I2C address of gas sonsor.
    @param {type}:  addres(uint8_t)
    @return: None
*/
template<class T>
void GAS_GM<T>::changeGMAddr(uint8_t address) {
    if (address == 0 || address > 127) {
        address = 0x08;
    }
    _Wire->beginTransmission(GM_ADDRESS); // transmit to device #4
    _Wire->write(CHANGE_I2C_ADDR);
    _Wire->write(address);// sends one byte
    _Wire->endTransmission();    // stop transmitting

    GM_ADDRESS = address;
}

#ifdef SOFTWAREWIRE
    template class GAS_GM<SoftwareWire>;
#endif

template class GAS_GM<TwoWire>;

