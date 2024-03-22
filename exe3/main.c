#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"

#define WINDOW_SIZE 5

QueueHandle_t xQueueData;
int buffer[WINDOW_SIZE] = {0}; // Initialize the fixed-sized array with zeros
int index = 0;
int sum = 0;
int count = 0;

void moving_average(int new_data) {
    // Se o buffer estiver cheio, subtrai o valor mais antigo do total
    if (count == WINDOW_SIZE) {
        sum -= buffer[index];
    } else {
        count++;
    }

    // Adiciona o novo valor ao total e ao buffer
    sum += new_data;
    buffer[index] = new_data;
    index = (index + 1) % WINDOW_SIZE;

    // Calcula e imprime a média móvel
    float average = (float) sum / count;
    printf("%2f\n", average);
}

// não mexer! Alimenta a fila com os dados do sinal
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int data_index = 0;
    int data;

    while (true) {
        int new_data = 0;
        if (xQueueReceive(xQueueData, &new_data, portMAX_DELAY)) { // Considere esperar indefinidamente ou ajustar conforme necessário
             moving_average(new_data);

            // Verificar se temos dados suficientes para calcular a média móvel
            // if (data_index == WINDOW_SIZE) {
            //     // Calcular a média móvel dos últimos WINDOW_SIZE dados
            //     float average = moving_average(data, WINDOW_SIZE);
            //     // Agora você pode usar a variável 'average' como quiser
            //     printf("Média Móvel: %.2f\n", average);
            // }
        }
        // Deixar esse delay
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;}
