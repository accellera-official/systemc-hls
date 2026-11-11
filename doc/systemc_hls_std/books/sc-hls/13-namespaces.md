---
style: chapter
---

# 13. Namespaces

*namespaces* (as defined in ISOC++ 10.3) are ***Supported*.** Non-const global variables are
***Not Supported*** for synthesis. Within a function body, only those names of variables
*<u>shall</u>* be used, which are declared previously in the function body or passed as parameters.
Global constants are ***Supported*** for synthesis.

Examples:

```cpp
// Example Namespace
namespace NSP {
    int var;
    constexpr int CNST = 42;
}

void foo(const int val) {
    using namespace NSP;

    int dummy = CNST; // OK: Note that the occurrence of name CNST may be
                      // replaced by the value ‘42’ by a synthesis tool.

    dummy = var; // Error: The name of a variable being declared in another
                 // namespace must not be used within a function body.

    var = val; // Error: The name of a variable being declared in another
               // namespace must not be used within a function body.
```

## Namespace definition

***Supported*** as defined in ISOC++ 10.3.1.

Nested namespace definitions are ***Supported***.
This allows the shorthand `namespace A::B::C { }` in place of nested namespace declarations.

### Unnamed namespaces

***Supported*** as defined in ISOC++ 10.3.1.1.

### Namespace member definitions

***Supported*** as defined in ISOC++ 10.3.1.2.

## Namespace alias

***Supported*** as defined in ISOC++ 10.3.2.

## The using declaration

***Supported*** as defined in ISOC++ 10.3.3.

## Using directive

***Supported*** as defined in ISOC++ 10.3.4.
