#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "ws2818b.pio.h"  

// Configurações da matriz de LEDs WS2812
#define LED_COUNT   25      // Total de LEDs na matriz (5x5)
#define LED_PIN     7       // Pino conectado à matriz WS2812 (GPIO 7)

// Configurações do LED RGB
#define LED_RGB_R   13      // Pino do LED vermelho (deve piscar)
#define LED_RGB_G   11      // Pino do LED verde (não utilizado)
#define LED_RGB_B   12      // Pino do LED azul (não utilizado)

// Configurações dos botões
#define BUTTON_A    5       // Botão A (para incrementar o número)
#define BUTTON_B    6       // Botão B (para decrementar o número)

// Tempo de debounce (em microssegundos)
// 200 ms = 200000 us
#define DEBOUNCE_TIME 200000

// Definição de um pixel no formato GRB (cada componente em 8 bits)
typedef struct pixel_t {
    uint8_t G, R, B;
} pixel_t;

typedef pixel_t npLED_t;  // Renomeia pixel_t para npLED_t para clareza

// Variáveis globais
volatile int a = 0;                  // Valor atual do dígito (0 a 9)
volatile bool button_flag_A = false; // Flag para botão A (incrementa)
volatile bool button_flag_B = false; // Flag para botão B (decrementa)
static volatile uint32_t last_time_A = 0;  // Último instante para botão A (debounce)
static volatile uint32_t last_time_B = 0;  // Último instante para botão B (debounce)

// Buffer de pixels que compõe a matriz de LEDs
npLED_t leds[LED_COUNT];

// Variáveis para uso do PIO
PIO np_pio;
uint sm;  // Máquina de estados (State Machine)

//*****************************************************************************
// Função: npInit
// Descrição: Inicializa a máquina PIO para controle dos LEDs WS2812
//*****************************************************************************
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true);  // Se nenhuma estiver livre, aborta
    }
    
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    
    // Inicializa o buffer de pixels (todos apagados)
    for (uint i = 0; i < LED_COUNT; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

//*****************************************************************************
// Função: npSetLED
// Descrição: Atribui uma cor (RGB) a um LED do buffer
//*****************************************************************************
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

//*****************************************************************************
// Função: npClear
// Descrição: Limpa o buffer de pixels (apaga os LEDs)
//*****************************************************************************
void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

//*****************************************************************************
// Função: npWrite
// Descrição: Escreve os dados do buffer nos LEDs WS2818B
//*****************************************************************************
void npWrite() {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);  // Aguarda 100 us para o RESET dos LEDs (conforme datasheet)
}

