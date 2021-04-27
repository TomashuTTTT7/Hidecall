#pragma once

#pragma region HC_CONFIG

// The highest number you can pass as a constant in asm instruction is
// maximum value of uint64_t integer, so 18446744073709551615.
// If concatenated number is higher than that, you'll get an error.
// In such a case, use HIDECALL_LEVEL macro with lower level number
// (6 by default)
#define HC_DEFAULT_RNG_LEVEL 6 // integer in range [1, 9]

// KEY. Change it to your own random 32-bit number
// It's not used for real encryption, but splits pointer decoding in two parts
#define HC_KEY 0x7D90A5F1

// Segment names
#define HC_DECODE_SEG  ".ddata" // there will be function which decodes obfuscated pointer
#define HC_HANDLER_SEG ".hdata" // there will be function which handles your call and contains obfuscated pointer

#pragma endregion

#define HC__JOIN1(a)                         a
#define HC__JOIN2(a, b)                      a##b
#define HC__JOIN3(a, b, c)                   a##b##c
#define HC__JOIN4(a, b, c, d)                a##b##c##d
#define HC__JOIN5(a, b, c, d, e)             a##b##c##d##e
#define HC__JOIN6(a, b, c, d, e, f)          a##b##c##d##e##f
#define HC__JOIN7(a, b, c, d, e, f, g)       a##b##c##d##e##f##g
#define HC__JOIN8(a, b, c, d, e, f, g, h)    a##b##c##d##e##f##g##h
#define HC__JOIN9(a, b, c, d, e, f, g, h, i) a##b##c##d##e##f##g##h##i

#define HC_JOIN1(a)                          HC__JOIN1(a)
#define HC_JOIN2(a, b)                       HC__JOIN2(a, b)
#define HC_JOIN3(a, b, c)                    HC__JOIN3(a, b, c)
#define HC_JOIN4(a, b, c, d)                 HC__JOIN4(a, b, c, d)
#define HC_JOIN5(a, b, c, d, e)              HC__JOIN5(a, b, c, d, e)
#define HC_JOIN6(a, b, c, d, e, f)           HC__JOIN6(a, b, c, d, e, f)
#define HC_JOIN7(a, b, c, d, e, f, g)        HC__JOIN7(a, b, c, d, e, f, g)
#define HC_JOIN8(a, b, c, d, e, f, g, h)     HC__JOIN8(a, b, c, d, e, f, g, h)
#define HC_JOIN9(a, b, c, d, e, f, g, h, i)  HC__JOIN9(a, b, c, d, e, f, g, h, i)

#define HC_PP_RNG1() HC_JOIN1(__COUNTER__)
#define HC_PP_RNG2() HC_JOIN2(__LINE__, __COUNTER__)
#define HC_PP_RNG3() HC_JOIN3(__LINE__, __LINE__, __COUNTER__)
#define HC_PP_RNG4() HC_JOIN4(__LINE__, __LINE__, __LINE__, __COUNTER__)
#define HC_PP_RNG5() HC_JOIN5(__LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define HC_PP_RNG6() HC_JOIN6(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define HC_PP_RNG7() HC_JOIN7(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define HC_PP_RNG8() HC_JOIN8(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)
#define HC_PP_RNG9() HC_JOIN9(__LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __LINE__, __COUNTER__)

#define HC_PP_RNG_LEVEL(level) HC_PP_RNG##level()

/*
* You can barely name this RNG, but this is the only thing i could think of.
* This macro concatenates linenumber a few times with counter at the end.
* This way it makes a huge decimal number, which then is converted to binary
* in assembly instruction. Only 32 bits are saved, what makes this
* number kinda unpredictable.
*/

#define PACK(...) __VA_ARGS__

/*
* USAGE:
* 
* @level level of linenumber concatenation. If the linenumber is too high,
*        you can get an error. Lower this value in such a case (6 by default)
* 
* @modifiers modifiers of your function, eg.:
*            __declspec(noinline) static
*            If for any reason it has a comma inside, use PACK()
*            If you don't have any modifiers, skip this part and proceed to next one
* 
* @ret return type of your function. If you specify a calling convention, pass it here
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
* And use PACK() macro if return type has comma.
* 
* See example.cpp for more examples with different function signatures.
*/

//change to 1 if you really want to try x64 - if your compiler supports x64 inline assembly
#define HC_IGNORE 0

// PP_RNG_LEVEL has to be "called" in this macro 2, 5, 10 or 20 times etc.
// I use additional code segments, because otherwise the real
// function is right after the obfuscated one (that's not secure for sure)


#pragma warning(disable:4731)

#if defined(__LP64__) || defined(_LP64) || defined(_WIN64)

#if HC_IGNORE

