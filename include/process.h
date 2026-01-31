#ifndef PROCESS_H
#define PROCESS_H

// Дескриптор процесса (скрытая структура)
typedef struct proc_handle* proc_t;

// Запуск процесса
// args - массив аргументов, последний элемент должен быть NULL
proc_t proc_start(const char* exe, const char* const args[]);

// Ожидание завершения с таймаутом (мс), -1 = бесконечно
// Возвращает: 0 - завершился, 1 - таймаут, -1 - ошибка
int proc_wait(proc_t p, int timeout_ms, int* exit_code);

// Проверка, работает ли процесс
int proc_is_running(proc_t p);

// Принудительное завершение
int proc_terminate(proc_t p);

// Освобождение ресурсов
void proc_close(proc_t p);

// Последняя ошибка
const char* proc_error(void);

#endif