#include <stdio.h>
#include <ctype.h>

int evaluate(const char* s, int* res) {
    int stack[1000];
    int top = -1;
    int state = 0;
    int num = 0;
    char op = '+';
    int start = 0;

    while (s[start] != '\0' && isspace(s[start])) {
        start++;
    }

    if (s[start] == '-') {
        op = '-';
        start++;
    } 

    for (int i = start; s[i] != '\0'; i++) {
        char c = s[i];
        
        if (isspace(c)) {
            if (state == 1) {
                state = 2;
            }
            continue;
        }
        
        if (isdigit(c)) {
            if (state == 2) {
                return 2;
            }
            num = num * 10 + (c - '0');
            state = 1;
        } else if (c == '+' || c == '-' || c == '*' || c == '/') {
            if (state == 0) {
                return 2;
            }
            
            if (op == '+') {
                stack[++top] = num;
            } else if (op == '-') {
                stack[++top] = -num;
            } else if (op == '*') {
                stack[top] *= num;
            } else if (op == '/') {
                if (num == 0) {
                    return 1;
                }
                stack[top] /= num;
            }
            
            op = c;
            num = 0;
            state = 0;
        } else {
            return 2;
        }
    }
    
    if (state == 0) {
        return 2;
    }
    
    if (op == '+') {
        stack[++top] = num;
    } else if (op == '-') {
        stack[++top] = -num;
    } else if (op == '*') {
        stack[top] *= num;
    } else if (op == '/') {
        if (num == 0) {
            return 1;
        }
        stack[top] /= num;
    }

    *res = 0;
    for (int i = 0; i <= top; i++) {
        *res += stack[i];
    }
    return 0;
}

int main() {
    char input[1024];
    
    if (fgets(input, sizeof(input), stdin)) {
        int res = 0;
        int status = evaluate(input, &res);
        
        if (status == 1) {
            printf("Error: Division by zero.\n");
        } else if (status == 2) {
            printf("Error: Invalid expression.\n");
        } else {
            printf("%d\n", res);
        }
    }
    return 0;
}