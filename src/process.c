#include "process.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 

static char last_error[256] = {0};

const char* proc_error(void) {
    return last_error[0] ? last_error : "no error";
}

static void set_error(const char* msg) {
    if (msg) strncpy(last_error, msg, sizeof(last_error) - 1);
    else last_error[0] = '\0';
}

#ifdef _WIN32
    #include <windows.h>
    
    struct proc_handle {
        HANDLE h;
    };
    
    proc_t proc_start(const char* exe, const char* const args[]) {
        if (!exe || !args) {
            set_error("null args");
            return NULL;
        }
        
        char cmd[2048] = {0};
        int pos = snprintf(cmd, sizeof(cmd), "\"%s\"", exe);
        if (pos < 0 || pos >= (int)sizeof(cmd)) {
            set_error("cmd too long");
            return NULL;
        }
        
        for (int i = 0; args[i]; i++) {
            pos += snprintf(cmd + pos, sizeof(cmd) - pos, " \"%s\"", args[i]);
            if (pos < 0 || pos >= (int)sizeof(cmd)) {
                set_error("cmd too long");
                return NULL;
            }
        }
        
        STARTUPINFO si = {0};
        PROCESS_INFORMATION pi = {0};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 
                          CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            set_error("CreateProcess failed");
            return NULL;
        }
        
        CloseHandle(pi.hThread);
        
        proc_t p = (proc_t)malloc(sizeof(struct proc_handle));
        if (!p) {
            CloseHandle(pi.hProcess);
            set_error("malloc failed");
            return NULL;
        }
        p->h = pi.hProcess;
        return p;
    }
    
    int proc_wait(proc_t p, int timeout_ms, int* exit_code) {
        if (!p) {
            set_error("invalid handle");
            return -1;
        }
        
        DWORD timeout = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
        DWORD res = WaitForSingleObject(p->h, timeout);
        
        if (res == WAIT_TIMEOUT) return 1;
        if (res != WAIT_OBJECT_0) {
            set_error("WaitForSingleObject failed");
            return -1;
        }
        
        DWORD code = 0;
        if (!GetExitCodeProcess(p->h, &code)) {
            set_error("GetExitCodeProcess failed");
            return -1;
        }
        
        if (exit_code) *exit_code = (int)code;
        return 0;
    }
    
    int proc_is_running(proc_t p) {
        if (!p) return -1;
        DWORD code = 0;
        if (!GetExitCodeProcess(p->h, &code)) return -1;
        return (code == STILL_ACTIVE) ? 1 : 0;
    }
    
    int proc_terminate(proc_t p) {
        if (!p) return 0;
        return TerminateProcess(p->h, 1) ? 1 : 0;
    }
    
    void proc_close(proc_t p) {
        if (p) {
            if (p->h) CloseHandle(p->h);
            free(p);
        }
    }
    
#else // LINUX
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <signal.h>
    #include <errno.h>
    
    struct proc_handle {
        pid_t pid;
        int status;
        int done;
    };
    
    proc_t proc_start(const char* exe, const char* const args[]) {
        if (!exe || !args) {
            set_error("null args");
            return NULL;
        }
        
        pid_t pid = fork();
        if (pid < 0) {
            set_error("fork failed");
            return NULL;
        }
        
        if (pid == 0) {
            // Дочерний процесс
            int fd = open("/dev/null", O_WRONLY);
            if (fd >= 0) {
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            execvp(exe, (char* const*)args);
            _exit(127); 
        }
        
        // Родитель
        proc_t p = (proc_t)malloc(sizeof(struct proc_handle));
        if (!p) {
            set_error("malloc failed");
            return NULL;
        }
        p->pid = pid;
        p->status = -1;
        p->done = 0;
        return p;
    }
    
    int proc_wait(proc_t p, int timeout_ms, int* exit_code) {
        if (!p) {
            set_error("invalid handle");
            return -1;
        }
        
        if (p->done) {
            if (exit_code && p->status != -1)
                *exit_code = WIFEXITED(p->status) ? WEXITSTATUS(p->status) : -1;
            return 0;
        }
        
        if (timeout_ms < 0) {
            // Бесконечное ожидание
            pid_t res = waitpid(p->pid, &p->status, 0);
            if (res < 0) {
                set_error("waitpid failed");
                return -1;
            }
            p->done = 1;
            if (exit_code)
                *exit_code = WIFEXITED(p->status) ? WEXITSTATUS(p->status) : -1;
            return 0;
        }
        
        // Ожидание с таймаутом
        int waited = 0;
        while (waited < timeout_ms) {
            pid_t res = waitpid(p->pid, &p->status, WNOHANG);
            if (res == p->pid) {
                p->done = 1;
                if (exit_code)
                    *exit_code = WIFEXITED(p->status) ? WEXITSTATUS(p->status) : -1;
                return 0;
            }
            usleep(10000); 
            waited += 10;
        }
        return 1; 
    }
    
    int proc_is_running(proc_t p) {
        if (!p) return -1;
        if (p->done) return 0;
        pid_t res = waitpid(p->pid, &p->status, WNOHANG);
        if (res == p->pid) {
            p->done = 1;
            return 0;
        }
        return (res >= 0) ? 1 : -1;
    }
    
    int proc_terminate(proc_t p) {
        if (!p || p->done) return 1;
        if (kill(p->pid, SIGKILL) != 0) return 0;
        usleep(50000); // даем время завершиться
        return 1;
    }
    
    void proc_close(proc_t p) {
        if (p) {
            if (!p->done) waitpid(p->pid, &p->status, WNOHANG);
            free(p);
        }
    }
#endif
