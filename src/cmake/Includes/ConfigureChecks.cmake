include (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
include (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
include (${CMAKE_ROOT}/Modules/CheckCSourceCompiles.cmake)


# Check for functions
check_function_exists (sendfile HAVE_SENDFILE)
check_function_exists (getifaddrs HAVE_GETIFADDRS)
check_function_exists (inotify_init HAVE_INOTIFY)

# Check for headers
check_include_files (linux/netlink.h HAVE_NETLINK)
check_include_files (machine/endian.h HAVE_MACHINE_ENDIAN_H)
check_include_files (mach/mach_time.h HAVE_MACH_MACH_TIME_H)
check_include_files (sys/inotify.h HAVE_SYS_INOTIFY_H)
check_include_files (dirent.h HAVE_DIRENT_H)

# Check for symbols
check_symbol_exists (__NR_clock_gettime sys/syscall.h HAVE_CLOCK_GETTIME_SYSCALL)
if (NOT HAVE_INOTIFY)
    check_symbol_exists (__NR_inotify_init sys/syscall.h HAVE_INOTIFY)
endif ()

# Check for libraries
if (NOT HAVE_CLOCK_GETTIME_SYSCALL)
    check_library_exists (rt clock_gettime "" HAVE_CLOCK_GETTIME)

    if (HAVE_CLOCK_GETTIME)
        set (minidlnad_LIBS ${minidlnad_LIBS} rt)
    endif ()
endif()

if (SQLite3_FOUND)
    check_library_exists (sqlite3 sqlite3_malloc "" HAVE_SQLITE3_MALLOC)
    check_library_exists (sqlite3 sqlite3_prepare_v2 "" HAVE_SQLITE3_PREPARE_V2)
endif ()

# FFmpeg headers path configuration
if (FFmpeg_FOUND)
    foreach (ffmpeg_component ${FFMPEG_COMPONENTS})
        string (TOUPPER "HAVE_LIB${ffmpeg_component}_${ffmpeg_component}_H" HEADER_VAR)
        check_include_files ("lib${ffmpeg_component}/${ffmpeg_component}.h" ${HEADER_VAR})

        if (NOT ${HEADER_VAR})
            string (TOUPPER "HAVE_${ffmpeg_component}_H" HEADER_VAR)
            check_include_files (${ffmpeg_component}.h ${HEADER_VAR})
        endif()
    endforeach ()
endif ()

macro (try_c_compile prologue body var)
    check_c_source_compiles("
${prologue}

int main() {
    ${body}

    return 0;
}" ${var})
endmacro()

# check if scandir needs const char cast
try_c_compile("#include <stdlib.h>
               #include <sys/types.h>
               #include <dirent.h>"
              "int filter(struct dirent *d);
               struct dirent **ptr = NULL;
               char *name = NULL;
               (void)scandir(name, &ptr, filter, alphasort);" SCANDIR_CONST)

set (SCANDIR_CONST NOT SCANDIR_CONST)
if (NOT SCANDIR_CONST)
    message (STATUS "No scandir const")
endif ()

# Check for struct ip_mreqn
try_c_compile("#include <netinet/in.h>"
              "struct ip_mreqn mreq;
              mreq.imr_address.s_addr = 0;" HAVE_STRUCT_IP_MREQN)

if (NOT HAVE_STRUCT_IP_MREQN)
    # We'll just have to try and use struct ip_mreq
    try_c_compile("#include <netinet/in.h>"
                  "struct ip_mreq mreq;
                   mreq.imr_interface.s_addr = 0;" HAVE_STRUCT_IP_MREQ)

    if (NOT HAVE_STRUCT_IP_MREQ)
        message (STATUS "No multicast support")
    endif ()
endif ()

# Check if struct dirent has member d_type
if (HAVE_DIRENT_H)
    try_c_compile("#include <dirent.h>"
                  "struct dirent de;
                  de.d_type = 0;" HAVE_STRUCT_DIRENT_D_TYPE)
endif ()

# Check we should use the daemon() libc function
check_c_source_compiles("
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    return daemon(0, 0);
" USE_DAEMON)

# Check for linux sendfile support

check_c_source_compiles("
#include <sys/types.h>
#include <sys/sendfile.h>

int main(void) {
    int tofd = 0, fromfd = 0;
    off_t offset;
    size_t total = 0;
    ssize_t nwritten = sendfile(tofd, fromfd, &offset, total);
    return nwritten;
}" HAVE_LINUX_SENDFILE_API)


# check for darwin sendfile support

check_c_source_compiles("
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

int main(void) {
    int fd = 0, s = 0;
    off_t offset = 0, len;
    struct sf_hdtr *hdtr = NULL;
    int flags = 0;
    int ret;
    ret = sendfile(fd, s, offset, &len, hdtr, flags);
    return ret;
}" HAVE_DARWIN_SENDFILE_API)

# check for freebsd sendfile support

check_c_source_compiles("
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

int main(void) {
    int fromfd=0, tofd=0, ret, total=0;
    off_t offset=0, nwritten;
    struct sf_hdtr hdr;
    struct iovec hdtrl;
    hdr.headers = &hdtrl;
    hdr.hdr_cnt = 1;
    hdr.trailers = NULL;
    hdr.trl_cnt = 0;
    hdtrl.iov_base = NULL;
    hdtrl.iov_len = 0;
    ret = sendfile(fromfd, tofd, offset, total, &hdr, &nwritten, 0);
}" HAVE_FREEBSD_SENDFILE_API)