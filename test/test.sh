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


test_mincc "
int put_int(int x);
int main() { put_int(0);  return 0; }"   "0\$"
test_mincc "
int main() { int put_int(int x); put_int(42); return 0; }"  "42\$"

test_mincc "
int put_int(int x);
int main() { put_int(1-6-10); return 0; }"          "-15\$"
test_mincc "
int put_int(int x);
int main() { put_int(2+110-92); return 0; }"         "20\$"
test_mincc "
int put_int(int x);
int main() { put_int(1000-990+121+92); return 0; }" "223\$"

test_mincc "
int put_int(int x);
int main() { put_int(1*2 + 5*8); return 0; }"     "42\$"
test_mincc "
int put_int(int x);
int main() { put_int(5*6 - 10/2); return 0; }"    "25\$"
test_mincc "
int put_int(int x);
int main() { put_int(100/3/3); return 0; }"       "11\$"
test_mincc "
int put_int(int x);
int main() { put_int(100%7 + 22%6 ) ; return 0; }" "6\$"

test_mincc "
int put_int(int x);
int main() { put_int((1+2) * 3); return 0; }"              "9\$"
test_mincc "
int put_int(int x);
int main() { put_int((6-3+1) * (9-6+7-2)); return 0; }"   "32\$"
test_mincc "
int put_int(int x);
int main() { put_int((4*1) * (4/3) ); return 0; }"         "4\$"

test_mincc "
int put_int(int x);
int main() { put_int(+ 1+6 ); return 0; }"       "7\$"
test_mincc "
int put_int(int x);
int main() { put_int(-5+7*5); return 0; }"      "30\$"
test_mincc "
int put_int(int x);
int main() { put_int(+5 + -5); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(+10 + (-8)); return 0; }"   "2\$"

test_mincc "
int put_int(int x);
int main() { put_int(1 << 3); return 0; }"          "8\$"
test_mincc "
int put_int(int x);
int main() { put_int(10 + (-1 <<2)); return 0; }"   "6\$"
test_mincc "
int put_int(int x);
int main() { put_int(9 >> 1); return 0; }"          "4\$"
test_mincc "
int put_int(int x);
int main() { put_int(23 + 7*(-6 >>1)); return 0; }" "2\$"

test_mincc "
int put_int(int x);
int main() { put_int(0 == -1); return 0; }"    "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 == 0); return 0; }"     "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 == 1); return 0; }"     "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 != -1); return 0; }"    "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 != 0); return 0; }"     "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 != 1); return 0; }"     "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 < -1); return 0; }"     "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 < 0); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 < 1); return 0; }"      "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 > -1); return 0; }"     "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 > 0); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0 > 1); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<=-1); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<= 0); return 0; }"      "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<= 1); return 0; }"      "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0>=-1); return 0; }"      "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0>= 0); return 0; }"      "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0>= 1); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<1 == 1<2); return 0; }" "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<1 == 1>2); return 0; }" "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<1 != 1<2); return 0; }" "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0<1 != 1>2); return 0; }" "1\$"

test_mincc "
int put_int(int x);
int main() { put_int(2 & 1); return 0; }"      "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(3 & 6); return 0; }"      "2\$"
test_mincc "
int put_int(int x);
int main() { put_int(2 ^ 1); return 0; }"      "3\$"
test_mincc "
int put_int(int x);
int main() { put_int(3 ^ 6); return 0; }"      "5\$"
test_mincc "
int put_int(int x);
int main() { put_int(2 | 1); return 0; }"      "3\$"
test_mincc "
int put_int(int x);
int main() { put_int(3 | 6); return 0; }"      "7\$"
test_mincc "
int put_int(int x);
int main() { put_int(0&1 ^ 1&1); return 0; }"  "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0&1 ^ 1&0); return 0; }"  "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(1&1 ^ 1&1); return 0; }"  "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0^1 | 1^1); return 0; }"  "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0^1 | 1^0); return 0; }"  "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(1^1 | 1^1); return 0; }"  "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(0&1 | 1&1); return 0; }"  "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(0&1 | 1&0); return 0; }"  "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(1&1 | 1&1); return 0; }"  "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(~2 & 7); return 0; }"     "5\$"

