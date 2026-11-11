---
style: chapter
---

#  12. Statements

Statements (as defined in ISOC++ Section 9) have ***Restricted Support***.

## Labeled statement

Labels are ***Supported*** as defined in ISOC++ 9.1.

## Compound statement

The *compound* statement (also, and equivalently, called *block*) is supported, as defined in
ISOC++ 9.3, to group sets of statements together.

## Selection statements

Selection statements include *if*, *if-else*, and *switch* statements.

Selection statements are ***Supported*** as defined in ISOC++ 9.4.

### The if statement

***Supported*** as defined in ISOC++ 9.4.1.

*constexpr if* statements are ***Supported***.
A *constexpr if* evaluates its condition at compile time, discarding the branch not taken. This enables template-based hardware generation with compile-time specialization.

Init-statements in *if* statements (ISOC++ 9.4.1) are ***Supported***.
This allows declaring a variable scoped to the *if* statement, e.g. `if (auto v = expr; condition) { ... }`.

### The switch statement

***Supported*** as defined in ISOC++ 9.4.2;

Init-statements in *switch* statements (ISOC++ 9.4.2) are ***Supported***.
This allows declaring a variable scoped to the *switch* statement.

## Iteration statements

The iteration statements *while*, *do*, *for* and range-based *for* are ***Supported*** as defined
in ISOC++ 9.5.

## Jump statements

Jump Statements (ISOC++ 9.6) have ***Restricted Support*** as described below.

### The break statement

The *break* statement (as defined in ISOC++ 9.6.1) has ***Restricted Support***.

The restriction is specified in Section [6](06-processes.md#6-processes): a *break* that exits the
infinite loop of an *SC_CTHREAD* or *SC_THREAD* process is ***Not Supported***.

### The continue statement

The *continue* statement (as defined in ISOC++ 9.6.2) is ***Supported***.

### The return statement

The *return* statement (as defined in ISOC++ 9.6.3) has ***Restricted Support***.

The restriction is specified in Section [6](06-processes.md#6-processes): a *return* statement that
occurs within the function defining an *SC_CTHREAD* or *SC_THREAD* process is ***Not Supported***.

### The goto statement

The *goto* statement (as defined in ISOC++ 9.6.4) is ***Not Supported***.

## Declaration statement

Declaration statements are ***Supported*** as defined in ISOC++ Section 10.

## Exception handling statements

The exception handling mechanisms (as defined in ISOC++ Section 18) are ***Not Supported***.
These include the *try*, *catch()*, and *throw* statements, and the special functions *terminate()*,
*unexpected()*, and *uncaught_exception()*.
