# 2024卒研

これはREADME.mdです。わかったこととか、わからないことを何でも追記してください。文法はMarkdownの形式に従っています。

## github について

githubではスクリプトやマクロを共同編集できます。1つのファイルをgithubで共同編集する流れを理解するためにはいくつかの専門用語を知ってもらう必要があります。
- ローカルリポジトリとリモートリポジトリ
    - リポジトリとはデータなどの保管場所であり、普段のローカルでの作業はローカルリポジトリに保存されるとも言えます。つまりリモートリポジトリとはみんなが共有するデータ保管場所のことであると言えます。リモートリポジトリとローカルリポジトリは頻繁に同期されている必要があります。
    - リモートリポジトリを提供するのがgithubというサービスです。
- コミットとプッシュ
    - ローカルのファイルに変更を加えたあとは2ステップで、共有ファイルに変更を反映します。
        - コミット(commit)-> ローカルリポジトリに変更を反映する
        - プッシュ(push)-> コミットの内容をリモートリポジトリに反映する

まずはgithubに登録してから、リモートリポジトリのコピーをローカルにダウンロードしてもらいます。

## はじめかた

このリポジトリに参加して共同編集するには、
- ローカルでの設定
    - ユーザー名とメールアドレスの設定(自由に設定できる)
    - `Username@PC ~ % git config --global user.name "ユーザー名"`
    - `Username@PC ~ % git config --global user.email "メールアドレス"`
- githubに登録してください https://github.com/
- あなたのユーザー名を教えてください
    - こちらで招待を送ります(もしかしたらこれは要らないかも....)
- 以下のリンクの5番「GitHubにSSHの設定をする」に従ってSSHの設定をしてください
    - https://prog-8.com/docs/git-env
- ローカルに作業用ディレクトリを作ります
    - 例 `Username@PC ~ % mkdir github_local_rep`
- リモートリポジトリをローカルにクローン(clone)します
    - `Username@PC ~/github_local_rep % git clone git@github.com:SATSUKIUM/2024B4Experiment.git`

## 変更をプッシュ(push)する

ローカルのファイルに変更を加えたあとは、共同作業者に共有するためにファイルをリモートリポジトリにプッシュ(push)する必要があります。
- 変更をステージング(追加)する
    - `Username@PC ~/github_local_rep % git add ./scripts/hoge.C`
- 変更をコミットする
    - コミットする際に、どんな変更を加えたかメッセージを入れると他の人が分かりやすいです。
    - `Username@PC ~/github_local_rep % git commit -m "I changed the part of ... in \"code.C\""`
- プッシュする
    - gitという仕組みにはbranch　という機能があり、同じプロジェクトの中に複数の流派を作れる...けど、そんな複雑なことはしない。mainという名前のbranchをデフォルトに設定する。
    - `Username@PC ~/github_local_rep % git branch -M main`
    - 次にプッシュする
    - `Username@PC ~/github_local_rep % git push -u origin main`
    - パスワードが求められるかと思うので、入力してエンターを押す。
  - これは笑い話なのですが、動かないコードをプッシュするのはやめてください

## リモートリポジトリでの変更をローカルリポジトリに反映する(プル(pull))

リモートリポジトリに他の人が変更を加えたら、それに同期するためにプル(pull)しましょう。(具体的な操作はまだ勉強中。mergeの仕方があんまりわかんない)
- `Username@PC ~/github_local_rep % git pull origin main` でプルできます。

# テスト
