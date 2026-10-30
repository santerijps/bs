#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "config.h"

#define RED(x) "\e[1;91m" x "\e[0m"
#define YELLOW(x) "\e[1;93m" x "\e[0m"

#define PRINT_VERBOSE(type, fmt, ...)\
    if (flag_verbose) {\
        fprintf(stderr, YELLOW("[bs] " type ": " fmt "\n"), ## __VA_ARGS__);\
    }

#define PRINT_ERROR(fmt, ...) fprintf(stderr, RED("[bs] error: " fmt "\n"), ## __VA_ARGS__)

int build_program(void);
int run_program(void);
int watch_mode(void);
void clear_terminal(void);

char flag_release = 0;
char flag_run = 0;
char flag_verbose = 0;
char flag_watch = 0;

char build_buffer[BS_BUILD_BUFFER_SIZE] = {0};
char run_buffer[BS_RUN_BUFFER_SIZE] = {0};

int main(int argc, char **argv) {
    int run_args_start_index = -1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != 0; j++) {
                switch (argv[i][j]) {
                    case 'h':
                        fprintf(
                            stderr,
                            "[bs] usage: build [-hdrRv]\n"
                            "        -r     Run the program after compiling.\n"
                            "        -R     Compile a release build.\n"
                            "        -v     Print the generated commands to STDERR.\n"
                            "        -w     Watch mode.\n"
                            "\n"
                        );
                        return 1;
                    case 'r':
                        flag_run = 1;
                        break;
                    case 'R':
                        flag_release = 1;
                        break;
                    case 'v':
                        flag_verbose = 1;
                        break;
                    case 'w':
                        flag_watch = 1;
                        break;
                    default:
                        PRINT_ERROR("Invalid argument: -%c", argv[i][j]);
                        return 1;
                }
            }
        }
        else {
            run_args_start_index = i;
            break;
        }
    }

    if (0 > snprintf(
        build_buffer, BS_BUILD_BUFFER_SIZE - 1,
        "%s %s -o %s %s", BS_COMPILER,
        flag_release ? BS_RELEASE_OPTIONS : BS_DEBUG_OPTIONS,
        BS_OUTPUT_PATH, BS_SOURCE_FILES
    )) {
        PRINT_ERROR("Failed to write to build buffer. Try increasing BS_COMMAND_BUILD_BUFFER_SIZE.");
        return 1;
    }

    if (flag_run) {
        int run_buffer_length = 0;
        int output_path_length = sizeof(BS_OUTPUT_PATH) - 1;

        for (int i = 0; i < output_path_length; i++) {
            run_buffer[run_buffer_length++] = BS_OUTPUT_PATH[i];
        }

        for (int i = run_args_start_index; 1 < i && i < argc; i++) {
            run_buffer[run_buffer_length++] = ' ';
            for (int j = 0; argv[i][j]; j++) {
                run_buffer[run_buffer_length++] = argv[i][j];
            }
        }
    }

    if (flag_watch) {
        clear_terminal();
        return watch_mode();
    }

    int build_result = build_program();
    if (build_result != 0) {
        return build_result;
    }

    if (flag_run) {
        int run_result = run_program();
        if (run_result != 0) {
            return run_result;
        }
    }

    return 0;
}

int build_program(void) {
    PRINT_VERBOSE("BUILD", "%s", build_buffer);

    int build_status = system(build_buffer);
    if (build_status != 0) {
        PRINT_ERROR("Build failed with error %d", build_status);
        return 1;
    }

    return 0;
}

int run_program(void) {
    PRINT_VERBOSE("  RUN", "%s", run_buffer);
    int run_status = system(run_buffer);
    PRINT_VERBOSE(" EXIT", "%d", run_status);
    return run_status;
}

