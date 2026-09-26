r"""課題の解答例(course/answers/)の確認。解答例と土台のサンプルを同じフレームで撮影して比べる。
  - 解答例がビルドでき、動いて絵が出る
  - 土台のサンプルと絵が変わっている(課題の効果が出ている)
比べた画像は course/build/<名前>/compare.png(土台 | 解答例 | 差 × 4)。効果が正しいかは、この画像を目で見て確かめる。

使い方: python course/check_answers.py [解答例の名前 ...]
"""
import json
import os
import shutil
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(HERE), "samples"))
import run  # noqa: E402  samples/run.py の関数を使う

ANSWERS = os.path.join(HERE, "answers")


def prepare(src, work):
    shutil.rmtree(work, ignore_errors=True)
    shutil.copytree(src, work, ignore=shutil.ignore_patterns("sample.json", "answer.json", "README.md"))
    shaders = os.path.join(work, "shaders")
    bin_dir = os.path.join(shaders, "bin")
    os.makedirs(bin_dir, exist_ok=True)
    for f in sorted(os.listdir(shaders)):
        stem, ext = os.path.splitext(f)
        if ext.lower() not in (".hlsl", ".fx"):
            continue
        profile = "vs_4_0" if stem.upper().endswith("VS") else "ps_4_0" if stem.upper().endswith("PS") else None
        if profile:
            run.compile_shader(os.path.join(shaders, f), os.path.join(bin_dir, stem + (".vso" if profile.startswith("vs") else ".pso")), profile, shaders)


def check(name):
    conf = json.load(open(os.path.join(ANSWERS, name, "answer.json"), encoding="utf-8"))
    work = os.path.join(HERE, "build", name)
    frame = conf.get("frame", 30)
    base_dir, ans_dir = os.path.join(work, "base"), os.path.join(work, "answer")
    prepare(os.path.join(run.HERE, conf["base"]), base_dir)
    prepare(os.path.join(ANSWERS, name), ans_dir)
    base_png = run.build_and_run(base_dir, os.path.join(base_dir, "src", "main.cpp"), "DX_DIRECT3D_11", frame)
    ans_png = run.build_and_run(ans_dir, os.path.join(ans_dir, "src", "main.cpp"), "DX_DIRECT3D_11", frame)
    ratio, raw, mean, _ = run.compare(base_png, ans_png, os.path.join(work, "compare.png"))
    a = np.asarray(Image.open(ans_png).convert("RGB")).astype(np.int16)
    drawn = float((a.max(axis=2) > 16).mean())
    ok = drawn > 0.01 and ratio > 0.001
    return ok, f"{conf['title']}  土台({conf['base']})と違う画素={ratio * 100:.2f}% 描かれた画素={drawn * 100:.1f}%  {os.path.join(work, 'compare.png')}"


def main():
    names = sys.argv[1:] or sorted(d for d in os.listdir(ANSWERS) if os.path.exists(os.path.join(ANSWERS, d, "answer.json")))
    ng = 0
    for name in names:
        try:
            ok, detail = check(name)
        except Exception as e:  # noqa: BLE001  1 本の失敗で全体を止めない
            ok, detail = False, str(e)
        ng += not ok
        print(f"[answers] {'OK' if ok else 'NG'} {name} {detail}", flush=True)
    print(f"[answers] OK {len(names) - ng} / NG {ng}")
    sys.exit(1 if ng else 0)


if __name__ == "__main__":
    main()
