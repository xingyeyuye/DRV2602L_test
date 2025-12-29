#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;

constexpr uint8_t I2C_SDA_PIN = 14;
constexpr uint8_t I2C_SCL_PIN = 12;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("DRV2605L Test");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);

  if (!drv.begin()) {
    Serial.println("ERROR: DRV2605L not found on I2C (check SDA/SCL pins, wiring, and address).");
    while (true) delay(1000);
  }

  // 选择马达类型: ERM (普通转子马达) 或 LRA (线性马达)
  // 如果你用的是普通震动马达，请使用 useERM();
  // 如果是线性马达，请使用 useLRA();
  drv.useERM();

  drv.selectLibrary(3); // 选择内置的波形库 1

  // I2C 触发模式
  drv.setMode(DRV2605_MODE_INTTRIG);
}

void loop() {
  Serial.println("Vibrating...");

  // 设置震动效果 (效果编号 1-117，具体参考 datasheet)
  // 效果 #1: "Strong Click - 100%"
  drv.setWaveform(0, 1);
  drv.setWaveform(1, 0); // 结束波形序列 (必须以0结尾)

  // 开始震动
  drv.go();

  delay(1000);
}
