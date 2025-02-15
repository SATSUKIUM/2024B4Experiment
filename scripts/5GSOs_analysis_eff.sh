#!/bin/bash

# ROOTスクリプトを実行する関数
run_root() {
    XX=$1
    root -l <<EOF
.L DRS4Ana.C
addGlobalChain("../data/PhysicsRun/EFFCIENCY/Run023.dat_2.root")
addGlobalChain("../data/PhysicsRun/evacuation/Run021.dat_2.root")
DRS4Ana obj
obj.EventSelection2_eff("0204", 0, 3, $XX, 0,1,1)
EOF
}

export -f run_root

# 並列実行（XX, YY の組み合わせ）
parallel run_root ::: "0,1" "1,0" "1,1" "1,2" "1,3"