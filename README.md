# Tiny Compiler

This project is a fully functional compiler for the TINY programming language, which is a language that supports basic arithmetic, boolean, and control flow operations. The compiler can scan, parse, and run code written in the TINY language.

## TINY Language Rules

- A program is a sequence of statements separated by semicolons (`;`).
- Comments are enclosed in curly braces (`{}`).
- Input/output operations are performed by `read` and `write` statements.
- Arithmetic expressions are composed of integers, variables, and operators (`+`, `-`, `*`, `/`, `^`).
- Boolean expressions are composed of two arithmetic expressions and a relational operator (`<`, `=`).
- Control flow statements are `repeat` and `if`. 
  - `repeat` has the form `repeat <statement> until <boolean>;`
  - `if` has the form `if <boolean> then <statement> [else <statement>] end;`
- Variable names can include only alphabetic characters (`a-z` or `A-Z`) and underscores (`_`).
- Variables are declared simply by assigning values to them using the assignment operator (`:=`).
- All variables are integers.
- There are no procedures or declarations.

## Compiler Implementation

The compiler is implemented in three phases:

### 1. Scanning (Tokenization)
The scanner takes a source code file written in the TINY language as an input and outputs a text file that contains the tokens found in the input file. Each token is represented on a single line in the following format: `[line number] [token type] [token value]`. For example:

    ```
    [1] if (If)
    [1] x (ID)
    [2] 200 (Num) 
    ```

### 2. Parsing (Syntax Analysis)
The parser takes the path of a source code file written in the TINY language as an input and outputs the terminal (leaf) nodes of the parse tree of the input file to the terminal or throws an exception once an error is found. For example:

    ```
    input file content: if x < y
    output:
    [If]
       [Oper][LessThan]
          [ID][x]
          [ID][y]

    input file content: temp := 20
    output:
    [Assign][temp]
          [Num][20]
    ```

### 3. Code Generation (Semantic Analysis)
The code generator takes the path of a source code file written in the TINY language as an input and outputs three things:

  - The symbol table of the code, which contains all the named variables and their attributes. For each variable, the following information is displayed: `[Var=variable name][Mem=initial value of the variable][Line=line numbers where the variable appears]`. For example:

      ```
      [Var=x][Mem=0][Line=2][Line=3][Line=6][Line=7]
      [Var=fact][Mem=1][Line=4][Line=6]
      ```

  - The syntax tree of the code, which is similar to the output of the previous phase but with each non-void (Integer or Boolean) node followed by its data type in square brackets. For example:

      ```
      input file content: if x < y
      output:
      [If]
         [Oper][LessThan][Boolean]
            [ID][x][Integer]
            [ID][y][Integer]

      input file content: temp := 20
      output:
      [Assign]
         [ID][temp][Integer]
            [Num][20][Integer]
      ```

  - The simulation of the compilation of the code, which mimics the execution of the code as if it was running on a machine. The only parts that interact with the terminal in this step are `read` and `write` statements. For example:

      ```
      input file content: read x; write x + 5;
      output:
      Enter x: 10
      val: 15
      ```


## Installation and Usage

To install and run the Tiny Compiler, you need to have C++ installed on your system. You can download C++ from [here].

To install the Tiny Compiler, you can clone this repository or download it as a zip file. To clone the repository, run the following command in your terminal:
```
git clone https://github.com/KhaledAshrafH/Tiny-Compiler.git
```

To compile and run the Tiny Compiler, you need to specify the path of the source code file written in the TINY language as an argument. For example, if your source code file is called `input.txt` and it is located in the same directory as the Tiny Compiler, you can run the following commands in your terminal:
```
g++ main.cpp -o tiny_compiler_main ./tiny_compiler_main input.txt
```

This will output the results of the three phases of the compiler: scanning, parsing, and code generation.


## Testing
### Testcase 1
Source code file:

```
{ factorial program }
read n;
fact := 1;
repeat
  fact := fact * n;
  n := n - 1;
until n = 0;
write fact;
```

Output:

```
Symbol table:
[Var=n][Mem=0][Line=2][Line=5][Line=6]
[Var=fact][Mem=1][Line=3][Line=4]

Syntax tree:
[Read]
   [ID][n][Integer]
[Assign][fact]
   [Num][1][Integer]
[Repeat]
   [Oper][Equal][Boolean]
      [ID][n][Integer]
      [Num][0][Integer]
   [Assign][fact]
      [Oper][Times][Integer]
         [ID][fact][Integer]
         [ID][n][Integer]
   [Assign][n]
      [Oper][Minus][Integer]
         [ID][n][Integer]
         [Num][1][Integer]
[Write]
   [ID][fact][Integer]

Simulation:
Enter n: 5
val: 120
```
### Testcase 2
Source code file:
```
{ Fibonacci sequence }
read n;
a := 0;
b := 1;
write a;
write b;
repeat
  c := a + b;
  write c;
  a := b;
  b := c;
until c > n;
```
Output:
```
Symbol table:
[Var=n][Mem=0][Line=2]
[Var=a][Mem=0][Line=3][Line=7][Line=9]
[Var=b][Mem=1][Line=4][Line=8][Line=10]
[Var=c][Mem=0][Line=6]

Syntax tree:
[Read]
   [ID]n[Integer
[Assign]a
   [Num]0[Integer
[Assign]b
   [Num]1[Integer
[Write]a
   [ID]a[Integer
[Write]b
   [ID]b[Integer
[Repeat]
   [Oper]GreaterThan[Boolean
      [ID]c[Integer
      [ID]n[Integer
   [Assign]c
      [Oper]Plus[Integer
         [ID]a[Integer
         [ID]b[Integer
   [Write]c
      [ID]c[Integer
   [Assign]a
      [ID]b[Integer
   [Assign]b
      [ID]c[Integer

Simulation:
Enter n: 10
val: 0
val: 1
val: 1
val: 2
val: 3
val: 5
val: 8
val: 13

```


## License

This project is licensed under the MIT License - see the LICENSE file for details.
