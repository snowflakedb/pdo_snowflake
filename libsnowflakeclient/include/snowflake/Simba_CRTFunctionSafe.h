// =================================================================================================
///  @file Simba_CRTFunctionSafe.h
///
///  C Run-Time library functions' definitions for cross platform use.
///
///  Copyright (C) 2019 Simba Technologies Incorporated.
// =================================================================================================

#ifndef _SIMBA_CRTFUNCTIONSAFE_H_
#define _SIMBA_CRTFUNCTIONSAFE_H_

#define __STDC_WANT_LIB_EXT1__ 1

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STDCALL
    // match snowflake define in platform.h
    #ifdef _WIN32
        #define STDCALL __stdcall
    #else
        #define STDCALL
    #endif
#endif

extern char* STDCALL sf_getenv(const char *name);
extern char* STDCALL sf_strerror(int);

#if defined(WIN32) || defined(_WIN64)   // For Windows

    /// @brief Copy a string.
    /// 
    /// @param out_dest         Destination string buffer. (NOT OWN)
    /// @param in_destSize      Size of the destination string buffer.
    /// @param in_src           Null-terminated source string buffer. (NOT OWN)
    /// 
    /// @return The destination string; NULL if an error occurred. (NOT OWN)
    inline char* sb_strcpy(
        char* out_dest,
        size_t in_destSize,
        const char* in_src)
    {
        if (0 == strcpy_s(out_dest, in_destSize, in_src))
        {
            return out_dest;
        }
        return NULL;
    }

    /// @brief Copy characters of one string to another.
    ///
    /// @param out_dest         Destination string. (NOT OWN)
    /// @param in_destSize      The size of the destination string, in characters. 
    /// @param in_src           Source string. (NOT OWN)
    /// @param in_sizeToCopy    Number of characters to be copied.
    /// 
    /// @return The destination string; NULL if a truncation or error occurred. (NOT OWN)
    inline char* sb_strncpy(
        char* out_dest,
        size_t in_destSize,
        const char* in_src,
        size_t in_sizeToCopy)
    {
        if (0 == strncpy_s(out_dest, in_destSize, in_src, in_sizeToCopy))
        {
            return out_dest;
        }
        return NULL;
    }

    /// @brief Append to a string.
    /// 
    /// @param out_dest         Destination string to append to. (NOT OWN)
    /// @param in_destSize      Size of the destination string buffer.
    /// @param in_src           Null-terminated source string buffer. (NOT OWN)
    /// 
    /// @return The destination string; NULL if an error occurred. (NOT OWN)
    inline char* sb_strcat(
        char* out_dest,
        size_t in_destSize,
        const char* in_src)
    {
        if (0 == strcat_s(out_dest, in_destSize, in_src))
        {
            return out_dest;
        }
        return NULL;
    }

    /// @brief Append characters of one string to another.
    ///
    /// @param out_dest         Destination string to append to. (NOT OWN)
    /// @param in_destSize      The size of the destination string buffer, in characters. 
    /// @param in_src           Source string. (NOT OWN)
    /// @param in_sizeToCopy    Number of characters to be copied.
    /// 
    /// @return The destination string; NULL if a truncation or error occurred. (NOT OWN)
    inline char* sb_strncat(
        char* out_dest,
        size_t in_destSize,
        const char* in_src,
        size_t in_sizeToCopy)
    {
        if (0 == strncat_s(out_dest, in_destSize, in_src, in_sizeToCopy))
        {
            return out_dest;
        }
        return NULL;
    }

    /// @brief Copy bytes between buffers. 
    /// 
    /// @param out_dest         Destination buffer. (NOT OWN)
    /// @param in_destSize      Size of the destination buffer. 
    /// @param in_src           Buffer to copy from. (NOT OWN)
    /// @param in_sizeToCopy    Number of bytes to copy.
    /// 
    /// @return A pointer to destination; NULL if an error occurred. (NOT OWN)
    inline void* sb_memcpy(
        void* out_dest,
        size_t in_destSize,
        const void* in_src,
        size_t in_sizeToCopy)
    {
        if (0 == memcpy_s(out_dest, in_destSize, in_src, in_sizeToCopy))
        {
            return out_dest;
        }
        return NULL;
    }

    /// @brief Write formatted output using a pointer to a list of arguments.
    /// 
    /// Note: To ensure that there is room for the terminating null, be sure that in_sizeToWrite is 
    /// strictly less than in_sizeOfBuffer.
    /// 
    /// @param out_buffer       Storage location for output. (NOT OWN)
    /// @param in_sizeOfBuffer  Size of buffer in characters. 
    /// @param in_sizeToWrite   Maximum number of characters to write (not including the 
    ///                         terminating null).
    /// @param in_format        Format control string. (NOT OWN)
    /// @param in_argPtr        Pointer to list of arguments.
    /// 
    /// @return The number of bytes written to the buffer, not counting the terminating null 
    /// character; -1 if the truncation or error occurred.
    inline int sb_vsnprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        size_t in_sizeToWrite,
        const char* in_format,
        va_list in_argPtr)
    {
        if (in_sizeToWrite >= in_sizeOfBuffer) return -1;

        return _vsnprintf_s(out_buffer, in_sizeOfBuffer, _TRUNCATE, in_format, in_argPtr);
    }

    /// @brief Write formatted data to a string. 
    /// 
    /// @param out_buffer       Storage location for output. (NOT OWN)
    /// @param in_sizeOfBuffer  Size of buffer in characters. 
    /// @param in_format        Format control string. (NOT OWN)
    /// @param ...              Optional arguments for printf style conversions.
    /// 
    /// @return The number of bytes written to the buffer, not counting the terminating null 
    /// character; -1 if an error occurred.
    inline int sb_sprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        const char* in_format,
        ...)
    {
        if (!in_sizeOfBuffer) return -1;

        va_list formatParams;
        va_start(formatParams, in_format);
        int bytesWritten = sb_vsnprintf(out_buffer, in_sizeOfBuffer, in_sizeOfBuffer - 1, in_format, formatParams);
        va_end(formatParams);

        return bytesWritten;
    }

    /// @brief Reads formatted data from a string. 
    /// 
    /// Note: A buffer size parameter is required when you use the type field characters 
    /// c, C, s, S, or string control sets that are enclosed in []. 
    ///
    /// @param in_buffer        Stored data. (NOT OWN)
    /// @param in_format        Format control string. (NOT OWN)
    /// @param ...              Optional arguments.
    /// 
    /// @return The number of fields that are successfully converted and assigned;
    /// -1 if an error occurred.
    inline int sb_sscanf(
        const char* in_buffer,
        const char* in_format,
        ...)
    {
        va_list formatParams;
        va_start(formatParams, in_format);
        int ret = vsscanf_s(in_buffer, in_format, formatParams);
        va_end(formatParams);

        return ret;
    }

    /// @brief Write formatted string to a FILE*. (See the standard C fprintf for details).
    /// 
    /// @param in_stream        Stream to write to. (NOT OWN)
    /// @param in_format        Format control string. (NOT OWN)
    /// 
    /// @return The number of bytes written to the stream, or a negative value if an error occurred.
    static inline int sb_fprintf(FILE* in_stream, const char* in_format, ...)
    {
        va_list vlist;
        va_start(vlist, in_format);
        int bytesWritten = vfprintf_s(in_stream, in_format, vlist);
        va_end(vlist);

        return bytesWritten;
    }


    /// @brief Write formatted string to a stdout*. (See the standard C fprintf for details).
    /// 
    /// @param in_format        Format control string. (NOT OWN)
    /// 
    /// @return The number of bytes written to the stream, or a negative value if an error occurred.
    static inline int sb_printf(const char* in_format, ...)
    {
        va_list vlist;
        va_start(vlist, in_format);
        int bytesWritten = vfprintf_s(stdout, in_format, vlist);
        va_end(vlist);

        return bytesWritten;
    }


