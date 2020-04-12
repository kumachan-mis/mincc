#!/usr/local/bin/zsh

setup_test() {
    gcc-9 -O2 -S -o ./build/testlib.s ./test/testlib.c
}

test_mincc() {
    MINCC=./build/mincc.out
    IN_C=./build/in.c
    OUT_ASSEMBLY=./build/out.s
    LIB=./build/testlib.s
    EXEC=./build/out

    input=$1
    expected=$2
    echo ${input} > ${IN_C}
    ${MINCC} ${IN_C} ${OUT_ASSEMBLY}

    if [ $? -ne 0 ]; then
        echo "\e[0;31m[FAIL]\e[m failed to compile"
        exit 1
    fi

    gcc-9 ${OUT_ASSEMBLY} ${LIB} -o ${EXEC}
    actual=$(${EXEC} | tr '\n' '$')

    if [ ${actual} = ${expected} ]; then
        echo "\e[0;32m[PASS]\e[m ${input}\n=> ${expected}"
    else
        echo "\e[0;31m[FAIL]\e[m ${input}\n=> ${expected} expected, but got ${actual}"
        exit 1
    fi

    rm -Rf ${IN_C} ${OUT_ASSEMBLY} ${EXEC}
}

teardown_test() {
    rm -Rf ./build/testlib.s
}


setup_test


test_mincc "int main() { put_int(0);  return 0; }"   "0\$"
test_mincc "int main() { put_int(42); return 0; }"  "42\$"

test_mincc "int main() { put_int(1-6-10); return 0; }"          "-15\$"
test_mincc "int main() { put_int(2+110-92); return 0; }"         "20\$"
test_mincc "int main() { put_int(1000-990+121+92); return 0; }" "223\$"

test_mincc "int main() { put_int(1*2 + 5*8); return 0; }"     "42\$"
test_mincc "int main() { put_int(5*6 - 10/2); return 0; }"    "25\$"
test_mincc "int main() { put_int(100/3/3); return 0; }"       "11\$"
test_mincc "int main() { put_int(100%7 + 22%6 ) ; return 0; }" "6\$"

test_mincc "int main() { put_int((1+2) * 3); return 0; }"              "9\$"
test_mincc "int main() { put_int((6-3+1) * (9-6+7-2)); return 0; }"   "32\$"
test_mincc "int main() { put_int((4*1) * (4/3) ); return 0; }"         "4\$"

test_mincc "int main() { put_int(+ 1+6 ); return 0; }"       "7\$"
test_mincc "int main() { put_int(-5+7*5); return 0; }"      "30\$"
test_mincc "int main() { put_int(+5 + -5); return 0; }"      "0\$"
test_mincc "int main() { put_int(+10 + (-8)); return 0; }"   "2\$"

test_mincc "int main() { put_int(1 << 3); return 0; }"          "8\$"
test_mincc "int main() { put_int(10 + (-1 <<2)); return 0; }"   "6\$"
test_mincc "int main() { put_int(9 >> 1); return 0; }"          "4\$"
test_mincc "int main() { put_int(23 + 7*(-6 >>1)); return 0; }" "2\$"

test_mincc "int main() { put_int(0 == -1); return 0; }"    "0\$"
test_mincc "int main() { put_int(0 == 0); return 0; }"     "1\$"
test_mincc "int main() { put_int(0 == 1); return 0; }"     "0\$"
test_mincc "int main() { put_int(0 != -1); return 0; }"    "1\$"
test_mincc "int main() { put_int(0 != 0); return 0; }"     "0\$"
test_mincc "int main() { put_int(0 != 1); return 0; }"     "1\$"
test_mincc "int main() { put_int(0 < -1); return 0; }"     "0\$"
test_mincc "int main() { put_int(0 < 0); return 0; }"      "0\$"
test_mincc "int main() { put_int(0 < 1); return 0; }"      "1\$"
test_mincc "int main() { put_int(0 > -1); return 0; }"     "1\$"
test_mincc "int main() { put_int(0 > 0); return 0; }"      "0\$"
test_mincc "int main() { put_int(0 > 1); return 0; }"      "0\$"
test_mincc "int main() { put_int(0<=-1); return 0; }"      "0\$"
test_mincc "int main() { put_int(0<= 0); return 0; }"      "1\$"
test_mincc "int main() { put_int(0<= 1); return 0; }"      "1\$"
test_mincc "int main() { put_int(0>=-1); return 0; }"      "1\$"
test_mincc "int main() { put_int(0>= 0); return 0; }"      "1\$"
test_mincc "int main() { put_int(0>= 1); return 0; }"      "0\$"
test_mincc "int main() { put_int(0<1 == 1<2); return 0; }" "1\$"
test_mincc "int main() { put_int(0<1 == 1>2); return 0; }" "0\$"
test_mincc "int main() { put_int(0<1 != 1<2); return 0; }" "0\$"
test_mincc "int main() { put_int(0<1 != 1>2); return 0; }" "1\$"

