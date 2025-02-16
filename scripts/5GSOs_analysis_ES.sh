#!/bin/bash

# ROOTスクリプトを実行する関数
run_root() {
    XX=$1
    root -l <<EOF
.L DRS4Ana.C
listChains("456")
DRS4Ana obj
obj.EventSelection2("0204", 0, 3, $XX, 2,2, 2,2, 2,2, 1,1,1)
EOF
}

export -f run_root

# 並列実行（XX, YY の組み合わせ）
parallel run_root ::: "0,1" "1,0" "1,1" "1,2" "1,3"
