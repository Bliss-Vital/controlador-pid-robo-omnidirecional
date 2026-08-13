# Guia de montagem da eletrônica

Este guia mostra passo a passo como conectar todos os componentes do robô,
com tabelas de pinagem e testes de verificação a cada etapa.

**Antes de começar, você deve ter:**
- Todos os componentes da [lista de materiais](HARDWARE.md) em mãos
- O chassi mecânico já montado (rodas nos motores, motores parafusados no chassi)
- Um computador  o [Arduino IDE](https://www.arduino.cc/en/software) instalado

## Visão geral das conexões
                    ┌──────────────────┐
                    │  Raspberry Pi    │
                    │  Pico (RP2040)   │
                    │  + IO Shield     │
                    └────────┬─────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
         ┌────▼────┐    ┌────▼────┐    ┌────▼─────┐
         │  MPU-   │    │  Ponte  │    │ Bateria  │
         │  9250   │    │   H     │    │ 2×18650  │
         │  (IMU)  │    │ L9110S  │    │  (7,4V)  │
         └─────────┘    └────┬────┘    └──────────┘
              I²C            │            alimenta
                        ┌────┴────┐       tudo
                        │ 4 motores│
                        │   DC     │
                        └─────────┘
```

O **Pico** é o cérebro. Ele lê a IMU pelo I²C, calcula o PID e envia PWM para a ponte H, que aciona os motores. A bateria alimenta a ponte H (que por sua vez alimenta o Pico via o IO Shield).

---

## Fase 1 — Preparar o Pico com o IO Shield

Antes de conectar qualquer coisa, encaixe o Raspberry Pi Pico no Keyestudio IO Shield. O Pico tem um lado com **furos** e um cantinho **cortado**: esse cantinho é o pino 1. Alinhe com a marcação no Shield.

<img width="325" height="527" alt="image" src="https://github.com/user-attachments/assets/86b23848-6eac-4282-89c7-dc0dee84a0de" />


✅ **Fase 1:** o Pico deve ficar bem encaixado, sem forçar. Se você conseguir ver alguma parte dos pinos metálicos, empurre mais.

---

## Fase 2 — Conectar e testar a IMU (MPU-9250)

A IMU se comunica pelo protocolo I²C, que usa apenas 2 fios de dados (SDA e SCL) + alimentação.

### Pinagem

| Pino da IMU | Pino do Pico | Cor sugerida do jumper |
|---|---|---|
| VCC | 3V3 (OUT) | vermelho |
| GND | GND | preto |
| SDA | GP16 | azul |
| SCL | GP17 | amarelo |

⚠️ **Muito importante:** a IMU funciona em **3,3 V**, NÃO em 5 V. Se você ligar em 5 V, ela queima. No IO Shield, procure a linha rotulada `3V3`.

<img width="407" height="290" alt="image" src="https://github.com/user-attachments/assets/9495e86a-a368-4ff6-af9a-4af84cee5df6" />

### Teste da IMU

Antes de conectar os motores, vale confirmar que o Pico "enxerga" a IMU:

1. Conecte o cabo do Pico ao computador.
2. Abra o Arduino IDE, cole o código de scan I²C disponibilizado, e faça upload
3. Abra o Monitor Serial (velocidade **115200**). Você deve ver:

```
Encontrado em 0x68
```

✅ **Checkpoint 2:** apareceu `0x68`? A IMU está conectada e funcionando. Pode ir para a próxima fase.

❌ **Não apareceu nada?** Confira: (1) fios SDA/SCL não invertidos, (2) VCC em 3V3 e não em 5V, (3) GND conectado, (4) o LED da IMU está aceso.

---

## Fase 3 — Conectar a ponte H L9110S de 4 canais (fiação)

Nesta fase você só faz as ligações. **Não conecte a bateria ainda** — vamos ligar tudo por último, quando estiver tudo revisado.

A ponte H de 4 canais controla os 4 motores. Cada motor tem 2 fios de controle vindos do Pico: um "EN" (enable/direção) e um "PWM" (velocidade).

### Pinagem dos motores (retirada do código `pid_lib_sem_mag_v3.ino`)

| Motor | Posição | Pino EN do Pico | Pino PWM do Pico |
|---|---|---|---|
| A | Frente-esquerda (LF) | GP1 | GP2 |
| B | Frente-direita (RF) | GP4 | GP3 |
| C | Trás-esquerda (LB) | GP5 | GP6 |
| D | Trás-direita (RD) | GP8 | GP7 |

⚠️ Cuidado: a numeração dos motores no código (A, B, C, D) segue a ordem física frente-esquerda, frente-direita, trás-esquerda, trás-direita. Se você conectar trocado, o robô vai andar em direções aleatórias quando o PID mandar ele girar. Anote qual motor é qual **antes de parafusar tudo no chassi**.

### Ligações da ponte H

A L9110S de 4 canais tem, além dos pinos de controle acima, mais 4 conexões de alimentação:

| Pino da ponte H | Onde conectar | Quando conectar |
|---|---|---|
| Saídas MOTOR-A/B/C/D | fios dos 4 motores correspondentes | agora |
| VMOTOR (alimentação dos motores) | fio vermelho do suporte de baterias | **só na Fase 4** |
| GND (alimentação dos motores) | fio preto do suporte de baterias | **só na Fase 4** |

<img width="383" height="370" alt="image" src="https://github.com/user-attachments/assets/08f6c3ba-d6c6-4eca-abb8-019ebe56726b" />

Deixe os fios da bateria **soltos**, sem conectar em nada, até a próxima fase.

✅ **Checkpoint 3:** os 8 fios de controle (2 por motor) estão conectados entre Pico e ponte H, os 4 motores estão parafusados nas suas posições e ligados na ponte H, e os fios da bateria estão preparados mas não conectados.

---

## Fase 4 — Ligar a alimentação e testar tudo junto

Chegou a hora da verdade. Aqui a bateria entra em cena e você vai testar os motores pela primeira vez.

### Como a energia vai fluir

```
2× baterias 18650 (7,4 V total, em série)
        │
        ▼
  Suporte de baterias
        │
        ▼
  Entrada VMOTOR da ponte H  ── alimenta os 4 motores
```

⚠️ **Sobre alimentar o Pico:** durante todos os testes deste guia, o Pico fica alimentado **pelo cabo USB do computador**. Ele só precisa da bateria depois, quando o robô for rodar autônomo — e essa configuração final depende do modelo específico da sua ponte H (algumas fornecem 5V regulado, outras não). Trate isso como próximo passo, não como parte do guia de montagem.

### Antes de conectar a bateria — checklist de segurança

Antes de qualquer coisa, com a bateria ainda solta e o USB ainda desconectado do Pico, confira:

- [ ] Os 2 fios da bateria (vermelho e preto) **não estão se tocando**
- [ ] Meça com o multímetro a tensão nos terminais do suporte: deve dar entre **7,4 V e 8,4 V**
- [ ] Se der invertido (−7,4 V), você conectou trocado — **conserte antes de continuar**
- [ ] Os 4 motores estão parafusados no chassi (para não saírem correndo pela mesa)
- [ ] O robô está **suspenso**, apoiado com as rodas no ar, ou dentro de uma caixa

### Sequência de ligação (sempre nesta ordem)

1. **Conecte o USB do Pico ao computador.** O LED do Pico acende.
2. **Só agora, conecte os fios da bateria na ponte H**: fio vermelho no `VMOTOR (+)`, fio preto no `GND (−)`. O LED da ponte H deve acender.
3. Abra o Monitor Serial do Arduino IDE (velocidade **115200**), com o firmware `pid_lib_sem_mag_v3.ino` já carregado.
4. Digite **`m`** e aperte Enter.

O comando `m` faz o robô girar por 2 segundos ignorando o PID. **Os 4 motores devem girar simultaneamente**.

### Sequência de desligamento (sempre nesta ordem)

Toda vez que terminar de mexer:

1. **Primeiro desconecte a bateria** da ponte H.
2. **Depois desconecte o USB** do Pico.

Isso evita que o Pico fique alimentando circuitos que estão momentaneamente sem terra de referência da bateria.

✅ **Checkpoint 4:** os 4 motores giraram com o comando `m`? Parabéns, sua eletrônica está pronta.

❌ **Um motor não girou?** Desligue tudo (bateria primeiro, USB depois). Verifique os 2 fios EN/PWM daquele motor.

❌ **Um motor girou para o lado errado?** Desligue tudo. Inverta os 2 fios da bobina do motor (os que vão da ponte H até o motor).

❌ **Nenhum motor girou, mas o Pico e o LED da ponte H estão acesos?** A alimentação chegou, mas os sinais de controle não. Confira as conexões dos pinos EN/PWM.

❌ **A ponte H não acende o LED?** Meça a tensão nos terminais VMOTOR: se está chegando tensão da bateria e o LED não acende, a ponte H pode estar com defeito ou você inverteu polaridade em algum passo.

❌ **Você sente cheiro de queimado ou algum componente está quente ao toque?** Desconecte a bateria imediatamente. Algo está em curto — não continue até identificar o problema.

---

## Pinagem completa (referência rápida)

Guarde esta tabela para consulta:

| Função | Pino do Pico |
|---|---|
| I²C SDA (IMU) | GP16 |
| I²C SCL (IMU) | GP17 |
| Motor A — EN | GP1 |
| Motor A — PWM | GP2 |
| Motor B — EN | GP4 |
| Motor B — PWM | GP3 |
| Motor C — EN | GP5 |
| Motor C — PWM | GP6 |
| Motor D — EN | GP8 |
| Motor D — PWM | GP7 |
| Alimentação 5V | VBUS (via ponte H) |
| Terra | GND |

---
