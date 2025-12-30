/**
 * @file drv2605l.cpp
 * @brief DRV2605L 触觉反馈驱动模块实现
 */

#include "drv2605l.h"

DRV2605L::DRV2605L(SoftI2C &i2c, uint8_t address)
    : i2c_(i2c), addr_(address) {}

bool DRV2605L::begin() {
    // 退出待机模式
    if (!write_reg(drv2605l_reg::MODE, 0x00)) {
        return false;
    }
 
    // 配置默认参数
    write_reg(drv2605l_reg::RTPIN, 0x00);
    write_reg(drv2605l_reg::WAVESEQ1, 1);      // strong click
    write_reg(drv2605l_reg::WAVESEQ2, 0);      // end sequence
    write_reg(drv2605l_reg::OVERDRIVE, 0);
    write_reg(drv2605l_reg::SUSTAINPOS, 0);
    write_reg(drv2605l_reg::SUSTAINNEG, 0);
    write_reg(drv2605l_reg::BREAK, 0);
    write_reg(drv2605l_reg::AUDIOMAX, 0x64);

    // 配置为 LRA 模式（设置 bit7 N_ERM_LRA = 1）
    uint8_t feedback = 0;
    if (read_reg(drv2605l_reg::FEEDBACK, &feedback)) {
        write_reg(drv2605l_reg::FEEDBACK, feedback | 0x80);
    }

    // 配置为 LRA 闭环模式（清除 bit5 ERM_OPEN_LOOP）
    uint8_t control3 = 0;
    if (read_reg(drv2605l_reg::CONTROL3, &control3)) {
        write_reg(drv2605l_reg::CONTROL3, control3 & ~0x20);
    }

    // 选择 LRA 专用波形库（库6）
    select_library(6);

    return true;
}

void DRV2605L::select_library(uint8_t library) {
    write_reg(drv2605l_reg::LIBRARY, library);
}

void DRV2605L::set_mode(uint8_t mode) {
    write_reg(drv2605l_reg::MODE, mode);
}

void DRV2605L::set_waveform(uint8_t slot, uint8_t effect) {
    if (slot > 7) {
        return;
    }
    write_reg(static_cast<uint8_t>(drv2605l_reg::WAVESEQ1 + slot), effect);
}

void DRV2605L::go() {
    write_reg(drv2605l_reg::GO, 1);
}

bool DRV2605L::read_status(uint8_t *out_status) {
    return read_reg(drv2605l_reg::STATUS, out_status);
}

bool DRV2605L::is_connected() {
    return i2c_.probe(addr_);
}

bool DRV2605L::write_reg(uint8_t reg, uint8_t value) {
    return i2c_.write_reg8(addr_, reg, value);
}

bool DRV2605L::read_reg(uint8_t reg, uint8_t *out_value) {
    return i2c_.read_reg8(addr_, reg, out_value);
}
