/*
 * SKETCH 1 — Calibrador do MPU-9250 (FastIMU)
 * ============================================
 *
 * Pinos I2C (Raspberry Pi Pico):
 *   SDA → GP16 (físico 21)   SCL → GP17 (físico 22)
 *
 * Serial Monitor: 115200 baud 
 *
 *   1. Deixe o robô PARADO e NIVELADO no chão
 *   2. Faça upload
 *   3. Quando pedir, faça o movimento de FIGURA DE 8 com o robô
 *      (para o magnetômetro)
 *   4. Anote os 9 valores que aparecem no final
 */

#include <Wire.h>
#include <FastIMU.h>

MPU9250  IMU;
calData  calib;

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) {}

  Serial.println("===== CALIBRADOR MPU-9250 =====");

  Wire.setSDA(16);
  Wire.setSCL(17);
  Wire.begin();
  Wire.setClock(400000);

  // Inicia com calibração vazia
  calib = {0};
  calib.valid = false;

  Serial.println("\n[IMU] Inicializando...");
  if (IMU.init(calib, 0x68) != 0) {
    Serial.println("  ERRO ao iniciar! Confira fiacao.");
    while (true) delay(1000);
  }
  Serial.println("  OK!");

  bool hasMag = IMU.hasMagnetometer();
  Serial.print("  Magnetometro: ");
  Serial.println(hasMag ? "SIM" : "NAO");

  // ---------- Calibração Accel/Gyro ----------
  Serial.println("\n[1/2] CALIBRANDO ACELEROMETRO E GIROSCOPIO");
  Serial.println("      Deixe o robo PARADO e NIVELADO!");
  Serial.println("      Iniciando em 3 segundos... NAO MEXA!");
  delay(3000);
  Serial.println("      Calibrando...");
  IMU.calibrateAccelGyro(&calib);
  Serial.println("      OK!");

  // ---------- Calibração Magnetômetro ----------
  if (hasMag) {
    Serial.println("\n[2/2] CALIBRANDO MAGNETOMETRO");
    Serial.println("      Faca o movimento de FIGURA DE 8 com o robo");
    Serial.println("      por ~15 segundos, girando em todas as direcoes!");
    Serial.println("      Comecando em 3 segundos...");
    delay(3000);
    Serial.println("      MOVA AGORA! (figura de 8)");
    IMU.calibrateMag(&calib);
    Serial.println("      OK!");
  }

  // ---------- Mostra os resultados ----------
  Serial.println("\n\n========================================");
  Serial.println("  ANOTE ESTES VALORES (cole no robo):");
  Serial.println("========================================\n");

  Serial.print("calib.accelBias[0] = ");
  Serial.print(calib.accelBias[0], 6); Serial.println(";");
  Serial.print("calib.accelBias[1] = ");
  Serial.print(calib.accelBias[1], 6); Serial.println(";");
  Serial.print("calib.accelBias[2] = ");
  Serial.print(calib.accelBias[2], 6); Serial.println(";");

  Serial.print("calib.gyroBias[0]  = ");
  Serial.print(calib.gyroBias[0], 6); Serial.println(";");
  Serial.print("calib.gyroBias[1]  = ");
  Serial.print(calib.gyroBias[1], 6); Serial.println(";");
  Serial.print("calib.gyroBias[2]  = ");
  Serial.print(calib.gyroBias[2], 6); Serial.println(";");

  if (hasMag) {
    Serial.print("calib.magBias[0]   = ");
    Serial.print(calib.magBias[0], 6); Serial.println(";");
    Serial.print("calib.magBias[1]   = ");
    Serial.print(calib.magBias[1], 6); Serial.println(";");
    Serial.print("calib.magBias[2]   = ");
    Serial.print(calib.magBias[2], 6); Serial.println(";");

    Serial.print("calib.magScale[0]  = ");
    Serial.print(calib.magScale[0], 6); Serial.println(";");
    Serial.print("calib.magScale[1]  = ");
    Serial.print(calib.magScale[1], 6); Serial.println(";");
    Serial.print("calib.magScale[2]  = ");
    Serial.print(calib.magScale[2], 6); Serial.println(";");
  }

  Serial.println("\n========================================");
  Serial.println("  Calibracao concluida. Copie acima.");
  Serial.println("========================================");
}

void loop() {
  // nada — calibração roda só uma vez no setup
}