//*****************************************************************************
// Definição da matriz de cores para os números de 0 a 9
// Cada número é representado por uma matriz 5x5 com valores RGB (no formato GRB)
//*****************************************************************************
const int numeros[10][5][5][3] = {
    // Número 0
    {
        {{0, 0, 0},   {136, 0, 159}, {136, 0, 159}, {136, 0, 159}, {0, 0, 0}},
        {{0, 0, 0},   {136, 0, 159}, {0, 0, 0},     {136, 0, 159}, {0, 0, 0}},
        {{0, 0, 0},   {136, 0, 159}, {0, 0, 0},     {136, 0, 159}, {0, 0, 0}},
        {{0, 0, 0},   {136, 0, 159}, {0, 0, 0},     {136, 0, 159}, {0, 0, 0}},
        {{0, 0, 0},   {136, 0, 159}, {136, 0, 159}, {136, 0, 159}, {0, 0, 0}}
    },
    // Número 1
    {
        {{0, 0, 0}, {0, 244, 255}, {0, 244, 255}, {0, 244, 255}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},       {0, 244, 255}, {0, 0, 0},       {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},       {0, 244, 255}, {0, 0, 0},       {0, 0, 0}},
        {{0, 0, 0}, {0, 244, 255}, {0, 244, 255}, {0, 0, 0},       {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},       {0, 244, 255}, {0, 0, 0},       {0, 0, 0}}
    },
    // Número 2
    {
        {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {0, 0, 0},   {0, 0, 0},   {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},   {0, 0, 0},   {255, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {0, 0, 0}}
    },
    // Número 3
    {
        {{0, 0, 0}, {15, 0, 255}, {15, 0, 255}, {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 0, 0},      {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {15, 0, 255}, {15, 0, 255}, {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 0, 0},      {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {15, 0, 255}, {15, 0, 255}, {15, 0, 255}, {0, 0, 0}}
    },
    // Número 4
    {
        {{0, 0, 0}, {251, 255, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 0, 0}, {251, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {251, 255, 0}, {251, 255, 0}, {251, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {251, 255, 0}, {0, 0, 0}, {251, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {251, 255, 0}, {0, 0, 0}, {251, 255, 0}, {0, 0, 0}}
    },
    // Número 5
    {
        {{0, 0, 0}, {255, 135, 0}, {255, 135, 0}, {255, 135, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},       {0, 0, 0},      {255, 135, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 135, 0}, {255, 135, 0}, {255, 135, 0}, {0, 0, 0}},
        {{0, 0, 0}, {255, 135, 0}, {0, 0, 0},      {0, 0, 0},     {0, 0, 0}},
        {{0, 0, 0}, {255, 135, 0}, {255, 135, 0}, {255, 135, 0}, {0, 0, 0}}
    },
    // Número 6
    {
        {{0, 0, 0}, {251, 0, 255}, {251, 0, 255}, {251, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {251, 0, 255}, {0, 0, 0},      {251, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {251, 0, 255}, {251, 0, 255}, {251, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {251, 0, 255}, {0, 0, 0},      {0, 0, 0},     {0, 0, 0}},
        {{0, 0, 0}, {251, 0, 255}, {251, 0, 255}, {251, 0, 255}, {0, 0, 0}}
    },
    // Número 7
    {
        {{0, 0, 0}, {0, 0, 0},      {0, 244, 255}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 244, 255}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 244, 255}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 0, 0},     {0, 244, 255}, {0, 0, 0}},
        {{0, 0, 0}, {0, 244, 255},      {0, 244, 255}, {0, 244, 255}, {0, 0, 0}}
    },
    // Número 8
    {
        {{0, 0, 0}, {83, 255, 0},  {83, 255, 0}, {83, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {83, 255, 0},  {0, 0, 0},    {83, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {83, 255, 0},  {83, 255, 0}, {83, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {83, 255, 0},  {0, 0, 0},    {83, 255, 0}, {0, 0, 0}},
        {{0, 0, 0}, {83, 255, 0},  {83, 255, 0}, {83, 255, 0}, {0, 0, 0}}
    },
    // Número 9
    {
        {{0, 0, 0}, {15, 0, 255}, {15, 0, 255}, {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0},      {0, 0, 0},      {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {15, 0, 255}, {15, 0, 255}, {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {15, 0, 255}, {0, 0, 0},      {15, 0, 255}, {0, 0, 0}},
        {{0, 0, 0}, {15, 0, 255}, {15, 0, 255}, {15, 0, 255}, {0, 0, 0}}
    }
};

//*****************************************************************************
// Função: getIndex
// Descrição: Retorna a posição linear do LED na matriz com base em (linha, coluna)
//             Assumindo uma matriz 5x5
//*****************************************************************************
int getIndex(int linha, int coluna) {
    return linha * 5 + coluna;
}

//*****************************************************************************
// Função: exibirNumero
// Descrição: Exibe um dígito (0 a 9) na matriz de LEDs utilizando a matriz "numeros"
//*****************************************************************************
void exibirNumero(int num) {
    npClear();
    if (num < 0 || num > 9) return;  // Garante que o número esteja no intervalo
    
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao,
                     numeros[num][linha][coluna][0],
                     numeros[num][linha][coluna][1],
                     numeros[num][linha][coluna][2]);
        }
    }
    npWrite();
}

//*****************************************************************************
// Função: gpio_irq_handler (callback único para ambos os botões)
//*****************************************************************************
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A) {
        if (current_time - last_time_A > DEBOUNCE_TIME) {
            last_time_A = current_time;
            button_flag_A = true;
        }
    } else if (gpio == BUTTON_B) {
        if (current_time - last_time_B > DEBOUNCE_TIME) {
            last_time_B = current_time;
            button_flag_B = true;
        }
    }
}

//*****************************************************************************
// Função principal
//*****************************************************************************
int main() {
    stdio_init_all();
    
    // Inicializa a matriz WS2812 na GPIO 7
    npInit(LED_PIN);
    npClear();
    npWrite();
    
    // Configura os botões com interrupção
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    
    // Registra UM único callback para ambos os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true);
    
    // Configura o LED vermelho do LED RGB (GPIO 13) como saída
    gpio_init(LED_RGB_R);
    gpio_set_dir(LED_RGB_R, GPIO_OUT);
    
    // Exibe o número inicial (0) na matriz WS2812
    exibirNumero(a);
    
    // Loop principal:
    // - Pisca o LED vermelho a 5 Hz (100 ms ligado, 100 ms desligado)
    // - Verifica as flags dos botões para atualizar a exibição do número
    while (true) {
        // Pisca o LED vermelho
        gpio_put(LED_RGB_R, 1);
        sleep_ms(100);
        gpio_put(LED_RGB_R, 0);
        sleep_ms(100);
        
        // Se o botão A foi pressionado, incrementa o dígito
        if (button_flag_A) {
            button_flag_A = false;
            a = (a + 1) % 10;  // Incrementa o valor cíclicamente (0 a 9)
            exibirNumero(a);
        }
        
        // Se o botão B foi pressionado, decrementa o dígito
        if (button_flag_B) {
            button_flag_B = false;
            if (a == 0)
                a = 9;
            else
                a--;
            exibirNumero(a);
        }
    }
    
    return 0;
}
