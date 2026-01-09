////////////////////////
// DON'T CHANGE THESE //
////////////////////////

#ifdef _WIN32
    #define _BS_EXT ".exe"
    #define _BS_SEP "\\"

    #include <direct.h>
    #define _BS_MKDIR(path) _mkdir(path)
#else
    #define _BS_EXT ""
    #define _BS_SEP "/"

    #include <sys/stat.h>
    #include <sys/types.h>
    #define _BS_MKDIR(path) mkdir(path, 0755)
#endif

////////////////////////////////
// CHANGE THESE AS YOU PLEASE //
////////////////////////////////

// Which compiler to use (e.g. gcc, clang, tcc).
// A full path can also be provided.
#define BS_COMPILER "gcc"

// Which source files should be included in the build.
// The value should be a space-separated string of .c/.cpp files.
// The asterisk (*) can also be used (e.g. src/*.c).
#define BS_SOURCE_FILES "src" _BS_SEP "*.c"

// Flags to be passed to the linker.
#define BS_LINKER_OPTS ""

// The directory into which the compiled executables are placed.
// The directory will be created if it doesn't exist.
// The value shouldn't end with a path separator (e.g. "/" or "\\").
#define BS_OUT_DIR "bin"

// Compiler flags to be used in debug builds.
#define BS_DEBUG_OPTS "-Wall -Wextra -pedantic -Og"

// The name of the compiled debug executable.
// The .exe file extension will be appended automatically, and is not needed here.
#define BS_DEBUG_OUT_FILE "app.debug"

// Compiler flags to be used in release builds.
#define BS_RELEASE_OPTS "-Wall -Wextra -Werror -pedantic -DNDEBUG -O3 -s"

// The name of the compiled release executable.
// The .exe file extension will be appended automatically, and is not needed here.
#define BS_RELEASE_OUT_FILE "app"
