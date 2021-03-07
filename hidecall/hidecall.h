#pragma once

/*
* The highest number you can pass as a constant in asm instruction is
* maximum value of uint64_t integer, so 18446744073709551615.
*
* If concatenated number is higher than that, you'll get an error.
* In such a case, use HIDECALL_LEVEL macro with lower level number
* (6 by default)
*/
#define DEFAULT_RNG_LEVEL 6

// KEY. Change it to your own random 32-bit number
// It's not used for real encryption, but makes reverse-engineering a bit harder
#define KEY 0x7D90A5F1

// Segment names
#define DECODE_SEG  ".ddata" // there will be function which decrypts obfuscated pointer
#define HANDLER_SEG ".hdata" // there will be function which handles your call and encodes the pointer


#define __JOIN1(a)                         a
#define __JOIN2(a, b)                      a##b
#define __JOIN3(a, b, c)                   a##b##c
#define __JOIN4(a, b, c, d)                a##b##c##d
#define __JOIN5(a, b, c, d, e)             a##b##c##d##e
#define __JOIN6(a, b, c, d, e, f)          a##b##c##d##e##f
#define __JOIN7(a, b, c, d, e, f, g)       a##b##c##d##e##f##g
#define __JOIN8(a, b, c, d, e, f, g, h)    a##b##c##d##e##f##g##h
#define __JOIN9(a, b, c, d, e, f, g, h, i) a##b##c##d##e##f##g##h##i
#define _JOIN1(a)                          __JOIN1(a)
#define _JOIN2(a, b)                       __JOIN2(a, b)
#define _JOIN3(a, b, c)                    __JOIN3(a, b, c)
#define _JOIN4(a, b, c, d)                 __JOIN4(a, b, c, d)
#define _JOIN5(a, b, c, d, e)              __JOIN5(a, b, c, d, e)
#define _JOIN6(a, b, c, d, e, f)           __JOIN6(a, b, c, d, e, f)
#define _JOIN7(a, b, c, d, e, f, g)        __JOIN7(a, b, c, d, e, f, g)
#define _JOIN8(a, b, c, d, e, f, g, h)     __JOIN8(a, b, c, d, e, f, g, h)
#define _JOIN9(a, b, c, d, e, f, g, h, i)  __JOIN9(a, b, c, d, e, f, g, h, i)
#define PP_RNG1(a)                         _JOIN1(__COUNTER__)
#define PP_RNG2(a, b)                      _JOIN2(__LINE__, __COUNTER__)
#define PP_RNG3(a, b, c)                   _JOIN3(__LINE__, __LINE__, __COUNTER__)
#define PP_RNG4(a, b, c, d)                _JOIN4(__LINE__, __LINE__, __LINE__, __COUNTER__)
#define PP_RNG5(a, b, c, d, e)             _JOIN5(__LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define PP_RNG6(a, b, c, d, e, f)          _JOIN6(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define PP_RNG7(a, b, c, d, e, f, g)       _JOIN7(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define PP_RNG8(a, b, c, d, e, f, g, h)    _JOIN8(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define PP_RNG9(a, b, c, d, e, f, g, h, i) _JOIN9(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)

#define PP_RNG_LEVEL(level) PP_RNG##level()

/*
* You can barely name this RNG, but this is the only thing i could think of.
* This macro concatenates linenumber a few times with counter at the end.
* This way it makes a huge decimal number, which then is converted to binary
* in assembly instruction. For x86 only 32 bits are saved, what makes this
* number kinda unpredictable. For x64 it's possible to reverse linenumber out 
* of the binary, so that's why i don't recommend it for this platform.
* You can use it anyway if you don't care about leaking line numbers.
* 
* And no, constexpr value does not work here, i tried
*/

