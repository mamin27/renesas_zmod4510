#!/bin/sh
# Compare src/reimpl/zmod4xxx_cleaning.c with lib/lib_zmod4xxx_cleaning.a on
# a mock I2C bus. Never touches a real sensor.
set -e
R="$(cd "$(dirname "$0")/.." && pwd)"
O="${TMPDIR:-/tmp}/reimpl_compare_cleaning"
mkdir -p "$O"
CC="${CC:-gcc}"
INC="-I$R/src/algos -I$R/src/sensors"
$CC -std=gnu99 -O2 -Dzmod4xxx_cleaning_run=re_zmod4xxx_cleaning_run $INC \
    -c "$R/src/reimpl/zmod4xxx_cleaning.c" -o "$O/re_cleaning.o"
$CC -std=gnu99 -O2 $INC -c "$R/src/sensors/zmod4xxx.c" -o "$O/drv.o"
$CC -std=gnu99 -O2 $INC -c "$R/test_script/reimpl_compare_cleaning.c" -o "$O/cmp.o"
$CC "$O/cmp.o" "$O/re_cleaning.o" "$O/drv.o" "$R/lib/lib_zmod4xxx_cleaning.a" \
    -lm -o "$O/reimpl_compare_cleaning"
"$O/reimpl_compare_cleaning"
