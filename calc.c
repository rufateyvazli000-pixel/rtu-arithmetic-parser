// Rufat Eyvazli — Student ID: 251ADB127
// Repository:https://github.com/rufateyvazli000-pixel/rtu-arithmetic-parser
// Compile: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// AI explanation topics: OpenAI ChatGPT (GPT-6), used for conceptual explanations,
// and clarification of C programming concepts.
// AI conversation : https://chatgpt.com/share/6adec106-b60c-83ed-bfda-4ae66a34c011
// AI explanation topics are identified by comments near the relevant code.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
// Each token has a type, value and position.
typedef enum {
    TOK_NUMBER,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_POWER,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_END,
    TOK_INVALID
} TokenType;

// A number can be an integer or a floating-point value.
typedef struct {
    long long integer;
    double floating;
    int is_float;
} Value;

typedef struct {
    TokenType type;
    Value value;
    size_t position;
} Token;

typedef struct {
    const char *input;
    size_t index;
    Token current;
    size_t error_position;
    int has_error;
} Parser;
// Part 1 — Tokenization
// AI explanation topics: Conceptual explanations for recognizing
// numbers, operators, token positions, and invalid input.
static Token next_token(const char *input, size_t *index) {
    // Skip spaces and tabs between tokens.
    while (input[*index] == ' ' || input[*index] == '\t') {
        (*index)++;
    }

    size_t start = *index;
    char c = input[*index];

    Token token = {0};
    token.type = TOK_INVALID;
    token.position = start + 1;

    if (c == '\0' || c == '\n') {
        token.type = TOK_END;
        return token;
    }

    if (isdigit((unsigned char)c)) {
    long long number = 0;
    int overflow = 0;

    while (isdigit((unsigned char)input[*index])) {
        int digit = input[*index] - '0';

        // Check integer overflow before multiplication.
        if (number > (LLONG_MAX - digit) / 10) {
            overflow = 1;
        }

        if (!overflow) {
            number = number * 10 + digit;
        }

        (*index)++;
    }

    if (input[*index] == '.') {
        (*index)++;

        // A decimal point must be followed by a digit.
        if (!isdigit((unsigned char)input[*index])) {
            token.type = TOK_INVALID;
            token.position = *index;
            return token;
        }

        while (isdigit((unsigned char)input[*index])) {
            (*index)++;
        }

        // strtod reads the complete decimal number.
        token.value.floating = strtod(input + start, NULL);

// Concept explained with AI: reject non-finite floating-point literals.
if (!isfinite(token.value.floating)) {
    token.type = TOK_INVALID;
    token.position = start + 1;
    return token;
}
        token.value.is_float = 1;
    } else {
        if (overflow) {
            token.type = TOK_INVALID;
            return token;
        }

        token.value.integer = number;
        token.value.is_float = 0;
    }

    token.type = TOK_NUMBER;
    return token;
}


    (*index)++;

    switch (c) {
        case '+':
            token.type = TOK_PLUS;
            break;
        case '-':
            token.type = TOK_MINUS;
            break;
        case '*':
            if (input[*index] == '*') {
                (*index)++;
                token.type = TOK_POWER;
            } else {
                token.type = TOK_MULTIPLY;
            }
            
            break;
        case '/':
            token.type = TOK_DIVIDE;
            break;
        case '(':
            token.type = TOK_LPAREN;
            break;
        case ')':
            token.type = TOK_RPAREN;
            break;
        default:
            token.type = TOK_INVALID;
            break;
    }

    return token;
}

static void advance(Parser *p) {
    p->current = next_token(p->input, &p->index);
}

static void parser_error(Parser *p) {
    if (!p->has_error) {
        p->has_error = 1;
        p->error_position = p->current.position;
    }
}


// Concept explained with AI: keep integer and floating-point results distinct.
static Value expr(Parser *p);

static Value int_value(long long n) {
    Value v = {0};
    v.integer = n;
    return v;
}

static Value float_value(double n) {
    Value v = {0};
    v.floating = n;
    v.is_float = 1;
    return v;
}

static double as_double(Value v) {
    return v.is_float ? v.floating : (double)v.integer;
}

