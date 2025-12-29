/**
 * @file drv2605l.h
 * @brief DRV2605L 触觉反馈驱动模块头文件
 * @details 封装 DRV2605L 芯片的高级操作接口
 */

#ifndef DRV2605L_H
#define DRV2605L_H

#include <Arduino.h>
#include "soft_i2c.h"

// ============================================================================
// 常量定义
// ============================================================================

// DRV2605L 默认 I2C 地址
constexpr uint8_t DRV2605L_DEFAULT_ADDR = 0x5A;

// ============================================================================
// 寄存器地址定义
// ============================================================================
namespace drv2605l_reg {
    constexpr uint8_t STATUS      = 0x00;  // 状态寄存器
    constexpr uint8_t MODE        = 0x01;  // 模式寄存器
    constexpr uint8_t RTPIN       = 0x02;  // 实时播放输入
    constexpr uint8_t LIBRARY     = 0x03;  // 波形库选择
    constexpr uint8_t WAVESEQ1    = 0x04;  // 波形序列寄存器1
    constexpr uint8_t WAVESEQ2    = 0x05;  // 波形序列寄存器2
    constexpr uint8_t WAVESEQ3    = 0x06;  // 波形序列寄存器3
    constexpr uint8_t WAVESEQ4    = 0x07;  // 波形序列寄存器4
    constexpr uint8_t WAVESEQ5    = 0x08;  // 波形序列寄存器5
    constexpr uint8_t WAVESEQ6    = 0x09;  // 波形序列寄存器6
    constexpr uint8_t WAVESEQ7    = 0x0A;  // 波形序列寄存器7
    constexpr uint8_t WAVESEQ8    = 0x0B;  // 波形序列寄存器8
    constexpr uint8_t GO          = 0x0C;  // 触发寄存器
    constexpr uint8_t OVERDRIVE   = 0x0D;  // 过驱动时间偏移
    constexpr uint8_t SUSTAINPOS  = 0x0E;  // 正向保持时间偏移
    constexpr uint8_t SUSTAINNEG  = 0x0F;  // 负向保持时间偏移
    constexpr uint8_t BREAK       = 0x10;  // 制动时间偏移
    constexpr uint8_t AUDIOMAX    = 0x13;  // 音频输入最大值
    constexpr uint8_t FEEDBACK    = 0x1A;  // 反馈控制寄存器
    constexpr uint8_t CONTROL3    = 0x1D;  // 控制寄存器3
}

// ============================================================================
// 工作模式定义
// ============================================================================
namespace drv2605l_mode {
    constexpr uint8_t INTTRIG     = 0x00;  // 内部触发模式
    constexpr uint8_t EXTTRIGEDGE = 0x01;  // 外部边沿触发模式
    constexpr uint8_t EXTTRIGLVL  = 0x02;  // 外部电平触发模式
    constexpr uint8_t PWMANALOG   = 0x03;  // PWM/模拟输入模式
    constexpr uint8_t AUDIOVIBE   = 0x04;  // 音频转振动模式
    constexpr uint8_t REALTIME    = 0x05;  // 实时播放模式
    constexpr uint8_t DIAGNOS     = 0x06;  // 诊断模式
    constexpr uint8_t AUTOCAL     = 0x07;  // 自动校准模式
}

/**
 * @class DRV2605L
 * @brief DRV2605L 触觉反馈驱动模块类
 */
class DRV2605L {
public:
    /**
     * @brief 构造函数
     * @param i2c SoftI2C 驱动实例引用
     * @param address 设备 I2C 地址，默认 0x5A
     */
    DRV2605L(SoftI2C &i2c, uint8_t address = DRV2605L_DEFAULT_ADDR);

    /**
     * @brief 初始化 DRV2605L
     * @return true 初始化成功，false 初始化失败
     */
    bool begin();

    /**
     * @brief 选择波形库
     * @param library 库编号 (1-6)
     */
    void select_library(uint8_t library);

    /**
     * @brief 设置工作模式
     * @param mode 工作模式，参见 drv2605l_mode 命名空间
     */
    void set_mode(uint8_t mode);

    /**
     * @brief 设置波形序列
     * @param slot 序列槽位 (0-7)
     * @param effect 效果编号 (1-123)，0表示序列结束
     */
    void set_waveform(uint8_t slot, uint8_t effect);

    /**
     * @brief 触发播放波形序列
     */
    void go();

    /**
     * @brief 读取状态寄存器
     * @param out_status 输出参数，存放状态值
     * @return true 读取成功，false 读取失败
     */
    bool read_status(uint8_t *out_status);

    /**
     * @brief 检测设备是否存在
     * @return true 设备存在，false 设备不存在
     */
    bool is_connected();

private:
    SoftI2C &i2c_;    // I2C 驱动引用
    uint8_t addr_;    // 设备地址

    bool write_reg(uint8_t reg, uint8_t value);
    bool read_reg(uint8_t reg, uint8_t *out_value);
};

#endif // DRV2605L_H
