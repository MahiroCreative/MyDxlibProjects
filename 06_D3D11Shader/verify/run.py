r"""仕様書の検証を実行する。

1. shaders/ の HLSL を CP932 に変換し、SDK の ShaderCompiler.exe でコンパイルする(定数バッファごとに作り分ける)
2. probe/main.cpp を Visual Studio の cl でビルドする
3. 実行して results.txt(判定)と dump.txt(定数バッファの中身)を作らせ、NG の数を表示する

使い方: python run.py [作業フォルダ]   (既定は verify/build)
"""
import os
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SDK = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\プロジェクトに追加すべきファイル_VC用"
COMPILER = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\Tool\ShaderCompiler\ShaderCompiler.exe"
DUMP_WIDTH = 1024
# MV1 の確認に使うモデル(SDK のサンプル用素材)
SAMPLE_DATA = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\サンプルプログラム実行用フォルダ"
SAMPLE_FILES = ["DxChara.x", "DxCharaEye.tga", "DxCharaEye2.tga", "Kao.bmp"]

# (出力名, 原本, プロファイル, 定義)
SHADERS = [
    ("DumpVS_PS", "DumpVS_PS.hlsl", "ps_4_0", {}),
    ("ClipVS", "ClipVS.hlsl", "vs_4_0", {}),
]
for slot, regs in (("b0", 54), ("b1", 16), ("b2", 18), ("b3", 162), ("b4", 4)):
    SHADERS.append((f"DumpVS_{slot}", "DumpVS.hlsl", "vs_4_0", {"DUMP_SLOT": slot, "DUMP_REGS": regs, "DUMP_WIDTH": f"{DUMP_WIDTH}.0f"}))
for slot, regs in (("b0", 54), ("b1", 5), ("b2", 6), ("b4", 4)):
    SHADERS.append((f"DumpPS_{slot}", "DumpPS.hlsl", "ps_4_0", {"DUMP_SLOT": slot, "DUMP_REGS": regs}))
# サンプル共通の宣言(samples/_shared)の並びの確認
SHADERS.append(("HlsliVS", "HlsliVS.hlsl", "vs_4_0", {"DUMP_WIDTH": f"{DUMP_WIDTH}.0f"}))
SHADERS.append(("HlsliPS", "HlsliPS.hlsl", "ps_4_0", {}))
SHARED = os.path.join(os.path.dirname(HERE), "samples", "_shared")


def vcvarsall():
    vswhere = os.path.join(os.environ["ProgramFiles(x86)"], "Microsoft Visual Studio", "Installer", "vswhere.exe")
    out = subprocess.check_output([vswhere, "-latest", "-products", "*", "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
                                   "-property", "installationPath", "-utf8"], encoding="utf-8").strip()
    return os.path.join(out, "VC", "Auxiliary", "Build", "vcvarsall.bat")


def compile_shaders(work):
    src_dir = os.path.join(work, "shader_src")
    bin_dir = os.path.join(work, "shaders")
    os.makedirs(src_dir, exist_ok=True)
    os.makedirs(bin_dir, exist_ok=True)
    # include される .hlsli も CP932 で同じ場所に置く
    for f in os.listdir(SHARED):
        if f.endswith(".hlsli"):
            text = open(os.path.join(SHARED, f), encoding="utf-8-sig").read()
            with open(os.path.join(src_dir, f), "w", encoding="cp932", newline="\r\n") as out:
                out.write(text)
    for name, src, profile, defines in SHADERS:
        text = open(os.path.join(HERE, "shaders", src), encoding="utf-8-sig").read()
        cp932 = os.path.join(src_dir, src)
        with open(cp932, "w", encoding="cp932", newline="\r\n") as f:
            f.write(text)
        ext = ".vso" if profile.startswith("vs") else ".pso"
        out = os.path.join(bin_dir, name + ext)
        args = [COMPILER, f"/T{profile}", f"/Fo{out}"] + [f"/D{k}={v}" for k, v in defines.items()] + [cp932]
        r = subprocess.run(args, capture_output=True)
        if r.returncode != 0 or not os.path.exists(out):
            print(r.stdout.decode("cp932", "replace"), r.stderr.decode("cp932", "replace"))
            raise SystemExit(f"シェーダーのコンパイルに失敗: {name}")
    print(f"[verify] シェーダー {len(SHADERS)} 本をコンパイルした")


def build_probe(work):
    bat = os.path.join(work, "build.bat")
    inc = os.path.join(HERE, "include")
    src = os.path.join(HERE, "probe", "main.cpp")
    lines = [
        "@echo off",
        f'call "{vcvarsall()}" x64 >nul 2>&1',
        f'cd /d "{work}"',
        f'cl /nologo /EHsc /W3 /std:c++20 /source-charset:.932 /execution-charset:.932 /D_WINDOWS /DWIN32 /O2 /MT /DNDEBUG '
        f'/I "{SDK}" /I "{inc}" "{src}" /Foprobe.obj /Feprobe.exe /link /SUBSYSTEM:WINDOWS /LIBPATH:"{SDK}" > build.log 2>&1',
    ]
    # cmd は bat を ANSI コードページで読むので、日本語のパスを含む bat は CP932 で書く
    with open(bat, "w", encoding="cp932", newline="\r\n") as f:
        f.write("\n".join(lines) + "\n")
    subprocess.run(["cmd.exe", "/d", "/c", bat])
    exe = os.path.join(work, "probe.exe")
    if not os.path.exists(exe):
        print(open(os.path.join(work, "build.log"), encoding="cp932", errors="replace").read())
        raise SystemExit("検証プログラムのビルドに失敗")
    print("[verify] 検証プログラムをビルドした")
    return exe


def main():
    work = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.join(HERE, "build")
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(work)
    compile_shaders(work)
    for f in SAMPLE_FILES:
        shutil.copy(os.path.join(SAMPLE_DATA, f), work)
    exe = build_probe(work)
    subprocess.run([exe], cwd=work, timeout=120)
    results = open(os.path.join(work, "results.txt"), encoding="utf-8").read()
    lines = [l for l in results.splitlines() if l.startswith(("OK ", "NG ")) and not l.startswith("NG count=")]
    for l in results.splitlines():
        print("[verify]", l)
    ng = sum(1 for l in lines if l.startswith("NG "))
    print(f"[verify] OK {len(lines) - ng} / NG {ng}  (中身の一覧: {os.path.join(work, 'dump.txt')})")
    sys.exit(1 if ng or not lines else 0)


if __name__ == "__main__":
    main()
