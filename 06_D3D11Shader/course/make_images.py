"""course の解説(*.html)で使う画像を images/ に作る。

使い方: python course/make_images.py
  - サンプルの画面: samples/run.py と同じ方法(verify_hook.h を差し込む)でビルド・実行して、決まったフレームを撮る。
  - 課題の解答例の画面: course/check_answers.py が撮ったもの(course/build/<名前>/base|answer/capture.png)を使う。
    先に python course/check_answers.py を動かしておく。
  - 「イメージ」の画像(01・03 の課題の完成イメージ): シェーダーと同じ計算を Python でした絵。

図(矢印や座標の説明)は画像ではなく、各 HTML の中に SVG で書いてある。
"""
import os
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, os.path.join(ROOT, "samples"))
import run  # noqa: E402  samples/run.py(ビルドと撮影)

OUT = os.path.join(HERE, "images")
WORK = os.path.join(HERE, "build", "images")
SDK_DATA = run.SDK_SAMPLE_DATA

# 08 回の見比べ用。NormalBox.mqo のマテリアルはてかりの色が 0 なので、そのままでは A05 と A11 の絵が同じになる
SPECULAR = ('\tModelHandle = MV1LoadModel( "NormalBox.mqo" ) ;',
            '\tModelHandle = MV1LoadModel( "NormalBox.mqo" ) ;\n'
            '\tMV1SetMaterialSpcColor( ModelHandle, 0, GetColorF( 1.0f, 1.0f, 1.0f, 1.0f ) ) ;\n'
            '\tMV1SetMaterialSpcPower( ModelHandle, 0, 60.0f ) ;')

CUBE = (120, 40, 520, 440)   # 箱のモデルの所
CHARA = (200, 20, 440, 460)  # キャラクターの所
PLATE = (0, 0, 128, 128)     # 2D の四角(左上に 128×128 で描かれる)
ROTATE = (80, 0, 560, 480)   # C3 の回る板(課題 5-2 で大きくなっても入る広さ)
FLAG = (330, 95, 610, 395)   # D1 の右の顔(なびく方。上の文字は入れない)


def capture(name, frame, tag="", patch=None):
    """サンプルをビルドして frame 回目の ScreenFlip の直前を撮る。撮った画像(PIL)を返す"""
    work = os.path.join(WORK, f"{name}_{frame}{tag}")
    run.prepare_port(name, {}, work)
    main = os.path.join(work, "src", "main.cpp")
    if patch:
        text = run.read_text(main)
        assert patch[0] in text, (name, patch[0])
        with open(main, "w", encoding="utf-8-sig", newline="\r\n") as f:
            f.write(text.replace(patch[0], patch[1]).replace("\r\n", "\n"))
    png = run.build_and_run(work, main, "DX_DIRECT3D_11", frame)
    print("撮影", name, frame, tag)
    return Image.open(png).convert("RGB")


def save(im, name, box=None, scale=None, width=None):
    if box:
        im = im.crop(box)
    if scale:
        im = im.resize((im.width * scale, im.height * scale), Image.NEAREST)
    if width:
        im = im.resize((width, round(im.height * width / im.width)), Image.LANCZOS)
    path = os.path.join(OUT, name)
    if name.endswith(".jpg"):
        im.save(path, quality=88)
    else:
        im.save(path, optimize=True)
    print("保存", name, im.size)


def answer(name, which):
    p = os.path.join(HERE, "build", name, which, "capture.png")
    if not os.path.exists(p):
        raise RuntimeError(f"{p} が無い。先に python course/check_answers.py を動かす")
    return Image.open(p).convert("RGB")


