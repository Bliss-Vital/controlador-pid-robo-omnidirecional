/*
 * pid_lib_sem_mag_v3.ino — PID na ORIENTACAO PADRAO
 * ==================================================
 * v3: sensor remontado com a placa PARA CIMA,
 *     X para FRENTE, Y para a ESQUERDA (orientacao padrao).
 *     -> SEM inversao de eixos (removida a correcao antiga).
 *     Mantem da v2: correcao do PWM (so reaplica quando muda)
 *     e o comando 'm' de teste manual dos motores.
 *
 * Usa a biblioteca "PID by Brett Beauregard" (PID_v1).
 * IMU: so accel + giro (updateIMU, 6 eixos) — sem magnetometro.
 * Calibracao accel/giro automatica ao ligar.
 * PID controla Rt (rotacao) -> move_robot move as rodas.
 *
 * Truque do yaw circular (salto 360->0):
 *   Setpoint = 0 (fixo)
 *   Input    = erro normalizado em [-180, 180]
 *   Output   = correcao -> vira Rt
 *
 * OBS: sem magnetometro o YAW deriva devagar. Bom para trajetos
 * curtos; em trajetos longos a direcao escorrega.
 *
 * Bibliotecas: FastIMU, MadgwickAHRS, PID_v1
 * Pinos IMU: SDA->GP16  SCL->GP17 | Serial 115200
 *
 * ===== COMANDOS =====
 *   a -> ARMA (guarda yaw atual como alvo, liga o PID)
 *   s -> DESARMA (para os motores)
 *   v -> liga/desliga a impressao dos dados
 *   m -> teste manual dos motores (gira 2s, ignora o PID)
 *   + / - -> velocidade de avanco Ly
 *   q/z -> Kp +/-   w/x -> Ki +/-   e/c -> Kd +/-
 *   k -> mostra ganhos
 */

#include "FastIMU.h"
#include <MadgwickAHRS.h>
#include <PID_v1.h>
#include <Wire.h>

// ===================== IMU =====================
#define IMU_ADDRESS 0x68
MPU9250  IMU;
Madgwick filter;
calData  calib = { 0 };
AccelData accelData;
GyroData  gyroData;
float yaw = 0;

// ===================== MOTORES (pinagem testada) =====================
const int motorA_EN = 1;  const int motorA_PWM = 2;  // Frente esquerda
const int motorB_EN = 4;  const int motorB_PWM = 3;  // Frente direita
const int motorC_EN = 5;  const int motorC_PWM = 6;  // Tras esquerda
const int motorD_EN = 8;  const int motorD_PWM = 7;  // Tras direita
int N = 110;
int LeftFront = 0, RightFront = 0, LeftBack = 0, RightBack = 0;

// ===================== PID (biblioteca) =====================
// Variaveis que a biblioteca usa (precisam ser double)
double pidInput;          // erro de yaw normalizado
double pidOutput;         // correcao calculada
double pidSetpoint = 0;   // alvo do erro = 0

double Kp = 2.0, Ki = 0.0, Kd = 0.1;   // ganhos de partida

// Cria o PID. DIRECT = saida sobe quando input sobe.
PID meuPID(&pidInput, &pidOutput, &pidSetpoint, Kp, Ki, Kd, DIRECT);

float setpointYaw = 0;    // direcao alvo (definida ao armar)
bool  pidLigado = false;
bool  printOn   = false;
double Ly = 0.0;          // avanco (0 na bancada)

// Mantem o erro em [-180, 180] (trata o salto 360->0)
float normalizaErro(float e) {
  while (e >  180) e -= 360;
  while (e < -180) e += 360;
  return e;
}

