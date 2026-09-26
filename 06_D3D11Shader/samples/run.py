r"""サンプルの動作確認。公式の Direct3D 9 版(原本)と Direct3D 11 移植版を同じフレームで撮影して比べる。

各サンプルのフォルダの sample.json:
  original          reference/d3d9_original からの原本のフォルダ
  original_main     原本の C++ のファイル名
  original_shaders  原本が読み込むコンパイル済みシェーダー名 → 原本の .fx。SDK の「サンプルプログラム実行用フォルダ」に
                    同じ名前のファイルがあればそれを使い、無ければ .fx を original_profiles(既定 vs_2_0 / ps_2_0)でコンパイルする
  assets            実行に必要な素材(サンプルのフォルダにあるもの。原本の実行にも同じものを使う)
  frame             何回目の ScreenFlip の直前を撮るか(WaitKey で止まるサンプルはそこで撮る)
  max_diff_ratio    差が大きい画素の割合の上限(既定 0.002 = 0.2%)。変えるときは diff_reason に理由を書く
  original_remove_lines  原本の C++ から除く行(SDK に無いヘッダーの include など。原本のままではビルドできないもの)
  original_patches  原本のシェーダーに当てる修正 [{file, old, new}](原本が今の DxLib では正しく動かない所を、
                    移植版と同じように直して比べるため。原本のファイルは書き換えない。理由は reason に書く)

手順(サンプルごと。作業フォルダは samples/build/<サンプル名>/):
  1. port: サンプルのフォルダを複写し、shaders/*VS.hlsl・*PS.hlsl を拡張機能と同じ手順(CP932 に変換 → ShaderCompiler.exe
     vs_4_0 / ps_4_0)で shaders/bin にコンパイル、src/main.cpp をビルドして Direct3D 11 で実行
  2. orig: 原本の C++(CP932 に変換)とコンパイル済みシェーダーで、Direct3D 9Ex で実行
  3. 2 枚の capture.png を比べ、compare.png(原本 | 移植版 | 差 × 4)を作る
どちらも _verify/verify_hook.h を /FI で差し込んで撮影・終了させる(サンプルのソースは書き換えない)。

使い方: python samples/run.py [--compare-only] [サンプル名 ...]   (省略時は sample.json のある全サンプル)
  --compare-only  ビルドと撮影をせず、前回の撮影結果で比べ直すだけ
"""
import json
import os
import shutil
import subprocess
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ORIGINALS = os.path.join(ROOT, "reference", "d3d9_original")
SHARED = os.path.join(HERE, "_shared")
HOOK = os.path.join(HERE, "_verify", "verify_hook.h")
SDK = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\プロジェクトに追加すべきファイル_VC用"
SDK_SAMPLE_DATA = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\サンプルプログラム実行用フォルダ"
COMPILER = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\Tool\ShaderCompiler\ShaderCompiler.exe"
DIFF_THRESHOLD = 24  # 1 画素の差(RGB の最大)がこれを超えたら「違う画素」