test_mincc "int main() { put_int(2 & 1); return 0; }"      "0\$"
test_mincc "int main() { put_int(3 & 6); return 0; }"      "2\$"
test_mincc "int main() { put_int(2 ^ 1); return 0; }"      "3\$"
test_mincc "int main() { put_int(3 ^ 6); return 0; }"      "5\$"
test_mincc "int main() { put_int(2 | 1); return 0; }"      "3\$"
test_mincc "int main() { put_int(3 | 6); return 0; }"      "7\$"
test_mincc "int main() { put_int(0&1 ^ 1&1); return 0; }"  "1\$"
test_mincc "int main() { put_int(0&1 ^ 1&0); return 0; }"  "0\$"
test_mincc "int main() { put_int(1&1 ^ 1&1); return 0; }"  "0\$"
test_mincc "int main() { put_int(0^1 | 1^1); return 0; }"  "1\$"
test_mincc "int main() { put_int(0^1 | 1^0); return 0; }"  "1\$"
test_mincc "int main() { put_int(1^1 | 1^1); return 0; }"  "0\$"
test_mincc "int main() { put_int(0&1 | 1&1); return 0; }"  "1\$"
test_mincc "int main() { put_int(0&1 | 1&0); return 0; }"  "0\$"
test_mincc "int main() { put_int(1&1 | 1&1); return 0; }"  "1\$"
test_mincc "int main() { put_int(~2 & 7); return 0; }"     "5\$"

test_mincc "int main() { put_int(12 == 12); return 0; }"         "1\$"
test_mincc "int main() { put_int(12 == 19); return 0; }"         "0\$"
test_mincc "int main() { put_int(12 != 12); return 0; }"         "0\$"
test_mincc "int main() { put_int(12 != 19); return 0; }"         "1\$"
test_mincc "int main() { put_int(1 == 1 && 2 == 2); return 0; }" "1\$"
test_mincc "int main() { put_int(1 != 1 && 2 == 2); return 0; }" "0\$"
test_mincc "int main() { put_int(1 == 1 && 2 != 2); return 0; }" "0\$"
test_mincc "int main() { put_int(1 != 1 && 2 != 2); return 0; }" "0\$"
test_mincc "int main() { put_int(1 == 1 || 2 == 2); return 0; }" "1\$"
test_mincc "int main() { put_int(1 != 1 || 2 == 2); return 0; }" "1\$"
test_mincc "int main() { put_int(1 == 1 || 2 != 2); return 0; }" "1\$"
test_mincc "int main() { put_int(1 != 1 || 2 != 2); return 0; }" "0\$"
test_mincc "int main() { put_int(!0); return 0; }"               "1\$"
test_mincc "int main() { put_int(!1); return 0; }"               "0\$"

test_mincc "
int main() {
    int x = 3;
    put_int(x + 1);
    return 0;
}"                            "4\$"
test_mincc "
int main() {
    int tmp =  2+110-94;
    put_int(tmp*2);
    return 0;
}"                           "36\$"
test_mincc "
int main() {
    int a, b, c = 6;
    a = 5;b = 2;
    put_int((a + c)*b*b);
    return 0;
}"                           "44\$"
test_mincc "
int main() {
    int a = 5, b, c;
    b = 12; c=13;
    put_int(a*a+b*b == c*c);
    return 0;
}"                            "1\$"
test_mincc "
int main() {
    int x = 1, y = 3;
    x = x + y; y = x + y;
    put_int(x*y);
    return 0;
}"                           "28\$"

test_mincc "
int main() {
    int x;
    ; x = 1;;
    put_int(x + 1);
    return 0;
}"                            "2\$"
test_mincc "
int main() {
    int x = 2;
    x = 1;
    put_int(x + 3);
    return 0;
    put_int(x + 1);
    return 0;
}"                            "4\$"

test_mincc "
int main() {
    int x = 10;
    put_int(*&x);
    return 0;
}"                            "10\$"

test_mincc "
int main() {
    int x = 0;
    int *y = &x;
    *y = 3;
    put_int(x);
    return 0;
}"                            "3\$"

test_mincc "
int main() {
    int x = 6;
    int y = 15;
    put_int(*(&x - 1));
    put_int(*(&y + 1));
    put_int(&x - &y);
    return 0;
}"                            "15\$6\$1\$"

