#!/bin/bash

# ROOTスクリプトを実行する関数
run_root() {
    XX=$1
    YY=$2
    root -l <<EOF
.L DRS4Ana.C
addGlobalChain("../data/PhysicsRun/ROOT_FILES/Run_005.dat.root")
DRS4Ana obj
obj.Plot_2Dhist_energy_with_cut5("0204", "NaI", "GSO", 0, 3, $XX, $YY, 2, 2, 1, 1)
EOF
}

export -f run_root

# 並列実行（XX, YY の組み合わせ）
parallel run_root ::: 0 1 1 1 1 ::: 1 0 1 2 3