void setup() {
  // ---- Motores ----
  pinMode(motorA_EN, OUTPUT);  pinMode(motorA_PWM, OUTPUT);
  pinMode(motorB_EN, OUTPUT);  pinMode(motorB_PWM, OUTPUT);
  pinMode(motorC_EN, OUTPUT);  pinMode(motorC_PWM, OUTPUT);
  pinMode(motorD_EN, OUTPUT);  pinMode(motorD_PWM, OUTPUT);
  pararTudo();

  // ---- I2C / Serial ----
  Wire.setSDA(16);
  Wire.setSCL(17);
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 3000) { ; }

  Serial.println("===== PID v3 (orientacao padrao, sem mag) =====");

  // ---- Inicia IMU ----
  if (IMU.init(calib, IMU_ADDRESS) != 0) {
    Serial.println("ERRO ao iniciar IMU!");
    while (true) { ; }
  }

  // ---- Calibracao automatica accel/giro ----
  Serial.println("Mantenha o robo PARADO! Calibrando em 5s...");
  delay(5000);
  Serial.println("Calibrando... NAO MEXA!");
  IMU.calibrateAccelGyro(&calib);
  delay(1000);
  IMU.init(calib, IMU_ADDRESS);
  filter.begin(100);

  // ---- Configura o PID ----
  meuPID.SetMode(AUTOMATIC);          // liga o calculo
  meuPID.SetOutputLimits(-100, 100);  // faixa da saida (casa com Rt -1..1)
  meuPID.SetSampleTime(10);           // 10 ms = 100 Hz

  Serial.println("Pronto!");
  Serial.println(">>> ROBO SUSPENSO para a bancada <<<");
  Serial.println("Comandos: a=armar  s=parar  v=dados  m=teste motores  k=ganhos");
  Serial.println("Sintonia: q/z=Kp  w/x=Ki  e/c=Kd");
  mostraGanhos();
}

void loop() {
  tratarComandos();

  static uint32_t nextUpdate = 0;
  uint32_t now = millis();
  if (now >= nextUpdate) {
    nextUpdate = now + 10;

    // ---- Le IMU ----
    IMU.update();
    IMU.getAccel(&accelData);
    IMU.getGyro(&gyroData);

    // ---- SEM correcao de eixos (orientacao padrao) ----
    filter.updateIMU(
      gyroData.gyroX,   gyroData.gyroY,   gyroData.gyroZ,
      accelData.accelX, accelData.accelY, accelData.accelZ
    );
    yaw = filter.getYaw();

    // ---- PID ----
    if (pidLigado) {
      // Input da biblioteca = erro normalizado
      pidInput = normalizaErro(setpointYaw - yaw);

      meuPID.Compute();   // a biblioteca calcula pidOutput

      // Output vira Rt (escala para faixa de move_robot ~ -1..1)
      float Rt = pidOutput / 100.0;
      if (Rt >  1) Rt =  1;
      if (Rt < -1) Rt = -1;

      // So reaplica aos motores se o comando mudou de verdade.
      // Evita reescrever o PWM 100x/s, o que picota o sinal
      // (motor apita mas nao gira).
      static float ultimoRt = 999;   // forca a 1a aplicacao
      static double ultimoLy = -1;
      if (abs(Rt - ultimoRt) > 0.02 || Ly != ultimoLy) {
        move_robot(Ly, 0, Rt);
        ultimoRt = Rt;
        ultimoLy = Ly;
      }

      static uint32_t nextPrint = 0;
      if (printOn && millis() >= nextPrint) {
        nextPrint = millis() + 100;
        Serial.print("Yaw: ");     Serial.print(yaw, 1);
        Serial.print(" | Erro: "); Serial.print(pidInput, 1);
        Serial.print(" | Out: ");  Serial.print(pidOutput, 1);
        Serial.print(" | Rt: ");   Serial.println(pidOutput / 100.0, 2);
      }
    } else {
      static uint32_t nextPrint2 = 0;
      if (printOn && millis() >= nextPrint2) {
        nextPrint2 = millis() + 200;
        Serial.print("[DESARMADO] Yaw: ");
        Serial.println(yaw, 1);
      }
    }
  }
}

