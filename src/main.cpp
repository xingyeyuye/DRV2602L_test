#include <Arduino.h>

constexpr uint8_t I2C_SDA_PIN = 14;
constexpr uint8_t I2C_SCL_PIN = 12; // 必须使用 IO12

constexpr uint8_t DRV2605_ADDR = 0x5A;

constexpr uint8_t DRV2605_REG_STATUS = 0x00;
constexpr uint8_t DRV2605_REG_MODE = 0x01;
constexpr uint8_t DRV2605_REG_RTPIN = 0x02;
constexpr uint8_t DRV2605_REG_LIBRARY = 0x03;
constexpr uint8_t DRV2605_REG_WAVESEQ1 = 0x04;
constexpr uint8_t DRV2605_REG_WAVESEQ2 = 0x05;
constexpr uint8_t DRV2605_REG_GO = 0x0C;
constexpr uint8_t DRV2605_REG_OVERDRIVE = 0x0D;
constexpr uint8_t DRV2605_REG_SUSTAINPOS = 0x0E;
constexpr uint8_t DRV2605_REG_SUSTAINNEG = 0x0F;
constexpr uint8_t DRV2605_REG_BREAK = 0x10;
constexpr uint8_t DRV2605_REG_AUDIOMAX = 0x13;
constexpr uint8_t DRV2605_REG_FEEDBACK = 0x1A;
constexpr uint8_t DRV2605_REG_CONTROL3 = 0x1D;

constexpr uint8_t DRV2605_MODE_INTTRIG = 0x00;

class SoftI2C {
public:
  SoftI2C(uint8_t sdaPin, uint8_t sclPin, uint16_t halfPeriodUs = 5)
      : sdaPin_(sdaPin), sclPin_(sclPin), halfPeriodUs_(halfPeriodUs) {}

  void begin() {
    digitalWrite(sclPin_, HIGH);
    pinMode(sclPin_, OUTPUT); // SCL: 推挽输出
    sdaRelease();             // SDA: 释放（开漏等效）
  }

  void setHalfPeriodUs(uint16_t halfPeriodUs) { halfPeriodUs_ = halfPeriodUs; }

  bool probe(uint8_t address) {
    start();
    bool ack = writeByte(static_cast<uint8_t>((address << 1) | 0x00));
    stop();
    return ack;
  }

  bool writeReg8(uint8_t address, uint8_t reg, uint8_t value) {
    start();
    if (!writeByte(static_cast<uint8_t>((address << 1) | 0x00)) || !writeByte(reg) ||
        !writeByte(value)) {
      stop();
      return false;
    }
    stop();
    return true;
  }

  bool readReg8(uint8_t address, uint8_t reg, uint8_t *out) {
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

private:
  uint8_t sdaPin_;
  uint8_t sclPin_;
  uint16_t halfPeriodUs_;

  void delayHalf() const { delayMicroseconds(halfPeriodUs_); }

  void sclHigh() {
    digitalWrite(sclPin_, HIGH);
    delayHalf();
  }

  void sclLow() {
    digitalWrite(sclPin_, LOW);
    delayHalf();
  }

  void sdaLow() {
    digitalWrite(sdaPin_, LOW);
    pinMode(sdaPin_, OUTPUT);
  }

  void sdaRelease() {
    pinMode(sdaPin_, INPUT_PULLUP); // 释放高电平（外部上拉 4.7k + 内部弱上拉）
  }

  bool sdaRead() const { return digitalRead(sdaPin_) != LOW; }

  void start() {
    sdaRelease();
    sclHigh();
    sdaLow();
    delayHalf();
    sclLow();
  }

  void stop() {
    sdaLow();
    delayHalf();
    sclHigh();
    sdaRelease();
    delayHalf();
  }

  bool writeByte(uint8_t data) {
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

  uint8_t readByte(bool ack) {
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
};

SoftI2C i2c(I2C_SDA_PIN, I2C_SCL_PIN);

static bool drvWrite8(uint8_t reg, uint8_t val) {
  return i2c.writeReg8(DRV2605_ADDR, reg, val);
}

static bool drvRead8(uint8_t reg, uint8_t *out) {
  return i2c.readReg8(DRV2605_ADDR, reg, out);
}

static bool drvInit() {
  uint8_t tmp = 0;

  if (!drvWrite8(DRV2605_REG_MODE, 0x00)) { // 退出 standby
    return false;
  }

  drvWrite8(DRV2605_REG_RTPIN, 0x00);
  drvWrite8(DRV2605_REG_WAVESEQ1, 1); // strong click
  drvWrite8(DRV2605_REG_WAVESEQ2, 0); // end sequence
  drvWrite8(DRV2605_REG_OVERDRIVE, 0);
  drvWrite8(DRV2605_REG_SUSTAINPOS, 0);
  drvWrite8(DRV2605_REG_SUSTAINNEG, 0);
  drvWrite8(DRV2605_REG_BREAK, 0);
  drvWrite8(DRV2605_REG_AUDIOMAX, 0x64);

  if (drvRead8(DRV2605_REG_FEEDBACK, &tmp)) {
    drvWrite8(DRV2605_REG_FEEDBACK, tmp & 0x7F); // ERM: 清 bit7 (N_ERM_LRA)
  }
  if (drvRead8(DRV2605_REG_CONTROL3, &tmp)) {
    drvWrite8(DRV2605_REG_CONTROL3, tmp | 0x20); // ERM_OPEN_LOOP
  }

  return true;
}

static void drvSelectLibrary(uint8_t lib) { drvWrite8(DRV2605_REG_LIBRARY, lib); }

static void drvSetMode(uint8_t mode) { drvWrite8(DRV2605_REG_MODE, mode); }

static void drvSetWaveform(uint8_t slot, uint8_t effect) {
  if (slot > 7) {
    return;
  }
  drvWrite8(static_cast<uint8_t>(DRV2605_REG_WAVESEQ1 + slot), effect);
}

static void drvGo() { drvWrite8(DRV2605_REG_GO, 1); }

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("DRV2605L Test (SoftI2C, SCL push-pull + SDA release)");

  i2c.begin();
  i2c.setHalfPeriodUs(5); // 调大更稳（更慢）

  if (!i2c.probe(DRV2605_ADDR)) {
    Serial.println("ERROR: DRV2605L not found on I2C.");
    while (true) delay(1000);
  }

  if (!drvInit()) {
    Serial.println("ERROR: DRV2605L init failed.");
    while (true) delay(1000);
  }

  drvSelectLibrary(3);
  drvSetMode(DRV2605_MODE_INTTRIG);

  uint8_t status = 0;
  if (drvRead8(DRV2605_REG_STATUS, &status)) {
    Serial.print("DRV2605 STATUS=0x");
    Serial.println(status, HEX);
  }
}

void loop() {
  Serial.println("Vibrating...");

  drvSetWaveform(0, 1);
  drvSetWaveform(1, 0);
  drvGo();

  delay(1000);
}
