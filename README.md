# Hidecall

Hidecall is header-only C++ library that obfuscates function calls and
hides them in decompiler, what makes reverse-engineering very annoying.

It is developed and supported for MSVC compiler

## Usage

In order to apply Hidecall, modify function definition signature as follows:
```cpp
//Before
static int foo(int a, char b)
{
    //Your code
}

//After
HIDECALL(static, int, foo, (int a, char b))
{
    //Your code
}
```
Basically you wrap your function signature into a macro and split it using commas:
- before return type
- before name
- after name

Use PACK() macro if return type has comma inside.

That's it! You don't have to modify any code other than that.
You can even use it for main function, there are no restrictions about it.

Your function should work exactly the same way as before applying hidecall. If something works differently, let me know by opening an issue.

For explanation visit [hidecall.h](hidecall/hidecall.h) and for more examples see [example.cpp](hidecall/example.cpp)

### Decompilation

Ghidra decompilation example:
![Preview](example.png)
As you can see, Hidecall cut off call paths and disabled default signature deduction

I tested it on some decompilers, including Ghidra, JEB, Snowman, Retdec and they are all fooled.

PS: I don't have IDA so i couldn't check it there.

### Types

Types of HIDECALL:

- __HIDECALL_DECLARE__ - hidecall function declaration
- __HIDECALL__ - hidecall definition for functions
- __HIDECALL_LEVEL__ - as above, but with external linenumber concatenation level
- __HIDECALL_CLASS__ - hidecall definition for class member functions
- __HIDECALL_CLASS_LEVEL__ - as above, but with external linenumber concatenation level

### Limitations

- No support for lambdas
- No support for x64 architecture, x86 only

### Consequences

- __MESS__ inside a namespace, because hidecall defines two additional
  functions per one protected function. That's why i suggest
  switching to hidecall after writing the actual code
- Increased compiled binary size

### Restrictions
 
- Do not use __\_\_COUNTER\_\___ in your program, because it is a part of obfuscation randomization and changing its value can break your program

## License
Licensed under the [MIT License](LICENSE)
