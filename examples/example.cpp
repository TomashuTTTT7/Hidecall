#define AFTER 1
#define BEFORE !AFTER

#include <string>
#include <tuple>
#include <iostream>
#include "hidecall.h"

#if BEFORE

static std::tuple<int, char> __stdcall getTuple();

class Base
{
public:
    virtual int print(const char* str, int a) const = 0;
};

class Derived : public Base
{
public:
    int print(const char* str, int a = 1) const override;
    static void __cdecl welcome()
    {
        std::cout << "HI!\n";
    }
    virtual void virt()
    {
        std::cout << "Virtual\n";
    }
};

int Derived::print(const char* str, int a) const
{
    std::cout << str << ' ' << a << std::endl;
    return 1;
}

static std::tuple<int, char> __stdcall getTuple()
{
    return std::make_tuple(-15, 'F');
}

#endif

#if AFTER

HIDECALL_DECLARE(static, PACK(std::tuple<int, char> __stdcall), getTuple, ())

class Base
{
public:
    virtual int print(const char* str, int a) const = 0;
};

class Derived : public Base
{
public:
    HIDECALL_DECLARE(, int, print,(const char* str, int a = 1) const); // note you can't use "override" keyword
    HIDECALL_CLASS(static, void __cdecl, welcome, ())
    {
        std::cout << "HI!\n";
    }
    virtual HIDECALL_CLASS(, void, virt, ()) // note "virtual" is before macro, because "virtual" keyword applies only to one function.
    {
        std::cout << "Virtual\n";
    }
};

HIDECALL_CLASS(, int, Derived::print, (const char* str, int a) const)
{
    std::cout << str << ' ' << a << std::endl;
    return 1;
}

HIDECALL(static, PACK(std::tuple<int, char> __stdcall), getTuple, ())
{
    return std::make_tuple(-15, 'F');
}

#endif

// There is no need to modify calls in the code after using HIDECALL
int main()
{
    Derived d;
    Base* b = &d;
    Derived* pD = &d;

    b->print("Hello!", 7);
    Derived::welcome();
    pD->virt();

    auto [i, c] = getTuple();

    std::cin.get();

    return i + c;
}
