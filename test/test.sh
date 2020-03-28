#!/usr/local/bin/zsh

setup_test() {
    gcc-9 -O2 -S -o ./build/testlib.s ./test/testlib.c
}

test_mincc() {
    MINCC=./build/mincc.out
    TESTLIB=./build/testlib.s
    ASSEMBLY=./build/out.s
    EXEC=./build/out

    input=$1
    expected=$2
    echo ${input} | ${MINCC} ${input} > ${ASSEMBLY}

    if [ $? -ne 0 ]; then
        echo "\e[0;31m[FAIL]\e[m failed to compile"
        exit 1
    fi

    gcc-9 ${ASSEMBLY} ${TESTLIB} -o ${EXEC}
    actual=$(${EXEC} | tr '\n' '$')

    if [ ${actual} = ${expected} ]; then
        echo "\e[0;32m[PASS]\e[m ${input}\n=> ${expected}"
    else
        echo "\e[0;31m[FAIL]\e[m ${input}\n=> ${expected} expected, but got ${actual}"
        exit 1
    fi

    rm -Rf ${ASSEMBLY} ${EXEC}
}

teardown_test() {
    rm -Rf ./build/testlib.s
}


setup_test


test_mincc "main() { put_int(0);  return 0; }"   "0\$"
test_mincc "main() { put_int(42); return 0; }"  "42\$"

test_mincc "main() { put_int(1-6-10); return 0; }"          "-15\$"
test_mincc "main() { put_int(2+110-92); return 0; }"         "20\$"
test_mincc "main() { put_int(1000-990+121+92); return 0; }" "223\$"

test_mincc "main() { put_int(1*2 + 5*8); return 0; }"     "42\$"
test_mincc "main() { put_int(5*6 - 10/2); return 0; }"    "25\$"
test_mincc "main() { put_int(100/3/3); return 0; }"       "11\$"
test_mincc "main() { put_int(100%7 + 22%6 ) ; return 0; }" "6\$"

test_mincc "main() { put_int((1+2) * 3); return 0; }"              "9\$"
test_mincc "main() { put_int((6-3+1) * (9-6+7-2)); return 0; }"   "32\$"
test_mincc "main() { put_int((4*1) * (4/3) ); return 0; }"         "4\$"

test_mincc "main() { put_int(+ 1+6 ); return 0; }"       "7\$"
test_mincc "main() { put_int(-5+7*5); return 0; }"      "30\$"
test_mincc "main() { put_int(+5 + -5); return 0; }"      "0\$"
test_mincc "main() { put_int(+10 + (-8)); return 0; }"   "2\$"

test_mincc "main() { put_int(1 << 3); return 0; }"          "8\$"
test_mincc "main() { put_int(10 + (-1 <<2)); return 0; }"   "6\$"
test_mincc "main() { put_int(9 >> 1); return 0; }"          "4\$"
test_mincc "main() { put_int(23 + 7*(-6 >>1)); return 0; }" "2\$"

test_mincc "main() { put_int(0 == -1); return 0; }"    "0\$"
test_mincc "main() { put_int(0 == 0); return 0; }"     "1\$"
test_mincc "main() { put_int(0 == 1); return 0; }"     "0\$"
test_mincc "main() { put_int(0 != -1); return 0; }"    "1\$"
test_mincc "main() { put_int(0 != 0); return 0; }"     "0\$"
test_mincc "main() { put_int(0 != 1); return 0; }"     "1\$"
test_mincc "main() { put_int(0 < -1); return 0; }"     "0\$"
test_mincc "main() { put_int(0 < 0); return 0; }"      "0\$"
test_mincc "main() { put_int(0 < 1); return 0; }"      "1\$"
test_mincc "main() { put_int(0 > -1); return 0; }"     "1\$"
test_mincc "main() { put_int(0 > 0); return 0; }"      "0\$"
test_mincc "main() { put_int(0 > 1); return 0; }"      "0\$"
test_mincc "main() { put_int(0<=-1); return 0; }"      "0\$"
test_mincc "main() { put_int(0<= 0); return 0; }"      "1\$"
test_mincc "main() { put_int(0<= 1); return 0; }"      "1\$"
test_mincc "main() { put_int(0>=-1); return 0; }"      "1\$"
test_mincc "main() { put_int(0>= 0); return 0; }"      "1\$"
test_mincc "main() { put_int(0>= 1); return 0; }"      "0\$"
test_mincc "main() { put_int(0<1 == 1<2); return 0; }" "1\$"
test_mincc "main() { put_int(0<1 == 1>2); return 0; }" "0\$"
test_mincc "main() { put_int(0<1 != 1<2); return 0; }" "0\$"
test_mincc "main() { put_int(0<1 != 1>2); return 0; }" "1\$"

