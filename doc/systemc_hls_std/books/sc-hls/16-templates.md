---
style: chapter
---


#  16. Templates

***Supported*** as described below (ISOC++ Section 17).

## Template parameters

***Supported*** (ISOC++ 17.1).

Declaring non-type template parameters with *auto* (ISOC++ 17.1) is ***Supported***.
This allows the type of a non-type template parameter to be deduced, e.g. `template<auto N>`.

## Names of template specializations

***Supported*** as defined in ISOC++ 17.2.

## Template arguments

***Supported*** as defined in ISOC++ 17.3.

### Template type arguments

***Supported*** as defined in ISOC++ 17.3.1.

### Template non-type arguments

***Supported*** as defined in ISOC++ 17.3.2.

### Template template arguments

***Supported*** as defined in ISOC++ 17.3.3.

## Type equivalence

***Supported*** as defined in ISOC++ 17.4.

## Template declarations

***Supported*** as defined in ISOC++ 17.5.

### Class Templates

***Supported*** as defined in ISOC++ 17.5.1.

#### Member functions of class templates

***Supported*** as defined in ISOC++ 17.5.1.1.

#### Member classes of class templates

***Supported*** as defined in ISOC++ 17.5.1.2.

#### Static data members of class templates

***Restricted Support*** (ISOC++ 17.5.1.3).
See Section [14.4.2](14-classes.md#1442-static-data-members) for static data members of classes and
Section [9.1.1](09-declarations#911-storage-class-specifiers) for the static storage class specifier.

### Member templates

***Supported*** as defined in ISOC++ 17.5.2.

### Variadic Templates

***Supported*** as defined in ISOC++ 17.5.3.

### Friends

***Supported*** as defined in ISOC++ 17.5.4.

### Class template partial specializations

***Supported*** as defined in ISOC++ 17.5.5.

#### Matching of class template partial specializations

***Supported*** as defined in ISOC++ 17.5.5.1.

#### Partial ordering of class template specializations

***Supported*** as defined in ISOC++ 17.5.5.2.

#### Members of class template specializations

***Supported*** as defined in ISOC++ 17.5.5.3.

### Function templates

***Supported*** as defined in ISOC++ 17.5.6.

#### Function template overloading

***Supported*** as defined in ISOC++ 17.5.6.1.

#### Partial ordering of function templates

***Supported*** as defined in ISOC++ 17.5.6.2.

### Alias templates

***Supported*** as defined in ISOC++ 17.5.7.

## Name resolution

***Supported*** as defined in ISOC++ 17.6.

### Locally declared names

***Supported*** as defined in ISOC++ 17.6.1.

### Dependent names

***Supported*** as defined in ISOC++ 17.6.2.

#### Dependent types

***Supported*** as defined in ISOC++ 17.6.2.1.

#### Type-dependent expressions

***Supported*** as defined in ISOC++ 17.6.2.2.

#### Value-dependent expressions

***Supported*** as defined in ISOC++ 17.6.2.3.

#### Dependent template arguments

***Supported*** as defined in ISOC++ 17.6.2.4.

### Non-dependent names

***Supported*** as defined in ISOC++ 17.6.3.

### Dependent name resolution

***Supported*** as defined in ISOC++ 17.6.4.

#### Point of instantiation

***Supported*** as defined in ISOC++ 17.6.4.1.

#### Candidate functions

***Supported*** as defined in ISOC++ 17.6.4.2.

### Friend names declared within a class template

***Supported*** as defined in ISOC++ 17.6.5.

## Template instantiation and specialization

***Supported*** as defined in ISOC++ 17.7.

### Implicit instantiation

***Supported*** as defined in ISOC++ 17.7.1.

### Explicit instantiation

***Supported*** as defined in ISOC++ 17.7.2.

### Explicit specialization

***Supported*** as defined in ISOC++ 17.7.3.

## Function template specializations

***Supported*** as defined in ISOC++ 17.8.

### Explicit template argument specification

***Supported*** as defined in ISOC++ 17.8.1.

### Template argument deduction

***Supported*** as defined in ISOC++ 17.8.2.

#### Deducing template arguments from a function call

***Supported*** as defined in ISOC++ 17.8.2.1.

#### Deducing template arguments taking the address of a function template

***Supported*** as defined in ISOC++ 17.8.2.2.

#### Deducing conversion function template arguments

***Supported*** as defined in ISOC++ 17.8.2.3.

#### Deducing template arguments during partial ordering

***Supported*** as defined in ISOC++ 17.8.2.4.

#### Deducing template arguments from a type

***Supported*** as defined in ISOC++ 17.8.2.5.

#### Deducing template arguments from a function declaration

***Supported*** as defined in ISOC++ 17.8.2.6.

### Overloaded resolution

***Supported*** as defined in ISOC++ 17.8.3.

## Deduction guides

Class template argument deduction (CTAD) and deduction guides (ISOC++ 17.9) are ***Supported***.
CTAD allows the template arguments of a class template to be deduced from the constructor arguments, reducing verbosity when instantiating template classes.

