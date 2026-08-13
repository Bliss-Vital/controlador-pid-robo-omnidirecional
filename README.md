# controlador-pid-robo-omnidirecional
Desenvolvimento e implementação de um controlador PID para um robô móvel omnidirecional, utilizando Raspberry Pi Pico (RP2040) e IMU MPU-9250 para aquisição de dados, estimativa de orientação e controle de movimento.

# Lista de materiais 

Os links levam a vendedores específicos que funcionaram para este projeto, mas o AliExpress rotaciona vendedores com frequência, então se o link
expirar use o **termo de busca** ao lado para encontrar equivalentes.

## Chassi, motores e rodas (kit único)

| Item | Qtd. | Especificação | Link |
|---|---|---|---|
| Kit chassi 4WD mecanum completo | 1 | Chassi de alumínio + 4 rodas mecanum 60 mm + 4 motores TT (3–6 V, dupla saída de eixo) + acopladores + parafusos | [AliExpress](https://pt.aliexpress.com/item/1005008396603634.html) — busca: `mecanum robot chassis 4wd tt motor` |
 Se preferir comprar peça por peça, procure separadamente por `mecanum wheel`(4 unidades, atenção aos rolos invertidos — 2 esquerda + 2 direita) e
`tt motor gear dual shaft` (4 unidades).

## Controle e sensoriamento

| Item | Qtd. | Especificação | Link |
|---|---|---|---|
| Raspberry Pi Pico W | 1 | RP2040 dual-core, 264 KB RAM. | [AliExpress](https://pt.aliexpress.com/item/1005009302191839.html) — busca: `raspberry pi pico rp2040` |
| Keyestudio Pico IO Shield | 1 | Placa de expansão que soqueta o Pico e expõe todos os pinos em terminais coloridos, facilita conexões sem solda | [Eletrônica da China](https://eletronicadachina.com.br/produto/keyestudio-raspberry-pi-pico-io-placa-escudo-para-raspberry-pi-pico-placa-de-desenvolvimento-eletronico-projetos-diy) — busca: `keyestudio pico io shield` |
| Sensor IMU MPU-9250 | 1 | 9 DoF (giroscópio + acelerômetro + magnetômetro), interface I²C, 3,3 V | [AliExpress](https://pt.aliexpress.com/item/1005006991230800.html) — busca: `mpu9250 module gy-9250` |
| Ponte H L9110S de 4 canais | 1 | Módulo com 4 canais em uma placa só, alimentação 2,5–12 V — cobre os 4 motores do robô | [AliExpress](https://pt.aliexpress.com/item/10000249812884.html) — busca: `l9110s 4 channel motor driver` |

## Alimentação

| Item | Qtd. | Especificação | Link |
|---|---|---|---|
| Bateria 18650 Li-Ion | 2 | 3,7 V nominal, 3000 mAh. O link é de um kit de 4 (você usa 2, sobram 2 reservas). | [AliExpress](https://pt.aliexpress.com/item/1005011663068083.html) — busca: `18650 battery 3000mah` |
| Suporte para 2 baterias 18650 | 1 | Formato lado a lado (série), com fios | [AliExpress](https://pt.aliexpress.com/item/1005006016303983.html) — busca: `18650 battery holder 2 slot` |
| Carregador de bateria 18650 | 1 | 2 slots, entrada micro-USB ou USB-C | busca: `18650 charger 2 slot` |


## Cabos e conexões

| Item | Qtd. | Especificação | Link |
|---|---|---|---|
| Kit de jumpers Dupont | 1 kit | 40–120 peças, 10–30 cm, 24 AWG. Inclui macho-macho, macho-fêmea e fêmea-fêmea. | [AliExpress](https://pt.aliexpress.com/item/1005003219096948.html) — busca: `dupont jumper wire kit` |

## Ferramentas (não incluídas na BOM, mas necessárias)

| Item | Especificação | Link |
|---|---|---|
| Kit de solda | Ferro de solda 80 W (110/220 V) + estanho + multímetro XL830L + acessórios. Você precisará soldar os terminais dos motores DC, se não vierem soldados. | [AliExpress](https://pt.aliexpress.com/item/1005006842091809.html) — busca: `soldering iron kit 80w multimeter` |
| Chave de fenda Phillips pequena | Para os parafusos M3 do chassi | qualquer bazar/loja de ferragens |
| Alicate de corte | Para ajustar comprimento dos fios | qualquer bazar/loja de ferragens |
| Protoboard | Para testes ou fixar o sensor no robô\

<img width="461" height="545" alt="image" src="https://github.com/user-attachments/assets/51fc51f5-581b-47dd-a6a6-acf69cabb375" />


