#!/bin/sh

log_and_run() {
    echo "[EXEC] $@"
    $@
}

GENERAL_CFALGS=(-std=gnu11 -Wall -Wextra)
DEBUG_CFLAGS=(-Og -g3 -fstack-protector)
RELEASE_CFLAGS=(-march=native -O3 -DNDEBUG -s)
CC="${CC:-cc}"

if [ "$CC" == "clang" ]; then
    DEBUG_CFLAGS+=(-fsanitize=address,undefined)
    RELEASE_CFLAGS+=(-fuse-ld=lld -flto -fcf-protection=none -Wl,-O3)
fi

if [ "$1" == "release" ]; then
    CFLAGS+=${RELEASE_CFLAGS[@]}
else
    CFLAGS+=${DEBUG_CFLAGS[@]}
fi

log_and_run $CC -I./src \
    "${GENERAL_CFALGS[@]}" \
    "${CFLAGS[@]}" \
    -o clox \
    ./src/build.c