// Part 2 — Expression parsing
// AI explanation topics: Conceptual explanations for recursive-descent
// parsing, parentheses, and operator precedence.
static Value primary(Parser *p) {
    if (p->current.type == TOK_NUMBER) {
        Value value = p->current.value;
        advance(p);
        return value;
    }

    if (p->current.type == TOK_LPAREN) {
        advance(p);
        Value value = expr(p);

        if (p->has_error) return int_value(0);

        if (p->current.type != TOK_RPAREN) {
            parser_error(p);
            return int_value(0);
        }

        advance(p);
        return value;
    }

    parser_error(p);
    return int_value(0);
}

// Right-associative exponentiation: 2**3**2 = 512.
// Part 3 — Exponentiation
// AI explanation topics: Conceptual explanations for
// right-associative powers and arithmetic overflow checking.
static Value power(Parser *p) {
    Value left = primary(p);
    if (p->has_error) return int_value(0);

    if (p->current.type == TOK_POWER) {
        size_t operator_position = p->current.position;
        advance(p);
        size_t exponent_position = p->current.position;
        Value right = power(p);

        if (p->has_error) return int_value(0);

        if (!right.is_float && right.integer < 0) {
            p->has_error = 1;
            p->error_position = exponent_position;
            return int_value(0);
        }

        if (left.is_float || right.is_float) {
            double result = pow(as_double(left), as_double(right));

            if (!isfinite(result)) {
                p->has_error = 1;
                p->error_position = operator_position;
                return int_value(0);
            }

            return float_value(result);
        }

        long long result = 1;
        long long base = left.integer;
        unsigned long long exponent = (unsigned long long)right.integer;

        // Exponentiation by squaring avoids huge iteration counts.
        while (exponent != 0) {
            if (exponent & 1ULL) {
                if (__builtin_mul_overflow(result, base, &result)) {
                    p->has_error = 1;
                    p->error_position = operator_position;
                    return int_value(0);
                }
            }

            exponent >>= 1;

            if (exponent != 0 &&
                __builtin_mul_overflow(base, base, &base)) {
                p->has_error = 1;
                p->error_position = operator_position;
                return int_value(0);
            }
        }

        return int_value(result);
    }

    return left;
}

// Part 4 — Multiplication and division
// AI explanation topics: Conceptual explanations for arithmetic operations,
// integer division, and division-by-zero handling.
static Value term(Parser *p) {
    Value result = power(p);

    while (!p->has_error &&
           (p->current.type == TOK_MULTIPLY ||
            p->current.type == TOK_DIVIDE)) {

        TokenType op = p->current.type;
        size_t operator_position = p->current.position;
        advance(p);
        size_t operand_position = p->current.position;
        Value right = power(p);

        if (p->has_error) return int_value(0);

        if (op == TOK_DIVIDE && as_double(right) == 0.0) {
            p->has_error = 1;
            p->error_position = operand_position;
            return int_value(0);
        }

        if (result.is_float || right.is_float) {
            double a = as_double(result);
            double b = as_double(right);
            double answer = op == TOK_MULTIPLY ? a * b : a / b;

            if (!isfinite(answer)) {
                p->has_error = 1;
                p->error_position = operator_position;
                return int_value(0);
            }

            result = float_value(answer);
        } else {
            long long answer;

            if (op == TOK_MULTIPLY) {
                if (__builtin_mul_overflow(result.integer, right.integer,
                                           &answer)) {
                    p->has_error = 1;
                    p->error_position = operator_position;
                    return int_value(0);
                }
            } else {
                if (result.integer == LLONG_MIN && right.integer == -1) {
                    p->has_error = 1;
                    p->error_position = operator_position;
                    return int_value(0);
                }
                answer = result.integer / right.integer;
            }

            result = int_value(answer);
        }
    }

    return result;
}

