"""仕様書(dxlib_d3d11_shader_spec.html)の 3.4 で使う画像を spec/images/ に作る。

使い方: python spec/make_images.py
  verify/run.py が撮った画像(verify/build/ortho2d_<番号>_ref.png / _test.png)を切り抜くだけ。先に python verify/run.py を動かしておく。
  ref = DrawGraph / DrawExtendGraph で描いたもの、test = 正射影カメラ + 3D の板 + 自作シェーダーで描いたもの。
図(矢印や表の図解)は画像ではなく、HTML の中に SVG で書いてある。
"""
import os

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
BUILD = os.path.join(os.path.dirname(HERE), "verify", "build")
OUT = os.path.join(HERE, "images")

FACE = (100, 50, 356, 306)     # 4 倍に拡大した顔の絵の所(256×256)
WAVE = (90, 30, 366, 330)      # 波で動かした絵(上下にはみ出す分も入れる)
EDGE = (206, 124, 246, 164)    # 補間ありで透過色の縁が違う所(目のまわり。40×40)


def load(n, kind):
    p = os.path.join(BUILD, f"ortho2d_{n}_{kind}.png")
    if not os.path.exists(p):
        raise RuntimeError(f"{p} が無い。先に python verify/run.py を動かす")
    return Image.open(p).convert("RGB")


def save(im, name):
    im.save(os.path.join(OUT, name), optimize=True)
    print("保存", name, im.size)


def main():
    os.makedirs(OUT, exist_ok=True)
    # 2: 4 倍・補間なし(完全に一致)
    save(load(2, "ref").crop(FACE), "ortho_nearest_ref.png")
    save(load(2, "test").crop(FACE), "ortho_nearest_test.png")
    # 4: 4 倍・補間あり・透過色あり(縁が違う。既知の違い)
    save(load(4, "ref").crop(EDGE).resize((200, 200), Image.NEAREST), "ortho_edge_ref.png")
    save(load(4, "test").crop(EDGE).resize((200, 200), Image.NEAREST), "ortho_edge_test.png")
    # 8: 頂点シェーダーで波打たせた絵
    save(load(8, "test").crop(WAVE), "ortho_wave.png")


if __name__ == "__main__":
    main()