// ===================== COMANDOS =====================
void tratarComandos() {
  if (!Serial.available()) return;
  char c = Serial.read();
  switch (c) {
    case 'a':
      setpointYaw = yaw;
      pidLigado = true;
      meuPID.SetMode(AUTOMATIC);
      Serial.print(">> ARMADO! Alvo de yaw = ");
      Serial.println(setpointYaw, 1);
      break;
    case 's':
      pidLigado = false;
      pararTudo();
      Serial.println(">> DESARMADO. Motores parados.");
      break;
    case 'v':
      printOn = !printOn;
      Serial.println(printOn ? ">> Impressao de dados LIGADA" : ">> Impressao de dados DESLIGADA");
      break;
    case 'm':
      // TESTE MANUAL: gira 2s (ignora o PID)
      Serial.println(">> TESTE MANUAL: girando 2s...");
      pidLigado = false;
      move_robot(0, 0, -1);
      delay(2000);
      pararTudo();
      Serial.println(">> Teste manual encerrado. Motores parados.");
      break;
    case '+': Ly += 0.1; if (Ly > 1) Ly = 1; Serial.print("Ly="); Serial.println(Ly,1); break;
    case '-': Ly -= 0.1; if (Ly < 0) Ly = 0; Serial.print("Ly="); Serial.println(Ly,1); break;
    case 'q': Kp += 0.1; atualizaGanhos(); break;
    case 'z': Kp -= 0.1; if (Kp < 0) Kp = 0; atualizaGanhos(); break;
    case 'w': Ki += 0.01; atualizaGanhos(); break;
    case 'x': Ki -= 0.01; if (Ki < 0) Ki = 0; atualizaGanhos(); break;
    case 'e': Kd += 0.01; atualizaGanhos(); break;
    case 'c': Kd -= 0.01; if (Kd < 0) Kd = 0; atualizaGanhos(); break;
    case 'k': mostraGanhos(); break;
  }
}

// Atualiza os ganhos na biblioteca em tempo real
void atualizaGanhos() {
  meuPID.SetTunings(Kp, Ki, Kd);
  mostraGanhos();
}

void mostraGanhos() {
  Serial.print("Kp="); Serial.print(Kp, 2);
  Serial.print("  Ki="); Serial.print(Ki, 2);
  Serial.print("  Kd="); Serial.println(Kd, 2);
}

void pararTudo() {
  setMotor(motorA_EN, motorA_PWM, 0);
  setMotor(motorB_EN, motorB_PWM, 0);
  setMotor(motorC_EN, motorC_PWM, 0);
  setMotor(motorD_EN, motorD_PWM, 0);
}

// ===================== FUNCOES TESTADAS (nao alterar) =====================
void setMotor(int enPin, int pwmPin, int velocidade) {
  int veloAbs = abs(velocidade);
  if (velocidade > 0) {
    analogWrite(enPin, 0);
    analogWrite(pwmPin, veloAbs);
  } else if (velocidade < 0) {
    analogWrite(enPin, veloAbs);
    analogWrite(pwmPin, 0);
  } else {
    analogWrite(pwmPin, 0);
    analogWrite(enPin, 0);
  }
}

void move_robot(float Ly, float Lx, float Rt) {
  float LF = Ly + Lx + Rt;
  float RF = Ly - Lx - Rt;
  float LB = Ly - Lx + Rt;
  float RB = Ly + Lx - Rt;

  float ALF = abs(LF), ARF = abs(RF), ALB = abs(LB), ARB = abs(RB);
  float maior = max(max(ALF, ARF), max(ALB, ARB));
  float Multi = (maior <= 1) ? (255 - N) : (255 - N) / maior;

  LeftFront  = (LF > 0) ? (int)(LF * Multi + N) : (LF < 0) ? (int)(LF * Multi - N) : 0;
  RightFront = (RF > 0) ? (int)(RF * Multi + N) : (RF < 0) ? (int)(RF * Multi - N) : 0;
  LeftBack   = (LB > 0) ? (int)(LB * Multi + N) : (LB < 0) ? (int)(LB * Multi - N) : 0;
  RightBack  = (RB > 0) ? (int)(RB * Multi + N) : (RB < 0) ? (int)(RB * Multi - N) : 0;

  setMotor(motorA_EN, motorA_PWM, LeftFront);
  setMotor(motorB_EN, motorB_PWM, RightFront);
  setMotor(motorC_EN, motorC_PWM, LeftBack);
  setMotor(motorD_EN, motorD_PWM, RightBack);
}