// Part 5 — Addition and subtraction
// AI explanation topics: Conceptual explanations for evaluating
// expressions and detecting arithmetic overflow.
static Value expr(Parser *p) {
    Value result = term(p);

    while (!p->has_error &&
           (p->current.type == TOK_PLUS ||
            p->current.type == TOK_MINUS)) {

        TokenType op = p->current.type;
        size_t operator_position = p->current.position;
        advance(p);
        Value right = term(p);

        if (p->has_error) return int_value(0);

        if (result.is_float || right.is_float) {
            double a = as_double(result);
            double b = as_double(right);
            double answer = op == TOK_PLUS ? a + b : a - b;

            if (!isfinite(answer)) {
                p->has_error = 1;
                p->error_position = operator_position;
                return int_value(0);
            }

            result = float_value(answer);
        } else {
            long long answer;
            int overflow = op == TOK_PLUS
                ? __builtin_add_overflow(result.integer, right.integer, &answer)
                : __builtin_sub_overflow(result.integer, right.integer, &answer);

            if (overflow) {
                p->has_error = 1;
                p->error_position = operator_position;
                return int_value(0);
            }

            result = int_value(answer);
        }
    }

    return result;
}


// Concept explained with AI: read arbitrarily long input lines.
// Part 6 — Dynamic input reading
// AI explanation topics: Conceptual explanations for dynamic memory
// allocation and processing long input lines.
static long long read_line(FILE *input, char **buffer, size_t *capacity) {
    size_t length = 0;
    int c = EOF;

    while ((c = fgetc(input)) != EOF) {
        if (length + 2 > *capacity) {
            size_t new_capacity = *capacity ? *capacity * 2 : 256;

            if (new_capacity <= *capacity ||
                new_capacity > (size_t)LLONG_MAX) {
                return -2;
            }

            char *new_buffer = realloc(*buffer, new_capacity);
            if (!new_buffer) return -2;

            *buffer = new_buffer;
            *capacity = new_capacity;
        }

        (*buffer)[length++] = (char)c;
        if (c == '\n') break;
    }

    if (ferror(input)) return -2;
    if (length == 0 && c == EOF) return -1;

    if (length + 1 > *capacity) {
        char *new_buffer = realloc(*buffer, length + 1);
        if (!new_buffer) return -2;
        *buffer = new_buffer;
        *capacity = length + 1;
    }

    (*buffer)[length] = '\0';
    return (long long)length;
}

// Join a directory and filename.
static char *join_path(const char *dir, const char *name) {
    size_t a = strlen(dir), b = strlen(name);

    if (a > SIZE_MAX - b - 2) return NULL;

    char *path = malloc(a + b + 2);
    if (!path) return NULL;

    snprintf(path, a + b + 2, "%s/%s", dir, name);
    return path;
}

// Create an output directory if it does not already exist.
static int ensure_directory(const char *path) {
    if (mkdir(path, 0777) == 0) return 0;

    if (errno == EEXIST) {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
            return 0;
    }

    perror(path);
    return -1;
}

// Get the filename without its final .txt extension.
static char *input_base(const char *path) {
    const char *name = strrchr(path, '/');
    name = name ? name + 1 : path;

    size_t len = strlen(name);
    if (len >= 4 && strcmp(name + len - 4, ".txt") == 0)
        len -= 4;

    char *base = malloc(len + 1);
    if (!base) return NULL;

    memcpy(base, name, len);
    base[len] = '\0';
    return base;
}