test_mincc "
int put_int(int x);
int main() { put_int(12 == 12); return 0; }"         "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(12 == 19); return 0; }"         "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(12 != 12); return 0; }"         "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(12 != 19); return 0; }"         "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 == 1 && 2 == 2); return 0; }" "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 != 1 && 2 == 2); return 0; }" "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 == 1 && 2 != 2); return 0; }" "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 != 1 && 2 != 2); return 0; }" "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 == 1 || 2 == 2); return 0; }" "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 != 1 || 2 == 2); return 0; }" "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 == 1 || 2 != 2); return 0; }" "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(1 != 1 || 2 != 2); return 0; }" "0\$"
test_mincc "
int put_int(int x);
int main() { put_int(!0); return 0; }"               "1\$"
test_mincc "
int put_int(int x);
int main() { put_int(!1); return 0; }"               "0\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 3;
    put_int(x + 1);
    return 0;
}"                            "4\$"
test_mincc "
int put_int(int x);
int main() {
    int tmp =  2+110-94;
    put_int(tmp*2);
    return 0;
}"                           "36\$"
test_mincc "
int put_int(int x);
int main() {
    int a, b, c = 6;
    a = 5;b = 2;
    put_int((a + c)*b*b);
    return 0;
}"                           "44\$"
test_mincc "
int put_int(int x);
int main() {
    int a = 5, b, c;
    b = 12; c=13;
    put_int(a*a+b*b == c*c);
    return 0;
}"                            "1\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 1, y = 3;
    x = x + y; y = x + y;
    put_int(x*y);
    return 0;
}"                           "28\$"

test_mincc "
int put_int(int x);
int main() {
    int x;
    ; x = 1;;
    put_int(x + 1);
    return 0;
}"                            "2\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 2;
    x = 1;
    put_int(x + 3);
    return 0;
    put_int(x + 1);
    return 0;
}"                            "4\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 10;
    put_int(*&x);
    return 0;
}"                            "10\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 0;
    int *y = &x;
    *y = 3;
    put_int(x);
    return 0;
}"                            "3\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 6;
    int y = 15;
    put_int(*(&x - 1));
    put_int(*(&y + 1));
    put_int(&x - &y);
    return 0;
}"                            "15\$6\$1\$"

test_mincc "
int put_int(int x);
int main() {
    int a[5];
    *a = 1;
    a[1] = -3;
    *(a+3) = 7;
    *(a+2) = -2;
    a[4] = 10;

    int i = 0;
    put_int(a[0]);
    put_int(a[1]);
    put_int(a[2]);
    put_int(a[3]);
    put_int(a[4]);
}"                            "1\$-3\$-2\$7\$10\$"

test_mincc "
int put_int(int x);

int main() {
    int* a[1];
    int x = 5;
    a[0] = &x;
    put_int(*a[0]);
    x = -1;
    put_int(*a[0]);
    return 0;
}"                            "5\$-1\$"
test_mincc "
int put_int(int x);

int main() {
    int x = 10, y = 15;
    int* p = &x;
    int* q = &x;
    put_int(p == q);
    q = &y;
    put_int(p != q);

    int a[10] = {};
    p = &a[0];
    q = &a[2];
    put_int(p < q);
    put_int(a + 1 < a + 10);

    return 0;
}"                            "1\$1\$1\$1\$"

test_mincc "
int put_int(int x);
int one();
int five();
int main() {
    int x, y;
    x = five(); y = one();
    put_int(x+y);
    return 0;
}"                            "6\$"
test_mincc "
int put_int(int x);
int five();
int main() {
    int x = five();;
    put_int(x+five());
    return 0;
}"                           "10\$"
test_mincc "
int put_int(int x);
int sum(int a, int b, int c, int d, int e, int f);
int main() {
    int x = 6, y = -3, z = 4;
    put_int(sum(x, y, z, x, -y, -z));
    return 0;
}"                           "12\$"
test_mincc "
int put_int(int x);
int square(int n);
int main() {
    int x = 6, y = 2;
    put_int(square(x*y));
    return 0;
}"                          "144\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 0, y = 3;
    if (x == 0) y = 2*y;
    put_int(y);
    return 0;
}"                            "6\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 1, y = 3;
    if (x == 0) y = 2*y;
    put_int(y);
    return 0;
}"                            "3\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 1, y = 3;
    if (x == 0) y = 2*y;
    else y = y / 2;
    put_int(y);
    return 0;
}"                            "1\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 0;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "1\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 9;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "1\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 10;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "2\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 99;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "2\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 100;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "3\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 379;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "3\$"

