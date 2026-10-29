#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "config.h"

int build_program(void);
int run_program(void);
int watch_file_changes(void);
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
                        fprintf(stderr, "\e[1;91m[bs] error: Invalid argument: -%c\n\e[0m", argv[i][j]);
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
        fprintf(stderr, "\e[1;91m[bs] error: Failed to write to build buffer. Try increasing BS_COMMAND_BUILD_BUFFER_SIZE.\n\e[0m");
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
        return watch_file_changes();
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
    if (flag_verbose) {
        fprintf(stderr, "\e[1;93m[bs] build: %s\n\e[0m", build_buffer);
    }

    int build_status = system(build_buffer);
    if (build_status != 0) {
        fprintf(stderr, "\e[1;91m[bs] error: Build failed with error %d\n\e[0m", build_status);
        return 1;
    }

    return 0;
}

int run_program(void) {
    if (flag_verbose) {
        fprintf(stderr, "\e[1;93m[bs]   run: %s\n\e[0m", run_buffer);
    }

    int run_status = system(run_buffer);

    if (flag_verbose) {
        fprintf(stderr, "\e[1;93m[bs]  exit: %d\n\e[0m", run_status);
    }

    return run_status;
}

int watch_file_changes(void) {
    clear_terminal();

    int build_result = build_program();
    if (build_result == 0 && flag_run) {
        run_program();
    }

#ifdef WIN32
    HANDLE change_handle = FindFirstChangeNotificationW((LPCWSTR)BS_WATCH_DIRECTORY, TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (change_handle == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[bs] error: FindFirstChangeNotificationW failed! Error code: %lu\n\e[0m", GetLastError());
        return 1;
    }

    DWORD last_changed = 0;

    for (;;) {
        DWORD wait_result = WaitForSingleObject(change_handle, 200);
        if (wait_result == WAIT_OBJECT_0) {
            if (GetTickCount() - last_changed >= 200) {
                clear_terminal();

                int build_result = build_program();
                if (build_result == 0 && flag_run) {
                    // TODO: If the program is still running, it should be killed
                    run_program();
                }

                last_changed = GetTickCount();
            }
        }

        if (!FindNextChangeNotification(change_handle)) {
            fprintf(stderr, "[bs] error: FindNextChangeNotification failed! Error code: %lu\n\e[0m", GetLastError());
            return 1;
        }
    }

    FindCloseChangeNotification(change_handle);
#else
    fprintf(stderr, "[bs] error: Watch mode is not implemented for this platform!\n");
#endif

    return 0;
}

inline void clear_terminal(void) {
    fwrite("\x1b[2J\x1b[0;0H", 1, 11, stdout);
}