def simulations():
    """01・03 の課題の完成イメージ。シェーダーと同じ計算を numpy でする(色は 0〜1)"""
    photo = np.asarray(Image.open(os.path.join(SDK_DATA, "TestTex1.jpg")).convert("RGB"), dtype=np.float32) / 255.0
    h, w, _ = photo.shape

    def out(a, name):
        save(Image.fromarray((np.clip(a, 0, 1) * 255 + 0.5).astype(np.uint8)), name)

    gray = photo @ np.array([0.299, 0.587, 0.114], dtype=np.float32)
    out(photo, "photo.jpg")
    out(np.repeat(gray[..., None], 3, 2), "sim_gray.jpg")
    out(1.0 - photo, "sim_nega.jpg")
    out(gray[..., None] * np.array([1.0, 0.8, 0.6], dtype=np.float32), "sim_sepia.jpg")
    half = photo.copy()
    half[:, : w // 2] = gray[:, : w // 2, None]
    out(half, "sim_halfgray.jpg")

    # テクスチャ座標をずらして読む(クランプ・最も近い画素)
    v, u = np.meshgrid((np.arange(h) + 0.5) / h, (np.arange(w) + 0.5) / w, indexing="ij")

    def sample(uu, vv):
        x = np.clip((uu * w).astype(int), 0, w - 1)
        y = np.clip((vv * h).astype(int), 0, h - 1)
        return photo[y, x]

    out(sample(u + 0.1, v), "sim_shift.jpg")
    out(sample(u + np.sin(v * 40.0 + 1.0) * 0.01, v), "sim_haze.jpg")
    out(sample(u + np.sin(v * 30.0 + 1.0) * 0.015, v + np.sin(u * 20.0 + 2.0) * 0.015), "sim_water.jpg")
    out(sample(np.floor(u * 32.0) / 32.0, np.floor(v * 32.0) / 32.0), "sim_mosaic.jpg")


def interpolated_triangle():
    """04 回の図。頂点の色(赤・緑・青)が、三角形の中でなめらかに混ぜられる様子"""
    w, h = 300, 260
    a, b, c = np.array([150.0, 12.0]), np.array([12.0, 248.0]), np.array([288.0, 248.0])
    y, x = np.mgrid[0:h, 0:w].astype(np.float32) + 0.5
    det = (b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1])
    la = ((b[1] - c[1]) * (x - c[0]) + (c[0] - b[0]) * (y - c[1])) / det
    lb = ((c[1] - a[1]) * (x - c[0]) + (a[0] - c[0]) * (y - c[1])) / det
    lc = 1.0 - la - lb
    rgb = np.stack([la, lb, lc], -1)
    inside = (la >= 0) & (lb >= 0) & (lc >= 0)
    img = np.zeros((h, w, 4), np.uint8)
    img[..., :3] = (np.clip(rgb, 0, 1) * 255).astype(np.uint8)
    img[..., 3] = inside * 255
    save(Image.fromarray(img, "RGBA"), "tri_interp.png")


def main():
    os.makedirs(OUT, exist_ok=True)
    interpolated_triangle()
    if "--diagram-only" in sys.argv:
        return

    # 01
    save(Image.open(os.path.join(SDK_DATA, "Tex1.bmp")).convert("RGB"), "tex1.png", scale=4)
    save(capture("C2_PixelShaderTest", 30), "c2_result.png", PLATE, scale=2)
    save(answer("01_grayscale", "answer"), "ans01.png", PLATE, scale=2)
    simulations()

    # 02・03
    for fr in (20, 50, 80, 110):
        save(capture("C5_SetPSConstFTest", fr), f"c5_f{fr}.png", PLATE, scale=2)
    save(answer("02_blink", "answer"), "ans02.png", PLATE, scale=2)
    save(answer("03_heat_haze", "answer"), "ans03.png", PLATE, scale=2)

    # 04・05・06
    for fr in (20, 50, 80, 110):
        save(capture("C1_VertexShaderTest", fr), f"c1_f{fr}.png", (0, 100, 640, 380), width=400)
        save(capture("C3_SetVSConstFMtxTest", fr), f"c3_f{fr}.png", ROTATE, width=240)
        save(capture("D1_Sprite2DVertexShader", fr), f"d1_f{fr}.png", FLAG, width=210)
    save(capture("D1_Sprite2DVertexShader", 30), "d1.png")
    save(answer("04_move_xy", "answer"), "ans04.png", width=400)
    save(answer("05_scale_rotate", "answer"), "ans05.png", ROTATE, width=240)
    save(answer("06_slime", "answer"), "ans06.png")

    # 07・08
    save(capture("A01_NormalMesh_NoLight", 30), "a01.png", CUBE, width=300)
    save(capture("A05_NormalMesh_DirLight", 30), "a05.png", CUBE, width=300)
    save(capture("A05_NormalMesh_DirLight", 20, "_spec", SPECULAR), "a05_spec.png", CUBE, width=300)
    save(capture("A11_NormalMesh_DirLight_Phong", 20, "_spec", SPECULAR), "a11_spec.png", CUBE, width=300)
    save(answer("07_half_lambert", "base"), "ans07_base.png", CUBE, width=300)
    save(answer("07_half_lambert", "answer"), "ans07.png", CUBE, width=300)
    save(answer("08_rim_light", "base"), "ans08_base.png", CUBE, width=300)
    save(answer("08_rim_light", "answer"), "ans08.png", CUBE, width=300)

    # 09
    save(capture("A15_SkinMesh4_DirLight_Toon", 30), "a15.png", CHARA)
    save(Image.open(os.path.join(ROOT, "samples", "A15_SkinMesh4_DirLight_Toon", "GradTex.bmp")).convert("RGB"), "gradtex.png", scale=2)
    save(answer("09_step_toon", "answer"), "ans09.png", CHARA)

    # 10
    save(capture("B6_Mirror", 30), "b6.jpg")
    save(capture("B1_3DAction_DepthShadow", 30), "b1.jpg")
    save(answer("10_shadow_strength", "base"), "ans10_base.jpg", width=400)
    save(answer("10_shadow_strength", "answer"), "ans10.jpg", width=400)


if __name__ == "__main__":
    main()
