# reference/d3d9_original — 公式 Direct3D 9 版サンプルの原本

移植の比較用に集めた、DxLib 公式の Direct3D 9 版サンプルです。**変更しません。**
DxLib の作者の権利物で、拡張機能の MIT ライセンスの対象外です。

## A_base(3D モデル描画の基本 21 本)

- 出どころ: SDK の `help/program/dxprogram_3DModelShaderBase.html` と、各サンプルの「実行に必要なファイル一式」(`https://dxlib.xsrv.jp/file/<名前>.zip`。2026-09-26 取得)。
- 各フォルダの直下は zip の中身(CP932・CRLF)。exe・ShaderCompiler.exe・コンパイル済みの .vso/.pso は除いた。
- `html/` は HTML から取り出した版(UTF-8。HTML の中の `<` がエスケープされていないので、コードの中のタグは div・b・a だけを消して取り出した)。zip 版との違いは、HTML 版の C++ に `SetUseDirect3DVersion( DX_DIRECT3D_9EX )` があることと、コメント・空行だけ。
- `index.json` は見出し・zip の URL・読み込むシェーダー名の一覧。
- 欠け:
  - `02_SkinMesh4_NoLight` と `11_NormalMesh_DirLight_Phong` は公式 zip が壊れている(サーバーが 63,152 バイトで切れたものを返す。2 回取り直して同じ)。HTML 版だけ。モデル(DxChara.x / NormalBox.mqo)は他のサンプルと同じもの。
  - `21_NormalMesh_DirSpotPointLight_NrmMap` は、HTML の「頂点シェーダーのプログラム」の見出しの中身が実はピクセルシェーダー(頂点シェーダーは HTML に無い)。zip には両方ある。

## B_applied(応用 7 本)

SDK の `help/program/<名前>/` をそのまま複写(CP932・CRLF)。説明は SDK の `help/program/dxprogram_<名前>.html`(鏡は `dxprogram_Mirror_Shader.html`)。

## C_function(関数リファレンスのサンプル 8 本)

SDK の `help/function/dxfunc_3d_shader.html` から取り出した(UTF-8)。
C7・C8 は Indexed 版の描画関数のサンプルで、シェーダーは C2・C1 と同じもの。
コンパイル済みの .vso/.pso と画像は SDK の `サンプルプログラム実行用フォルダ` にある。