#define HIDECALL_CLASS_LEVEL(level, modifiers, ret, fname, ...)                 \
__pragma(code_seg(push, seg1, HC_HANDLER_SEG))                                  \
__declspec(noinline) modifiers ret fname __VA_ARGS__ {                          \
    __asm {                                                                     \
        __asm mov rsp, rbp                                                      \
        __asm mov rbp, fname##_HC + HC_PP_RNG_LEVEL(level) + 1 +                \
                (((HC_PP_RNG_LEVEL(level) & 0x7fffffff) % 17) / 4)              \
        __asm call fname##_HC_sub                                               \
        __asm pop rax                                                           \
        __asm xor rbp, rax                                                      \
        __asm xor rax, rbp                                                      \
        __asm xor rbp, rax                                                      \
        __asm jmp rdx                                                           \
    }                                                                           \
}                                                                               \
__pragma(code_seg(pop, seg1))                                                   \
__pragma(code_seg(push, seg2, HC_DECODE_SEG))                                   \
__declspec(noinline) void fname##_HC_sub() {                                    \
    __asm {                                                                     \
        __asm add rbp, ((-HC_PP_RNG_LEVEL(level) + 1) & HC_KEY) + HC_KEY + 1    \
        __asm add rbp, ((-HC_PP_RNG_LEVEL(level) + 2) & ~HC_KEY) +              \
                (((HC_PP_RNG_LEVEL(level) & 0x7fffffff) % 23) / 2) + ~HC_KEY    \
        __asm and rbp, 0x7ffffffffffffff0                                       \
        __asm xor rbp, rdx                                                      \
        __asm xor rdx, rbp                                                      \
        __asm xor rbp, rdx                                                      \
    }                                                                           \
}                                                                               \
__pragma(code_seg(pop, seg2))                                                   \
modifiers ret fname##_HC __VA_ARGS__

#else

#define HIDECALL_CLASS_LEVEL(level, modifiers, ret, fname, ...)
#error HIDECALL: x64 architecture is not supported (at least for msvc)

#endif

#else

#define HIDECALL_CLASS_LEVEL(level, modifiers, ret, fname, ...)                 \
__pragma(code_seg(push, seg1, HC_HANDLER_SEG))                                  \
__declspec(noinline) modifiers ret fname __VA_ARGS__ {                          \
    __asm {                                                                     \
        __asm mov esp, ebp                                                      \
        __asm mov ebp, fname##_HC + HC_PP_RNG_LEVEL(level) + 1 +                \
                (((HC_PP_RNG_LEVEL(level) & 0x7fffffff) % 17) / 4)              \
        __asm call fname##_HC_sub                                               \
        __asm pop eax                                                           \
        __asm xor ebp, eax                                                      \
        __asm xor eax, ebp                                                      \
        __asm xor ebp, eax                                                      \
        __asm jmp edx                                                           \
    }                                                                           \
}                                                                               \
__pragma(code_seg(pop, seg1))                                                   \
__pragma(code_seg(push, seg2, HC_DECODE_SEG))                                   \
__declspec(noinline) void fname##_HC_sub() {                                    \
    __asm {                                                                     \
        __asm add ebp, ((-HC_PP_RNG_LEVEL(level) + 1) & HC_KEY) + HC_KEY + 1    \
        __asm add ebp, ((-HC_PP_RNG_LEVEL(level) + 2) & ~HC_KEY) +              \
                (((HC_PP_RNG_LEVEL(level) & 0x7fffffff) % 23) / 2) + ~HC_KEY    \
        __asm and ebp, 0x7ffffff0                                               \
        __asm xor ebp, edx                                                      \
        __asm xor edx, ebp                                                      \
        __asm xor ebp, edx                                                      \
    }                                                                           \
}                                                                               \
__pragma(code_seg(pop, seg2))                                                   \
modifiers ret fname##_HC __VA_ARGS__

#endif

/*
* Types of HIDECALL:
*
* HIDECALL_DECLARE - hidecall function declaration
* HIDECALL - hidecall definition for functions
* HIDECALL_LEVEL - as above, but with external linenumber concatenation level
* HIDECALL_CLASS - hidecall definition for class member functions
* HIDECALL_CLASS_LEVEL - as above, but with external linenumber concatenation level
*/
#define HIDECALL_DECLARE(modifiers, ret, fname, ...) void fname##_HC_sub(); modifiers ret fname __VA_ARGS__; modifiers ret fname##_HC __VA_ARGS__;
#define HIDECALL_LEVEL(level, modifiers, ret, fname, ...) HIDECALL_DECLARE(modifiers, ret, fname, __VA_ARGS__) HIDECALL_CLASS_LEVEL(level, modifiers, ret, fname, __VA_ARGS__)
#define HIDECALL(modifiers, ret, fname, ...) HIDECALL_LEVEL(HC_DEFAULT_RNG_LEVEL, modifiers, ret, fname, __VA_ARGS__)
#define HIDECALL_CLASS(modifiers, ret, fname, ...) HIDECALL_CLASS_LEVEL(HC_DEFAULT_RNG_LEVEL, modifiers, ret, fname, __VA_ARGS__)
