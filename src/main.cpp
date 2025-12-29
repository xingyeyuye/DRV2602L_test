/**
 * @file main.cpp
 * @brief DRV2605L 触觉反馈驱动测试程序
 * @details 应用层 - 调用模块层接口实现振动控制
 */

#include <Arduino.h>
#include "drv2605l.h"

// ============================================================================
// 引脚配置
// ============================================================================
constexpr uint8_t I2C_SDA_PIN = 14;
constexpr uint8_t I2C_SCL_PIN = 12;  // 必须使用 IO12

// ============================================================================
// 全局对象
// ============================================================================
SoftI2C i2c(I2C_SDA_PIN, I2C_SCL_PIN);
DRV2605L haptic(i2c);

// ============================================================================
// 初始化
// ============================================================================
void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("DRV2605L Test (SoftI2C, SCL push-pull + SDA release)");

    // 初始化 I2C 总线
    i2c.begin();
    i2c.setHalfPeriodUs(5);  // 调大更稳（更慢）

    // 检测设备是否存在
    if (!haptic.isConnected()) {
        Serial.println("ERROR: DRV2605L not found on I2C.");
        while (true) delay(1000);
    }

    // 初始化 DRV2605L
    if (!haptic.begin()) {
        Serial.println("ERROR: DRV2605L init failed.");
        while (true) delay(1000);
    }

    // 配置触觉驱动
    haptic.selectLibrary(3);
    haptic.setMode(DRV2605L_MODE::INTTRIG);

    // 读取并打印状态
    uint8_t status = 0;
    if (haptic.readStatus(&status)) {
        Serial.print("DRV2605 STATUS=0x");
        Serial.println(status, HEX);
    }
}

// ============================================================================
// 主循环
// ============================================================================
void loop() {
    Serial.println("Vibrating...");

    // 设置波形序列并触发
    haptic.setWaveform(0, 1);  // 效果1: Strong Click
    haptic.setWaveform(1, 0);  // 结束序列
    haptic.go();

    delay(1000);
}
