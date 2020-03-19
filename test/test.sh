#!/usr/local/bin/zsh

test_initial_src() {
    MINCC=./build/mincc.out
    ASSEMBLY=./build/out.s
    EXEC=./build/out

    input=$1
    ${MINCC} ${input} > ${ASSEMBLY} && gcc-9 ${ASSEMBLY} -o ${EXEC} && ${EXEC}

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

test_all() {
    test_initial_src  0  0
    test_initial_src 42 42
    echo "OK"
}

test_all
