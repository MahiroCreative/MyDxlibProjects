"""samples/index.html の一覧に載せる、各サンプルの実行画面の縮小画像を samples/images/ に作る。

使い方: python samples/make_images.py
  samples/run.py が撮った画像(samples/build/<サンプル名>/port/capture.png)を縮めるだけ。先に python samples/run.py を動かしておく。
  比べ方の説明用に、1 本だけ「原本 | 移植版 | 差 × 4」の画像(compare.png)も縮めて置く。
"""
import os

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(HERE, "build")
OUT = os.path.join(HERE, "images")
COMPARE_EXAMPLE = "A15_SkinMesh4_DirLight_Toon"


def main():
    os.makedirs(OUT, exist_ok=True)
    names = sorted(d for d in os.listdir(HERE) if os.path.exists(os.path.join(HERE, d, "sample.json")))
    for name in names:
        p = os.path.join(BUILD, name, "port", "capture.png")
        if not os.path.exists(p):
            raise RuntimeError(f"{p} が無い。先に python samples/run.py を動かす")
        im = Image.open(p).convert("RGB").resize((200, 150), Image.LANCZOS)
        im.save(os.path.join(OUT, f"{name}.jpg"), quality=85)
    print("縮小画像", len(names), "本")
    im = Image.open(os.path.join(BUILD, COMPARE_EXAMPLE, "compare.png")).convert("RGB")
    im = im.resize((im.width * 3 // 5, im.height * 3 // 5), Image.LANCZOS)
    im.save(os.path.join(OUT, "compare_example.jpg"), quality=88)
    print("比べた画像の例", im.size)


if __name__ == "__main__":
    main()
