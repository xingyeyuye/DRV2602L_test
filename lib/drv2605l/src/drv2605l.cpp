/**
 * @file drv2605l.cpp
 * @brief DRV2605L 触觉反馈驱动模块实现
 */

#include "drv2605l.h"

DRV2605L::DRV2605L(SoftI2C &i2c, uint8_t address)
    : i2c_(i2c), addr_(address) {}

bool DRV2605L::begin() {
    // 退出待机模式
    if (!writeReg(DRV2605L_REG::MODE, 0x00)) {
        return false;
    }

    // 配置默认参数
    writeReg(DRV2605L_REG::RTPIN, 0x00);
    writeReg(DRV2605L_REG::WAVESEQ1, 1);      // strong click
    writeReg(DRV2605L_REG::WAVESEQ2, 0);      // end sequence
    writeReg(DRV2605L_REG::OVERDRIVE, 0);
    writeReg(DRV2605L_REG::SUSTAINPOS, 0);
    writeReg(DRV2605L_REG::SUSTAINNEG, 0);
    writeReg(DRV2605L_REG::BREAK, 0);
    writeReg(DRV2605L_REG::AUDIOMAX, 0x64);

    // 配置为 ERM 模式（清除 bit7 N_ERM_LRA）
    uint8_t feedback = 0;
    if (readReg(DRV2605L_REG::FEEDBACK, &feedback)) {
        writeReg(DRV2605L_REG::FEEDBACK, feedback & 0x7F);
    }

    // 配置为 ERM 开环模式
    uint8_t control3 = 0;
    if (readReg(DRV2605L_REG::CONTROL3, &control3)) {
        writeReg(DRV2605L_REG::CONTROL3, control3 | 0x20);
    }

    return true;
}

void DRV2605L::selectLibrary(uint8_t library) {
    writeReg(DRV2605L_REG::LIBRARY, library);
}

void DRV2605L::setMode(uint8_t mode) {
    writeReg(DRV2605L_REG::MODE, mode);
}

void DRV2605L::setWaveform(uint8_t slot, uint8_t effect) {
    if (slot > 7) {
        return;
    }
    writeReg(static_cast<uint8_t>(DRV2605L_REG::WAVESEQ1 + slot), effect);
}

void DRV2605L::go() {
    writeReg(DRV2605L_REG::GO, 1);
}

bool DRV2605L::readStatus(uint8_t *status) {
    return readReg(DRV2605L_REG::STATUS, status);
}

bool DRV2605L::isConnected() {
    return i2c_.probe(addr_);
}

bool DRV2605L::writeReg(uint8_t reg, uint8_t value) {
    return i2c_.writeReg8(addr_, reg, value);
}

bool DRV2605L::readReg(uint8_t reg, uint8_t *value) {
    return i2c_.readReg8(addr_, reg, value);
}