test_mincc "main() { put_int(2 & 1); return 0; }"      "0\$"
test_mincc "main() { put_int(3 & 6); return 0; }"      "2\$"
test_mincc "main() { put_int(2 ^ 1); return 0; }"      "3\$"
test_mincc "main() { put_int(3 ^ 6); return 0; }"      "5\$"
test_mincc "main() { put_int(2 | 1); return 0; }"      "3\$"
test_mincc "main() { put_int(3 | 6); return 0; }"      "7\$"
test_mincc "main() { put_int(0&1 ^ 1&1); return 0; }"  "1\$"
test_mincc "main() { put_int(0&1 ^ 1&0); return 0; }"  "0\$"
test_mincc "main() { put_int(1&1 ^ 1&1); return 0; }"  "0\$"
test_mincc "main() { put_int(0^1 | 1^1); return 0; }"  "1\$"
test_mincc "main() { put_int(0^1 | 1^0); return 0; }"  "1\$"
test_mincc "main() { put_int(1^1 | 1^1); return 0; }"  "0\$"
test_mincc "main() { put_int(0&1 | 1&1); return 0; }"  "1\$"
test_mincc "main() { put_int(0&1 | 1&0); return 0; }"  "0\$"
test_mincc "main() { put_int(1&1 | 1&1); return 0; }"  "1\$"
test_mincc "main() { put_int(~2 & 7); return 0; }"     "5\$"

test_mincc "main() { put_int(12 == 12); return 0; }"         "1\$"
test_mincc "main() { put_int(12 == 19); return 0; }"         "0\$"
test_mincc "main() { put_int(12 != 12); return 0; }"         "0\$"
test_mincc "main() { put_int(12 != 19); return 0; }"         "1\$"
test_mincc "main() { put_int(1 == 1 && 2 == 2); return 0; }" "1\$"
test_mincc "main() { put_int(1 != 1 && 2 == 2); return 0; }" "0\$"
test_mincc "main() { put_int(1 == 1 && 2 != 2); return 0; }" "0\$"
test_mincc "main() { put_int(1 != 1 && 2 != 2); return 0; }" "0\$"
test_mincc "main() { put_int(1 == 1 || 2 == 2); return 0; }" "1\$"
test_mincc "main() { put_int(1 != 1 || 2 == 2); return 0; }" "1\$"
test_mincc "main() { put_int(1 == 1 || 2 != 2); return 0; }" "1\$"
test_mincc "main() { put_int(1 != 1 || 2 != 2); return 0; }" "0\$"
test_mincc "main() { put_int(!0); return 0; }"               "1\$"
test_mincc "main() { put_int(!1); return 0; }"               "0\$"

test_mincc "
main() {
    x = 3;
    put_int(x + 1);
    return 0;
}"                            "4\$"
test_mincc "
main() {
    tmp =  2+110-94;
    put_int(tmp*2);
    return 0;
}"                           "36\$"
test_mincc "
main() {
    a = 5;b = 2;c = 6;
    put_int((a + c)*b*b);
    return 0;
}"                           "44\$"
test_mincc "
main() {
    a = 5;b = 12; c=13;
    put_int(a*a+b*b == c*c);
    return 0;
}"                            "1\$"
test_mincc "
main() {
    x = 1;y = 3;
    x = x + y; y = x + y;
    put_int(x*y);
    return 0;
}"                           "28\$"

