# Mapeamento de pinos

| Componente | Pino do componente | Pino do ESP32 | Função |
|---|---|---|---|
| MQ-3 | VCC | 3V3 (simulação) | Alimentação |
| MQ-3 | GND | GND | Referência |
| MQ-3 | AOUT | GPIO34 (ADC1) | Leitura analógica |
| LED + resistor 220 Ω | Ânodo, via resistor | GPIO25 | Alerta visual |
| LED | Cátodo | GND | Retorno |
| Buzzer ativo | Positivo | GPIO26 | Alerta sonoro |
| Buzzer ativo | Negativo | GND | Retorno |

## Observações

- `GPIO34` pertence ao canal **ADC1** do ESP32, que continua utilizável com o Wi-Fi ativo (diferente dos pinos do canal ADC2, que ficam indisponíveis para leitura analógica enquanto o Wi-Fi está em uso).
- Na simulação, o sensor é alimentado pelo pino `3V3`. Em uma implementação física com o MQ-3 real (alimentação de 5V), a saída analógica deve passar por um divisor de tensão (por exemplo, 10 kΩ e 20 kΩ) antes de chegar ao ESP32, cujas entradas ADC suportam no máximo 3,3V.
- O ADC do ESP32 opera em 12 bits (`analogReadResolution(12)`), retornando valores de 0 a 4095.
