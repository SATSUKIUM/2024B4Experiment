import sys
import subprocess
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QVBoxLayout, QWidget
from PyQt5.QtCore import Qt

class DragDropWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("ROOT File Processor")
        self.setGeometry(100, 100, 400, 300)

        # 中央ウィジェットの設定
        self.label = QLabel("ここにファイルをドラッグ＆ドロップしてください", self)
        self.label.setAlignment(Qt.AlignCenter)
        self.label.setStyleSheet("font-size: 16px;")

        central_widget = QWidget()
        layout = QVBoxLayout()
        layout.addWidget(self.label)
        central_widget.setLayout(layout)
        self.setCentralWidget(central_widget)

        # ドラッグ＆ドロップを有効化
        self.setAcceptDrops(True)

    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.accept()
        else:
            event.ignore()

    def dropEvent(self, event):
        files = [url.toLocalFile() for url in event.mimeData().urls()]
        if files:
            file_path = files[0]  # 最初のファイルのみ処理
            self.label.setText(f"処理中: {file_path}")

            # ROOTマクロの実行
            self.run_root_macro(file_path)

    def run_root_macro(self, file_path):
        # ROOTのマクロをsubprocessで実行し、返り値を取得
        macro = "binary2tree_kashima.C"  # マクロの名前を指定
        try:
            # ROOTを実行し標準出力をキャプチャ
            result = subprocess.run(
                ["root", "-l", f'{macro}("{file_path}")'],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            # 処理後のファイル確認
            if os.path.exists(output_file):
                self.label.setText(f"処理が正常に完了しました！\n出力ファイル: {output_file}")
            else:
                self.label.setText(f"エラー: 出力ファイルが生成されませんでした。\n詳細:\n{result.stderr.strip()}")

        except Exception as e:
            self.label.setText(f"予期しないエラーが発生しました: {e}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = DragDropWindow()
    window.show()
    sys.exit(app.exec_())