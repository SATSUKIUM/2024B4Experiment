# 8スレッドの並列処理でbinary2tree_kashima.Cを回すようにしました。.rootファイルはROOT_FILESというフォルダに移送するようにした。

#!/bin/bash

# .datファイルが格納されているフォルダー
DAT_FOLDER="../data/20241212/huruno1_m_0/"

# ROOTスクリプトのパス
ROOT_SCRIPT="binary2tree_kashima.C"

# .rootファイルを移動させる共通のディレクトリ
ROOT_DEST_FOLDER="${DAT_FOLDER}/ROOT_FILES"
echo "Root destination folder: $ROOT_DEST_FOLDER"

# ROOT_DEST_FOLDERが存在しない場合は作成
mkdir -p "$ROOT_DEST_FOLDER"

# ターゲットディレクトリに書き込み権限を与える
chmod -R u+w "$ROOT_DEST_FOLDER"

export ROOT_DEST_FOLDER
export ROOT_SCRIPT
# .datファイルのリストを作成
find "$DAT_FOLDER" -type f -name "*.dat" | parallel -j 8 '
    dat_file="{}"
    
    echo "Processing $dat_file..."
    
    # ROOTコマンドを実行
    root -b -l -q "$ROOT_SCRIPT(\"$dat_file\")"
    
    # .rootファイルの生成を待ってから移動
    root_file="${dat_file}.root"  # .datファイルに対して .dat.root
    #echo $root_file
    
    # .rootファイルが存在すれば移動
    if [[ -f "$root_file" ]]; then
        mv "$root_file" "$ROOT_DEST_FOLDER/"
        # echo ${ROOT_DEST_FOLDER}
        echo "$root_file has been moved to $ROOT_DEST_FOLDER"
    else
        echo "Error: $root_file not found after processing $dat_file"
    fi
'

echo "All files processed and moved."