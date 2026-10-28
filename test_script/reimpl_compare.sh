#!/bin/sh
# Build src/reimpl with renamed symbols next to lib/lib_no2_o3.a and compare.
set -e
R="$(cd "$(dirname "$0")/.." && pwd)"
O="${TMPDIR:-/tmp}/reimpl_compare"
mkdir -p "$O"
CC="${CC:-gcc}"
REN="-Dinit_no2_o3=re_init_no2_o3 -Dinit_no2_o3_internal=re_init_no2_o3_internal \
-Dcalc_no2_o3=re_calc_no2_o3 -Dcalc_no2_o3_internal=re_calc_no2_o3_internal \
-Dcalc_no2_o3_O3_nnet=re_calc_no2_o3_O3_nnet -Dcalc_no2_o3_NO2_nnet=re_calc_no2_o3_NO2_nnet \
-Dno2_o3_ver=re_no2_o3_ver"
INC="-I$R/src/algos -I$R/src/sensors -I$R/src/reimpl"
$CC -std=gnu99 -O2 -ffp-contract=off $REN $INC -c "$R/src/reimpl/no2_o3.c" -o "$O/re_no2_o3.o"
$CC -std=gnu99 -O2 -ffp-contract=off $REN $INC -c "$R/src/reimpl/no2_o3_nnet.c" -o "$O/re_nnet.o"
$CC -std=gnu99 -O2 $INC -c "$R/src/reimpl/no2_o3_data.c" -o "$O/re_data1.o"
$CC -std=gnu99 -O2 $INC -c "$R/src/reimpl/no2_o3_nnet_data.c" -o "$O/re_data2.o"
$CC -std=gnu99 -O2 $INC -c "$R/test_script/reimpl_compare.c" -o "$O/cmp.o"
$CC "$O/cmp.o" "$O/re_no2_o3.o" "$O/re_nnet.o" "$O/re_data1.o" "$O/re_data2.o" \
    "$R/lib/lib_no2_o3.a" -lm -o "$O/reimpl_compare"
"$O/reimpl_compare" "$@"
