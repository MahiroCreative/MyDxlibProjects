# DxLib 開発環境(VSCode 拡張機能)

DxLib(C++)のプロジェクト作成・ビルド・実行・デバッグを、左の DxLib パネルのボタンだけで行えるようにします。

## 使い方

1. 左のアクティビティバーの DxLib アイコンを押す。
2. 環境欄が赤い項目を、横のボタンで直す(Visual Studio、DxLib SDK)。
3. 「新規プロジェクト作成」→ 名前・作成先・テンプレートを選んで作成。
4. F5 か「▶ 実行」。

## 開発

```
npm install
npm run compile      # dist/extension.js を作る
npm run typecheck    # 型検査
npm run package      # VSIX を作る
```

このフォルダを VSCode で開いて F5 を押すと、拡張機能開発ホストが起動します。

## ライセンス

この拡張機能自体(このフォルダの内容)は MIT ライセンスです([LICENSE](LICENSE))。

DxLib 本体(SDK・ソースパッケージ)はこのライセンスの対象外です。`../00_DxLib_Make/` に置いてある DxLib のソースパッケージは、DxLib の作者(Yu Tanaka 氏)のライセンスに従います。この拡張機能は DxLib 本体を同梱・再配布しません。生徒は各自で DxLib SDK を入手し、パネルからフォルダを指定して使います。

## 生徒への配布

`npm run release` で `release/` フォルダ(VSIX + `install.bat` + `README.txt`)ができる。フォルダごと zip にして渡す。生徒は `install.bat` をダブルクリックするだけ。

VSIX を直接ダブルクリックしてはいけない。Visual Studio が入っている PC では Visual Studio のインストーラーが起動して失敗する。詳しくは [DESIGN.md](../DESIGN.md) の 18 章。