test_mincc "
int put_int(int x);
int main() {
    int x = 3;
    while (x < 50) x = put_int(x*x);
    put_int(x + 1);
    return 0;
}"                    "9\$81\$82\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 2;
    while (x <  1) x = put_int(x*x);
    put_int(x + 1);
    return 0;
}"                            "3\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 3;
    do x = put_int(x*x); while (x < 50);
    put_int(x + 1);
    return 0;
}"                    "9\$81\$82\$"
test_mincc "
int put_int(int x);
int main() {
    int x = 2;
    do x = put_int(x*x); while (x <  1);
    put_int(x + 1);
    return 0;
}"                         "4\$5\$"

test_mincc "
int put_int(int x);
int main() {
    int sum = 0, n = 5;
    for (n=1; n<=10; n = n+1)
        sum = n + sum;
    put_int(sum);
    return 0;
}"                           "55\$"
test_mincc "
int put_int(int x);
int main() {
    int sum = 0, n = 1;
    for (; n <= 10; n = n+1)
        sum = n + sum;
    put_int(sum);
    return 0;
}"                           "55\$"
test_mincc "
int put_int(int x);
int main() {
    int n = 1;
    for (;n <= 10;) n = n + 1;
    put_int(2*n);
    return 0;
}"                           "22\$"
test_mincc "
int put_int(int x);
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
int put_int(int x);
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
int put_int(int x);
int main() {
    int a[6];
    int i = 0;
    for (i = 0; i < 6; i = i + 1) {
        if (i == 0) a[i] = 0;
        else a[i] = i + a[i-1];
    }
    for (i = 0; i < 6; i = i + 1) {
        put_int(a[i]);
    }
}"                          "0\$1\$3\$6\$10\$15\$"

test_mincc "
int put_int(int n);

int main() {
    int i = 0;
    while(i < 10) {
        if (i % 2 == 1) {
            i++;
            continue;
        }
        put_int(i);
        i++;
    }
    return 0;
}"                          "0\$2\$4\$6\$8\$"

test_mincc "
int put_int(int n);

int main() {
    int i = 0;
    do {
        if (i % 2 == 1) {
            i++;
            continue;
        }
        put_int(i);
        i++;
    } while (i < 10);
    return 0;
}"                          "0\$2\$4\$6\$8\$"

test_mincc "
int put_int(int n);

int main() {
    int i = 0;
    for (i = 0; i < 10; i++) {
        if (i % 2 == 1) continue;
        put_int(i);
    }
    return 0;
}"                          "0\$2\$4\$6\$8\$"

test_mincc "
int put_int(int n);

int main() {
    int x = 5;
    while (1) {
        put_int(x);
        if (x == 0) break;
        x--;
    }
    return 0;
}"                          "5\$4\$3\$2\$1\$0\$"

test_mincc "
int put_int(int n);

int main() {
    int x = 5;
    do {
        put_int(x);
        if (x == 0) break;
        x--;
    } while (1);
    return 0;
}"                          "5\$4\$3\$2\$1\$0\$"

test_mincc "
int put_int(int n);

int main() {
    int x;
    for(x = 5;;x--) {
        put_int(x);
        if (x == 0) break;
    }
    return 0;
}"                          "5\$4\$3\$2\$1\$0\$"

test_mincc "
int put_int(int x);
int five();
int six() { return 6; }
int main() {
    int x; 
    x = 10;
    put_int(2*x - six() + five());
    return 0;
}"                           "19\$"

test_mincc "
int put_int(int x);
int incr(int x) { return x + 1; }
int main() {
    int x; 
    x = -6;
    put_int(incr(incr(x)));
    put_int(x);
    return 0;
}"                           "-4\$-6\$"

test_mincc "
int put_int(int x);
int incr(int x) { return x + 1; }
int main() {
    int x; 
    int* y = &x;
    x = 17;
    put_int(incr(incr(*y)));
    put_int(*y);
    put_int(x);
    return 0;
}"                           "19\$17\$17\$"