#define sb_getenv   sf_getenv
#define sb_strerror sf_strerror

#else   // For all other platforms except Windows

    /// @brief Copy a string.
    /// 
    /// @param out_dest         Destination string buffer. (NOT OWN)
    /// @param in_destSize      Size of the destination string buffer.
    /// @param in_src           Null-terminated source string buffer. (NOT OWN)
    /// 
    /// @return The destination string; NULL if an error occurred. (NOT OWN)
    static inline char* sb_strcpy(
        char* out_dest,
        size_t in_destSize,
        const char* in_src)
    {
        return strcpy(out_dest, in_src);
    }

    /// @brief Copy characters of one string to another.
    ///
    /// @param out_dest         Destination string. (NOT OWN)
    /// @param in_destSize      The size of the destination string, in characters. 
    /// @param in_src           Source string. (NOT OWN)
    /// @param in_sizeToCopy    Number of characters to be copied.
    /// 
    /// @return The destination string; NULL if a truncation or error occurred. (NOT OWN)
    static inline char* sb_strncpy(
        char* out_dest,
        size_t in_destSize,
        const char* in_src,
        size_t in_sizeToCopy)
    {
        return strncpy(out_dest, in_src, in_sizeToCopy);
    }

    /// @brief Append to a string.
    /// 
    /// @param out_dest         Destination string to append to. (NOT OWN)
    /// @param in_destSize      Size of the destination string buffer.
    /// @param in_src           Null-terminated source string buffer. (NOT OWN)
    /// 
    /// @return The destination string; NULL if an error occurred. (NOT OWN)
    static inline char* sb_strcat(
        char* out_dest,
        size_t in_destSize,
        const char* in_src)
    {
        return strcat(out_dest, in_src);
    }

    /// @brief Append characters of one string to another.
    ///
    /// @param out_dest         Destination string to append to. (NOT OWN)
    /// @param in_destSize      The size of the destination string buffer, in characters. 
    /// @param in_src           Source string. (NOT OWN)
    /// @param in_sizeToCopy    Number of characters to be copied.
    /// 
    /// @return The destination string; NULL if a truncation or error occurred. (NOT OWN)
    static inline char* sb_strncat(
        char* out_dest,
        size_t in_destSize,
        const char* in_src,
        size_t in_sizeToCopy)
    {
        return strncat(out_dest, in_src, in_sizeToCopy);
    }

    /// @brief Copy bytes between buffers. 
    /// 
    /// @param out_dest         Destination buffer. (NOT OWN)
    /// @param in_destSize      Size of the destination buffer. 
    /// @param in_src           Buffer to copy from. (NOT OWN)
    /// @param in_sizeToCopy    Number of bytes to copy.
    /// 
    /// @return A pointer to destination; NULL if an error occurred. (NOT OWN)
    static inline void* sb_memcpy(
        void* out_dest,
        size_t in_destSize,
        const void* in_src,
        size_t in_sizeToCopy)
    {
        return memcpy(out_dest, in_src, in_sizeToCopy);
    }

    /// @brief Write formatted output using a pointer to a list of arguments.
    /// 
    /// Note: To ensure that there is room for the terminating null, be sure that in_sizeToWrite is 
    /// strictly less than in_sizeOfBuffer.
    /// 
    /// @param out_buffer       Storage location for output. (NOT OWN)
    /// @param in_sizeOfBuffer  Size of buffer in characters. 
    /// @param in_sizeToWrite   Maximum number of characters to write (not including the 
    ///                         terminating null).
    /// @param in_format        Format control string. (NOT OWN)
    /// @param in_argPtr        Pointer to list of arguments.
    /// 
    /// @return The number of bytes written to the buffer, not counting the terminating null 
    /// character; -1 if the truncation or error occurred.
    static inline int sb_vsnprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        size_t in_sizeToWrite,
        const char* in_format,
        va_list in_argPtr)
    {
        // vsnprintf takes the size including terminating null while in_sizeToWrite does not
        // include terminating null.
        if (in_sizeToWrite >= in_sizeOfBuffer) return -1;

        int ret = vsnprintf(out_buffer, in_sizeToWrite + 1, in_format, in_argPtr);
        if ((ret < 0) || ((size_t)ret > in_sizeToWrite)) return -1;

        return ret;
    }

    /// @brief Write formatted data to a string. 
    /// 
    /// @param out_buffer       Storage location for output. (NOT OWN)
    /// @param in_sizeOfBuffer  Size of buffer in characters. 
    /// @param in_format        Format control string. (NOT OWN)
    /// @param ...	            Optional arguments for printf style conversions.
    /// 
    /// @return The number of bytes written to the buffer, not counting the terminating null 
    /// character; -1 if an error occurred.
    static inline int sb_sprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        const char* in_format,
        ...)
    {
        va_list formatParams;
        va_start(formatParams, in_format);
        int bytesWritten = sb_vsnprintf(out_buffer, in_sizeOfBuffer, in_sizeOfBuffer - 1, in_format, formatParams);
        va_end(formatParams);

        return bytesWritten;
    }

#define sb_sscanf sscanf
#define sb_fprintf fprintf
#define sb_printf printf
#define sb_strerror strerror

#define sb_getenv getenv

#endif

    /// @brief Open a file.
    /// 
    /// @param out_file         A pointer to the file pointer that will receive the pointer to the
    ///                         opened file. (NOT OWN)
    /// @param in_filename      The name of the file to open. (NOT OWN)
    /// @param in_mode          Type of access permitted. (NOT OWN)
    /// 
    /// @return A pointer to the opened file; a NULL pointer if an error occurred. (OWN)
    static inline FILE* sb_fopen(FILE** out_file, const char* in_filename, const char* in_mode)
    {
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64)
        return fopen_s(out_file, in_filename, in_mode) ? NULL : *out_file;
#else
        return *out_file = fopen(in_filename, in_mode);
#endif
    }



#ifdef __cplusplus
}
#endif



#endif
