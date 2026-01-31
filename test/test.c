#include "../include/process.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #define WORKER "demo_worker.exe"
    #define SLEEP_MS(ms) Sleep(ms)
#else
    #define WORKER "./demo_worker"
    #define SLEEP_MS(ms) usleep((ms)*1000)
#endif

void print_separator() {
    printf("\n%s\n", "----------------------------------------");
}

void demo_counting() {
    printf("\n[ДЕМО 1] Запуск счётчика в фоновом режиме\n");
    print_separator();
    
    const char* args[] = {"count", "10", NULL};
    proc_t p = proc_start(WORKER, args);
    
    if (!p) {
        printf("❌ Ошибка запуска: %s\n", proc_error());
        return;
    }
    
    printf("✅ Процесс запущен (фоновый режим)\n");
    printf("⏳ Основная программа продолжает работу...\n");
    
    // Имитируем работу основной программы
    for (int i = 0; i < 3; i++) {
        printf("   ... основная программа работает (%d/3) ...\n", i+1);
        SLEEP_MS(500);
    }
    
    printf("⏳ Ожидаем завершения дочернего процесса...\n");
    int code = -1;
    int res = proc_wait(p, -1, &code);
    
    if (res == 0) {
        printf("✅ Дочерний процесс завершился с кодом %d\n", code);
    } else {
        printf("❌ Ошибка ожидания: %s\n", proc_error());
    }
    
    proc_close(p);
}

void demo_calculation() {
    printf("\n[ДЕМО 2] Параллельные вычисления\n");
    print_separator();
    
    // Запускаем два процесса одновременно
    const char* args1[] = {"calc", "15", "27", NULL};
    const char* args2[] = {"calc", "100", "250", NULL};
    
    proc_t p1 = proc_start(WORKER, args1);
    proc_t p2 = proc_start(WORKER, args2);
    
    if (!p1 || !p2) {
        printf("❌ Ошибка запуска: %s\n", proc_error());
        if (p1) proc_close(p1);
        if (p2) proc_close(p2);
        return;
    }
    
    printf("✅ Запущено 2 процесса для параллельных вычислений\n");
    printf("   Процесс 1: 15 + 27\n");
    printf("   Процесс 2: 100 + 250\n");
    
    // Ждём оба процесса
    int code1 = -1, code2 = -1;
    printf("⏳ Ожидание завершения обоих процессов...\n");
    
    if (proc_wait(p1, -1, &code1) == 0) {
        printf("✅ Процесс 1 завершился с кодом %d\n", code1);
    }
    
    if (proc_wait(p2, -1, &code2) == 0) {
        printf("✅ Процесс 2 завершился с кодом %d\n", code2);
    }
    
    proc_close(p1);
    proc_close(p2);
}

void demo_timeout() {
    printf("\n[ДЕМО 3] Ожидание с таймаутом\n");
    print_separator();
    
    const char* args[] = {"count", "15", NULL};
    proc_t p = proc_start(WORKER, args);
    
    if (!p) {
        printf("❌ Ошибка запуска: %s\n", proc_error());
        return;
    }
    
    printf("✅ Запущен процесс (считает до 15, ~3 секунды)\n");
    printf("⏳ Ждём только 1 секунду (таймаут)...\n");
    
    int res = proc_wait(p, 1000, NULL);
    
    if (res == 1) {
        printf("⏰ Таймаут! Процесс всё ещё работает\n");
        
        // Проверяем статус
        int status = proc_is_running(p);
        if (status == 1) {
            printf("🔍 Статус: процесс активен\n");
        }
        
        // Дожидаемся завершения
        printf("⏳ Дожидаемся полного завершения...\n");
        proc_wait(p, -1, NULL);
        printf("✅ Процесс успешно завершился\n");
    } else {
        printf("❌ Неожиданный результат: %d\n", res);
    }
    
    proc_close(p);
}

void demo_exit_codes() {
    printf("\n[ДЕМО 4] Проверка кодов возврата\n");
    print_separator();
    
    int test_codes[] = {0, 1, 42};
    for (int i = 0; i < 3; i++) {
        char code_str[8];
        sprintf(code_str, "%d", test_codes[i]);
        const char* args[] = {"exit", code_str, NULL};
        
        printf("Тест кода %d: ", test_codes[i]);
        fflush(stdout);
        
        proc_t p = proc_start(WORKER, args);
        int exit_code = -1;
        proc_wait(p, 1500, &exit_code);
        
        if (exit_code == test_codes[i]) {
            printf("✅ получен %d\n", exit_code);
        } else {
            printf("❌ ожидался %d, получен %d\n", test_codes[i], exit_code);
        }
        
        proc_close(p);
    }
}

void demo_hello() {
    printf("\n[ДЕМО 5] Приветствие от дочернего процесса\n");
    print_separator();
    
    const char* args[] = {"hello", NULL};
    proc_t p = proc_start(WORKER, args);
    
    if (!p) {
        printf("❌ Ошибка запуска: %s\n", proc_error());
        return;
    }
    
    printf("✅ Запущен процесс с анимацией приветствия\n");
    printf("⏳ Ожидание завершения...\n");
    
    proc_wait(p, -1, NULL);
    printf("✅ Процесс завершился успешно\n");
    
    proc_close(p);
}

int main() {
    printf("  НАГЛЯДНАЯ ДЕМОНСТРАЦИЯ БИБЛИОТЕКИ ПРОЦЕССОВ          \n");
    printf("  Платформа: %40s  \n", 
    #ifdef _WIN32
        "Windows"
    #else
        "Linux/POSIX"
    #endif
    );
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    demo_hello();
    demo_counting();
    demo_calculation();
    demo_timeout();
    demo_exit_codes();
    
    printf("    ВСЕ ДЕМОНСТРАЦИИ ЗАВЕРШЕНЫ УСПЕШНО                \n");    
    return 0;
}