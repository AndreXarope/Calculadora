#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "expressao.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ----------------------------------------------------------
   AUXILIARES
---------------------------------------------------------- */

static int isFunctionToken(const char *t) {
    if (!t) return 0;
    return (
        strcmp(t,"sen")==0 || strcmp(t,"sin")==0 ||
        strcmp(t,"cos")==0 ||
        strcmp(t,"tg")==0  || strcmp(t,"tan")==0 ||
        strcmp(t,"log")==0 ||
        strcmp(t,"raiz")==0 || strcmp(t,"sqrt")==0
    );
}

static int isOperatorToken(const char *t) {
    return (t && strlen(t)==1 && strchr("+-*/%^", t[0]));
}

static char * strip_outer_parentheses(char *s) {
    size_t len = strlen(s);
    if (len >= 2 && s[0]=='(' && s[len-1]==')') {
        int depth = 0, ok = 1;
        for (size_t i=0; i<len-1; i++) {
            if (s[i]=='(') depth++;
            else if (s[i]==')') {
                depth--;
                if (depth==0 && i < len-2) { ok = 0; break; }
            }
        }
        if (ok) {
            char *r = strdup(s+1);
            r[len-2] = '\0';
            free(s);
            return r;
        }
    }
    return s;
}

/* ----------------------------------------------------------
   POSFIXA → INFIXA
---------------------------------------------------------- */

char * getFormaInFixa(char *Str) {
    if (!Str) return NULL;

    char *copy = strdup(Str);
    if (!copy) return NULL;

    char *tok = strtok(copy, " ");
    size_t cap = 64, top = 0;
    char **stack = malloc(cap * sizeof(char *));

    while (tok) {

        /* Funções unárias */
        if (isFunctionToken(tok)) {
            if (top < 1) goto err;

            char *a = stack[--top];
            size_t need = strlen(tok) + strlen(a) + 4;

            char *r = malloc(need);
            sprintf(r, "%s(%s)", tok, a);
            free(a);

            stack[top++] = r;
        }

        /* Operadores binários */
        else if (isOperatorToken(tok)) {
            if (top < 2) goto err;

            char *b = stack[--top];
            char *a = stack[--top];

            size_t need = strlen(a) + strlen(b) + 6;
            char *r = malloc(need);
            sprintf(r, "(%s%c%s)", a, tok[0], b);

            free(a); free(b);
            stack[top++] = r;
        }

        /* Operando */
        else {
            stack[top++] = strdup(tok);
        }

        tok = strtok(NULL, " ");
    }

    free(copy);

    if (top != 1) goto err;

    char *final = strip_outer_parentheses(stack[0]);
    free(stack);

    return final;

err:
    free(copy);
    for (size_t i=0; i<top; i++) free(stack[i]);
    free(stack);
    return NULL;
}

/* ----------------------------------------------------------
   AVALIA POSFIXA
---------------------------------------------------------- */

float getValorPosFixa(char *StrPosFixa) {
    if (!StrPosFixa) return NAN;

    char *copy = strdup(StrPosFixa);
    if (!copy) return NAN;

    char *tok = strtok(copy, " ");

    size_t cap = 64, top = 0;
    double *stack = malloc(cap * sizeof(double));

    while (tok) {

        /* Funções unárias */
        if (isFunctionToken(tok)) {
            if (top < 1) goto err_val;

            double a = stack[--top];
            double r = NAN;

            if (!strcmp(tok,"sen") || !strcmp(tok,"sin"))
                r = sin(a * M_PI / 180.0);
            else if (!strcmp(tok,"cos"))
                r = cos(a * M_PI / 180.0);
            else if (!strcmp(tok,"tg") || !strcmp(tok,"tan"))
                r = tan(a * M_PI / 180.0);
            else if (!strcmp(tok,"log")) {
                if (a <= 0) goto err_val;
                r = log10(a);
            }
            else if (!strcmp(tok,"raiz") || !strcmp(tok,"sqrt")) {
                if (a < 0) goto err_val;
                r = sqrt(a);
            }

            stack[top++] = r;
        }

        /* Operadores binários */
        else if (isOperatorToken(tok)) {
            if (top < 2) goto err_val;

            double b = stack[--top];
            double a = stack[--top];
            double r = NAN;

            switch (tok[0]) {
                case '+': r = a + b; break;
                case '-': r = a - b; break;
                case '*': r = a * b; break;
                case '/': if (b==0) goto err_val; r = a / b; break;
                case '%': if (b==0) goto err_val; r = fmod(a,b); break;
                case '^': r = pow(a,b); break;
            }

            stack[top++] = r;
        }

        /* Números */
        else {
            char *end;
            double v = strtod(tok, &end);
            if (end == tok) goto err_val;

            stack[top++] = v;
        }

        tok = strtok(NULL, " ");
    }

    double result = (top == 1 ? stack[0] : NAN);

    free(copy);
    free(stack);
    return (float)result;

err_val:
    free(copy);
    free(stack);
    return NAN;
}
