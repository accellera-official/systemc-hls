---
style: chapter
---

# 14. Classes

Restrictions pertaining to classes defined in the SystemC LRM are specified in the respective
Section in this standard.

This Section describes the support for user-defined classes.

***Restricted Support*** as described below (ISOC++ Section 12).

## 14.1 Class names

***Supported*** as defined in ISOC++ 12.1.

## 14.2 Class members

***Supported*** as defined in ISOC++ 12.2.

## 14.3 Member functions

***Supported*** as defined in ISOC++ 12.2.1.

### 14.3.1 Nonstatic member functions

***Supported*** as defined in ISOC++ 12.2.2.

### 14.3.2 The this pointer

***Supported*** as defined in ISOC++ 12.2.2.1.

## 14.4 Static members

***Restricted Support***.

### 14.4.1 Static member functions

***Supported*** (ISOC++ 12.2.3.1) as specified in Section [9.1](09-declarations#91-specifiers).

### 14.4.2 Static data members

Static data members (ISOC++ 12.2.3.2) have ***Restricted Support*** as specified in
Section [9.1](09-declarations#91-specifiers).

## 14.5 Unions

***Not supported*** (ISOC++ 12.3).

## 14.6 Bit-fields

Bit-fields, as defined in ISOC++ 12.2.4, are ***Not Supported***.

## 14.7 Nested class declarations

***Supported*** as defined in ISOC++ 12.2.5.

## 14.8 Local class declarations

***Supported*** as defined in ISOC++ 12.4.

## 14.9 Nested type names

***Supported*** as defined in ISOC++ 12.2.6.

##  14.10 Derived classes

***Supported*** as defined in ISOC++ Section [14](14-classes.md#14-classes).

###  14.10.1 Multiple base classes

***Supported*** as defined in ISOC++ 13.1.

###  14.10.2 Member name lookup

***Supported*** as defined in ISOC++ 13.2.

###  14.10.3 Virtual functions

Virtual functions, as defined in ISOC++ 13.3, have ***Restricted Support*** provided that when such
functions are called, the type of the "*this*" object can be statically determined.

###  14.10.4 Abstract classes

***Supported*** as defined in ISOC++ 13.4.

## 14.11 Member access control

***Supported*** as defined in ISOC++ Section 14.

###  14.11.1 Access specifiers

***Supported*** as defined in ISOC++ 14.1.

###  14.11.2 Accessibility of base classes and base class members

***Supported*** as defined in ISOC++ 14.2.

###  14.11.3 Access declarations

***Supported*** as defined in ISOC++ 14.3.

###  14.11.4 Friends

***Supported*** as defined in ISOC++ 14.4.

###  14.11.5 Protected member access

***Supported*** as defined in ISOC++ 14.5.

###  14.11.6 Access to virtual functions

***Supported*** as defined in ISOC++ 14.6.

###  14.11.7 Multiple access

***Supported*** as defined in ISOC++ 14.7.

###  14.11.8 Nested classes

***Supported*** as defined in ISOC++ 14.7.

## 14.12 Special member functions

***Restricted Support*** as described below (ISOC++ Section 15).

###  14.12.1 Constructors

Constructors of user defined classes (as defined in ISOC++ 15.1) are ***Supported***.

###  14.12.2 Temporary objects

***Supported*** as defined in ISOC++ 15.2.

###  14.12.3 Conversions

***Supported*** as defined in ISOC++ 15.3.

####  14.12.3.1 Conversion by constructor

***Supported*** as defined in ISOC++ 15.3.1.

####  14.12.3.1 Conversion functions

***Supported*** as defined in ISOC++ 15.3.2.

###  14.12.4 Destructors

***Supported*** as defined in ISOC++ 15.4.

###  14.12.5 Free store

***Not supported*** (ISOC++ 15.5).

###  14.12.6 Initialization

***Supported*** (ISOC++ 15.6).

####  14.12.6.1 Explicit initialization

*Explicit initialization* (as defined in ISOC++ 15.6.1) is ***Supported***.

####  14.12.6.2 Initializing bases and members

***Supported*** as defined in ISOC++ 15.6.2.

### 14.12.7 Copying class objects

***Supported*** as defined in ISOC++ 15.8.