test_mincc "
main() {
    ; x = 1;;
    put_int(x + 1);
    return 0;
}"                            "2\$"
test_mincc "
main() {
    x = 1;
    put_int(x + 3);
    return 0;
    put_int(x + 1);
    return 0;
}"                            "4\$"


test_mincc "
main() {
    x = five(); y = one();
    put_int(x+y);
    return 0;
}"                            "6\$"
test_mincc "
main() {
    x = five();
    put_int(x+five());
    return 0;
}"                           "10\$"
test_mincc "
main() {
    x = 6; y = -3; z = 4;
    put_int(sum(x, y, z, x, -y, -z));
    return 0;
}"                           "12\$"
test_mincc "
main() {
    x = 6; y = 2;
    put_int(square(x*y));
    return 0;
}"                          "144\$"

test_mincc "
main() {
    x = 0; y = 3;
    if (x == 0) y = 2*y;
    put_int(y);
    return 0;
}"                            "6\$"
test_mincc "
main() {
    x = 1; y = 3;
    if (x == 0) y = 2*y;
    put_int(y);
    return 0;
}"                            "3\$"
test_mincc "
main() {
    x = 1; y = 3;
    if (x == 0) y = 2*y;
    else y = y / 2;
    put_int(y);
    return 0;
}"                            "1\$"

test_mincc "
main() {
    x = 0;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "1\$"
test_mincc "
main() {
    x = 9;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "1\$"
test_mincc "
main() {
    x = 10;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "2\$"
test_mincc "
main() {
    x = 99;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "2\$"
test_mincc "
main() {
    x = 100;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "3\$"
test_mincc "
main() {
    x = 379;
    if (x < 10)       put_int(1);
    else if (x < 100) put_int(2);
    else              put_int(3);
    return 0;
}"                            "3\$"

test_mincc "
main() {
    x = 3;
    while (x < 50) x = put_int(x*x);
    put_int(x + 1);
    return 0;
}"                    "9\$81\$82\$"
test_mincc "
main() {
    x = 2;
    while (x <  1) x = put_int(x*x);
    put_int(x + 1);
    return 0;
}"                            "3\$"
test_mincc "
main() {
    x = 3;
    do x = put_int(x*x); while (x < 50);
    put_int(x + 1);
    return 0;
}"                    "9\$81\$82\$"
test_mincc "
main() {
    x = 2;
    do x = put_int(x*x); while (x <  1);
    put_int(x + 1);
    return 0;
}"                         "4\$5\$"

test_mincc "
main() { 
    sum = 0; n = 5;
    for (n=1; n<=10; n = n+1)
        sum = n + sum;
    put_int(sum);
    return 0;
}"                           "55\$"
test_mincc "
main() { 
    sum = 0; n = 1;
    for (; n <= 10; n = n+1)
        sum = n + sum;
    put_int(sum);
    return 0;
}"                           "55\$"
test_mincc "
main() { 
    n = 1;
    for (;n <= 10;) n = n + 1;
    put_int(2*n);
    return 0;
}"                           "22\$"
test_mincc "
main() { 
    ret = 0; i = 0; j = 2;
    for (; i < 8; i = i+2)
        for (j=0; j < 8; j = j+1)
            ret = ret + 1;
    put_int(ret);
    return 0;
}"                           "32\$"
test_mincc "
main() { 
    sum = 0; n = 1;
    while (n <= 8) {
        sum = sum + n*n;
        n = n + 1;
    }
    put_int(sum);
    return 0;
}"                          "204\$"

test_mincc "
six() { return 6; }
main() {
    x = 10;
    put_int(2*x - six() + five());
    return 0;
}"                           "19\$"

test_mincc "
fib(n) {
    if (n <= 0)           return 0;
    if (n == 1 || n == 2) return 1;
    return fib(n-1) + fib(n-2);
}
main() {
    put_int(fib(11));
    return 0;
}"                            "89\$"

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
    put_int(calc(4, 2, 9, -2, -1, 13));
    return 0;
}"                           "12\$"


teardown_test
