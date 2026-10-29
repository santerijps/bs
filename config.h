#define BS_COMPILER             "gcc"
#define BS_SOURCE_FILES         "src/hello.c"
#define BS_OUTPUT_PATH          "hello"

#define BS_DEBUG_OPTIONS        "-Wall -Wextra -Werror -pedantic"
#define BS_RELEASE_OPTIONS      "-Wall -Wextra -Werror -pedantic -O3 -s -march=native"

// TODO: changing to some directory doesn't work
#define BS_WATCH_DIRECTORY      "."

#define BS_RUN_BUFFER_SIZE      4096
#define BS_BUILD_BUFFER_SIZE    4096
