/*
 * IMU — Acelerometro + Giroscopio (sem magnetometro)

 * - Calibracao automatica ao ligar (robo parado)
 * - Correcao de eixos: sensor de cabeca para baixo
 */

#include "FastIMU.h"
#include <MadgwickAHRS.h>
#include <Wire.h>

#define IMU_ADDRESS 0x68
MPU9250  IMU;
Madgwick filter;
calData  calib = { 0 };
AccelData accelData;
GyroData  gyroData;

float roll = 0, pitch = 0, yaw = 0;

void setup() {
  // ---- I2C (pinos do Pico) ----
  Wire.setSDA(16);
  Wire.setSCL(17);
  Wire.begin();
  Wire.setClock(400000);

  // ---- Serial (timeout 3s para nao travar sem USB) ----
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { ; }

  Serial.println("===== IMU: accel + giro (sem mag) =====");

  // ---- Inicia o sensor ----
  int err = IMU.init(calib, IMU_ADDRESS);
  if (err != 0) {
    Serial.print("ERRO ao inicializar: ");
    Serial.println(err);
    while (true) { ; }
  }

  // ---- Calibracao automatica accel/giro ----
  Serial.println("Mantenha o robo PARADO e NIVELADO!");
  Serial.println("Calibrando em 5 segundos...");
  delay(5000);
  Serial.println("Calibrando... NAO MEXA!");
  IMU.calibrateAccelGyro(&calib);
  Serial.println("Calibracao concluida!");
  delay(1000);
  IMU.init(calib, IMU_ADDRESS);   // reaplica os offsets

  // ---- Filtro Madgwick a 100 Hz ----
  filter.begin(100);

  Serial.println("Robo plano: roll e pitch devem ficar perto de 0.\n");
}

void loop() {
  // Atualiza a cada 10 ms (100 Hz)
  static uint32_t nextUpdate = 0;
  uint32_t now = millis();
  if (now >= nextUpdate) {
    nextUpdate = now + 10;

    // ---- Le sensor ----
    IMU.update();
    IMU.getAccel(&accelData);
    IMU.getGyro(&gyroData);

    // ---- Correcao de eixos: sensor de cabeca para baixo ----
    float ax =  accelData.accelX;
    float ay = -accelData.accelY;   // invertido
    float az = -accelData.accelZ;   // invertido
    float gx =  gyroData.gyroX;
    float gy = -gyroData.gyroY;     // invertido
    float gz = -gyroData.gyroZ;     // invertido

    // ---- Filtro so com 6 eixos (updateIMU = sem magnetometro) ----
    filter.updateIMU(gx, gy, gz, ax, ay, az);

    roll  = filter.getRoll();
    pitch = filter.getPitch();
    yaw   = filter.getYaw();
  }

  // Imprime a cada 100 ms (10 Hz)
  static uint32_t nextPrint = 0;
  if (millis() >= nextPrint) {
    nextPrint = millis() + 100;
    Serial.print("Roll: ");     Serial.print(roll,  1);
    Serial.print(" | Pitch: "); Serial.print(pitch, 1);
    Serial.print(" | Yaw: ");   Serial.println(yaw, 1);
  }
}
