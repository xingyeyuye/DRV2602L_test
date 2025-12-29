/**
 * @file soft_i2c.h
 * @brief 软件 I2C 驱动层头文件
 * @details 提供软件模拟 I2C 通信功能，适用于无硬件 I2C 或需要灵活配置引脚的场景
 */

#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <Arduino.h>

/**
 * @class SoftI2C
 * @brief 软件 I2C 驱动类
 * @details 使用 GPIO 模拟 I2C 通信协议，SCL 使用推挽输出，SDA 使用开漏等效模式
 */
class SoftI2C {
public:
    /**
     * @brief 构造函数
     * @param sdaPin SDA 数据线引脚
     * @param sclPin SCL 时钟线引脚
     * @param halfPeriodUs 半周期延时（微秒），默认5us，对应约100kHz
     */
    SoftI2C(uint8_t sdaPin, uint8_t sclPin, uint16_t halfPeriodUs = 5);

    /**
     * @brief 初始化 I2C 总线
     * @details 配置 SCL 为推挽输出，SDA 为开漏等效模式
     */
    void begin();

    /**
     * @brief 设置半周期延时
     * @param halfPeriodUs 半周期延时（微秒），值越大通信越慢但越稳定
     */
    void setHalfPeriodUs(uint16_t halfPeriodUs);

    /**
     * @brief 探测指定地址的设备是否存在
     * @param address 7位设备地址
     * @return true 设备存在，false 设备不存在
     */
    bool probe(uint8_t address);

    /**
     * @brief 写入一个8位寄存器
     * @param address 7位设备地址
     * @param reg 寄存器地址
     * @param value 要写入的值
     * @return true 写入成功，false 写入失败
     */
    bool writeReg8(uint8_t address, uint8_t reg, uint8_t value);

    /**
     * @brief 读取一个8位寄存器
     * @param address 7位设备地址
     * @param reg 寄存器地址
     * @param out 输出参数，存放读取的值
     * @return true 读取成功，false 读取失败
     */
    bool readReg8(uint8_t address, uint8_t reg, uint8_t *out);

private:
    uint8_t sdaPin_;      ///< SDA 引脚
    uint8_t sclPin_;      ///< SCL 引脚
    uint16_t halfPeriodUs_; ///< 半周期延时

    void delayHalf() const;
    void sclHigh();
    void sclLow();
    void sdaLow();
    void sdaRelease();
    bool sdaRead() const;
    void start();
    void stop();
    bool writeByte(uint8_t data);
    uint8_t readByte(bool ack);
};

#endif // SOFT_I2C_H
