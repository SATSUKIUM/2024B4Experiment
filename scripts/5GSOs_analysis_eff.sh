#!/bin/bash

# ROOTスクリプトを実行する関数
run_root() {
    XX=$1
    YY=$2
    root -l <<EOF
.L DRS4Ana.C
addGlobalChain("../data/PhysicsRun/EFFCIENCY/Run023.dat_2.root")
DRS4Ana obj
obj.EventSelection2_eff("0204", 0, 3, $XX, $YY)
EOF
}

export -f run_root

# 並列実行（XX, YY の組み合わせ）
parallel run_root ::: $(printf "0 1\n1 0\n1 1\n1 2\n1 3")
