#!/usr/local/bin/zsh

test_mincc() {
    MINCC=./build/mincc.out
    ASSEMBLY=./build/out.s
    EXEC=./build/out

    TESTLIB=./test/testlib.c

    input=$1
    echo ${input} | ${MINCC} ${input} > ${ASSEMBLY}

    if [ $? -ne 0 ]; then
        echo "\e[0;31m[FAIL]\e[m ${input} => ${expected} expected, but failed to compile"
        exit 1
    fi

    gcc-9 ${ASSEMBLY} ${TESTLIB} -o ${EXEC}
    ${EXEC}

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

test_mincc "return 0;"   0
test_mincc "return 42;" 42

test_mincc "return 1-6+10;"            5
test_mincc "return 2+110-92;"         20
test_mincc "return 1000-990+121+92;" 223

test_mincc "return 1*2 + 5*8;"     42
test_mincc "return 5*6 - 10/2;"    25
test_mincc "return 100/3/3;"       11
test_mincc "return 100%7 + 22%6 ;"  6

test_mincc "return (1+2) * 3;"              9
test_mincc "return (6-3+1) * (9-6+7-2); "  32
test_mincc "return (4*1) * (4/3) ;"         4

test_mincc "return + 1+6 ; "      7
test_mincc "return -5+7*5;"      30
test_mincc "return +5 + -5;"      0
test_mincc "return +10 + (-8);"   2

test_mincc "return 1 << 3;"          8
test_mincc "return 10 + (-1 <<2);"   6
test_mincc "return 9 >> 1;"          4
test_mincc "return 23 + 7*(-6 >>1);" 2

test_mincc "return 0 == -1;"    0
test_mincc "return 0 == 0;"     1
test_mincc "return 0 == 1;"     0
test_mincc "return 0 != -1;"    1
test_mincc "return 0 != 0;"     0
test_mincc "return 0 != 1;"     1
test_mincc "return 0 < -1;"     0
test_mincc "return 0 < 0;"      0
test_mincc "return 0 < 1;"      1
test_mincc "return 0 > -1;"     1
test_mincc "return 0 > 0;"      0
test_mincc "return 0 > 1;"      0
test_mincc "return 0<=-1;"      0
test_mincc "return 0<= 0;"      1
test_mincc "return 0<= 1;"      1
test_mincc "return 0>=-1;"      1
test_mincc "return 0>= 0;"      1
test_mincc "return 0>= 1;"      0
test_mincc "return 0<1 == 1<2;" 1
test_mincc "return 0<1 == 1>2;" 0
test_mincc "return 0<1 != 1<2;" 0
test_mincc "return 0<1 != 1>2;" 1

test_mincc "return 2 & 1;"      0
test_mincc "return 3 & 6;"      2
test_mincc "return 2 ^ 1;"      3
test_mincc "return 3 ^ 6;"      5
test_mincc "return 2 | 1;"      3
test_mincc "return 3 | 6;"      7
test_mincc "return 0&1 ^ 1&1;"  1
test_mincc "return 0&1 ^ 1&0;"  0
test_mincc "return 1&1 ^ 1&1;"  0
test_mincc "return 0^1 | 1^1;"  1
test_mincc "return 0^1 | 1^0;"  1
test_mincc "return 1^1 | 1^1;"  0
test_mincc "return 0&1 | 1&1;"  1
test_mincc "return 0&1 | 1&0;"  0
test_mincc "return 1&1 | 1&1;"  1
test_mincc "return ~2 & 7;"     5

test_mincc "return 12 == 12;"         1
test_mincc "return 12 == 19;"         0
test_mincc "return 12 != 12;"         0
test_mincc "return 12 != 19;"         1
test_mincc "return 1 == 1 && 2 == 2;" 1
test_mincc "return 1 != 1 && 2 == 2;" 0
test_mincc "return 1 == 1 && 2 != 2;" 0
test_mincc "return 1 != 1 && 2 != 2;" 0
test_mincc "return 1 == 1 || 2 == 2;" 1
test_mincc "return 1 != 1 || 2 == 2;" 1
test_mincc "return 1 == 1 || 2 != 2;" 1
test_mincc "return 1 != 1 || 2 != 2;" 0
test_mincc "return !0;"               1
test_mincc "return !1;"               0

test_mincc "x = 3; return x + 1;"                            4
test_mincc "tmp =  2+110-94; return tmp*2;"                 36
test_mincc "a = 5;b = 2;c = 6; return (a + c)*b*b;"         44
test_mincc "a = 5;b = 12; c=13; return a*a+b*b == c*c;"      1
test_mincc "x = 1;y = 3; x = x + y; y = x + y; return x*y;" 28

test_mincc "; x = 1;; return x + 1;"            2
test_mincc "x = 1; return x + 3; return x + 1;" 4


test_mincc "x = five(); y = one(); return x+y;"  6
test_mincc "x = five(); return x+five();"       10

