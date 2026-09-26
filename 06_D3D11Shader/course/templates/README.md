# course/templates — 各回のテンプレートファイル(.dxtemplate)

拡張機能(05_VSCodeExtention)の「新規プロジェクト作成」フォームで、「テンプレートファイル (.dxtemplate) を選ぶ...」からここのファイルを選ぶと、その回のサンプルがそのままプロジェクトとして作れます。

## 置き場所についての注意

**拡張機能本体(`05_VSCodeExtention/extension/`)には入れていません。** 中身に DxLib SDK 付属のサンプル素材(`Tex1.bmp`・`Kao.bmp`・`DxChara.x` など)が入っており、DxLib 作者の権利物で、拡張機能の MIT ライセンスの対象外だからです(`CLAUDE.md`、`06_D3D11Shader/samples/README.md` と同じ扱い)。
拡張機能そのもの(VSIX)の大きさには影響しません。

## 一覧

| ファイル | 講座 | 元のサンプル |
|---|---|---|
| `01_pixel_shader.dxtemplate` | 01 | C2_PixelShaderTest |
| `02_constant_buffer.dxtemplate` | 02・03 | C5_SetPSConstFTest |
| `04_vertex_shader.dxtemplate` | 04 | C1_VertexShaderTest |
| `05_matrix.dxtemplate` | 05 | C3_SetVSConstFMtxTest |
| `06_sprite_vertex.dxtemplate` | 06 | D1_Sprite2DVertexShader |
| `07a_model_nolight.dxtemplate` | 07 | A01_NormalMesh_NoLight |
| `07b_model_dirlight.dxtemplate` | 07・08 | A05_NormalMesh_DirLight |
| `08b_model_phong.dxtemplate` | 08 | A11_NormalMesh_DirLight_Phong |
| `09_toon.dxtemplate` | 09 | A15_SkinMesh4_DirLight_Toon |
| `10a_mirror.dxtemplate` | 10 | B6_Mirror |
| `10b_shadow.dxtemplate` | 10 | B1_3DAction_DepthShadow |

## 作り直し方

サンプル(`samples/`)を直したら、テンプレートも作り直します。

```
node course/make_templates.js      # samples/ から .dxtemplate を作り直す
node course/verify_templates.js    # zip として正しく読めるか確かめる(拡張機能の読み方と同じ形式)
```

- `template.json` はテンプレートの表示名・説明。サンプルの `sample.json` とは別物(テンプレートの一覧表示用)。
- `sample.json`・`README.md`・`shaders/bin`(コンパイル済みシェーダー)は入れない。生徒が「すべてコンパイル」する。
