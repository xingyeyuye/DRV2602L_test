/**
 * @file soft_i2c.cpp
 * @brief 软件 I2C 驱动层实现
 */

#include "soft_i2c.h"

SoftI2C::SoftI2C(uint8_t sda_pin, uint8_t scl_pin, uint16_t half_period_us)
    : sda_pin_(sda_pin), scl_pin_(scl_pin), half_period_us_(half_period_us) {}

void SoftI2C::begin() {
    digitalWrite(scl_pin_, HIGH);
    pinMode(scl_pin_, OUTPUT); // SCL: 推挽输出
    sda_release();             // SDA: 释放（开漏等效）
}

void SoftI2C::set_half_period_us(uint16_t half_period_us) {
    half_period_us_ = half_period_us;
}

bool SoftI2C::probe(uint8_t address) {
    start();
    bool ack = write_byte(static_cast<uint8_t>((address << 1) | 0x00));
    stop();
    return ack;
}

bool SoftI2C::write_reg8(uint8_t address, uint8_t reg, uint8_t value) {
    start();
    if (!write_byte(static_cast<uint8_t>((address << 1) | 0x00)) || 
        !write_byte(reg) ||
        !write_byte(value)) {
        stop();
        return false;
    }
    stop();
    return true;
}

bool SoftI2C::read_reg8(uint8_t address, uint8_t reg, uint8_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    start();
    if (!write_byte(static_cast<uint8_t>((address << 1) | 0x00)) || !write_byte(reg)) {
        stop();
        return false;
    }

    start(); // repeated start
    if (!write_byte(static_cast<uint8_t>((address << 1) | 0x01))) {
        stop();
        return false;
    }

    *out_value = read_byte(false); // 最后 1 字节读完发送 NACK
    stop();
    return true;
}

void SoftI2C::delay_half() const {
    delayMicroseconds(half_period_us_);
}

void SoftI2C::scl_high() {
    digitalWrite(scl_pin_, HIGH);
    delay_half();
}

void SoftI2C::scl_low() {
    digitalWrite(scl_pin_, LOW);
    delay_half();
}

void SoftI2C::sda_low() {
    digitalWrite(sda_pin_, LOW);
    pinMode(sda_pin_, OUTPUT);
}

void SoftI2C::sda_release() {
    pinMode(sda_pin_, INPUT_PULLUP); // 释放高电平（外部上拉 4.7k + 内部弱上拉）
}

bool SoftI2C::sda_read() const {
    return digitalRead(sda_pin_) != LOW;
}

void SoftI2C::start() {
    sda_release();
    scl_high();
    sda_low();
    delay_half();
    scl_low();
}

void SoftI2C::stop() {
    sda_low();
    delay_half();
    scl_high();
    sda_release();
    delay_half();
}

bool SoftI2C::write_byte(uint8_t data) {
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
        if (data & mask) {
            sda_release();
        } else {
            sda_low();
        }
        delay_half();
        scl_high();
        scl_low();
    }

    sda_release();
    delay_half();
    scl_high();
    bool ack = !sda_read();
    scl_low();
    return ack;
}

uint8_t SoftI2C::read_byte(bool ack) {
    uint8_t data = 0;
    sda_release();

    for (uint8_t i = 0; i < 8; i++) {
        data <<= 1;
        scl_high();
        if (sda_read()) {
            data |= 0x01;
        }
        scl_low();
    }

    if (ack) {
        sda_low();  // ACK: 拉低
    } else {
        sda_release(); // NACK: 释放
    }
    delay_half();
    scl_high();
    scl_low();
    sda_release();

    return data;
}
