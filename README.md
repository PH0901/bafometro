# Bafômetro Inteligente

Projeto acadêmico de Objetos Inteligentes Conectados (Universidade Presbiteriana Mackenzie, Faculdade de Computação e Informática), Grupo 10: Pedro H Fernandes, Thomas G. Simplicio e Gabriel D Aurichi, sob orientação do Prof. André Luis de Oliveira.

O projeto busca contribuir com a meta 3.6 do ODS 3 da ONU (redução de mortes e feridos em acidentes de trânsito). Um bafômetro conectado, baseado em microcontrolador **ESP32** e sensor de gás **MQ-3**, identifica situações de suspeita de embriaguez e transmite essa informação em tempo real, via **Wi-Fi** e protocolo **MQTT**, a uma central de monitoramento — apoiando a identificação mais ágil de condutores alcoolizados.

Todo o protótipo é simulado no [Wokwi](https://wokwi.com), um simulador online de circuitos eletrônicos que executa o firmware real (C/C++ sobre o framework Arduino) com suporte a Wi-Fi e comunicação com a internet.

## 1. Descrição do funcionamento e uso

1. O ESP32 inicializa os atuadores (LED e buzzer desligados), conecta-se à rede Wi-Fi (no Wokwi, a rede `Wokwi-GUEST`) e ao broker MQTT público [HiveMQ](https://www.hivemq.com/) (`broker.hivemq.com`, porta 1883).
2. A cada ciclo (~1s), o firmware lê o sensor MQ-3 (média de 8 amostras, para reduzir ruído) pelo pino analógico `GPIO34` (canal ADC1, compatível com o uso simultâneo do Wi-Fi).
3. O valor lido é comparado a um limiar calibrado (`LIMIAR`, em `firmware/wifi-scan.ino`). Acima do limiar, o LED vermelho (`GPIO25`) e o buzzer ativo (`GPIO26`) são acionados.
4. O ESP32 publica uma mensagem JSON no tópico MQTT `transito/bafometro/leitura`, no formato:
   ```json
   {"id":"bafometro01","valor":2310,"alerta":true}
   ```
5. Uma central de monitoramento (no ambiente de testes, o cliente web do próprio HiveMQ) assina esse tópico e recebe as leituras em tempo real.

**Para reproduzir:**
1. Acesse [wokwi.com](https://wokwi.com), crie um novo projeto ESP32.
2. Substitua o `diagram.json` do projeto pelo arquivo em [`firmware/diagram.json`](firmware/diagram.json) e o sketch pelo arquivo em [`firmware/wifi-scan.ino`](firmware/wifi-scan.ino).
3. Instale a biblioteca `PubSubClient` (por Nick O'Leary) — veja [`firmware/libraries.txt`](firmware/libraries.txt).
4. Inicie a simulação, ajuste o controle "GAS (PPM)" do sensor para variar a concentração simulada.
5. Para ver as mensagens chegando, abra o [cliente web do HiveMQ](http://www.hivemq.com/demos/websocket-client/), conecte em `broker.hivemq.com`, porta `8884` (WebSocket/SSL), e assine o tópico `transito/bafometro/leitura`.

## 2. Software e documentação de código

O firmware está em [`firmware/wifi-scan.ino`](firmware/wifi-scan.ino), escrito em C/C++ sobre o framework Arduino. Principais funções:

| Função | Responsabilidade |
|---|---|
| `conectarWiFi()` | Conecta o ESP32 à rede Wi-Fi, com retry em loop até sucesso |
| `conectarMQTT()` | Conecta (ou reconecta) ao broker MQTT com um client ID único |
| `lerSensor()` | Lê o ADC do MQ-3 e calcula a média de `N_AMOSTRAS` leituras |
| `atuadores(bool alerta)` | Liga/desliga o LED e o buzzer de acordo com o estado de alerta |
| `setup()` | Inicializa pinos, Wi-Fi, MQTT e o pré-aquecimento simulado do sensor |
| `loop()` | Garante reconexão de Wi-Fi/MQTT, lê o sensor a cada `INTERVALO_MS` e publica o JSON |

Parâmetros ajustáveis no topo do arquivo: pinos (`PIN_MQ3`, `PIN_LED`, `PIN_BUZZER`), `LIMIAR` de detecção, `N_AMOSTRAS`, `PREAQUECER_MS`, `INTERVALO_MS`, credenciais Wi-Fi e configuração do broker MQTT (`MQTT_HOST`, `MQTT_PORT`, `MQTT_TOPIC`, `DEVICE_ID`).

## 3. Descrição do hardware

Plataforma de prototipagem: **ESP32 DevKit V1**, simulado no Wokwi (nenhuma peça física/impressão 3D utilizada — protótipo inteiramente simulado nesta fase).

| Componente | Papel no projeto | Pino do ESP32 |
|---|---|---|
| ESP32 DevKit V1 | Microcontrolador: lê o sensor, decide o alerta, publica via MQTT | — |
| Sensor de gás MQ-3 (simulado via `wokwi-gas-sensor`) | Sensor: detecta a concentração de álcool no ar | `GPIO34` (AOUT, ADC1) |
| LED vermelho + resistor 220 Ω | Atuador: alerta visual | `GPIO25` |
| Buzzer ativo | Atuador: alerta sonoro | `GPIO26` |

Mapeamento completo de pinos em [`docs/pinagem.md`](docs/pinagem.md).

> **Observação:** o Wokwi não possui um modelo nativo do sensor MQ-3; a simulação usa o componente genérico `wokwi-gas-sensor` (originalmente modelado a partir do MQ-2), que reproduz a mesma interface elétrica (VCC, GND, saída analógica e saída digital com comparador) como equivalente funcional para fins de prototipagem.

## 4. Interfaces, protocolos e módulos de comunicação

- **Wi-Fi (TCP/IP):** o ESP32 possui Wi-Fi 802.11 b/g/n nativo — não há módulo de comunicação serial-USB intermediário (como um Arduino UNO + Firmata); o próprio microcontrolador conecta-se diretamente à rede.
- **MQTT:** protocolo de publicação/assinatura sobre TCP/IP, usado para o envio dos dados à central de monitoramento. Biblioteca utilizada: [PubSubClient](https://github.com/knolleary/pubsubclient) (Nick O'Leary).
  - Broker: `broker.hivemq.com` (público, porta 1883 para o firmware; porta 8884/WebSocket-SSL para o cliente web de monitoramento)
  - Tópico: `transito/bafometro/leitura`
  - Payload: JSON `{"id": string, "valor": int, "alerta": bool}`
  - QoS: 0

Mais detalhes do payload e do fluxo de mensagens em [`docs/protocolo-mqtt.md`](docs/protocolo-mqtt.md).

## 5. Confirmação do requisito de comunicação via internet

✅ O projeto possui comunicação/controle via internet (TCP/IP) e uso do protocolo MQTT, conforme exigido: o ESP32 publica os dados do sensor em tempo real para um broker MQTT acessível pela internet (`broker.hivemq.com`), e uma central de monitoramento (cliente MQTT externo) recebe essas mensagens de qualquer lugar com acesso à internet.

## Estrutura do repositório

```
bafometro/
├── README.md
├── firmware/
│   ├── wifi-scan.ino       # firmware do ESP32
│   ├── diagram.json        # circuito do Wokwi
│   └── libraries.txt       # dependências (PubSubClient)
└── docs/
    ├── pinagem.md          # mapeamento completo de pinos
    └── protocolo-mqtt.md   # detalhes do protocolo MQTT e payload
```

## Artigo acadêmico

A documentação completa do projeto (introdução, materiais e métodos, resultados, conclusões e referências em formato ABNT) está no artigo entregue à disciplina.
