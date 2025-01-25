# README.mdはscripts内のコード、関数の使用説明書を書く場所です
大きく分けて、DRS4Anaとそれ以外です。それ以外は単独で動くものが多いです。
## .Lのタイプ
単独のスクリプトを使うとき、`root hoge.C`とすることがあるはず。このとき、そのスクリプトに引数を渡したければどうするのがいいでしょうか。答えは、
- `root` (ROOTの起動)
- `.L hoge.C` (hoge.Cというスクリプトの読み込み。hoge.C内に記述された関数を全て読み込む。)
- `hoge("../data.txt")` (hoge.C内のhoge(TString filepath){...}という関数を使う。)<br>
とすれば引数を渡せます。
## root hoge.Cのタイプ
単独のスクリプトを使うとき、`root hoge.C`とするときの挙動は、「勝手にhoge.C内のhoge関数が読み込まれる」です。
## DRS4Anaについて
DRS4Ana.h(ヘッダファイル)とDRS4Ana.C(ソースファイル)のペアで動くDRS4Anaクラスの総称。

- クラスとは
    - 複数の関数を柔軟に使えるようひとまとめにしたデータと関数のセット
    - スクリプトでは、毎回書かれた通りの動作しかできないが、クラスを使えばユーザーの操作で関数①の後に関数②を使ったり、関数①の後に関数③を使ってから関数②を使うこともできる。
- ヘッダファイルとソースファイル
    - 一つのコード内でクラスの定義と、クラスの使用を記述できるが、一般的にはファイルを分けて記述しておくのがふつう。
### `Makedir_Date()`
`./figure/`にYYYYMMDDのフォルダを作る
### `IfFile_duplication(TString folderPath, TString &fileName)`
同じファイルがある場合に名前を変えてくれる関数<br>
例えば、"./figure/YYYYMMDD"というパスと、hoge.pdfを渡せば、そのディレクトリにhoge.pdfとhoge2.pdfが存在する場合に、渡した"hoge.pdf"を"hoge3.pdf"に変えてくれる関数
### `PlotPedestalMean(Int_t iBoard, Int_t iCh, Double_t Vcut)`
記録されるパルスの下に直流成分が乗っていることがある。これはベースラインまたはペデスタルと呼ばれ、パルスの正しい電荷量を計算するにはベースライン(ペデスタル)を引く処理が必要である。<br>
`GetChargeIntegral`ではそのような処理をしているが、ベースライン電圧がどれくらいか知ることはデバッグの上で重要である。<br>
ペデスタルの分布を描く関数。
### `GetChargeIntegral(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t TcutMin = 0, Double_t TcutMax = 1000)`
- 負パルスか正パルスのどちらを使用しているかによって、異常な波形がある場合に-9999.9を返す
- `GetPedestal`でペデスタルを得る
- fTime(1番目のセルからの時間)のある領域にわたって、fWaveform(電圧値)の和をとる
### `automated_peaksearch(Int_t iBoard, Int_t iCh, Double_t Vcut, Double_t xmin, Double_t xmax, Int_t numPeaks, Double_t fitRange = 2.0)`
chargeIntegralのヒストグラムに自動でピークサーチしてフィッティングをする関数であるが、**レガシーな仕様**が含まれているため、使用は推奨されない
### `PlotTriggerRate(Int_t iCh = 0)`
DAQの時のレートをプロットする
### `Overlay_PlotWaves(Int_t iBoard=0, Int_t iCh=0)`
波形の重ね書き2次元ヒストグラムを表示する。対話モードでやった方が早いよ。
### `DEBUG_timebin(Int_t iBoard = 0, Int_t iCh = 0)`
timing calibrationで取得される、各セルごとの時間幅などを出力するデバッグ用のコード
### `Plot_wave_two_boards(Int_t iCh_master = 0, Int_t iCh_slave = 0, Int_t EventID = 0, Int_t canvas_index)`と`Plot_waves_two_boards(Int_t event_num_initial = 0, Int_t iCh_master = 0, Int_t iCh_slave = 0)`
マスターとスレイブからチャンネルをひとつづつ選んで横に並べて波形を1イベントづつ見るコード。**仕様が古いのでもう使うことはないんじゃないかな。**
