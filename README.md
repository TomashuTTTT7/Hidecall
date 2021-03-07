# Hidecall

Hidecall is header-only C++ library that obfuscates function calls and
hides them in decompiler, what makes reverse-engineering very annoying.

It works for MSVC compiler and it's recommended only for x86 platform.
Check out why in [hidecall.h](hidecall/hidecall.h)

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

Use PACK() if return type has comma inside.

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

- HIDECALL - hidecall definition for functions
- HIDECALL_LEVEL - as above, but with external linenumber concatenation level
- HIDECALL_DECLARE - hidecall function declaration
- HIDECALL_CLASS - hidecall definition for class member functions
- HIDECALL_CLASS_LEVEL - as above, but with external linenumber concatenation level

### Limitations

- No support for lambdas

### Consequences

- MESS inside a namespace, because hidecall defines two additional
  functions per one protected function. That's why i suggest
  switching to hidecall after writing the actual code
- Increased compiled binary size

### Restrictions
 
- Do not use __\_\_COUNTER\_\___ in your program, because it is a part of obfuscation randomization and changing its value can break your program

## License
Licensed under the [MIT License](LICENSE)