/*
* Usage:
* 
* @level level of linenumber concatenation. If the linenumber is too high,
*        you can get an error. Lower this value in such a case (6 by default)
* 
* @modifiers modifiers of your function, eg.:
*            __declspec(noinline) static
*            If for any reason it has a comma inside, use PACK()
*            If you don't have any modifiers, skip this part and proceed to next one
* 
* @ret return type of your function. If you specify a calling convention, pass ith here
*      int __cdecl
*      If it has a comma inside, use PACK macro, eg.:
*      PACK(std::map<std::string, std::string>)
* 
* @fname your function's name
* 
* @... your function's arguments. Don't use PACK() macro, but keep the parenthesis, eg:
*      (int a, unsigned long b, std::string& str)
* 
* At the end macro can look like this:
* HIDECALL(, int, main, (int argc, char* argv[]))
* 
* Basically you wrap your function signature into a macro and split it using commas:
* - before return type
* - before name
* - after name
* And use PACK() if return type has comma.
* 
* See example.cpp for more examples with different function signatures.
*/

#define PACK(...) __VA_ARGS__


// PP_RNG_LEVEL has to be "called" in this macro 2, 5, 10 or 20 times etc.
// I use additional code segments, because otherwise the real
// function is right after the obfuscated one
// You can change segment names if you want .
#define HIDECALL_CLASS_LEVEL(level, modifiers, ret, fname, ...)             \
__pragma(code_seg(push, seg1, HANDLER_SEG))                                 \
__declspec(noinline) modifiers ret fname __VA_ARGS__ {                      \
    __asm {                                                                 \
        __asm leave                                                         \
        __asm push ebp                                                      \
        __asm mov ebp, fname##_HC + PP_RNG_LEVEL(level)                     \
        __asm call fname##_HC_sub                                           \
        __asm pop eax                                                       \
        __asm xchg ebp, eax                                                 \
        __asm jmp edx                                                       \
    }                                                                       \
}                                                                           \
__pragma(code_seg(pop, seg1))                                               \
__pragma(code_seg(push, seg2, DECODE_SEG))                                  \
__declspec(noinline) void fname##_HC_sub() {                                \
    __asm {                                                                 \
        /* Some obfuscation */                                              \
        __asm add ebp, ((-PP_RNG_LEVEL(level)) & KEY) +                     \
                (((PP_RNG_LEVEL(level) & 0x7fffffff) % 19) / 2) + KEY + 2   \
        __asm add ebp, ((-PP_RNG_LEVEL(level) + 2) & ~KEY) +                \
                (((PP_RNG_LEVEL(level) & 0x7fffffff) % 13) / 2) + ~KEY      \
        __asm shr ebp, 4                                                    \
        __asm xchg ebp, edx                                                 \
        __asm shl edx, 4                                                    \
    }                                                                       \
}                                                                           \
__pragma(code_seg(pop, seg2))                                               \
modifiers ret fname##_HC __VA_ARGS__


/*
* Types of HIDECALL:
*
* HIDECALL - hidecall definition for functions
* HIDECALL_LEVEL - as above, but with external linenumber concatenation level
* HIDECALL_DECLARE - hidecall function declaration
* HIDECALL_CLASS - hidecall definition for class member functions
* HIDECALL_CLASS_LEVEL - as above, but with external linenumber concatenation level
*/
#define HIDECALL_DECLARE(modifiers, ret, fname, ...) void fname##_HC_sub(); modifiers ret fname __VA_ARGS__; modifiers ret fname##_HC __VA_ARGS__;
#define HIDECALL_LEVEL(level, modifiers, ret, fname, ...) HIDECALL_DECLARE(modifiers, ret, fname, __VA_ARGS__) HIDECALL_CLASS_LEVEL(level, modifiers, ret, fname, __VA_ARGS__)

#define HIDECALL(modifiers, ret, fname, ...) HIDECALL_LEVEL(DEFAULT_RNG_LEVEL, modifiers, ret, fname, __VA_ARGS__)
#define HIDECALL_CLASS(modifiers, ret, fname, ...) HIDECALL_CLASS_LEVEL(DEFAULT_RNG_LEVEL, modifiers, ret, fname, __VA_ARGS__)