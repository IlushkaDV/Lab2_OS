#ifndef PROCESS_H
#define PROCESS_H

typedef struct proc_handle* proc_t;

proc_t proc_start(const char* exe, const char* const args[]);

int proc_wait(proc_t p, int timeout_ms, int* exit_code);

int proc_is_running(proc_t p);

int proc_terminate(proc_t p);

void proc_close(proc_t p);

const char* proc_error(void);

#endif
