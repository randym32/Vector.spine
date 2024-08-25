/* Cross-compiler packing macros
   Copyright 2024 Randall Maas
*//**@file
    @brief Cross-compiler packing macros

    This file contains packing macros used to allow convention C/C++ access to
    fields while ensuring that the struct occupies a predictable memory layout,
    without regard to compiler or processor architecture specific layout rules.
    This is often used in binary exchange formats.
*/
#pragma once

#if defined(__clang__) || defined(__GNUC__)
/** Pack a struct to ensure it has consistent memory layout
    @param ... the struct to pack

    This macro is used to pack a struct by adjusting the alignment of its
    members.  It ensures that the struct occupies a predictable memory layout,
    without regard to compiler or processor architecture specific layout rules.
    This can be particularly useful in binary exchange formats.

    Usage example:
    @code
    PACK(struct MyStruct
    {
        uint8_t a;
        uint16_t b;
        uint32_t c;
    });
    @endcode

    @note The packed struct may have performance implications on some architectures
    due to unaligned access. Use with caution and only when necessary.
*/
#define PACK( ... )  __VA_ARGS__ __attribute__((__packed__))
#elif defined(_MSC_VER)
/** Pack a struct to ensure it has consistent memory layout
    @param ... the struct to pack

    This macro is used to pack a struct by adjusting the alignment of its
    members.  It ensures that the struct occupies a predictable memory layout,
    without regard to compiler or processor architecture specific layout rules.
    This can be particularly useful in binary exchange formats.

    Usage example:
    @code
    PACK(struct MyStruct
    {
        uint8_t a;
        uint16_t b;
        uint32_t c;
    });
    @endcode

    @note The packed struct may have performance implications on some architectures
    due to unaligned access. Use with caution and only when necessary.
*/
#define PACK(...) __pragma( pack(push, 1) ) __VA_ARGS__  __pragma( pack(pop))
#endif
