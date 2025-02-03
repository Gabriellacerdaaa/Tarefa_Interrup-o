# Display de Dígitos com Matriz WS2812B


Projeto para exibir dígitos (0-9) em uma matriz de LEDs WS2812B (5x5) usando o Raspberry Pi Pico (BitDogLab). O sistema permite alterar o dígito com botões e indica atividade com um LED RGB (canal vermelho).

---

## 📋 Recursos

- **Display de Números:** Exibe dígitos com mapeamento 5x5 colorido.
- **Botões com Interrupção:** Incremento (GPIO 5) e decremento (GPIO 6) com debounce.
- **Indicador Visual:** LED vermelho (GPIO 13) pisca a 5 Hz.
- **Controle via PIO:** Comunicação com os LEDs WS2812B utilizando a máquina de estados.

---

## ⚙️ Hardware

- **Placa:** BitDogLab / Raspberry Pi Pico  
- **Matriz WS2812B (5x5)**
- **Botões:** GPIO 5 (A) e GPIO 6 (B)
- **LED RGB:** Canal vermelho no GPIO 13  
- *(Outros pinos: LED_WS2812B no GPIO 7)*

---


## 🚀 Funcionamento

- **Inicialização:** Exibe o dígito `0` na matriz.
- **Botão A:** Incrementa o dígito (0 → 9, cíclico).
- **Botão B:** Decrementa o dígito (9 → 0, cíclico).
- **LED Vermelho:** Pisca para indicar atividade.

---

## 📄 Licença

[MIT License](LICENSE)

---

Desenvolvido por **[GABRIEL SANTOS DE LACERDA]**  
Estudande de engenharia elétrica - UFOB
😀🚀
