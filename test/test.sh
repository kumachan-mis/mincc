#!/usr/local/bin/zsh

test_mincc() {
    MINCC=./build/mincc.out
    ASSEMBLY=./build/out.s
    EXEC=./build/out

    input=$1
    echo ${input} | ${MINCC} ${input} > ${ASSEMBLY}
    gcc-9 ${ASSEMBLY} -o ${EXEC}
    ${EXEC}

    actual=$?
    expected=$2
    if [ ${actual} = ${expected} ]; then
        echo "${input} => ${actual}"
    else
        echo "${input} => ${expected} expected, but got ${actual}"
        exit 1
    fi

    rm -Rf ${ASSEMBLY} ${EXEC}
}

test_mincc  "0;" 0
test_mincc "42;" 42

test_mincc "1-6+10;"            5
test_mincc "2+110-92;"         20
test_mincc "1000-990+121+92;" 223

test_mincc "1*2 + 5*8;"     42
test_mincc "5*6 - 10/2;"    25
test_mincc "100/3/3;"       11
test_mincc "100%7 + 22%6 ;"  6

test_mincc "(1+2) * 3;"              9
test_mincc "(6-3+1) * (9-6+7-2); "  32
test_mincc "(4*1) * (4/3) ;"         4

test_mincc "+ 1+6 ; "      7
test_mincc "-5+7*5;"      30
test_mincc "+5 + -5;"      0
test_mincc "+10 + (-8);"   2

test_mincc "1 << 3;"          8
test_mincc "10 + (-1 <<2);"   6
test_mincc "9 >> 1;"          4
test_mincc "23 + 7*(-6 >>1);" 2

test_mincc "0 == -1;"    0
test_mincc "0 == 0;"     1
test_mincc "0 == 1;"     0
test_mincc "0 != -1;"    1
test_mincc "0 != 0;"     0
test_mincc "0 != 1;"     1
test_mincc "0 < -1;"     0
test_mincc "0 < 0;"      0
test_mincc "0 < 1;"      1
test_mincc "0 > -1;"     1
test_mincc "0 > 0;"      0
test_mincc "0 > 1;"      0
test_mincc "0<=-1;"      0
test_mincc "0<= 0;"      1
test_mincc "0<= 1;"      1
test_mincc "0>=-1;"      1
test_mincc "0>= 0;"      1
test_mincc "0>= 1;"      0
test_mincc "0<1 == 1<2;" 1
test_mincc "0<1 == 1>2;" 0
test_mincc "0<1 != 1<2;" 0
test_mincc "0<1 != 1>2;" 1

test_mincc "2 & 1;"      0
test_mincc "3 & 6;"      2
test_mincc "2 ^ 1;"      3
test_mincc "3 ^ 6;"      5
test_mincc "2 | 1;"      3
test_mincc "3 | 6;"      7
test_mincc "0&1 ^ 1&1;"  1
test_mincc "0&1 ^ 1&0;"  0
test_mincc "1&1 ^ 1&1;"  0
test_mincc "0^1 | 1^1;"  1
test_mincc "0^1 | 1^0;"  1
test_mincc "1^1 | 1^1;"  0
test_mincc "0&1 | 1&1;"  1
test_mincc "0&1 | 1&0;"  0
test_mincc "1&1 | 1&1;"  1
test_mincc "~2 & 7;"     5

test_mincc "12 == 12;"         1
test_mincc "12 == 19;"         0
test_mincc "12 != 12;"         0
test_mincc "12 != 19;"         1
test_mincc "1 == 1 && 2 == 2;" 1
test_mincc "1 != 1 && 2 == 2;" 0
test_mincc "1 == 1 && 2 != 2;" 0
test_mincc "1 != 1 && 2 != 2;" 0
test_mincc "1 == 1 || 2 == 2;" 1
test_mincc "1 != 1 || 2 == 2;" 1
test_mincc "1 == 1 || 2 != 2;" 1
test_mincc "1 != 1 || 2 != 2;" 0
test_mincc "!0;"               1
test_mincc "!1;"               0

test_mincc "x = 3; x + 1;"                            4
test_mincc "tmp =  2+110-94; tmp*2;"                 36
test_mincc "a = 5;b = 2;c = 6; (a + c)*b*b;"         44
test_mincc "a = 5;b = 12; c=13; a*a+b*b == c*c;"      1
test_mincc "x = 1;y = 3; x = x + y; y = x + y; x*y;" 28
echo "OK"
