---
style: chapter
---

# 16. Templates

The support for templates (ISOC++ Section 17) in this standard is explained below.

## 16.1 Template parameters

***Supported*** (ISOC++ 17.1).

Declaring non-type template parameters with *auto* (ISOC++ 17.1) is ***Supported***.
This allows the type of a non-type template parameter to be deduced, e.g. `template<auto N>`.

## 16.2 Names of template specializations

***Supported*** as defined in ISOC++ 17.2.

## 16.3 Template arguments

***Supported*** as defined in ISOC++ 17.3.

### 16.3.1 Template type arguments

***Supported*** as defined in ISOC++ 17.3.1.

### 16.3.2 Template non-type arguments

***Supported*** as defined in ISOC++ 17.3.2.

### 16.3.3 Template template arguments

***Supported*** as defined in ISOC++ 17.3.3.

## 16.4 Type equivalence

***Supported*** as defined in ISOC++ 17.4.

## 16.5 Template declarations

***Supported*** as defined in ISOC++ 17.5.

### 16.5.1 Class templates

***Supported*** as defined in ISOC++ 17.5.1.

#### 16.5.1.1 Member functions of class templates

***Supported*** as defined in ISOC++ 17.5.1.1.

#### 16.5.1.2 Member classes of class templates

***Supported*** as defined in ISOC++ 17.5.1.2.

#### 16.5.1.3 Static data members of class templates

***Restricted Support*** (ISOC++ 17.5.1.3).
See Section [14.4.2](14-classes.md#1442-static-data-members) for static data members of classes and
Section [9.1.1](09-declarations.md#911-storage-class-specifiers) for the static storage class specifier.

### 16.5.2 Member templates

***Supported*** as defined in ISOC++ 17.5.2.

### 16.5.3 Variadic templates

***Supported*** as defined in ISOC++ 17.5.3.

### 16.5.4 Friends

***Supported*** as defined in ISOC++ 17.5.4.

### 16.5.5 Class template partial specializations

***Supported*** as defined in ISOC++ 17.5.5.

#### 16.5.5.1 Matching of class template partial specializations

***Supported*** as defined in ISOC++ 17.5.5.1.

#### 16.5.5.2 Partial ordering of class template specializations

***Supported*** as defined in ISOC++ 17.5.5.2.

#### 16.5.5.3 Members of class template specializations

***Supported*** as defined in ISOC++ 17.5.5.3.

### 16.5.6 Function templates

***Supported*** as defined in ISOC++ 17.5.6.

#### 16.5.6.1 Function template overloading

***Supported*** as defined in ISOC++ 17.5.6.1.

#### 16.5.6.2 Partial ordering of function templates

***Supported*** as defined in ISOC++ 17.5.6.2.

### 16.5.7 Alias templates

***Supported*** as defined in ISOC++ 17.5.7.

## 16.6 Name resolution

***Supported*** as defined in ISOC++ 17.6.

### 16.6.1 Locally declared names

***Supported*** as defined in ISOC++ 17.6.1.

### 16.6.2 Dependent names

***Supported*** as defined in ISOC++ 17.6.2.

#### 16.6.2.1 Dependent types

***Supported*** as defined in ISOC++ 17.6.2.1.

#### 16.6.2.2 Type-dependent expressions

***Supported*** as defined in ISOC++ 17.6.2.2.

#### 16.6.2.3 Value-dependent expressions

***Supported*** as defined in ISOC++ 17.6.2.3.

#### 16.6.2.4 Dependent template arguments

***Supported*** as defined in ISOC++ 17.6.2.4.

### 16.6.3 Non-dependent names

***Supported*** as defined in ISOC++ 17.6.3.

### 16.6.4 Dependent name resolution

***Supported*** as defined in ISOC++ 17.6.4.

#### 16.6.4.1 Point of instantiation

***Supported*** as defined in ISOC++ 17.6.4.1.

#### 16.6.4.2 Candidate functions

***Supported*** as defined in ISOC++ 17.6.4.2.

### 16.6.5 Friend names declared within a class template

***Supported*** as defined in ISOC++ 17.6.5.

## 16.7 Template instantiation and specialization

***Supported*** as defined in ISOC++ 17.7.

### 16.7.1 Implicit instantiation

***Supported*** as defined in ISOC++ 17.7.1.

### 16.7.2 Explicit instantiation

***Supported*** as defined in ISOC++ 17.7.2.

### 16.7.3 Explicit specialization

***Supported*** as defined in ISOC++ 17.7.3.

## 16.8 Function template specializations

***Supported*** as defined in ISOC++ 17.8.

### 16.8.1 Explicit template argument specification

***Supported*** as defined in ISOC++ 17.8.1.

### 16.8.2 Template argument deduction

***Supported*** as defined in ISOC++ 17.8.2.

#### 16.8.2.1 Deducing template arguments from a function call

***Supported*** as defined in ISOC++ 17.8.2.1.

#### 16.8.2.2 Deducing template arguments taking the address of a function template

***Supported*** as defined in ISOC++ 17.8.2.2.

#### 16.8.2.3 Deducing conversion function template arguments

***Supported*** as defined in ISOC++ 17.8.2.3.

#### 16.8.2.4 Deducing template arguments during partial ordering

***Supported*** as defined in ISOC++ 17.8.2.4.

#### 16.8.2.5 Deducing template arguments from a type

***Supported*** as defined in ISOC++ 17.8.2.5.

#### 16.8.2.6 Deducing template arguments from a function declaration

***Supported*** as defined in ISOC++ 17.8.2.6.

### 16.8.3 Overload resolution

***Supported*** as defined in ISOC++ 17.8.3.

## 16.9 Deduction guides

Class template argument deduction (CTAD) and deduction guides (ISOC++ 17.9) are ***Supported***.
CTAD allows the template arguments of a class template to be deduced from the constructor arguments, reducing verbosity when instantiating template classes.

