# 05_VSCodeExtention

DxLib の開発環境を VSCode 上でボタン操作だけで作れるようにする拡張機能。生徒配布用。

- **最初に `HANDOFF.md` を読む**(現在の状態、中断した作業、手動確認の続き、環境の落とし穴、ユーザーとの取り決めがある)。
- 設計は `DESIGN.md` が正。仕様を変えるときは先に DESIGN.md を直す。
- 拡張機能本体は `extension/`。`npm run compile` でビルド、`npm run typecheck` で型検査、`npm run package` で VSIX。
- `00_DxLib_Make/` は DxLib 3.24f のソースパッケージ。D3D11 シェーダーの原本と定数ヘッダーの参照用で、変更しない。DxLib 作者のライセンスに従い、拡張機能の MIT ライセンス(`extension/LICENSE`)の対象外。git 管理下に置く(2026-09-23 ユーザー決定)。
- 文字コードの注意: DxLib のヘッダーとシェーダー原本は CP932。読むときは変換する。生成するソースは BOM なし UTF-8。
- 拡張機能自体(`extension/`)のライセンスは MIT(2026-09-23 ユーザー決定)。
