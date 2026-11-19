---
style: chapter
---

# 9. Declarations

Declarations, as defined for ISOC++ Section 10, have ***Restricted Support*** and those specific
restrictions and coding guidelines are listed in the subsequent sections and chapters.

The *static_assert* and *using* declarations (ISOC++ Section 10) are ***Supported***.

***
**Note:**

ISOC++ allows *static_assert* with a single argument (no message string required).

***

## 9.1 Specifiers

Specifiers (ISOC++ 10.1) have ***Restricted Support***.

### 9.1.1 Storage class specifiers

Storage class specifiers (ISOC++ 10.1.1) have ***Restricted Support***.

1. The *register* storage class specifier was removed in C++17 (the keyword remains reserved) and
   is ***Not Supported***.
2. The *mutable* specifier is ***Supported***.
3. The *extern* specifier has ***Restricted Support***, as specified in Section
   [4.1](04-translation-units.md#41-translation-units-and-their-analysis).
4. The *static* specifier has ***Restricted Support***.
    1. Static variables have ***Restricted Support***. Only *const* static variables are supported.
    2. Static functions and static member functions are ***Supported***.
    3. Static data members of *<u>class</u>* have ***Restricted Support***.
       Only *const* static data members of *<u>class</u>* are ***Supported***.

***
**Note:**

The auto keyword used to be a storage class specifier in ISO/IEC 14882:2003 and was Ignored in the 1.4.7 version of the Accellera SystemC Synthesis Standard.

***

Examples:
```cpp
static class my_class c; // Static non-const variable, Not Supported

static void my_func() { } // Static function definition, Supported

static int var; // Static non-const variable, Not Supported

static const int const_var1 = 5; // Static const variable, Supported
static constexpr int const_var2 = 5; // Static const variable, Supported

struct my_struct {
    static int member; // Non-const, Not Supported
    static const int const_member1;   // Static const variable, Supported
    static constexpr int const_member2 = 5;   // Static const variable, Supported
    static void my_member_func() { } // Static member function, Supported
};
```

### 9.1.2 Function specifiers

The function specifiers have ***Restricted Support***.

- The *inline* specifier is ***Supported*** as defined in ISOC++ 10.1.6.

- The *explicit* specifier is ***Supported*** as defined in ISOC++ 10.1.2.

- The *virtual* specifier has ***Restricted Support*** as defined in ISOC++ 10.1.2 and 13.3.
  The restrictions are described in Section
  [14.10.3](14-classes.md#14103-virtual-functions).

### 9.1.3 The typedef specifier

The typedef specifier and alias declarations (`using X = T;`) are ***Supported*** as defined in ISOC++ 10.1.3.

### 9.1.4 The friend specifier

***Supported*** as defined in ISOC++ 10.1.4.

### 9.1.5 The constexpr specifier

The *constexpr* specifier (ISOC++ 10.1.5) is ***Supported***.

### 9.1.6 The inline specifier

The *inline* specifier for variables (ISOC++ 10.1.6) is ***Supported***.
Inline variables allow definition of constants in header files without violating the one-definition rule, which is useful for shared compile-time configuration across translation units.

### 9.1.7 Type specifiers

Type specifiers (ISOC++ 10.1.7) have ***Restricted Support***.

#### 9.1.7.1 cv-qualifiers

The cv-qualifiers (ISOC++ 10.1.7.1) have ***Restricted Support***.

1.  The cv-qualifier *const* is ***Supported***.

2.  The cv-qualifier *volatile* is ***Not Supported***.

#### 9.1.7.2 Simple type specifiers

Simple type specifiers (ISOC++ 10.1.7.2) have ***Restricted Support***.
They are ***Supported*** for types that are ***Supported*** as specified in
Sections [8](08-types.md#8-types), [14](14-classes.md#14-classes) and
[16](16-templates.md#16-templates).

The *decltype* specifier is also ***Supported***.

Support of the *auto* specifier is covered in
Section [9.1.7.4](09-declarations.md#9174-auto-specifier).

#### 9.1.7.3 Elaborated type specifiers

Elaborated type specifiers (ISOC++ 10.1.7.3) are ***Supported***.

#### 9.1.7.4 auto specifier

Auto type specifiers (ISOC++ 10.1.7.4) are ***Supported***.

***
**Note:**

ISOC++ updated the deduction rules for *auto* from braced-init-list (ISOC++ 10.1.7.4.1): direct initialization `auto x{1}` deduces the element type, while copy initialization `auto x = {1}` deduces `std::initializer_list`.

***

## 9.2 Enumerations

***Supported*** as defined in ISOC++ 10.2.

Direct list initialization of scoped and unscoped enumerations with a fixed underlying type (ISOC++ 11.6.4) is ***Supported***.

***
**Note:**

ISOC++ allows direct list initialization of enumerations with a fixed underlying type, e.g. `enum class byte : unsigned char { }; byte b{42};`

***

## 9.3 Namespaces

The support of namespaces (ISOC++ 10.3) is covered in
Section [13](13-namespaces.md#13-namespaces).

## 9.4 The asm declaration

***Not Supported***.

## 9.5 Linkage specifications

Linkage specifications (ISOC++ 10.5), such as `extern "C"` and `extern "C++"`, are ***Not Supported***.
See Section [4](04-translation-units.md#4-translation-units) on translation units.

## 9.6 Attributes

Attributes (ISOC++ 10.6) are ***Ignored*** as specified in the following sections.

### 9.6.1 Attribute syntax and semantics

Attribute syntax and semantics are ***Ignored***.

### 9.6.2 Alignment specifier

The *alignas* specifier is ***Ignored***.

### 9.6.3 Carries dependency attribute

The *carries_dependency* attribute is ***Ignored***.

### 9.6.4 Deprecated attribute

The *deprecated* attribute is ***Ignored***.

### 9.6.5 Noreturn attribute

The *noreturn* attribute is ***Ignored***.