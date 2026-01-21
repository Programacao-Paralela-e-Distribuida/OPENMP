#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Definição para garantir inteiros de 64 bits
typedef long long int int64;

#pragma omp declare target
int is_prime(int64 n) {
    if (n <= 1) return 0;
    if (n == 2 || n == 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    
    // Otimização: Loop 6k +/- 1
    for (int64 i = 5; i * i <= n; i += 6) {
         if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}
#pragma omp end declare target

#define GPU_WORKLOAD 90 // Percentual de trabalho para a GPU

int main(int argc, char *argv[]) { /* off_primos.c */
    int64 num_primes_gpu = 0, num_primes_cpu = 0, num_primes = 0, gpu_end;
    int64 n = (argc < 2) ? 100000000 : strtoll(argv[1], NULL, 10);
    double start_time, end_time;

    int num_devices = omp_get_num_devices();
    int default_dev = omp_get_default_device();
    
    printf("Calculando primos até %lld\n", n);
    printf("Dispositivos OpenMP disponíveis: %d (Default: %d)\n", num_devices, default_dev);

    // CORREÇÃO: Cast para long long antes da multiplicação para evitar overflow
    gpu_end = (n * (int64)GPU_WORKLOAD) / 100;
    
    // Garante que o fim da GPU seja ímpar para a lógica do passo 2 funcionar
    if (gpu_end % 2 == 0) gpu_end--; 

    start_time = omp_get_wtime();

    // --- GPU (Assíncrono) ---
    #pragma omp target teams distribute parallel for reduction(+:num_primes_gpu) nowait
    for (int64 i = 3; i <= gpu_end; i += 2) { 
        if (is_prime(i)) num_primes_gpu++;
    }

    // --- CPU (Processa o restante enquanto GPU trabalha) ---
    // A CPU começa imediatamente após o fim da GPU (próximo ímpar)
    int64 cpu_start = gpu_end + 2;
    
    #pragma omp parallel for reduction(+:num_primes_cpu) schedule(dynamic, 10000)
    for (int64 j = cpu_start; j <= n; j += 2) {
        if (is_prime(j)) num_primes_cpu++;
    }

    // Aguarda as tarefas terminarem
    #pragma omp taskwait

    // Total = GPU + CPU + 1 (O número 2, único par primo)
    num_primes = num_primes_gpu + num_primes_cpu + 1;

    end_time = omp_get_wtime();

    printf("Total primos GPU: %lld | Total primos CPU: %lld\n", num_primes_gpu, num_primes_cpu);
    printf("Total de primos até %lld: %lld\n", n, num_primes);
    printf("Tempo: %lf segundos\n", end_time - start_time);
    
    return 0;
}
