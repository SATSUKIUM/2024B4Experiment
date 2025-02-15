#!/bin/bash

# ROOTスクリプトを実行する関数
run_root() {
    XX=$1
    YY=$2
    root -l <<EOF
.L DRS4Ana.C
addGlobalChain("../data/PhysicsRun/EFFICIENCY/Run023.dat.root")
DRS4Ana obj
obj.EventSelection2_eff("0204", 0, 3, $XX, $YY)
EOF
}

export -f run_root

# 並列実行（XX, YY の組み合わせ）
parallel run_root ::: 0 1 1 1 1 ::: 1 0 1 2 3