test_mincc "
int put_int(int x);

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
int put_int(int x);

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
int put_int(int x);
int fib(int n, int a[46]);

int main() {
    int i, a[46];
    for (i = 0; i <= 45; i = i + 1) a[i] = -1;
    put_int(fib(45, a));
    return 0;
}

int fib(int n, int a[46]) {
    if (a[n] != -1)       return a[n];
    if (n == 0)           return a[n] = 0;
    if (n == 1 || n == 2) return a[n] = 1;
    return a[n] = fib(n-1, a) + fib(n-2, a);
}"                            "1134903170\$"

test_mincc "
int put_int(int x);
int sq_array(int *a);

int main() {
    int a[6];
    sq_array(a);

    int i;
    for (i = 0; i < 6; i = i + 1) {
        put_int(a[i]);
    }
    return 0;
}

int sq_array(int a[6]) {
    int i;
    for (i = 0; i < 6; i = i + 1) {
        a[i] = i*i;
    }
    return a[5];
}"                            "0\$1\$4\$9\$16\$25\$"

test_mincc "
int put_int(int x);
int add(int x, int y), sub(int x, int y);
int calc(int x1, int x2, int x3, int x4, int x5, int x6);

int main() {
    put_int(calc(4, 2, 9, -2, -1, 13));
    return 0;
}
int add(int x, int y) {
    return x + y;
}

int sub(int x, int y) {
    return x - y;
}

int calc(int x1, int x2, int x3, int x4, int x5, int x6) {
    return sub(add(x1, x2*x3), add(x4, x5+x6));
}"                           "12\$"

test_mincc "
int put_int(int x);

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

test_mincc "
char put_char(char x);

int main() {
    char c = 'a';
    put_char(c);
    c = 'b';
    put_char(c);
    return 0;
}"                           "a\$b\$"

test_mincc "
char put_char(char x);

char next(char x) {
    return x + 1;
}

int main() {
    put_char(next('a'));
    put_char(next('b'));
    put_char(next('t'));
    put_char(next('k'));
    put_char(next('y'));
    return 0;
}"                           "b\$c\$u\$l\$z\$"

test_mincc "
char* put_str(char* str);
int main() {
    char str[4];
    str[0] = 'a';
    str[1] = 'b';
    str[2] = 'c';
    str[3] = '\0';
    put_str(str);
    return 0;
}"                           "abc\$"

test_mincc "
int x;
int x;
int put_int(int x);
int main() {
    int y = x + 7;
    put_int(x);
    put_int(y);
    x = x - 3;
    put_int(x);
    put_int(y);
}"                           "0\$7\$-3\$7\$"

test_mincc "
char x = 'a';
char y;

char put_char(char c);
int main() {
    int z = 'c';
    put_char(x);
    put_char(y);
    put_char(z);
}

char y = 'b';
"                            "a\$b\$c\$"

test_mincc "
int x;
int* y = &x;

int put_int(int x);
int main() {
    put_int(*y);
    *y = *y + 3;
    put_int(x);
    put_int(*y);
    return 0;
}"                           "0\$3\$3\$"

test_mincc "
int x[5];

int put_int(int x);

int main() {
    3[x] = 4;
    x[1] = -7;
    put_int(x[0]);
    put_int(1[x]);
    put_int(*(x + 3));
    return 0;
}"                           "0\$-7\$4\$"

test_mincc "
int x[5];
int a;

int put_int(int x);

int assign() {
    a = -2;
    int i = 0;
    for (i = 0; i < 5; i = i + 1)
        x[i] = i*i;
}

int put_x() {
    int i = 0;
    for (i = 0; i < 5; i = i + 1)
        put_int(x[i]);
}

int put_a() {
    put_int(a);
}

int a;
int a = 9;

int main() {
    int x = -5;
    int b;
    b = 6;
    int a = 4;
    put_int(a);
    put_int(b);
    put_int(x);
    put_a();
    assign();
    a = -7;
    put_a();
    put_x();
    put_int(a);
    put_int(x);
    put_int(b);
    return 0;
}"                           "4\$6\$-5\$9\$-2\$0\$1\$4\$9\$16\$-7\$-5\$6\$"

