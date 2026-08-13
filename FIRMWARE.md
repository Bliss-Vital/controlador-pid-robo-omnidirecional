# Guia do firmware

Este projeto foi desenvolvido na plataforma **Arduino IDE** e roda no
**Raspberry Pi Pico (RP2040)**. Os códigos estão escritos em C++ com a
API do Arduino, o que torna o firmware compatível com qualquer placa
suportada pelo core do Pico para Arduino.

---

## Plataforma e ambiente

- **IDE:** [Arduino IDE 2.x](https://www.arduino.cc/en/software)
- **Placa:** Raspberry Pi Pico ou Pico W (RP2040)
- **Core Arduino para RP2040:** [arduino-pico](https://github.com/earlephilhower/arduino-pico) do Earle Philhower
- **Linguagem:** C++ (com API do Arduino)
- **Velocidade do Monitor Serial:** 115200 baud

### Como instalar o core do Pico no Arduino IDE

1. Abra o Arduino IDE.
2. Vá em `Arquivo → Preferências`.
3. No campo "URLs adicionais para Gerenciador de Placas", cole:
```
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
```
4. Vá em `Ferramentas → Placa → Gerenciador de Placas`, procure por "Pico" e instale **Raspberry Pi Pico/RP2040** por Earle Philhower.
5. Depois de instalado, selecione a placa em `Ferramentas → Placa → Raspberry Pi RP2040 Boards → Raspberry Pi Pico` (ou Pico W, conforme o seu).

---

## Bibliotecas necessárias

Todas as bibliotecas abaixo podem ser instaladas pelo próprio Arduino
IDE em `Ferramentas → Gerenciar Bibliotecas` (basta buscar pelo nome).

| Biblioteca | Autor | Para que serve neste projeto |
|---|---|---|
| **Wire** | Arduino (nativa) | Comunicação I²C com o sensor MPU-9250. Já vem instalada com o Arduino IDE, não precisa baixar. |
| **FastIMU** | LiquidCGS | Interface de alto nível com o MPU-9250 (e outros sensores IMU). Cuida da leitura dos registros, conversão de unidades brutas para físicas e calibração automática de acelerômetro e giroscópio. |
| **MadgwickAHRS** | Arduino | Implementa o filtro Madgwick, um algoritmo de fusão sensorial que combina os dados do acelerômetro e giroscópio (e opcionalmente magnetômetro) para estimar a orientação do robô (ângulos de roll, pitch e yaw). |
| **PID_v1** | Brett Beauregard | Biblioteca clássica de controle PID em C++. Faz toda a matemática do controle proporcional-integral-derivativo, incluindo saturação de saída, tempo de amostragem fixo e ajuste de ganhos em tempo de execução. |

💡 **Como instalar cada uma:** no Arduino IDE, vá em `Ferramentas → Gerenciar Bibliotecas`, digite o nome no campo de busca, clique na biblioteca correta e depois em `Instalar`.

---

## O que cada código faz

Cada arquivos `.ino` tem um papel específico no projeto. Eles foram escritos para serem usados **em
sequência**, do mais simples ao mais completo.

### 1. `Scanner_I2C.ino` — diagnóstico de fiação

**O que faz:** varre todos os endereços do barramento I²C (0x01 a 0x7F) e imprime no Monitor Serial quais dispositivos responderam.

**Quando usar:** logo após ligar a IMU pela primeira vez, para confirmar que a fiação (SDA no GP16, SCL no GP17) está correta e que o sensor está sendo detectado.

**O que você deve ver no Monitor Serial:**
```
Scanner I2C
Encontrado: 0x68
```

O endereço `0x68` é o endereço padrão do MPU-9250. Se aparecer, sua fiação está OK e você pode passar para o próximo código. Se não aparecer nada, revise as conexões da IMU seguindo o [guia de montagem](MONTAGEM.md).

**Bibliotecas usadas:** apenas `Wire` (nativa).

---

### 2. `calibrar_accel_giro_magn.ino` — calibração dos sensores

**O que faz:** executa a calibração dos sensores da IMU e imprime os valores de offset (bias) que devem ser aplicados nas leituras subsequentes.

- Primeiro calibra o **acelerômetro e o giroscópio** com o robô parado e nivelado.
- Depois calibra o **magnetômetro** com o movimento de "figura de 8" no ar.

**Por que calibrar:** sensores MEMS têm pequenos vieses de fabricação. Sem calibração, o giroscópio parado marca uma velocidade angular que não existe, e o robô "gira sozinho" mesmo sem comando. A calibração mede esses vieses e permite subtraí-los das leituras.

**Quando usar:** uma única vez, logo após montar o robô, ou sempre que trocar a IMU de posição no chassi. Os valores obtidos servem de referência para os outros códigos.

**Bibliotecas usadas:** `Wire`, `FastIMU`.

📝 **Nota:** o código PID atual (`pid_lib_sem_mag_v3.ino`) faz sua **própria calibração automática** de acelerômetro e giroscópio ao ligar. Portanto, este calibrador é útil principalmente para inspeção didática dos valores e para uso futuro quando o magnetômetro for incorporado ao PID.

---

### 3. `imu_accel_giro.ino` — leitura da orientação (sem controle)

**O que faz:** lê a IMU a 100 Hz, aplica o filtro Madgwick de fusão sensorial (usando apenas acelerômetro e giroscópio) e imprime os ângulos de **roll**, **pitch** e **yaw** no Monitor Serial.

**Quando usar:** para verificar visualmente que a estimativa de orientação está funcionando **antes** de acoplar o controle PID. Se você inclinar o robô para um lado, o roll deve mudar. Se inclinar para frente/trás, o pitch. Se girar, o yaw.

**O que esperar:**
- Robô plano sobre a mesa: roll e pitch devem ficar próximos de 0°.
- Yaw acumula erro lentamente com o tempo (deriva), porque neste código não há magnetômetro para corrigir.

**Bibliotecas usadas:** `Wire`, `FastIMU`, `MadgwickAHRS`.

---

### 4. `pid_lib_sem_mag_v3.ino` — código principal (PID + motores)

**O que faz:** este é o **código principal do projeto**. Junta tudo:
- Lê a IMU a 100 Hz com filtro Madgwick (sem magnetômetro).
- Recebe comandos pelo Monitor Serial para armar/desarmar o PID.
- Calcula o erro de yaw e aplica o controle PID para manter a orientação.
- Envia comandos PWM aos 4 motores via ponte H L9110S.

**Comandos disponíveis no Monitor Serial (115200 baud):**

| Tecla | O que faz |
|---|---|
| `a` | ARMA o PID (guarda o yaw atual como alvo e liga o controle) |
| `s` | DESARMA (para os motores) |
| `v` | Liga/desliga a impressão de dados de debug |
| `m` | Teste manual dos motores (gira 2s, ignora o PID) |
| `+` / `-` | Aumenta/diminui a velocidade de avanço Ly |
| `q` / `z` | Aumenta/diminui o ganho Kp |
| `w` / `x` | Aumenta/diminui o ganho Ki |
| `e` / `c` | Aumenta/diminui o ganho Kd |
| `k` | Mostra os ganhos atuais |

**Bibliotecas usadas:** `Wire`, `FastIMU`, `MadgwickAHRS`, `PID_v1`.
---

## Ordem sugerida de uso

1. **`Scanner_I2C.ino`** → confirma que a IMU responde.
2. **`imu_accel_giro.ino`** → confirma que a estimativa de orientação funciona.
3. **`pid_lib_sem_mag_v3.ino`** com o comando `m` → confirma que os motores giram (sem envolver o PID ainda).
4. **`pid_lib_sem_mag_v3.ino`** com o comando `a` → arma o PID e faz a sintonia dos ganhos com `q/z/w/x/e/c`.

O `calibrar_accel_giro_magn.ino` pode ser usado a qualquer momento para inspecionar os valores de bias, mas não é obrigatório para rodar o PID, porque o código principal já faz sua própria calibração de accel e giro ao ligar.

---

## Estado atual e próximos passos

**Versão atual do PID (`v3`):** usa 6 eixos (acelerômetro + giroscópio). O magnetômetro não é utilizado.

**Consequência:** o ângulo de yaw tem uma **deriva lenta ao longo do tempo**, porque depende apenas da integração da velocidade angular do giroscópio, sem correção absoluta. Para trajetos curtos (poucos segundos a um minuto), a deriva é aceitável. Para trajetos longos, o robô perde a referência de direção.

**Próxima versão prevista:** incorporar o magnetômetro (usando `updateAHRS` no lugar de `updateIMU` na biblioteca Madgwick), o que dará uma referência absoluta de norte magnético e eliminará a deriva. O calibrador de magnetômetro (`calibrar_accel_giro_magn.ino`) já está pronto para essa integração.
