# verify — 仕様書の検証

DxLib が自作シェーダー用にセットする定数バッファの中身を、実際に GPU から読み戻して確かめます。

```
python verify/run.py
```

- 必要なもの: Visual Studio(C++)、SDK(`../00_DxLib_VC/`)、Python。
- 結果: `verify/build/results.txt`(OK / NG の判定)、`verify/build/dump.txt`(場面ごとの定数バッファの全フィールド)。
- 2026-09-27: 61 項目すべて OK(DxLib 3.24f、Visual Studio 2026)。内訳は定数バッファ(`probe/`)51 項目と、2D の絵を正射影カメラ + 3D の板で描く方法(`ortho2d/`。samples/D1 の Camera2D.h とシェーダーをそのまま使う)10 項目。samples/_shared の .hlsli の並びが DxLib と一致することも、この中で確かめている。

## しくみ

1. 検証用シェーダー(`shaders/`)が、定数バッファ(`uint4` の配列として宣言)の 32 ビット 1 個を、32 ビット浮動小数点の画面の 1 画素に書く。下位 16 ビットを R、上位 16 ビットを G に入れる(16 ビットの整数は float で誤差なく表せる)。B は「描いた印」の 0.5。
2. 頂点シェーダー側のバッファ(b0〜b4)は、`DrawPrimitive3DToShader` で点を 1 個ずつ描いて読む(`DumpVS.hlsl`)。ピクセルシェーダー側のバッファ(b0〜b2、b4)は、描画先の全体を覆う板を描いて読む(`DumpPS.hlsl`。2D と 3D の両方)。
3. `probe/main.cpp` が `GetPixelF` で読み戻してビット列を復元し、DxLib のソースパッケージの構造体(`include/`。原本の複写)として解釈する。DxLib の関数(`GetCameraViewMatrix` など)で取れる値や、設定した値から計算した値と比べる。

## ファイル

| ファイル | 中身 |
|---|---|
| `run.py` | シェーダーのコンパイル(CP932 に変換 → ShaderCompiler.exe)、ビルド(cl)、実行 |
| `probe/main.cpp` | 場面の設定と判定(定数バッファ) |
| `ortho2d/main.cpp` | 2D の絵を正射影カメラ + 3D の板で描いたものと、`DrawGraph` で描いたものを画素単位で比べる(比べた 2 枚は `build/ortho2d_<番号>_ref.png` / `_test.png`) |
| `shaders/` | 検証用シェーダー |
| `include/` | DxLib 3.24f のソースパッケージの `Windows/DxShader_*_D3D11.h` の複写(CP932。変更しない) |
