#!/bin/bash

# ROOTスクリプトを実行する関数
run_root() {
    XX=$1
    YY=$2
    FUNC_CALL=$3
    root -l <<EOF
.L DRS4Ana.C
addGlobalChain("../data/PhysicsRun/ROOT_FILES/Run_005.dat.root")
DRS4Ana obj
obj.${FUNC_CALL}($XX, $YY)
EOF
}

export -f run_root

# 並列実行（XX, YY の組み合わせ）
FUNC_CALL=$1
parallel run_root ::: 0 1 1 1 1 ::: 1 0 1 2 3 ::: "$FUNC_CALL"