test_mincc "
int main() {
    int x, y;
    x = five(); y = one();
    put_int(x+y);
    return 0;
}"                            "6\$"
test_mincc "
int main() {
    int x = five();;
    put_int(x+five());
    return 0;
}"                           "10\$"
test_mincc "
int main() {
    int x = 6, y = -3, z = 4;
    put_int(sum(x, y, z, x, -y, -z));
    return 0;
}"                           "12\$"
test_mincc "
int main() {
    int x = 6, y = 2;
    put_int(square(x*y));
    return 0;
}"                          "144\$"

test_mincc "
int main() {
    int x = 0, y = 3;
    if (x == 0) y = 2*y;
    put_int(y);
    return 0;
}"                            "6\$"
test_mincc "
int main() {
    int x = 1, y = 3;
    if (x == 0) y = 2*y;
    put_int(y);
    return 0;
}"                            "3\$"
test_mincc "
int main() {
    int x = 1, y = 3;
    if (x == 0) y = 2*y;
    else y = y / 2;
    put_int(y);
    return 0;
}"                            "1\$"

test_mincc "
int main() {
    int x = 0;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "1\$"
test_mincc "
int main() {
    int x = 9;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "1\$"
test_mincc "
int main() {
    int x = 10;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "2\$"
test_mincc "
int main() {
    int x = 99;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "2\$"
test_mincc "
int main() {
    int x = 100;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "3\$"
test_mincc "
int main() {
    int x = 379;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "3\$"

test_mincc "
int main() {
    int x = 3;
    while (x < 50) x = put_int(x*x);
    put_int(x + 1);
    return 0;
}"                    "9\$81\$82\$"
test_mincc "
int main() {
    int x = 2;
    while (x <  1) x = put_int(x*x);
    put_int(x + 1);
    return 0;
}"                            "3\$"
test_mincc "
int main() {
    int x = 3;
    do x = put_int(x*x); while (x < 50);
    put_int(x + 1);
    return 0;
}"                    "9\$81\$82\$"
test_mincc "
int main() {
    int x = 2;
    do x = put_int(x*x); while (x <  1);
    put_int(x + 1);
    return 0;
}"                         "4\$5\$"

test_mincc "
int main() {
    int sum = 0, n = 5;
    for (n=1; n<=10; n = n+1)
        sum = n + sum;
    put_int(sum);
    return 0;
}"                           "55\$"
test_mincc "
int main() {
    int sum = 0, n = 1;
    for (; n <= 10; n = n+1)
        sum = n + sum;
    put_int(sum);
    return 0;
}"                           "55\$"
test_mincc "
int main() {
    int n = 1;
    for (;n <= 10;) n = n + 1;
    put_int(2*n);
    return 0;
}"                           "22\$"
test_mincc "
int main() {
    int ret = 0, i, j;
    i = 0; j = 2;
    for (; i < 8; i = i+2)
        for (j=0; j < 8; j = j+1)
            ret = ret + 1;
    put_int(ret);
    return 0;
}"                           "32\$"
test_mincc "
int main() {
    int sum = 0, n = 1;
    while (n <= 8) {
        sum = sum + n*n;
        n = n + 1;
    }
    put_int(sum);
    return 0;
}"                          "204\$"

test_mincc "
int six() { return 6; }
int main() {
    int x; 
    x = 10;
    put_int(2*x - six() + five());
    return 0;
}"                           "19\$"

test_mincc "
int incr(int x) { return x + 1; }
int main() {
    int x; 
    x = -6;
    put_int(incr(incr(x)));
    put_int(x);
    return 0;
}"                           "-4\$-6\$"

test_mincc "
int incr(int* x) {
    *x = *x + 1;
    return *x;
}

int main() {
    int x; 
    x = 4;
    put_int(incr(&x));
    put_int(x);
    return 0;
}"                           "5\$5\$"

test_mincc "
int fib(int n) {
    if (n <= 0)           return 0;
    if (n == 1 || n == 2) return 1;
    return fib(n-1) + fib(n-2);
}
int main() {
    put_int(fib(11));
    return 0;
}"                            "89\$"

test_mincc "
int add(int x, int y) {
    return x + y;
}

int sub(int x, int y) {
    return x - y;
}

int calc(int x1, int x2, int x3, int x4, int x5, int x6) {
    return sub(add(x1, x2*x3), add(x4, x5+x6));
}

int main() {
    put_int(calc(4, 2, 9, -2, -1, 13));
    return 0;
}"                           "12\$"

test_mincc "
int main() {
    int x = 1, y = 5;
    put_int(x);
    put_int(y);
    {
        int x;
        x = 2;
        put_int(x);
        put_int(y);
    }
    put_int(x);
    put_int(y);
}"                           "1\$5\$2\$5\$1\$5\$"

teardown_test
