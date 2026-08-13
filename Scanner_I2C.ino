#include <Wire.h>

void setup() {
  Serial.begin(115200);

  Wire.setSDA(16);   // pinos utilizados no projeto
  Wire.setSCL(17);
  Wire.begin();

  delay(2000);

  Serial.println("Scanner I2C");

  byte count = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);

    if (Wire.endTransmission() == 0) {
      Serial.print("Encontrado: 0x");
      Serial.println(address, HEX);
      count++;
    }
  }

  if (count == 0)
    Serial.println("Nenhum dispositivo encontrado.");
}

void loop() {}