int watch_mode(void) {
#ifdef WIN32
    STARTUPINFO startup_info = {0};
    PROCESS_INFORMATION process_information = {0};
    DWORD ts_last_write = 0;
    BOOL exit_code_was_checked = FALSE;

    // BUILD AND RUN INITIALLY
    int build_result = build_program();
    if (build_result == 0 && flag_run) {
        PRINT_VERBOSE("  RUN", "%s", run_buffer);
        if (!CreateProcessA(
            NULL, run_buffer, NULL, NULL,
            FALSE, 0, NULL, NULL,
            &startup_info, &process_information
        )) {
            PRINT_ERROR("CreateProcessA failed: %lu", GetLastError());
            return 1;
        }
    }

    HANDLE change_notification_handle = FindFirstChangeNotificationW((LPCWSTR)BS_WATCH_DIRECTORY, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (change_notification_handle == INVALID_HANDLE_VALUE) {
        PRINT_ERROR("FindFirstChangeNotificationW failed: %lu", GetLastError());
        return 1;
    }

    do {
        // PRINT PROCESS EXIT CODE IF THE PROCESS HAS FINISHED
        if (build_result == 0 && flag_run && flag_verbose && !exit_code_was_checked) {
            switch (WaitForSingleObject(process_information.hProcess, 0)) {
                case WAIT_FAILED:
                    PRINT_ERROR("WaitForSingleObject failed: %lu", GetLastError());
                    return 1;

                case WAIT_OBJECT_0: {
                    DWORD exit_code;
                    if (!GetExitCodeProcess(process_information.hProcess, &exit_code)) {
                        PRINT_ERROR("GetExitCodeProcess failed: %lu", GetLastError());
                        return 1;
                    }
                    PRINT_VERBOSE(" EXIT", "%lu", exit_code);
                    exit_code_was_checked = TRUE;
                    break;
                }
            }
        }

        // POLL FOR CHANGE NOTIFICATIONS
        switch (WaitForSingleObject(change_notification_handle, 200)) {
            case WAIT_FAILED:
                PRINT_ERROR("WaitForSingleObject failed: %lu", GetLastError());
                return 1;

            case WAIT_OBJECT_0:
                // RECOMPILE ONLY EVERY 200 MS
                if (GetTickCount() - ts_last_write < 200) {
                    continue;
                }
                break;

            default:
                continue;
        }

        clear_terminal();

        // HANDLE EXISTING PROCESS
        if (process_information.hProcess != NULL) {
            switch (WaitForSingleObject(process_information.hProcess, 0)) {
                case WAIT_FAILED:
                    PRINT_ERROR("WaitForSingleObject failed: %lu", GetLastError());
                    return 1;

                // PROCESS IS STILL RUNNING? KILL IT
                case WAIT_TIMEOUT:
                    if (!TerminateProcess(process_information.hProcess, EXIT_FAILURE)) {
                        PRINT_ERROR("TerminateProcess failed: %lu", GetLastError());
                        return 1;
                    }
                    if (WaitForSingleObject(process_information.hProcess, INFINITE) == WAIT_FAILED) {
                        PRINT_ERROR("WaitForSingleObject failed: %lu", GetLastError());
                        return 1;
                    }
                    break;
            }

            ZeroMemory(&startup_info, sizeof(startup_info));
            ZeroMemory(&process_information, sizeof(process_information));
        }

        // COMPILE AND RUN THE PROGRAM
        build_result = build_program();
        if (build_result == 0 && flag_run) {
            PRINT_VERBOSE("  RUN", "%s", run_buffer);
            if (!CreateProcessA(
                NULL, run_buffer, NULL, NULL,
                FALSE, 0, NULL, NULL,
                &startup_info, &process_information
            )) {
                PRINT_ERROR("CreateProcessA failed: %lu", GetLastError());
                return 1;
            }
            exit_code_was_checked = FALSE;
        }

        // ONLY ONCE THE PROGRAM HAS BE COMPILED AND RUN DO WE UPDATE THE TIMESTAMP
        ts_last_write = GetTickCount();

    } while (FindNextChangeNotification(change_notification_handle));

    PRINT_ERROR("FindNextChangeNotification failed: %lu", GetLastError());

    if (!FindCloseChangeNotification(change_notification_handle)) {
        PRINT_ERROR("FindCloseChangeNotification failed: %lu", GetLastError());
        return 1;
    }
#else
    fprintf(stderr, "[bs] error: Watch mode is not implemented for this platform!\n");
#endif
    return 0;
}

inline void clear_terminal(void) {
    fwrite("\x1b[2J\x1b[0;0H", 1, 11, stdout);
}
