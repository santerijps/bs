#include "bs.h"
#include <stdio.h>
#include <stdlib.h>

#define _BS_DEBUG_OUT BS_DEBUG_OUT_FILE _BS_EXT
#define _BS_RELEASE_OUT BS_RELEASE_OUT_FILE _BS_EXT
#define _BS_DEBUG_BUILD_COMMAND BS_COMPILER " " BS_SOURCE_FILES " " BS_LINKER_OPTS " " BS_DEBUG_OPTS " -o " BS_OUT_DIR _BS_SEP _BS_DEBUG_OUT
#define _BS_RELEASE_BUILD_COMMAND BS_COMPILER " " BS_SOURCE_FILES " " BS_LINKER_OPTS " " BS_RELEASE_OPTS " -o " BS_OUT_DIR _BS_SEP _BS_RELEASE_OUT

#define _BS_USAGE\
    "bs - build system v0.1.0\n"\
    "Usage: bs [...flags [-r [...args]]]\n"\
    "Command line flags:\n"\
    "   -h (help)           Print this usage information\n"\
    "   -r (run)            Run the program after compiling successfully\n"\
    "   -R (release)        Compile a release build\n"\
    "   -v (verbose)        Print shell commands that are run\n"\
    "User config:\n"\
    "   Compiler:           " BS_COMPILER "\n"\
    "   Source files:       " BS_SOURCE_FILES "\n"\
    "   Linker flags:       " BS_LINKER_OPTS "\n"\
    "   Debug output:       " BS_DEBUG_OUT_FILE _BS_EXT "\n"\
    "   Debug flags:        " BS_DEBUG_OPTS "\n"\
    "   Release output:     " BS_RELEASE_OUT_FILE _BS_EXT "\n"\
    "   Release flags:      " BS_RELEASE_OPTS "\n"\
    "\n"

int main(int argc, char **argv) {
    char run_args_buffer[8192] = {0};
    char *build_command = _BS_DEBUG_BUILD_COMMAND;
    char *executable_path = _BS_DEBUG_OUT;
    char run_after_build = 0;
    char be_verbose = 0;
    int i;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j]; j++) {
                switch (argv[i][j]) {
                case 'h': fprintf(stderr, _BS_USAGE); return 0;
                case 'R':
                    build_command = _BS_RELEASE_BUILD_COMMAND;
                    executable_path = _BS_RELEASE_OUT;
                    break;
                case 'r': run_after_build = 1; break;
                case 'v': be_verbose = 1; break;
                default: fprintf(stderr, "%s: Undefined flag '%c'\n", argv[0], argv[i][j]); return 1;
                }
            }
            if (run_after_build) {
                goto concatenate_run_args;
            }
        } else {
            fprintf(stderr, "%s: Undefined argument '%s'\n", argv[0], argv[i]);
            return 1;
        }
    }

    if (run_after_build) concatenate_run_args: {
        char *run_args_ptr = (char*) &run_args_buffer;
        for (i = i + 1; i < argc; i++) {
            run_args_ptr += snprintf(run_args_ptr, sizeof(run_args_buffer), "%s ", argv[i]);
        }
    }

    _BS_MKDIR(BS_OUT_DIR);

    if (be_verbose) fprintf(stderr, "BUILD: %s\n", build_command);
    if (system(build_command)) return 1;

    if (run_after_build) {
        char run_buffer[8192] = {0};
        snprintf(run_buffer, sizeof(run_buffer), "%s %s", executable_path, run_args_buffer);

        if (be_verbose) fprintf(stderr, "  RUN: %s\n\n", run_buffer);
        if (system(run_buffer)) return 1;
    }

    return 0;
}
