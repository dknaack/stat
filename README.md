# stat

`stat` is a simple imperative programming language. It only contains basic features like `if` statements and `while` loops and it only uses numbers.
There is also a print and read statement in order to print and read in numbers respectively.

## Syntax

Here's an example program written in stat:

```
read a; /* read in the multiplier */
read b; /* read in the multiplicant */

c = 0;
while a != 0 {
  if a % 2 == 1 {
    c = c + b;
  }
  
  a = a / 2;
  b = b * 2;
}

print c; /* print out the product of a * b */
```

## Compiling stat

To compile stat simply use make:

    $ make

## Running stat

`stat` takes the source file as its first argument:

    $ stat source.stat
