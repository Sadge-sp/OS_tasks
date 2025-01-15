#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <wchar.h> 
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#ifdef _WIN32
int launch(const wchar_t *process) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    char process_mb[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, process, -1, process_mb, sizeof(process_mb), NULL, NULL);

    if (!CreateProcessA(process_mb, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess failed (%d).\n", GetLastError());
        return -1; 
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    if (!GetExitCodeProcess(pi.hProcess, &exit_code)) {
        printf("GetExitCodeProcess failed (%d).\n", GetLastError());
        return -1; 
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return exit_code; 
}
#else
int launch(const char *process) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return -1;
    } else if (pid == 0) {
        execlp(process, process, (char *)NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status); 
        } else {
            return -1;
        }
    }
}
#endif
