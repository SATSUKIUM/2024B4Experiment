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
    - こちらで招待を送ります。届いたメールから招待を承認してください。
- 以下のリンクの5番「GitHubにSSHの設定をする」に従ってSSHの設定をしてください
    - https://prog-8.com/docs/git-env
- ローカルに作業用ディレクトリを作ります
    - 例 `Username@PC ~ % mkdir github_local_rep`
- リモートリポジトリをローカルにクローン(clone)します
    - `Username@PC ~/github_local_rep % git clone git@github.com:SATSUKIUM/2024B4Experiment.git`
    - 次にいちおうおまじないとして、pullしておいてください。`Username@PC ~/github_local_rep % git pull origin main`

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

リモートリポジトリに他の人が変更を加えたら、それに同期するためにプル(pull)しましょう。後述の理由で、定期的にpullしておくようにしましょう。
- `Username@PC ~/github_local_rep % git pull origin main` でプルできます。

## 競合(conflict)
複数人での作業の際に起こりうる競合と、その解消手段について説明する。
### 競合とは
同じファイルを複数人が編集するとき、編集内容が競合してしまうことがあります。例えば、下向きに時間軸を取って、
- リモートリポジトリにとあるファイルが存在し、AさんとBさんがpullする。
- Aさんが変更してpushする
- Bさんが変更してpushする
    - このとき、Aさんの変更はBさんのpushによって上書きされてしまいそうです。
このような時に、Aさんの変更とBさんの変更は競合しているといいます。そもそも競合を防ぐためにはBさんが変更を行う前にpullしておけば良いと言えますが、これらが同時に起こる時には競合は避けられません。(ただし、各々のbranchで作業すれば頻繁な競合は避けることができる。)

gitという仕組みは、ユーザーからpushの命令があったときにリモートリポジトリとローカルリポジトリの変更時刻を比較し、リモートリポジトリの方が自分の変更より新しい場合(競合している場合)、pushを拒否します。
### 競合の解消
競合が起こったとき、pushが拒否され、エラーメッセージが返されます。競合を解消してから改めてpushしましょう。
- まずはリモートリポジトリをpullします。 `Username@PC ~/github_local_rep % git pull origin main`
- 次に、自動でmergeしてもらいます。 `Usename@PC ~/github_local_rep % git merge origin/main`
- gitにより、競合が発生している箇所が以下のようにマークされます
```
<<<<<<< HEAD (current change)
自分の変更
======
競合している変更
>>>>>>> origin/main
```
- これらの競合を手動で解消してから改めてpushします。
    - 実際に競合相手のところに赴き、拳で解決してもよい
### 競合せざるを得ない場合
githubのローカルリポジトリで作業する場合、例えば環境依存のコンフィグ(設定)ファイルなどは各々の環境ごとに違っているはずです。そのようなファイルはむしろ共有すべきではなく、各々が別のファイルを所有すべきです。それらはgithubのリモートリポジトリにプッシュしないように適宜設定を行う必要があります。<br>
`.gitignore`にプッシュしたくないフォルダを記述しましょう。
- gitのローカルリポジトリ直下(例: `~/2024B4Experiment/`)に`.gitignore`というファイルを作ります。
    - `Username@PC ~/github_local_rep % touch .gitignore`
- `.gitignore`というファイルが生成されるはずなので、それを例えばVSCodeなどで開いて、共有すべきでないフォルダやファイルを書きましょう。__.gitignore自身を記述することをお勧めします__
#### 記述例
```
./.vscode/
./scripts/.vscode/
./scripts/.DS_Store
.gitignore
.DS_Store
FETCH_HEAD
./data/
./scripts/output/
```
### 不必要なファイルを共有してしまった場合
.gitignoreの設定を変更しようと考える時、それはすでに不必要なファイルをプッシュしてしまってリモートリポジトリがとっ散らかってしまっている状況であると推測します。とっ散らかったファイルの片付けについても説明します。<br>
必要なことは
- まずは.gitignoreに適切に追記する
- リモートリポジトリにある要らないファイルを削除する
    - `Username@PC ~/github_local_rep % git rm --cached .DS_Store` など
- 要らないファイルを削除したということをリポジトリに適用する(コミット, プッシュ)
    - `Username@PC ~/github_local_rep % git commit -m "remove trash"` など
    - `Username@PC ~/github_local_rep % git push origin main` など

## Tips
- 動かないテストコードをどう管理するか悩む
    - `/scripts/test/`などのディレクトリを作って、その中にテストコードをぶち込んでおく。
    - もし共有したくなければ、.gitignoreに `./scripts/test/`などと書いておく。

## 注意
- Windows環境ではコロン(:)はディレクトリシステム上で特殊文字として認識されるため、ファイル名にコロンが含まれれているときにエラーを吐きます。まあ、Windowsなんて使ってる人いないだろうけど...

# 参考ページ
ChatGPTに聞いたら割と分かりやすく教えてくれます。
- パソコンにgithubを設定する方法について https://prog-8.com/docs/git-env
- 競合とmergeについて https://qiita.com/Hashimoto-Noriaki/items/0bcd4c5592bc1305c145
- Markdown記法について https://qiita.com/tbpgr/items/989c6badefff69377da7


