#ifndef SNOWFLAKE_PLATFORM_H
#define SNOWFLAKE_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef _WIN32
// Windows
#define STDCALL __stdcall
#include <windows.h>
#include <direct.h>
#include <time.h>

typedef HANDLE SF_THREAD_HANDLE;
typedef CONDITION_VARIABLE SF_CONDITION_HANDLE;
typedef CRITICAL_SECTION SF_CRITICAL_SECTION_HANDLE;
typedef SRWLOCK SF_RWLOCK_HANDLE;
typedef HANDLE SF_MUTEX_HANDLE;

#define PATH_SEP '\\'
#define ALTER_PATH_SEP '/'
//On windows MAX_PATH is defined as 255

#else
#define STDCALL
// Get uname output for Linux and MacOSX
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <pthread.h>

typedef pthread_t SF_THREAD_HANDLE;
typedef pthread_cond_t SF_CONDITION_HANDLE;
typedef pthread_mutex_t SF_CRITICAL_SECTION_HANDLE;
typedef pthread_rwlock_t SF_RWLOCK_HANDLE;
typedef pthread_mutex_t SF_MUTEX_HANDLE;

#define PATH_SEP '/'
#define MAX_PATH PATH_MAX

#endif

// Defined for #pragma warning messages.
#define MACRO_TO_STRING2(x) #x
#define MACRO_TO_STRING(x) MACRO_TO_STRING2(x)

// For marking functions and variables deprecated, i.e. like following:
// #define FOO SF_MACRO_DEPRECATED_WARNING("FOO is deprecated, use BAR instead") BAR
#if defined(_MSC_VER)
#define SF_MACRO_DEPRECATED_WARNING(MSG) __pragma(message ( __FILE__ "(" MACRO_TO_STRING(__LINE__) ") : warning C4996: " MSG))
#else
#define SF_MACRO_DEPRECATED_WARNING(MSG)  _Pragma(MACRO_TO_STRING(GCC warning MSG))
#endif

// Defined for unused function parameters.
#define SF_UNUSED(x) (void)(x)

#include "SF_CRTFunctionSafe.h"

struct tm *STDCALL sf_gmtime(const time_t *timep, struct tm *result);

struct tm *STDCALL sf_localtime(const time_t *timep, struct tm *result);

void STDCALL sf_tzset(void);

int STDCALL sf_setenv(const char *name, const char *value);

char *STDCALL sf_getenv_s(const char *name, char *outbuf, size_t bufsize);

int STDCALL sf_unsetenv(const char *name);

int STDCALL sf_mkdir(const char *path);

#define SF_ERROR_BUFSIZE  1024
char* STDCALL sf_strerror_s(int errnum, char* outbuf, size_t bufsize);

int STDCALL
_thread_init(SF_THREAD_HANDLE *thread, void *(*proc)(void *), void *arg);

int STDCALL _thread_join(SF_THREAD_HANDLE thread);

void STDCALL _thread_exit();

int STDCALL _cond_init(SF_CONDITION_HANDLE *cond);

int STDCALL _cond_broadcast(SF_CONDITION_HANDLE *cond);

int STDCALL _cond_signal(SF_CONDITION_HANDLE *cond);

int STDCALL
_cond_wait(SF_CONDITION_HANDLE *cond, SF_CRITICAL_SECTION_HANDLE *lock);

int STDCALL _cond_term(SF_CONDITION_HANDLE *cond);

int STDCALL _critical_section_init(SF_CRITICAL_SECTION_HANDLE *lock);

int STDCALL _critical_section_lock(SF_CRITICAL_SECTION_HANDLE *lock);

int STDCALL _critical_section_unlock(SF_CRITICAL_SECTION_HANDLE *lock);

int STDCALL _critical_section_term(SF_CRITICAL_SECTION_HANDLE *lock);

int STDCALL _rwlock_init(SF_RWLOCK_HANDLE *lock);

int STDCALL _rwlock_rdlock(SF_RWLOCK_HANDLE *lock);

int STDCALL _rwlock_rdunlock(SF_RWLOCK_HANDLE *lock);

int STDCALL _rwlock_wrlock(SF_RWLOCK_HANDLE *lock);

int STDCALL _rwlock_wrunlock(SF_RWLOCK_HANDLE *lock);

int STDCALL _rwlock_term(SF_RWLOCK_HANDLE *lock);

int STDCALL _mutex_init(SF_MUTEX_HANDLE *lock);

int STDCALL _mutex_lock(SF_MUTEX_HANDLE *lock);

int STDCALL _mutex_unlock(SF_MUTEX_HANDLE *lock);

int STDCALL _mutex_term(SF_MUTEX_HANDLE *lock);

const char *STDCALL sf_os_name();

void STDCALL sf_os_version(char *ret, size_t size);

int STDCALL sf_strncasecmp(const char *s1, const char *s2, size_t n);

char *STDCALL sf_filename_from_path(const char *path);

void STDCALL sf_log_timestamp(char* tsbuf, size_t tsbufsize);

int STDCALL sf_create_directory_if_not_exists(const char * directoryName);

int STDCALL sf_create_directory_if_not_exists_recursive(const char * directoryName);

int STDCALL sf_is_directory_exist(const char * directoryName);

int STDCALL sf_delete_directory_if_exists(const char * directoryName);

void STDCALL sf_get_tmp_dir(char * tmpDir);

void STDCALL sf_get_uniq_tmp_dir(char * tmpDir);

void STDCALL sf_get_username(char * username, int bufLen);

void STDCALL sf_delete_uniq_dir_if_exists(const char *tmpfile);

void STDCALL sf_memory_error_handler();

// this should be called by application before any calls of sfclient
void STDCALL sf_exception_on_memory_failure();

void STDCALL sf_get_callers_executable_path(char* out_buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif
