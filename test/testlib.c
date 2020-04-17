int one() {
    return 1;
}

int five() {
    return 5;
}

int square(int x) {
    return x*x;
}

int sum(int x1, int x2, int x3, int x4, int x5, int x6) {
    return x1 + x2 + x3 + x4 + x5 + x6;
}

int printf(const char * restrict format, ...);
int put_int(int x) {
    printf("%d\n", x);
    return x;
}

int put_char(char x) {
    printf("%c\n", x);
    return x;
}

char* put_str(char* x) {
    printf("%s\n", x);
    return x;
}
