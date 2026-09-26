# 06_D3D11Shader

DxLib 3.24f の Direct3D 11 シェーダー教材。計画と進み具合は `README.md` が正。

- 仕様書(`spec/`)の記述には必ず根拠を付ける。根拠は「DxLib ソースの場所(`00_DxLib_Make` からの相対パス:行)」か「`verify/` での描画確認の項目名」。ソースを読んだだけの推論は「推測」と明記する。
- 検証は `python verify/run.py`。DxLib が自作シェーダー用にセットする定数バッファを浮動小数点の画面に書き出して読み戻し、DxLib の関数で取れる値と比べる。
- DxLib のソースパッケージは `../05_VSCodeExtention/00_DxLib_Make/`(CP932。変更しない)。SDK は `../00_DxLib_VC/`。
- ShaderCompiler.exe は CP932 のソースしか読めない。教材の HLSL は UTF-8 で書き、コンパイル時に CP932 に変換する。
- `reference/d3d9_original/` は公式の原本。変更しない。
- 動作確認できたものだけを `samples/` に置く。
