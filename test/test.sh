#!/usr/local/bin/zsh

test_mincc() {
    MINCC=./build/mincc.out
    ASSEMBLY=./build/out.s
    EXEC=./build/out

    TESTLIB=./test/testlib.c

    input=$1
    echo ${input} | ${MINCC} ${input} > ${ASSEMBLY}

    if [ $? -ne 0 ]; then
        echo "\e[0;31m[FAIL]\e[m failed to compile"
        exit 1
    fi

    gcc-9 ${ASSEMBLY} ${TESTLIB} -o ${EXEC}
    ${EXEC} > /dev/null

    actual=$?
    expected=$2
    if [ ${actual} = ${expected} ]; then
        echo "\e[0;32m[PASS]\e[m ${input} => ${actual}"
    else
        echo "\e[0;31m[FAIL]\e[m ${input} => ${expected} expected, but got ${actual}"
        exit 1
    fi

    rm -Rf ${ASSEMBLY} ${EXEC}
}

test_mincc "main() { return 0; }"   0
test_mincc "main() { return 42; }" 42

test_mincc "main() { return 1-6+10; }"            5
test_mincc "main() { return 2+110-92; }"         20
test_mincc "main() { return 1000-990+121+92; }" 223

test_mincc "main() { return 1*2 + 5*8; }"     42
test_mincc "main() { return 5*6 - 10/2; }"    25
test_mincc "main() { return 100/3/3; }"       11
test_mincc "main() { return 100%7 + 22%6 ; }"  6

test_mincc "main() { return (1+2) * 3; }"              9
test_mincc "main() { return (6-3+1) * (9-6+7-2); }"   32
test_mincc "main() { return (4*1) * (4/3) ; }"         4

test_mincc "main() { return + 1+6 ; }"      7
test_mincc "main() { return -5+7*5; }"      30
test_mincc "main() { return +5 + -5; }"      0
test_mincc "main() { return +10 + (-8); }"   2

test_mincc "main() { return 1 << 3; }"          8
test_mincc "main() { return 10 + (-1 <<2); }"   6
test_mincc "main() { return 9 >> 1; }"          4
test_mincc "main() { return 23 + 7*(-6 >>1); }" 2

test_mincc "main() { return 0 == -1; }"    0
test_mincc "main() { return 0 == 0; }"     1
test_mincc "main() { return 0 == 1; }"     0
test_mincc "main() { return 0 != -1; }"    1
test_mincc "main() { return 0 != 0; }"     0
test_mincc "main() { return 0 != 1; }"     1
test_mincc "main() { return 0 < -1; }"     0
test_mincc "main() { return 0 < 0; }"      0
test_mincc "main() { return 0 < 1; }"      1
test_mincc "main() { return 0 > -1; }"     1
test_mincc "main() { return 0 > 0; }"      0
test_mincc "main() { return 0 > 1; }"      0
test_mincc "main() { return 0<=-1; }"      0
test_mincc "main() { return 0<= 0; }"      1
test_mincc "main() { return 0<= 1; }"      1
test_mincc "main() { return 0>=-1; }"      1
test_mincc "main() { return 0>= 0; }"      1
test_mincc "main() { return 0>= 1; }"      0
test_mincc "main() { return 0<1 == 1<2; }" 1
test_mincc "main() { return 0<1 == 1>2; }" 0
test_mincc "main() { return 0<1 != 1<2; }" 0
test_mincc "main() { return 0<1 != 1>2; }" 1

test_mincc "main() { return 2 & 1; }"      0
test_mincc "main() { return 3 & 6; }"      2
test_mincc "main() { return 2 ^ 1; }"      3
test_mincc "main() { return 3 ^ 6; }"      5
test_mincc "main() { return 2 | 1; }"      3
test_mincc "main() { return 3 | 6; }"      7
test_mincc "main() { return 0&1 ^ 1&1; }"  1
test_mincc "main() { return 0&1 ^ 1&0; }"  0
test_mincc "main() { return 1&1 ^ 1&1; }"  0
test_mincc "main() { return 0^1 | 1^1; }"  1
test_mincc "main() { return 0^1 | 1^0; }"  1
test_mincc "main() { return 1^1 | 1^1; }"  0
test_mincc "main() { return 0&1 | 1&1; }"  1
test_mincc "main() { return 0&1 | 1&0; }"  0
test_mincc "main() { return 1&1 | 1&1; }"  1
test_mincc "main() { return ~2 & 7; }"     5

test_mincc "main() { return 12 == 12; }"         1
test_mincc "main() { return 12 == 19; }"         0
test_mincc "main() { return 12 != 12; }"         0
test_mincc "main() { return 12 != 19; }"         1
test_mincc "main() { return 1 == 1 && 2 == 2; }" 1
test_mincc "main() { return 1 != 1 && 2 == 2; }" 0
test_mincc "main() { return 1 == 1 && 2 != 2; }" 0
test_mincc "main() { return 1 != 1 && 2 != 2; }" 0
test_mincc "main() { return 1 == 1 || 2 == 2; }" 1
test_mincc "main() { return 1 != 1 || 2 == 2; }" 1
test_mincc "main() { return 1 == 1 || 2 != 2; }" 1
test_mincc "main() { return 1 != 1 || 2 != 2; }" 0
test_mincc "main() { return !0; }"               1
test_mincc "main() { return !1; }"               0

