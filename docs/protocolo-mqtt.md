# Protocolo MQTT

## Visão geral

O MQTT (Message Queuing Telemetry Transport) é um protocolo leve de publicação/assinatura sobre TCP/IP, amplamente usado em soluções de Internet das Coisas por seu baixo consumo de banda e energia — características adequadas a dispositivos embarcados conectados via Wi-Fi, como o ESP32 deste projeto.

## Configuração utilizada

| Parâmetro | Valor |
|---|---|
| Broker | `broker.hivemq.com` (público) |
| Porta (firmware, MQTT puro) | `1883` |
| Porta (cliente web, WebSocket/SSL) | `8884` |
| Tópico | `transito/bafometro/leitura` |
| QoS | `0` |
| Retain | não |
| Biblioteca (firmware) | [PubSubClient](https://github.com/knolleary/pubsubclient) (Nick O'Leary) |

## Formato da mensagem (payload)

Cada mensagem publicada é um objeto JSON:

```json
{
  "id": "bafometro01",
  "valor": 2310,
  "alerta": true
}
```

| Campo | Tipo | Descrição |
|---|---|---|
| `id` | string | Identificador único do dispositivo, permitindo escalar a solução para múltiplos bafômetros reportando à mesma central |
| `valor` | int | Leitura do ADC do sensor MQ-3 (0–4095) |
| `alerta` | bool | `true` quando `valor` ultrapassa o limiar calibrado (`LIMIAR` no firmware); `false` caso contrário |

## Fluxo de comunicação

1. O ESP32 conecta-se ao broker com um `clientId` único (`bafometro01-<hex aleatório>`), gerado a cada (re)conexão.
2. A cada ciclo de leitura (`INTERVALO_MS`, padrão 1000 ms), o ESP32 publica uma mensagem no tópico `transito/bafometro/leitura`.
3. A central de monitoramento (no ambiente de testes, o [cliente web do HiveMQ](http://www.hivemq.com/demos/websocket-client/)) assina esse mesmo tópico e recebe cada mensagem em tempo real, junto com o horário de recebimento.
4. Caso a conexão Wi-Fi ou MQTT caia, o firmware tenta reconectar automaticamente (ver `conectarWiFi()` e `conectarMQTT()` em `firmware/wifi-scan.ino`) sem interromper a leitura local do sensor — o alerta visual/sonoro continua funcionando mesmo sem conectividade; apenas o envio remoto fica temporariamente pausado até a reconexão.

## Limitação conhecida

O broker público HiveMQ utilizado nos testes não possui autenticação nem criptografia — adequado para fins de prototipagem e demonstração, mas inadequado para uma aplicação real de fiscalização, que exigiria um broker privado, autenticação por credenciais e conexão criptografada (TLS).
