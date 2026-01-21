#include <stdio.h>
#include <omp.h>
int main(int argc, char* argv[]) {  /* off_funcoes.c */
    int num_devices, default_device, current_device;
    int initial_device_flag;
    // Obtém o número de dispositivos alvo disponíveis
    num_devices = omp_get_num_devices();
    printf("Número de dispositivos alvo disponíveis: %d\n", num_devices);
    if (num_devices > 0) {
        // Obtém o dispositivo padrão atual
        default_device = omp_get_default_device();
        printf("Dispositivo padrão atual: %d\n", default_device);
        // Define um novo dispositivo padrão (se houver mais de um)
        if (num_devices > 1) {
            int new_default_device = (default_device + 1) % num_devices;
            omp_set_default_device(new_default_device);
            printf("Dispositivo padrão definido para: %d\n", omp_get_default_device());
        }
        // Executa uma região target para verificar em qual dispositivo a tarefa está rodando
        #pragma omp target map(tofrom: initial_device_flag)
        {
            // Verifica se a tarefa está sendo executada no dispositivo hospedeiro
            initial_device_flag = omp_is_initial_device();
            // Obtém o número do dispositivo em que a thread está executando
            current_device = omp_get_device_num();
            printf("Dentro da região target - Dispositivo de execução: %d, É dispositivo inicial? %s\n",
                   current_device, initial_device_flag ? "Sim" : "Não");
        }
        printf("Após a região target - Valor do dispositivo inicial é retornado: %s\n",
               initial_device_flag ? "Sim" : "Não");
    } else {
        printf("Nenhum dispositivo alvo disponível.\n");
    }
    // Verifica o status no dispositivo hospedeiro
    initial_device_flag = omp_is_initial_device();
    printf("Fora da região target (no host) - É dispositivo inicial? %s\n",
           initial_device_flag ? "Sim" : "Não");
    printf("Fora da região target (no host) - Dispositivo atual: %d\n", omp_get_device_num()); // No host, geralmente retorna 0
    return 0;
}