test_mincc "
main() {
    x = 3;
    return x + 1;
}"                            4
test_mincc "
main() {
    tmp =  2+110-94;
    return tmp*2;
}"                           36
test_mincc "
main() {
    a = 5;b = 2;c = 6;
    return (a + c)*b*b;
}"                           44
test_mincc "
main() {
    a = 5;b = 12; c=13;
    return a*a+b*b == c*c;
}"                            1
test_mincc "
main() {
    x = 1;y = 3;
    x = x + y; y = x + y;
    return x*y;
}"                           28

test_mincc "
main() {
    ; x = 1;;
    return x + 1;
}"                            2
test_mincc "
main() {
    x = 1;
    return x + 3;
    return x + 1;
}"                            4


test_mincc "
main() {
    x = five(); y = one();
    return x+y;
}"                            6
test_mincc "
main() {
    x = five();
    return x+five();
}"                           10
test_mincc "
main() {
    x = 6; y = -3; z = 4;
    return sum(x, y, z, x, -y, -z);
}"                           12
test_mincc "
main() {
    x = 6; y = 2;
    return square(x*y);
}"                          144

test_mincc "
main() {
    x = 0; y = 3;
    if (x == 0) y = 2*y;
    return y;
}"                            6
test_mincc "
main() {
    x = 1; y = 3;
    if (x == 0) y = 2*y;
    return y;
}"                            3
test_mincc "
main() {
    x = 1; y = 3;
    if (x == 0) y = 2*y;
    else y = y / 2;
    return y;
}"                            1

test_mincc "
main() {
    x = 0;
    if (x < 10)       return 1;
    else if (x < 100) return 2;
    return 3;
}"                            1
test_mincc "
main() {
    x = 9;
    if (x < 10)       return 1;
    else if (x < 100) return 2;
    return 3;
}"                            1
test_mincc "
main() {
    x = 10;
    if (x < 10)       return 1;
    else if (x < 100) return 2;
    return 3;
}"                            2
test_mincc "
main() {
    x = 99;
    if (x < 10)       return 1;
    else if (x < 100) return 2;
    return 3;
}"                            2
test_mincc "
main() {
    x = 100;
    if (x < 10)       return 1;
    else if (x < 100) return 2;
    return 3;
}"                            3
test_mincc "
main() {
    x = 379;
    if (x < 10)       return 1;
    else if (x < 100) return 2;
    return 3;
}"                            3

test_mincc "
main() {
    x = 3;
    while (x < 50) x = put_int(x*x);
    return x + 1;
}"                                                     82
test_mincc "
main() {
    x = 2;
    while (x <  1) x = put_int(x*x);
    return x + 1;
}"                                                      3
test_mincc "
main() {
    x = 3;
    do x = put_int(x*x); while (x < 50);
    return x + 1;
}"                                                     82
test_mincc "
main() {
    x = 2;
    do x = put_int(x*x); while (x <  1);
    return x + 1;
}"                                                      5

test_mincc "
main() { 
    sum = 0; n = 5;
    for (n=1; n<=10; n = n+1)
        sum = n + sum;
    return sum;
}"                                                     55
test_mincc "
main() { 
    sum = 0; n = 1;
    for (; n <= 10; n = n+1)
        sum = n + sum;
    return sum;
}"                                                     55
test_mincc "
main() { 
    n = 1;
    for (;n <= 10;) n = n + 1;
    return 2*n;
}"                                                     22
test_mincc "
main() { 
    ret = 0; i = 0; j = 2;
    for (; i < 8; i = i+2)
        for (j=0; j < 8; j = j+1)
            ret = ret + 1;
    return ret;
}"                                                     32
test_mincc "
main() { 
    sum = 0; n = 1;
    while (n <= 8) {
        sum = sum + n*n;
        n = n + 1;
    }
    return sum;
}"                                                    204                            

test_mincc "
six() { return 6; }
main() {
    x = 10;
    return 2*x - six() + five();
}"                                                      19

test_mincc "
fib(n) {
    if (n <= 0)           return 0;
    if (n == 1 || n == 2) return 1;
    return fib(n-1) + fib(n-2);
}
main() { return fib(11); }"                            89

test_mincc "
add(x, y) {
    return x + y;
}

sub(x, y) {
    return x - y;
}

calc(x1, x2, x3, x4, x5, x6) {
    return sub(add(x1, x2*x3), add(x4, x5+x6));
}

main() {
    return calc(4, 2, 9, -2, -1, 13);
}"                                                    12
