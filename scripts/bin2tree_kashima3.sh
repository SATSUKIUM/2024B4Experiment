#!/bin/bash

# .datファイルが格納されているフォルダー
DAT_FOLDER="../../data/raw_data/20241126"

# ROOTスクリプトのパス
ROOT_SCRIPT="binary2tree_sato4.C"

# 移動先のフォルダパス
ROOT_DEST_FOLDER="${DAT_FOLDER}/ROOT_FILES"


# 対象フォルダー内のすべての.datファイルを処理
for dat_file in "$DAT_FOLDER"/*.dat; do
    if [[ -f "$dat_file" ]]; then
        echo "Processing $dat_file..."
        
        # ROOTコマンドを実行
        root -l -q "$ROOT_SCRIPT(\"$dat_file\",2,0)" #引数に、何台のボードがあるか、デバッグ用のフラグを追加してる

        # 処理が終わったら移動
        root_file = "${dat_file%.dat}.root"
        if [[ -f "$root_file"]]; then
            mv "$root_file" "ROOT_DEST_FOLDER"
            echo "$root_file has been moved to $ROOT_DEST_FOLDER"
        fi
    else
        echo "No .dat files found in $DAT_FOLDER"
    fi
done

echo "All files processed."