test_mincc "
int puts(char* s);
int main() {
    puts(\"abc\");
    puts(\"Hello World!\");
    puts(\"minimal cc\");
    return 0;
}"                           "abc\$Hello World!\$minimal cc\$"

test_mincc "
int mincc_strlen(char* str) {
    int len = 0;
    char* p = str;
    for (p = str; *p != '\0'; p++) len++;
    return len;
}

int put_int(int x);
int main() {
    put_int(mincc_strlen(\"Hello World!\"));
    put_int(mincc_strlen(\"abc\"));
    put_int(mincc_strlen(\"minimal cc\"));
    put_int(mincc_strlen(\"\"));
    return 0;
}"                           "12\$3\$10\$0\$"

test_mincc "
int put_int(int x);

int ga[3] = {6, 7, 8};
int ga_fill[3] = {9};

int main() {
    int la[3] = {1, 2, 3};
    int la_fill[3] = {4};
    int i = 0;
    for (i = 0; i < 3; i = i + 1) {
        put_int(la[i]);
    }
    for (i = 0; i < 3; i = i + 1) {
        put_int(la_fill[i]);
    }
    for (i = 0; i < 3; i = i + 1) {
        put_int(ga[i]);
    }
    for (i = 0; i < 3; i = i + 1) {
        put_int(ga_fill[i]);
    }
    return 0;
}"                           "1\$2\$3\$4\$0\$0\$6\$7\$8\$9\$0\$0\$"

test_mincc "
char* put_str(char* s);

char* global[2] = { \"global0\", \"global1\" };

int main() {
    char* local[2] = { \"local0\", \"local1\" };
    put_str(local[0]);
    put_str(local[1]);
    put_str(global[0]);
    put_str(global[1]);
    return 0;
}"                           "local0\$local1\$global0\$global1\$"

test_mincc "
char* put_str(char* str);

char* global_ptr = \"global ptr\";
char global_str[11] = \"global str\";
char global_str_fill[20] = \"global str fill\";

int main() {
    char* local_ptr = \"local ptr\";
    char local_str[10] = \"local str\";
    char local_str_fill[20] = \"local str fill\";

    put_str(local_ptr);
    put_str(local_str);
    put_str(local_str_fill);

    put_str(global_ptr);
    put_str(global_str);
    put_str(global_str_fill);
    return 0;
}"                           "local ptr\$local str\$local str fill\$global ptr\$global str\$global str fill\$"

test_mincc "
int put_int(int x);

int main() {
    int i = 0;
    int a[2] = {10, 11};

    i = 0;
    put_int(a[i++]);
    put_int(i);
    i = 0;
    put_int(a[++i]);
    put_int(i);
    return 0;
}"                           "10\$1\$11\$1\$"

test_mincc "
int put_int(int x);

int main() {
    int i = 0;
    int a[2] = {10, 11};

    i = 1;
    put_int(a[i--]);
    put_int(i);
    i = 1;
    put_int(a[--i]);
    put_int(i);
    return 0;
}"                           "11\$0\$10\$0\$"

test_mincc "
int put_int(int x);

int g[3][5] = {{1, 2, 3}, {6, 7, 8, 9, 10}};
int main() {
    int l[3][5] = {{-1, -2, -3}, {-6, -7, -8, -9, -10}};
    put_int(l[0][1]);
    put_int(l[0][3]);
    put_int(l[1][2]);
    put_int(l[0][6]);
    put_int(l[2][4]);
    put_int(g[0][1]);
    put_int(g[0][3]);
    put_int(g[1][2]);
    put_int(g[0][6]);
    put_int(g[2][4]);
    return 0;
}"                           "-2\$0\$-8\$-7\$0\$2\$0\$8\$7\$0\$"

test_mincc "
int put_int(int x);

int main() {
    int a[2][1][1] = {};
    put_int(a[0][0][0]);
    put_int(a[1][0][0]);
    return 0;
}"                           "0\$0\$"

test_mincc "
char* put_str(char* s);

char g[2][10] = { \"global0\", \"g1\" };

int main() {
    char l[2][10] = {\"l0\", \"local1\"};
    put_str(l[0]);
    put_str(l[1]);
    put_str(g[0]);
    put_str(g[1]);
    return 0;
}"                           "l0\$local1\$global0\$g1\$"

teardown_test
