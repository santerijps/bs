#include <stdio.h>
#include <stdlib.h>
#include "config.h"

int main(int argc, char **argv) {
    char flag_dryrun = 0;
    char flag_release = 0;
    char flag_run = 0;
    char flag_verbose = 0;

    char build_buffer[BS_BUILD_BUFFER_SIZE] = {0};
    char run_buffer[BS_RUN_BUFFER_SIZE] = {0};
    int run_args_start_index = -1;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != 0; j++) {
                switch (argv[i][j]) {
                    case 'h':
                        fprintf(
                            stderr,
                            "[bs] usage: build [-hdrRv]\n"
                            "        -d     Dry runs the build process and prints steps to STDERR.\n"
                            "        -r     Run the program after compiling.\n"
                            "        -R     Compile a release build.\n"
                            "        -v     Print the generated commands to STDERR.\n"
                            "\n"
                        );
                        return 1;
                    case 'd':
                        flag_dryrun = 1;
                        flag_verbose = 1;
                        break;
                    case 'r':
                        flag_run = 1;
                        break;
                    case 'R':
                        flag_release = 1;
                        break;
                    case 'v':
                        flag_verbose = 1;
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

    if (flag_verbose) {
        fprintf(stderr, "\e[1;93m[bs] build: %s\n\e[0m", build_buffer);
    }

    if (!flag_dryrun) {
        int build_status = system(build_buffer);
        if (build_status != 0) {
            fprintf(stderr, "\e[1;91m[bs] error: Build failed with error %d\n\e[0m", build_status);
            return 1;
        }
    }

    if (flag_run) {
        int run_buffer_length = 0;
        int output_path_length = (sizeof(BS_OUTPUT_PATH) - 1);
        for (int i = 0; i < output_path_length; i++) {
            run_buffer[run_buffer_length++] = BS_OUTPUT_PATH[i];
        }

        for (int i = run_args_start_index; 1 < i && i < argc; i++) {
            run_buffer[run_buffer_length++] = ' ';
            for (int j = 0; argv[i][j]; j++) {
                run_buffer[run_buffer_length++] = argv[i][j];
            }
        }

        if (flag_verbose) {
            fprintf(stderr, "\e[1;93m[bs]   run: %s\n\e[0m", run_buffer);
        }

        if (!flag_dryrun) {
            int run_status = system(run_buffer);
            if (run_status != 0) {
                fprintf(stderr, "[bs] error: Run failed with error %d\n", run_status);
                return 1;
            }
        }
    }

    return 0;
}
