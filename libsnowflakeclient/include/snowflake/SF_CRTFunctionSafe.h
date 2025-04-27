/*
 * Copyright (c) 2024 Snowflake Computing, Inc. All rights reserved.
 */

#ifndef _SF_CRTFUNCTIONSAFE_H_
#define _SF_CRTFUNCTIONSAFE_H_

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

    /// @brief Copy bytes between buffers.
    ///
    /// @param out_dest         Destination buffer. (NOT OWN)
    /// @param in_destSize      Size of the destination buffer.
    /// @param in_src           Buffer to copy from. (NOT OWN)
    /// @param in_sizeToCopy    Number of bytes to copy.
    ///
    /// @return A pointer to destination; NULL if an error occurred. (NOT OWN)
    static inline void* sf_memcpy(
        void* out_dest,
        size_t in_destSize,
        const void* in_src,
        size_t in_sizeToCopy)
    {
#if defined(_WIN32) || defined(_WIN64)
        if (0 == memcpy_s(out_dest, in_destSize, in_src, in_sizeToCopy))
        {
            return out_dest;
        }
        return NULL;
#else
        if (in_sizeToCopy > in_destSize)
        {
            return NULL;
        }
        return memcpy(out_dest, in_src, in_sizeToCopy);
#endif
    }

    // helper function for sf_strcpy, sf_strncpy and sbcat on non-Windows
    static inline char* sf_copy(void* dst, size_t dstsize, char const* src, long long srclen)
    {
        const size_t copyLen = (srclen < 0) ? strlen(src) + 1 : srclen;

        //NOTE: this copies the terminul:
        return (char*)sf_memcpy(dst, dstsize, src, copyLen);
    }

    // helper function for sf_strcat and sf_strncat
    static inline char* sf_cat(char* dst, size_t dstsize, char const* src, size_t srclen)
    {
        size_t dstlen = strlen(dst);
        return dstsize < dstlen ?
            NULL :
            sf_copy(dst + dstlen, dstsize - dstlen,
                        src, srclen < 0 ? strlen(src) + 1 : srclen);
    }