// Process one input file and create its required output file.
// Part 7 — File processing
// AI explanation topics: Conceptual explanations for
// input processing, error positions, and output generation.
static int process_file(const char *input_path, const char *outdir) {
    int status = 1;
    FILE *input = NULL;
    FILE *output = NULL;
    char *base = NULL;
    char *output_name = NULL;
    char *output_path = NULL;
    char *line = NULL;
    size_t capacity = 0;
    size_t file_offset = 0;
    size_t error_position = 0;
    int has_error = 0;

    input = fopen(input_path, "r");
    if (!input) {
        perror(input_path);
        goto cleanup;
    }

    base = input_base(input_path);
    if (!base) goto cleanup;

    const char *suffix = "_Rufat_Eyvazli_251ADB127.txt";
    size_t name_length = strlen(base) + strlen(suffix) + 1;
    output_name = malloc(name_length);
    if (!output_name) goto cleanup;

    snprintf(output_name, name_length, "%s%s", base, suffix);

    output_path = join_path(outdir, output_name);
    if (!output_path) goto cleanup;

    output = fopen(output_path, "w");
    if (!output) {
        perror(output_path);
        goto cleanup;
    }

    for (;;) {
        long long length = read_line(input, &line, &capacity);

        if (length == -1) break;
        if (length == -2) {
            fprintf(stderr, "Failed to read input\n");
            goto cleanup;
        }

        size_t line_length = (size_t)length;
        char *start = line;

        while (*start == ' ' || *start == '\t')
            start++;

        if (*start == '\n' || *start == '\0' || *start == '#') {
            file_offset += line_length;
            continue;
        }

        Parser p = {0};
        p.input = start;
        advance(&p);

        Value result = expr(&p);

        if (!p.has_error && p.current.type != TOK_END)
            parser_error(&p);

        if (p.has_error) {
            error_position = file_offset +
                             (size_t)(start - line) +
                             p.error_position;
            has_error = 1;
            break;
        }

        if (result.is_float)
            fprintf(output, "%.15g\n", result.floating);
        else
            fprintf(output, "%lld\n", result.integer);

        file_offset += line_length;
    }

    if (has_error) {
        if (fclose(output) != 0) {
            output = NULL;
            goto cleanup;
        }
        output = fopen(output_path, "w");
        if (!output) {
            perror(output_path);
            goto cleanup;
        }
        fprintf(output, "ERROR:%zu\n", error_position);
    }

    if (fclose(output) != 0) {
        output = NULL;
        goto cleanup;
    }
    output = NULL;
    status = 0;

cleanup:
    if (output) fclose(output);
    if (input) fclose(input);
    free(base);
    free(output_name);
    free(output_path);
    free(line);
    return status;
}

// Part 8 — Command-line interface
// AI explanation topics: Conceptual explanations for CLI options,
// directory processing, and output filename generation.
int main(int argc, char *argv[]) {
    const char *directory = NULL;
    const char *outdir_option = NULL;
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 ||
            strcmp(argv[i], "--dir") == 0) {
            if (directory || ++i >= argc) {
                fprintf(stderr, "Invalid directory option\n");
                return 1;
            }
            directory = argv[i];
        } else if (strcmp(argv[i], "-o") == 0 ||
                   strcmp(argv[i], "--output-dir") == 0) {
            if (outdir_option || ++i >= argc) {
                fprintf(stderr, "Invalid output option\n");
                return 1;
            }
            outdir_option = argv[i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        } else {
            if (input_file) {
                fprintf(stderr, "Too many input files\n");
                return 1;
            }
            input_file = argv[i];
        }
    }

    if ((directory && input_file) || (!directory && !input_file)) {
        fprintf(stderr,
                "Usage: %s [-d DIR] [-o OUTDIR] [input.txt]\n",
                argv[0]);
        return 1;
    }

    char *default_dir = NULL;
    const char *outdir = outdir_option;

    if (!outdir) {
        char *base = input_base(directory ? directory : input_file);
        if (!base) return 1;

        const char *suffix = "_Rufat_251ADB127";
        default_dir = malloc(strlen(base) + strlen(suffix) + 1);
        if (!default_dir) {
            free(base);
            return 1;
        }

        sprintf(default_dir, "%s%s", base, suffix);
        free(base);
        outdir = default_dir;
    }

    if (ensure_directory(outdir) != 0) {
        free(default_dir);
        return 1;
    }

    int status = 0;

    if (directory) {
        DIR *dir = opendir(directory);
        if (!dir) {
            perror(directory);
            free(default_dir);
            return 1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            size_t len = strlen(entry->d_name);

            if (len < 4 ||
                strcmp(entry->d_name + len - 4, ".txt") != 0)
                continue;

            char *path = join_path(directory, entry->d_name);
            if (!path) {
                status = 1;
                break;
            }

            struct stat st;
            if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
                if (process_file(path, outdir) != 0)
                    status = 1;
            }

            free(path);
        }

        if (closedir(dir) != 0) status = 1;
    } else {
        status = process_file(input_file, outdir);
    }

    free(default_dir);
    return status;
}
