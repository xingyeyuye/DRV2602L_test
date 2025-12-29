/**
 * @file soft_i2c.cpp
 * @brief 软件 I2C 驱动层实现
 */

#include "soft_i2c.h"

SoftI2C::SoftI2C(uint8_t sdaPin, uint8_t sclPin, uint16_t halfPeriodUs)
    : sdaPin_(sdaPin), sclPin_(sclPin), halfPeriodUs_(halfPeriodUs) {}

void SoftI2C::begin() {
    digitalWrite(sclPin_, HIGH);
    pinMode(sclPin_, OUTPUT); // SCL: 推挽输出
    sdaRelease();             // SDA: 释放（开漏等效）
}

void SoftI2C::setHalfPeriodUs(uint16_t halfPeriodUs) {
    halfPeriodUs_ = halfPeriodUs;
}

bool SoftI2C::probe(uint8_t address) {
    start();
    bool ack = writeByte(static_cast<uint8_t>((address << 1) | 0x00));
    stop();
    return ack;
}

bool SoftI2C::writeReg8(uint8_t address, uint8_t reg, uint8_t value) {
    start();
    if (!writeByte(static_cast<uint8_t>((address << 1) | 0x00)) || 
        !writeByte(reg) ||
        !writeByte(value)) {
        stop();
        return false;
    }
    stop();
    return true;
}

bool SoftI2C::readReg8(uint8_t address, uint8_t reg, uint8_t *out) {
    if (out == nullptr) {
        return false;
    }

    start();
    if (!writeByte(static_cast<uint8_t>((address << 1) | 0x00)) || !writeByte(reg)) {
        stop();
        return false;
    }

    start(); // repeated start
    if (!writeByte(static_cast<uint8_t>((address << 1) | 0x01))) {
        stop();
        return false;
    }

    *out = readByte(/*ack=*/false); // 最后 1 字节读完发送 NACK
    stop();
    return true;
}

void SoftI2C::delayHalf() const {
    delayMicroseconds(halfPeriodUs_);
}

void SoftI2C::sclHigh() {
    digitalWrite(sclPin_, HIGH);
    delayHalf();
}

void SoftI2C::sclLow() {
    digitalWrite(sclPin_, LOW);
    delayHalf();
}

void SoftI2C::sdaLow() {
    digitalWrite(sdaPin_, LOW);
    pinMode(sdaPin_, OUTPUT);
}

void SoftI2C::sdaRelease() {
    pinMode(sdaPin_, INPUT_PULLUP); // 释放高电平（外部上拉 4.7k + 内部弱上拉）
}

bool SoftI2C::sdaRead() const {
    return digitalRead(sdaPin_) != LOW;
}

void SoftI2C::start() {
    sdaRelease();
    sclHigh();
    sdaLow();
    delayHalf();
    sclLow();
}

void SoftI2C::stop() {
    sdaLow();
    delayHalf();
    sclHigh();
    sdaRelease();
    delayHalf();
}

bool SoftI2C::writeByte(uint8_t data) {
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
        if (data & mask) {
            sdaRelease();
        } else {
            sdaLow();
        }
        delayHalf();
        sclHigh();
        sclLow();
    }

    sdaRelease();
    delayHalf();
    sclHigh();
    bool ack = !sdaRead();
    sclLow();
    return ack;
}

uint8_t SoftI2C::readByte(bool ack) {
    uint8_t data = 0;
    sdaRelease();

    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        sclHigh();
        if (sdaRead()) {
            data |= 0x01;
        }
        sclLow();
    }

    if (ack) {
        sdaLow(); // ACK: 拉低
    } else {
        sdaRelease(); // NACK: 释放
    }
    delayHalf();
    sclHigh();
    sclLow();
    sdaRelease();

    return data;
}