/// @brief Copy a string.
    ///
    /// @param out_dest         Destination string buffer. (NOT OWN)
    /// @param in_destSize      Size of the destination string buffer.
    /// @param in_src           Null-terminated source string buffer. (NOT OWN)
    ///
    /// @return The destination string; NULL if an error occurred. (NOT OWN)
    static inline char* sf_strcpy(
        char* out_dest,
        size_t in_destSize,
        const char* in_src)
    {
#if defined(_WIN32) || defined(_WIN64)
        if (0 == strcpy_s(out_dest, in_destSize, in_src))
        {
            return out_dest;
        }
        return NULL;
#else
        return sf_copy(out_dest, in_destSize, in_src, -1);
#endif
    }

    /// @brief Copy characters of one string to another.
    ///
    /// @param out_dest         Destination string. (NOT OWN)
    /// @param in_destSize      The size of the destination string, in characters.
    /// @param in_src           Source string. (NOT OWN)
    /// @param in_sizeToCopy    Number of characters to be copied.
    ///
    /// @return The destination string; NULL if a truncation or error occurred. (NOT OWN)
    static inline char* sf_strncpy(
        char* out_dest,
        size_t in_destSize,
        const char* in_src,
        size_t in_sizeToCopy)
    {
#if defined(_WIN32) || defined(_WIN64)
        if (0 == strncpy_s(out_dest, in_destSize, in_src, in_sizeToCopy))
        {
            return out_dest;
        }
        return NULL;
#else
        return sf_copy(out_dest, in_destSize, in_src, in_sizeToCopy);
#endif
    }

    /// @brief Append to a string.
    ///
    /// @param out_dest         Destination string to append to. (NOT OWN)
    /// @param in_destSize      Size of the destination string buffer.
    /// @param in_src           Null-terminated source string buffer. (NOT OWN)
    ///
    /// @return The destination string; NULL if an error occurred. (NOT OWN)
    static inline char* sf_strcat(
        char* out_dest,
        size_t in_destSize,
        const char* in_src)
    {
#if defined(_WIN32) || defined(_WIN64)
        if (0 == strcat_s(out_dest, in_destSize, in_src))
        {
            return out_dest;
        }
        return NULL;
#else
        return sf_cat(out_dest, in_destSize, in_src, -1);
#endif
    }

    /// @brief Append characters of one string to another.
    ///
    /// @param out_dest         Destination string to append to. (NOT OWN)
    /// @param in_destSize      The size of the destination string buffer, in characters.
    /// @param in_src           Source string. (NOT OWN)
    /// @param in_sizeToCopy    Number of characters to be copied.
    ///
    /// @return The destination string; NULL if a truncation or error occurred. (NOT OWN)
    static inline char* sf_strncat(
        char* out_dest,
        size_t in_destSize,
        const char* in_src,
        size_t in_sizeToCopy)
    {
#if defined(_WIN32) || defined(_WIN64)
        if (0 == strncat_s(out_dest, in_destSize, in_src, in_sizeToCopy))
        {
            return out_dest;
        }
        return NULL;
#else
        return sf_cat(out_dest, in_destSize, in_src, in_sizeToCopy);
#endif
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
    static inline int sf_vsnprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        size_t in_sizeToWrite,
        const char* in_format,
        va_list in_argPtr)
    {
#if defined(_WIN32) || defined(_WIN64)
        int ret = _vsnprintf_s(out_buffer, in_sizeOfBuffer, _TRUNCATE, in_format, in_argPtr);
#else
        int ret = vsnprintf(out_buffer, in_sizeToWrite + 1, in_format, in_argPtr);
        if ((size_t)ret > in_sizeToWrite)
        {
            ret = -1;
        }
#endif
        return ret;
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
    static inline int sf_sprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        const char* in_format,
        ...)
    {
        va_list ap;
        va_start(ap, in_format);
        const int ret = sf_vsnprintf(out_buffer, in_sizeOfBuffer, in_sizeOfBuffer - 1, in_format, ap);
        va_end(ap);

        return ret;
    }

    /// @brief Write formatted data to a string.
    ///
    /// @param out_buffer       Storage location for output. (NOT OWN)
    /// @param in_sizeOfBuffer  Size of buffer in characters.
    /// @param in_sizeToWrite   Maximum number of characters to write (not including the
    ///                         terminating null).
    /// @param in_format        Format control string. (NOT OWN)
    /// @param ...              Optional arguments for printf style conversions.
    ///
    /// @return The number of bytes written to the buffer, not counting the terminating null
    /// character; -1 if an error occurred.
    static inline int sf_snprintf(
        char* out_buffer,
        size_t in_sizeOfBuffer,
        size_t in_sizeToWrite,
        const char* in_format,
        ...)
    {
        va_list ap;
        va_start(ap, in_format);
        const int ret = sf_vsnprintf(out_buffer, in_sizeOfBuffer, in_sizeToWrite, in_format, ap);
        va_end(ap);

        return ret;
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
#if defined(_WIN32) || defined(_WIN64)
    static inline int sf_sscanf(
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
#else
#define sf_sscanf sscanf
#endif

    /// @brief Write formatted string to a FILE* using a pointer to a list of arguments.
    ///
    /// @param in_stream        Stream to write to. (NOT OWN)
    /// @param in_format        Format control string. (NOT OWN)
    /// @param in_argPtr        Pointer to list of arguments.
    ///
    /// @return The number of bytes written to the stream, not counting the terminating null
    /// character; -1 if the truncation or error occurred.
    static inline int sf_vfprintf(FILE* in_stream, const char* in_format, va_list in_argPtr)
    {
        return
#if defined(_WIN32) || defined(_WIN64)
        vfprintf_s(
#else
        vfprintf(
#endif
            in_stream,
            in_format,
            in_argPtr);
    }

    /// @brief Write formatted string to a FILE*. (See the standard C fprintf for details).
    ///
    /// @param in_stream        Stream to write to. (NOT OWN)
    /// @param in_format        Format control string. (NOT OWN)
    ///
    /// @return The number of bytes written to the stream, or a negative value if an error occurred.
    static inline int sf_fprintf(FILE* in_stream, const char* in_format, ...)
    {
        va_list ap;
        va_start(ap, in_format);
        const int ret = sf_vfprintf(in_stream, in_format, ap);
        va_end(ap);

        return ret;
    }

    /// @brief Write formatted string to a stdout*. (See the standard C fprintf for details).
    ///
    /// @param in_format        Format control string. (NOT OWN)
    ///
    /// @return The number of bytes written to the stream, or a negative value if an error occurred.
    static inline int sf_printf(const char* in_format, ...)
    {
        va_list ap;
        va_start(ap, in_format);
        const int ret = sf_vfprintf(stdout, in_format, ap);
        va_end(ap);

        return ret;
    }

    /// @brief Open a file.
    ///
    /// @param out_file         A pointer to the file pointer that will receive the pointer to the
    ///                         opened file. (NOT OWN)
    /// @param in_filename      The name of the file to open. (NOT OWN)
    /// @param in_mode          Type of access permitted. (NOT OWN)
    ///
    /// @return A pointer to the opened file; a NULL pointer if an error occurred. (OWN)
    static inline FILE* sf_fopen(FILE** out_file, const char* in_filename, const char* in_mode)
    {
#if defined(_WIN32) || defined(_WIN64)
        return fopen_s(out_file, in_filename, in_mode) ? (*out_file = NULL) : *out_file;
#elif defined(__APPLE__)
        return *out_file = fopen(in_filename, in_mode);
#else
        return *out_file = fopen64(in_filename, in_mode);
#endif
    }

    /// @brief Extract tokens from strings.
///
/// This function wraps the strtok_s (Windows) or strtok_r (POSIX) functions.
/// On the first call, in_str should provide a pointer to the string to be parsed,
/// while the in_context pointer helps keep track of the parsing. To retrieve the
/// next token, in_str should be NULL and the same in_context should be passed in.
/// The delimiter character(s) may vary for successive calls. 
/// For more information, please see: https://linux.die.net/man/3/strtok_r and 
/// https://msdn.microsoft.com/en-us/library/ftsafwz3.aspx.
/// Note: The input string will be modified when using this function. 
///
/// @param in_str               String to be parsed. (NOT OWN)
/// @param in_delim             Delimiter for the tokens. (NOT OWN)
/// @param in_context           Pointer to maintain context. (NOT OWN)
/// 
/// @return The next token; NULL if there are no more tokens.
    inline char* sf_strtok(
        char* in_str,
        const char* in_delim,
        char** in_context)
    {
#ifdef _MSC_VER
        return strtok_s(in_str, in_delim, in_context);
#else
        return strtok_r(in_str, in_delim, in_context);
#endif
    }

#ifdef __cplusplus
}
#endif

#endif
