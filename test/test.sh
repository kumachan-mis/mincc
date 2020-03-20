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

test_mincc  0 0
test_mincc 42 42

test_mincc "1-6+10"            5
test_mincc "2+110-92"         20
test_mincc "1000-990+121+92" 223

test_mincc "1*2+5*8"    42
test_mincc "5*6-10/2"   25
test_mincc "100/3/3"    11
test_mincc "100%7+22%6"  6
echo "OK"
