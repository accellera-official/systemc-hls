---
style: chapter
---

# 12. Statements

Statements (as defined in ISOC++ Section 9) have ***Restricted Support***.

## 12.1 Labeled statement

Labels are ***Supported*** as defined in ISOC++ 9.1.

## 12.2 Expression statement

***Supported*** as defined in ISOC++ 9.2, including the null statement.

## 12.3 Compound statement

The *compound* statement (also, and equivalently, called *block*) is ***Supported***, as defined in
ISOC++ 9.3, to group sets of statements together.

## 12.4 Selection statements

Selection statements include *if*, *if-else*, and *switch* statements.

Selection statements are ***Supported*** as defined in ISOC++ 9.4.

### 12.4.1 The if statement

***Supported*** as defined in ISOC++ 9.4.1.

*constexpr if* statements (ISOC++ 9.4.1) are ***Supported***.
A *constexpr if* evaluates its condition at compile time, discarding the branch not taken. In a template, the discarded branch is not instantiated. This enables template-based hardware generation with compile-time specialization.

Init-statements in *if* statements (ISOC++ 9.4.1) are ***Supported***.
This allows declaring a variable scoped to the *if* statement, e.g. `if (auto v = expr; condition) { ... }`.

### 12.4.2 The switch statement

***Supported*** as defined in ISOC++ 9.4.2.

Init-statements in *switch* statements (ISOC++ 9.4.2) are ***Supported***.
This allows declaring a variable scoped to the *switch* statement.

## 12.5 Iteration statements

The iteration statements *while*, *do*, *for* and range-based *for* are ***Supported*** as defined
in ISOC++ 9.5.
The supported forms of infinite loop in a thread process are given in Section
[6.2.2](06-processes.md#622-thread-process-body).

## 12.6 Jump statements

Jump Statements (ISOC++ 9.6) have ***Restricted Support*** as described below.

### 12.6.1 The break statement

The *break* statement (as defined in ISOC++ 9.6.1) has ***Restricted Support***.

The restriction is that a *break* that exits the infinite loop of an *SC_CTHREAD* or *SC_THREAD*
process (see Section [6.2.2](06-processes.md#622-thread-process-body)) is ***Not Supported***.

### 12.6.2 The continue statement

The *continue* statement (as defined in ISOC++ 9.6.2) is ***Supported***.

### 12.6.3 The return statement

The *return* statement (as defined in ISOC++ 9.6.3) has ***Restricted Support***.

The restriction is that a *return* statement that occurs within the function defining an
*SC_CTHREAD* or *SC_THREAD* process (see Section
[6.2.2](06-processes.md#622-thread-process-body)) is ***Not Supported***.

### 12.6.4 The goto statement

The *goto* statement (as defined in ISOC++ 9.6.4) is ***Not Supported***.

## 12.7 Declaration statement

Declaration statements are ***Supported*** as defined in ISOC++ 9.7.

## 12.8 Exception handling

The exception handling mechanisms (as defined in ISOC++ Section 18) are ***Not Supported***.
These include the *try-block*, the *throw-expression*, and the function *terminate()*.
