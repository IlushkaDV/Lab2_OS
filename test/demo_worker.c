#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

void show_progress(int steps, const char* label) {
    printf("%s [", label);
    fflush(stdout);
    for (int i = 0; i < steps; i++) {
        printf("=");
        fflush(stdout);
        SLEEP_MS(150);
    }
    printf("] DONE\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("demo_worker\n");
        printf("Использование:\n");
        printf("  demo_worker count N    - считает до N с анимацией\n");
        printf("  demo_worker calc A B   - складывает два числа\n");
        printf("  demo_worker exit CODE  - завершается с указанным кодом\n");
        printf("  demo_worker hello      - приветствие с анимацией\n");
        return 0;
    }

    const char* cmd = argv[1];

    if (strcmp(cmd, "count") == 0 && argc >= 3) {
        int n = atoi(argv[2]);
        if (n < 1) n = 1;
        if (n > 20) n = 20; 
        
        printf("Счётчик запущен: ");
        fflush(stdout);
        
        for (int i = 1; i <= n; i++) {
            printf("%d ", i);
            fflush(stdout);
            SLEEP_MS(200);
        }
        printf("\n✅ Счёт завершён до %d\n", n);
        return 0;
    }

    if (strcmp(cmd, "calc") == 0 && argc >= 4) {
        int a = atoi(argv[2]);
        int b = atoi(argv[3]);
        int sum = a + b;
        
        printf("Вычисление: %d + %d = %d\n", a, b, sum);
        SLEEP_MS(300);
        printf("✅ Результат: %d\n", sum);
        return 0;
    }

    if (strcmp(cmd, "exit") == 0 && argc >= 3) {
        int code = atoi(argv[2]);
        printf("Завершение с кодом %d через 1 секунду...\n", code);
        SLEEP_MS(1000);
        return code;
    }

    if (strcmp(cmd, "hello") == 0) {
        printf("Приветствие от процесса:\n");
        SLEEP_MS(200);
        show_progress(10, "Загрузка модуля приветствия");
        SLEEP_MS(200);
        printf("👋 Привет! Я дочерний процесс!\n");
        SLEEP_MS(200);
        printf("🌍 Работаю на %s\n", 
        #ifdef _WIN32
            "Windows"
        #else
            "Linux/POSIX"
        #endif
        );
        return 0;
    }

    printf("Неизвестная команда: %s\n", cmd);
    return 127;
}