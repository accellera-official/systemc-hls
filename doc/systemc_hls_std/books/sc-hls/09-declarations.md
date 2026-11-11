---
style: chapter
---

# 9. Declarations

Declarations, as defined for ISOC++ Section 10, have ***Restricted Support*** and those specific
restrictions and coding guidelines are listed in the subsequent sections and chapters.

The *static_assert* and *using* declarations (ISOC++ Section 10) are ***Supported***.

| NOTE |
|----|
| ISOC++ 2017 allows *static_assert* with a single argument (no message string required). |

## 9.1 Specifiers

Specifiers (ISOC++ 10.1) have ***Restricted Support***.

### 9.1.1 Storage class specifiers

Storage class specifiers (ISOC++ 10.1.1) have ***Restricted Support***.

1. The *register* specifier was removed from the language and is ***Not Supported***.
1. The *mutable* specifier is ***Supported***.
1. The *extern* specifier has ***Restricted Support***, as specified in Section
   [4.1](04-translation-units.md#41-translation-units-and-their-analysis).
1. The *static* specifier has ***Restricted Support***.
    1. Static *<u>class</u>* definitions are ***Supported***.
    1. Static variables have ***Restricted Support***. Only *const* static variables are supported.
    1. Static functions and static member functions are ***Supported***.
    1. Static data members of *<u>class</u>* have ***Restricted Support***.
       Only *const* static data members of *<u>class</u>* are ***Supported***.

| Note |
|----|
| The auto keyword used to be a storage class specifier in ISO/IEC 14882:2003 and was Ignored in the 1.4.7 version of the Accellera SystemC Synthesis Standard. |

Examples:
```cpp
static class my_class c; // Static class definition, supported

static void my_func() { } // Static function definition, supported

static int var; // Static non-const variable, not supported

static const int const_var1 = 5; // Static const variable, supported
static constexpr int const_var2 = 5; // Static const variable, supported

struct my_struct {
    static int member; // Non-const, not supported
    static const int const_member1;   // Static const variable, supported
    static constexpr int const_member2;   // Static const variable, supported
    static void my_member_func() { } // Static member function, supported
};
```

### 9.1.2 Function specifiers

The function specifiers, as defined in ISOC++ 10.1.2, have ***Restricted Support***.

- The *inline* specifier is ***Supported*** as defined in ISOC++.

- The *explicit* specifier is ***Supported*** as defined in ISOC++.

- The *virtual* specifier is ***Supported***.
  Virtual functions are supported with the limitations described in Section
  [14.10.3](14-classes.md#14103-virtual-functions)

### 9.1.3 The typedef specifier

***Supported*** as defined in ISOC++ 10.1.3.

### 9.1.4 The friend specifier

***Supported*** as defined in ISOC++ 10.1.4.

### 9.1.5 The constexpr specifier

The *constexpr* specifier (ISOC++ 10.1.5) is ***Supported***.

### 9.1.5a The inline specifier

The *inline* specifier for variables (ISOC++ 10.1.6) is ***Supported***.
Inline variables allow definition of constants in header files without violating the one-definition rule, which is useful for shared compile-time configuration across translation units.

### 9.1.6 Type specifiers

#### 9.1.6.1 Type specifiers (ISOC++ 10.1.7) have *Restricted Support*.

#### 9.1.6.2 cv-qualifiers

The cv-qualifiers (ISOC++ 10.1.7.1) have ***Restricted** **Supported***.

1.  The cv-qualifier *const* is ***Supported***

2.  The cv-qualifier *volatile* is ***Not Supported***.

#### 9.1.6.3 Simple type specifiers

Simple type specifiers (ISOC++ 10.1.7.2) have ***Restricted Support***.
They are ***Supported*** for types that are ***Supported*** as specified in
Sections [8](08-types.md#8-types), [14](14-classes.md#14-classes) and
[16](16-templates.md#16-templates).

The *decltype* specifier is also ***Supported***.

Support of the *auto* specifier is covered in
Section [9.1.6.5](09-declarations.md#9165-auto-specifier).

#### 9.1.6.4 Elaborated type specifiers

Elaborated type specifiers (ISOC++ 10.1.7.3) are ***Supported***.

#### 9.1.6.5 Auto specifier

Auto type specifiers (ISOC++ 10.1.7.4) are ***Supported***.

| NOTE |
|----|
| ISOC++ 2017 updated the deduction rules for *auto* from braced-init-list (ISOC++ 10.1.7.4.1): direct initialization `auto x{1}` deduces the element type, while copy initialization `auto x = {1}` deduces `std::initializer_list`. |

## 9.2 Enumerations

***Supported*** as defined in ISOC++ 10.2.

Direct list initialization of scoped and unscoped enumerations with a fixed underlying type (ISOC++ 11.6.4) is ***Supported***.

| NOTE |
|----|
| ISOC++ 2017 allows direct list initialization of enumerations with a fixed underlying type, e.g. `enum class byte : unsigned char { }; byte b{42};` |

## 9.3 Namespaces

The support of namespaces (ISOC++ 10.3) is covered in
Section [13](13-namespaces.md#namespaces).

## 9.4 The asm declaration

***Not supported***.

## 9.5 Linkage specifications

External linkage (as described in ISOC++ 10.5) is ***Not Supported***. i
See Section [4](04-translation-units.md#4-translation-units) on translation units.

## 9.6 Attributes

Attributes (ISOC++ 10.6) are ***Ignored*** as specified in the following sections.

### 9.6.1 Attribute syntax and semantics

Attribute syntax and semantics are ***Ignored***.

### 9.6.2 Alignment specifier

The *alignas* specifier is ***Ignored***.

### 9.6.3 Noreturn attribute

The *noreturn* attribute is ***Ignored***.

### 9.6.4 Carries dependency attribute

The *carries_dependency* attribute is ***Ignored***.

### 9.6.5 Deprecated attribute

The *deprecated* attribute is ***Ignored***.