def vcvarsall():
    vswhere = os.path.join(os.environ["ProgramFiles(x86)"], "Microsoft Visual Studio", "Installer", "vswhere.exe")
    out = subprocess.check_output([vswhere, "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                                   "-property", "installationPath", "-utf8"], encoding="utf-8").strip()
    return os.path.join(out, "VC", "Auxiliary", "Build", "vcvarsall.bat")


def read_text(path):
    b = open(path, "rb").read()
    for enc in ("utf-8-sig", "cp932"):
        try:
            return b.decode(enc)
        except UnicodeDecodeError:
            pass
    raise ValueError(path)


def write_cp932(path, text):
    with open(path, "w", encoding="cp932", newline="\r\n") as f:
        f.write(text.replace("\r\n", "\n"))


def compile_shader(src, out, profile, include_dir=None):
    """src(任意の文字コード)を CP932 に変換して一時フォルダでコンパイルする。include_dir の .hlsli も一緒に置く"""
    tmp = out + ".src"
    shutil.rmtree(tmp, ignore_errors=True)
    os.makedirs(tmp)
    for d in [include_dir] if include_dir else []:
        for f in os.listdir(d):
            if f.endswith((".hlsli", ".fxh", ".h")):
                write_cp932(os.path.join(tmp, f), read_text(os.path.join(d, f)))
    cp = os.path.join(tmp, os.path.basename(src))
    write_cp932(cp, read_text(src))
    if os.path.exists(out):
        os.remove(out)  # 前の出力が残っていると、コンパイルの失敗に気づけない
    r = subprocess.run([COMPILER, f"/T{profile}", f"/Fo{out}", cp], capture_output=True)
    if r.returncode != 0 or not os.path.exists(out):
        raise RuntimeError(f"シェーダーのコンパイルに失敗: {src}\n{r.stdout.decode('cp932', 'replace')}{r.stderr.decode('cp932', 'replace')}")


def build_and_run(work, main_cpp, d3d, frame, defines=""):
    """main_cpp を verify_hook.h 付きでビルドし、work を作業フォルダにして実行する。capture.png のパスを返す"""
    src = os.path.join(work, "_main.cpp")
    # ソースは BOM 付き UTF-8 にそろえる(/source-charset:.932 でも BOM があれば UTF-8 として読まれる)
    with open(src, "w", encoding="utf-8-sig", newline="\r\n") as f:
        f.write(read_text(main_cpp).replace("\r\n", "\n"))
    bat = os.path.join(work, "_build.bat")
    lines = [
        "@echo off",
        f'call "{vcvarsall()}" x64 >nul 2>&1',
        f'cd /d "{work}"',
        f'cl /nologo /EHsc /W3 /std:c++20 /source-charset:.932 /execution-charset:.932 /D_WINDOWS /DWIN32 /O2 /MT /DNDEBUG '
        f'/DVERIFY_D3D={d3d} /DVERIFY_FRAME={frame} {defines} /FI"{HOOK}" /I "{SDK}" _main.cpp /Fo_main.obj /Fe_sample.exe '
        f'/link /SUBSYSTEM:WINDOWS /LIBPATH:"{SDK}" > _build.log 2>&1',
    ]
    with open(bat, "w", encoding="cp932", newline="\r\n") as f:
        f.write("\n".join(lines) + "\n")
    subprocess.run(["cmd.exe", "/d", "/c", bat])
    exe = os.path.join(work, "_sample.exe")
    if not os.path.exists(exe):
        raise RuntimeError("ビルドに失敗:\n" + read_text(os.path.join(work, "_build.log")))
    capture = os.path.join(work, "capture.png")
    try:
        r = subprocess.run([exe], cwd=work, timeout=120)
    except subprocess.TimeoutExpired:
        raise RuntimeError("120 秒で終わらなかった")
    if r.returncode == 3:
        raise RuntimeError(f"指定の Direct3D({d3d})で起動できなかった")
    if not os.path.exists(capture):
        raise RuntimeError(f"撮影できなかった(終了コード {r.returncode})")
    return capture


def prepare_port(name, conf, work):
    sample = os.path.join(HERE, name)
    shutil.rmtree(work, ignore_errors=True)
    shutil.copytree(sample, work, ignore=shutil.ignore_patterns("sample.json", "README.md"))
    shaders = os.path.join(work, "shaders")
    # サンプルの .hlsli は _shared と同じものであること
    for f in os.listdir(SHARED):
        mine = os.path.join(sample, "shaders", f)
        if os.path.exists(mine) and read_text(mine) != read_text(os.path.join(SHARED, f)):
            raise RuntimeError(f"{name}/shaders/{f} が _shared/{f} と違う(python samples/sync_shared.py で揃える)")
    bin_dir = os.path.join(shaders, "bin")
    os.makedirs(bin_dir, exist_ok=True)
    n = 0
    for f in sorted(os.listdir(shaders)):
        stem, ext = os.path.splitext(f)
        if ext.lower() not in (".hlsl", ".fx"):
            continue
        if stem.upper().endswith("VS"):
            profile, out_ext = "vs_4_0", ".vso"
        elif stem.upper().endswith("PS"):
            profile, out_ext = "ps_4_0", ".pso"
        else:
            continue
        compile_shader(os.path.join(shaders, f), os.path.join(bin_dir, stem + out_ext), profile, shaders)
        n += 1
    return n


def prepare_original(name, conf, work):
    orig = os.path.join(ORIGINALS, conf["original"])
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(work)
    for a in conf.get("assets", []):
        shutil.copy(os.path.join(HERE, name, a), os.path.join(work, os.path.basename(a)))
    profiles = conf.get("original_profiles", {"vs": "vs_2_0", "ps": "ps_2_0"})
    patches = conf.get("original_patches", [])
    for binary, fx in conf.get("original_shaders", {}).items():
        sdk_bin = os.path.join(SDK_SAMPLE_DATA, binary)
        out = os.path.join(work, binary)
        src = os.path.join(orig, fx)
        mine = [p for p in patches if p["file"] == os.path.basename(fx)]
        if mine:
            # 原本の不具合を、移植版と同じように直した写しでコンパイルする(原本のファイルは書き換えない)
            text = read_text(src)
            for p in mine:
                if p["old"] not in text:
                    raise RuntimeError(f"original_patches の置き換え元が原本に無い: {p['file']}: {p['old']}")
                text = text.replace(p["old"], p["new"], 1)
            src = os.path.join(work, "_patched_" + os.path.basename(fx))
            write_cp932(src, text)
        if os.path.exists(sdk_bin) and not mine:
            shutil.copy(sdk_bin, out)
        else:
            compile_shader(src, out, profiles["vs" if binary.endswith(".vso") else "ps"])
    main = os.path.join(orig, conf["original_main"])
    remove = conf.get("original_remove_lines", [])
    if remove:
        # 原本のままではビルドできない行(SDK に無いヘッダーの include など)だけを除いた写しでビルドする。原本は書き換えない
        text = read_text(main).replace("\r\n", "\n")
        for line in remove:
            if line + "\n" not in text:
                raise RuntimeError(f"original_remove_lines の行が原本に無い: {line}")
            text = text.replace(line + "\n", "", 1)
        main = os.path.join(work, "_original_main.cpp")
        with open(main, "w", encoding="utf-8-sig", newline="\r\n") as f:
            f.write(text)
    return main


def compare(orig_png, port_png, out_png):
    a = np.asarray(Image.open(orig_png).convert("RGB")).astype(np.int16)
    b = np.asarray(Image.open(port_png).convert("RGB")).astype(np.int16)
    if a.shape != b.shape:
        raise RuntimeError(f"画面の大きさが違う {a.shape} {b.shape}")
    d = np.abs(a - b).max(axis=2)
    raw_ratio = float((d > DIFF_THRESHOLD).mean())
    # Direct3D 9 と 11 はピクセルの中心の位置が半ピクセル違うので、模様の境目が 1 画素ずれたり、
    # テクスチャを拡大して補間している所では境目の中間の色が変わったりする。
    # 相手の画像の周り 3x3 の画素の色の範囲(チャンネルごとの最小〜最大)に収まっていれば一致とみなす(両方向で確かめる)。
    # 平らな面の色やライティングの違いは、周りの画素も同じように違うので、これでも見つかる。
    ratio = max(float((out_of_range(a, b) > DIFF_THRESHOLD).mean()), float((out_of_range(b, a) > DIFF_THRESHOLD).mean()))
    mean = float(np.abs(a - b).mean())
    # 背景だけ(真っ黒)の絵どうしを「一致」としないよう、描かれた画素の割合も見る
    drawn = float((a.max(axis=2) > 16).mean())
    diff_img = np.clip(np.abs(a - b) * 4, 0, 255).astype(np.uint8)
    h, w = a.shape[:2]
    canvas = Image.new("RGB", (w * 3 + 16, h), (64, 64, 64))
    canvas.paste(Image.open(orig_png).convert("RGB"), (0, 0))
    canvas.paste(Image.open(port_png).convert("RGB"), (w + 8, 0))
    canvas.paste(Image.fromarray(diff_img), (w * 2 + 16, 0))
    canvas.save(out_png)
    return ratio, raw_ratio, mean, drawn


def out_of_range(a, b):
    """a の各画素が、b の同じ位置の周り 3x3 の画素の色の範囲(チャンネルごとの最小〜最大)から、どれだけはみ出しているか"""
    h, w = a.shape[:2]
    pad = np.pad(b, ((1, 1), (1, 1), (0, 0)), mode="edge")
    shifted = np.stack([pad[dy:dy + h, dx:dx + w] for dy in range(3) for dx in range(3)])
    lo, hi = shifted.min(axis=0), shifted.max(axis=0)
    return np.maximum(np.maximum(lo - a, a - hi), 0).max(axis=2)


def run_sample(name, compare_only=False):
    conf = json.load(open(os.path.join(HERE, name, "sample.json"), encoding="utf-8"))
    work = os.path.join(HERE, "build", name)
    frame = conf.get("frame", 30)
    if compare_only:
        # 前回の撮影結果で比べ直すだけ
        n = "-"
        port_png = os.path.join(work, "port", "capture.png")
        orig_png = os.path.join(work, "orig", "capture.png")
    else:
        n = prepare_port(name, conf, os.path.join(work, "port"))
        port_png = build_and_run(os.path.join(work, "port"), os.path.join(work, "port", "src", "main.cpp"), "DX_DIRECT3D_11", frame)
        main = prepare_original(name, conf, os.path.join(work, "orig"))
        orig_png = build_and_run(os.path.join(work, "orig"), main, "DX_DIRECT3D_9EX", frame)
    ratio, raw_ratio, mean, drawn = compare(orig_png, port_png, os.path.join(work, "compare.png"))
    limit = conf.get("max_diff_ratio", 0.002)
    ok = ratio <= limit and drawn > 0.01
    return ok, (f"shaders={n} frame={frame} 違う画素={ratio * 100:.3f}%(基準 {limit * 100:.1f}%。1 画素のずれを許さないと {raw_ratio * 100:.3f}%) "
                f"平均差={mean:.2f} 描かれた画素={drawn * 100:.1f}%  {os.path.join(work, 'compare.png')}")


def main():
    args = sys.argv[1:]
    compare_only = "--compare-only" in args
    args = [a for a in args if a != "--compare-only"]
    names = args or sorted(d for d in os.listdir(HERE) if os.path.exists(os.path.join(HERE, d, "sample.json")))
    ng = 0
    for name in names:
        try:
            ok, detail = run_sample(name, compare_only)
        except Exception as e:  # noqa: BLE001  1 本の失敗で全体を止めない
            ok, detail = False, str(e)
        ng += not ok
        print(f"[samples] {'OK' if ok else 'NG'} {name} {detail}", flush=True)
    print(f"[samples] OK {len(names) - ng} / NG {ng}")
    sys.exit(1 if ng else 0)


if __name__ == "__main__":
    main()
