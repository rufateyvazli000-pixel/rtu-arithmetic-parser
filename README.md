# Arithmetic Parser in C

RTU graded laboratory assignment.

Student: Rufat Eyvazli
Student ID: 251ADB127

## Compilation

```bash
gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
```

## Usage

Single file:
```bash
./calc input.txt
```

Custom output directory:
```bash
./calc -o results input.txt
```

Directory processing:
```bash
./calc -d input_folder
```

Long options:
```bash
./calc --dir input_folder --output-dir results
```

## Supported Operations

- Addition and subtraction
- Multiplication and division
- Exponentiation (**)
- Parentheses
- Integer and decimal numbers

## Error Handling

Invalid expressions produce ERROR:<position>.
Positions are counted from the beginning of the input file.

## AI Assistance
OpenAI ChatGPT was used to explain C programming concepts
and assist with writing and improving code comments.
The program implementation was written by the student.
