r"""課題の解答例(course/answers/)を、土台のサンプル(samples/)の写しに決まった書き換えを当てて作る。
書き換えた所には【課題 n-m】のコメントを付ける。書き換え元が見つからなければ止まる。

使い方: python course/make_answers.py   (作り直すときも同じ。answers/ は上書きされる)
確かめ方: python course/check_answers.py
"""
import json
import os
import shutil

HERE = os.path.dirname(os.path.abspath(__file__))
SAMPLES = os.path.join(os.path.dirname(HERE), "samples")
ANSWERS = os.path.join(HERE, "answers")
SDK_DATA = r"C:\GitHub\MyDxlibProjects\00_DxLib_VC\サンプルプログラム実行用フォルダ"

# 2D の課題は、色の変化が分かりやすいように写真(256×256)を使う
PHOTO = ('texhandle = LoadGraph( "Tex1.bmp" ) ;',
         'texhandle = LoadGraph( "TestTex1.jpg" ) ;    // 【課題】色の変化が分かりやすいように写真にした(256×256)')

# C5 の「r, g, b を渡す」所を「経過時間を渡す」に変える(課題 2・3 で共通)
C5_TIME_DECL = ("    FLOAT4 f4 ;\n", "    FLOAT4 f4 ;\n    float time = 0.0f ;   // 【課題】経過時間(秒)\n")
C5_TIME_SET = ("        f4.x = r / 255.0f ;\n        f4.y = g / 255.0f ;\n        f4.z = b / 255.0f ;\n        f4.w = 1.0f ;\n",
               "        // 【課題】r, g, b の代わりに、経過時間(秒)を x に入れて渡す(1 フレームを 1/60 秒として数える)\n"
               "        time += 1.0f / 60.0f ;\n        f4.x = time ;\n        f4.y = 0.0f ;\n        f4.z = 0.0f ;\n        f4.w = 0.0f ;\n")
C5_CB = ("    float4 cfMultiplyColor ;    // テクスチャの色に乗算する値",
         "    float4 cfTime ;             // 【課題】x: 経過時間(秒)。C++ の f4.x")

ANSWER_LIST = [
    {
        "name": "01_grayscale", "base": "C2_PixelShaderTest", "lesson": "01", "title": "課題 1-1: 白黒にする",
        "assets": ["TestTex1.jpg"],
        "edits": {
            "src/main.cpp": [PHOTO],
            "shaders/PixelShaderTestPS.hlsl": [(
                "    // 出力する色は青成分と赤成分を逆転したもの\n"
                "    PSOutput.Output.r = lTextureColor.b ;\n    PSOutput.Output.g = lTextureColor.g ;\n    PSOutput.Output.b = lTextureColor.r ;\n",
                "    // 【課題 1-1】明るさを求めて、r・g・b に同じ値を入れる(白黒)。\n"
                "    // 重みは人の目の感じ方に合わせたもの(緑をいちばん明るく感じる)。合計は 1\n"
                "    float lBright = dot( lTextureColor.rgb, float3( 0.299f, 0.587f, 0.114f ) ) ;\n"
                "    PSOutput.Output.r = lBright ;\n    PSOutput.Output.g = lBright ;\n    PSOutput.Output.b = lBright ;\n")],
        },
    },
    {
        "name": "02_blink", "base": "C5_SetPSConstFTest", "lesson": "02", "title": "課題 2-1: 時間で点滅させる",
        "frame": 60,  # 1 秒後(明るさ 0.57 倍)で撮って、暗くなっていることを確かめる
        "assets": ["TestTex1.jpg"],
        "edits": {
            "src/main.cpp": [PHOTO, C5_TIME_DECL, C5_TIME_SET],
            "shaders/SetPSConstFTestPS.hlsl": [C5_CB, (
                "    // 出力する色はテクスチャの色と定数を乗算したもの\n    PSOutput.Output = lTextureColor * cfMultiplyColor ;\n",
                "    // 【課題 2-1】経過時間で明るさを 0〜1 の間で波のように変える。sin は -1〜1 なので、0.5 倍して 0.5 足す\n"
                "    float lBright = sin( cfTime.x * 3.0f ) * 0.5f + 0.5f ;\n"
                "    PSOutput.Output = float4( lTextureColor.rgb * lBright, lTextureColor.a ) ;\n")],
        },
    },
    {
        "name": "03_heat_haze", "base": "C5_SetPSConstFTest", "lesson": "03", "title": "課題 3-1: 陽炎のように揺らす",
        "assets": ["TestTex1.jpg"],
        "edits": {
            "src/main.cpp": [PHOTO, C5_TIME_DECL, C5_TIME_SET],
            "shaders/SetPSConstFTestPS.hlsl": [C5_CB, (
                "    lTextureColor  = DiffuseMapTexture.Sample( DiffuseMapSampler, PSInput.TextureCoord0 ) ;\n",
                "    // 【課題 3-1】読む場所(テクスチャ座標)の x を、y の位置と時間で決まる量だけずらす。\n"
                "    // 形は変えずに「どこの色を持ってくるか」をずらすので、陽炎のように揺れて見える\n"
                "    float2 lUV = PSInput.TextureCoord0 ;\n"
                "    lUV.x += sin( lUV.y * 40.0f + cfTime.x * 5.0f ) * 0.01f ;\n"
                "    lTextureColor  = DiffuseMapTexture.Sample( DiffuseMapSampler, lUV ) ;\n"), (
                "    // 出力する色はテクスチャの色と定数を乗算したもの\n    PSOutput.Output = lTextureColor * cfMultiplyColor ;\n",
                "    // 【課題 3-1】色はそのまま出す\n    PSOutput.Output = lTextureColor ;\n")],
        },
    },
    {
        "name": "04_move_xy", "base": "C1_VertexShaderTest", "lesson": "04", "title": "課題 4-1: 上下にも動かす",
        "edits": {
            "src/main.cpp": [(
                "        vsparam->y = 0.0f ;\n",
                "        vsparam->y = ( float )( x / 2 ) ;    // 【課題 4-1】x の半分だけ上下にも動かす(斜めに往復する)\n")],
        },
    },
    {
        "name": "05_scale_rotate", "base": "C3_SetVSConstFMtxTest", "lesson": "05", "title": "課題 5-2: 回しながら大きさも変える",
        "edits": {
            "src/main.cpp": [(
                "        mtx = MGetRotZ( angle ) ;\n",
                "        // 【課題 5-2】拡大縮小の行列と回転の行列を掛け合わせる(先に拡大縮小、次に回転)。大きさは 0.5〜1.5 倍で変わる\n"
                "        mtx = MMult( MGetScale( VGet( 1.0f + 0.5f * sinf( angle ), 1.0f + 0.5f * sinf( angle ), 1.0f ) ), MGetRotZ( angle ) ) ;\n")],
        },
    },
    {
        "name": "06_slime", "base": "D1_Sprite2DVertexShader", "lesson": "06", "title": "課題 6-2: スライムのぷるぷる",
        "edits": {
            "shaders/Sprite2DVS.hlsl": [(
                "\t// 旗のようになびかせる: 左端(u = 0)を留めて、右へ行くほど大きく上下にゆらす\n"
                "\tfloat wave = sin(pos.x * (6.2831853f / g_WaveLength) - g_Time * g_Speed);\n"
                "\tpos.y += wave * g_Amplitude * input.TexCoords0.x;\n",
                "\t// 【課題 6-2】スライムのぷるぷる: 下の端(v = 1)を留めて、縦に伸びたら横に縮む、縦に縮んだら横に伸びる\n"
                "\tfloat wave = sin(g_Time * g_Speed);\n"
                "\tpos.y -= (1.0f - input.TexCoords0.y) * wave * g_Amplitude;			// 上へ行くほど大きく上下に\n"
                "\tpos.x -= (input.TexCoords0.x - 0.5f) * wave * g_Amplitude * 0.5f;	// 真ん中から左右に(縦と逆向き)\n")],
            "src/main.cpp": [(
                "\t// なびかせる板。縦横 24 マスに分けて頂点を増やす(4 隅だけではなめらかに曲がらない)\n",
                "\t// 【課題 6-2】この課題の動きは板の 4 隅だけでもできるが、マス目のままでもよい\n"
                "\t// なびかせる板。縦横 24 マスに分けて頂点を増やす(4 隅だけではなめらかに曲がらない)\n"),
                ("\tparam->Amplitude = 16.0f;\n", "\tparam->Amplitude = 24.0f;	// 【課題 6-2】伸び縮みの大きさ(画素)\n")],
        },
    },
    {
        "name": "07_half_lambert", "base": "A05_NormalMesh_DirLight", "lesson": "07", "title": "課題 7-2: ハーフランバート",
        "edits": {
            "shaders/NormalMesh_DirLightVS.hlsl": [(
                "\tlLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;\n",
                "\tlLightLitDest = lit( lLightLitParam.x, lLightLitParam.y, lLightLitParam.w ) ;\n\n"
                "\t// 【課題 7-2】ハーフランバート: 明るさ(-1〜1)を 0.5 倍して 0.5 足し、0〜1 にしてから 2 乗する。\n"
                "\t// ライトの反対側も真っ暗にならず、やわらかい陰影になる\n"
                "\tlLightLitDest.y = pow( lLightLitParam.x * 0.5f + 0.5f, 2.0f ) ;\n")],
        },
    },
    {
        "name": "08_rim_light", "base": "A11_NormalMesh_DirLight_Phong", "lesson": "08", "title": "課題 8-2: 輪郭を光らせる(リムライト)",
        "edits": {
            "shaders/NormalMesh_DirLight_PhongPS.hlsl": [(
                "\tPSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;\n",
                "\tPSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + SpecularColor.rgb ;\n\n"
                "\t// 【課題 8-2】リムライト: 視線と法線が直角に近い所(輪郭)ほど明るくする。\n"
                "\t// dot( 法線, 視点への向き ) は正面で 1、輪郭で 0。1 から引いて 3 乗し、輪郭の近くだけに絞る\n"
                "\tfloat lRim = pow( 1.0f - saturate( dot( Normal, V_to_Eye ) ), 3.0f ) ;\n"
                "\tPSOutput.Color0.rgb += lRim * float3( 0.6f, 0.8f, 1.0f ) ;\n")],
        },
    },
    {
        "name": "09_step_toon", "base": "A15_SkinMesh4_DirLight_Toon", "lesson": "09", "title": "課題 9-2: 画像を使わずに 2 段のトゥーン",
        "edits": {
            "shaders/SkinMesh4_DirLight_ToonPS.hlsl": [(
                "\tToonColor = ToonTexture.Sample( ToonTextureSampler, PSInput.ToonCoords0 ) ;\n",
                "\t// 【課題 9-2】グラデーションの画像を使わず、明るさ 0.5 を境にして 2 段(明るい・暗い)にする\n"
                "\tToonColor = PSInput.ToonCoords0.x > 0.5f ? float4( 1.0f, 1.0f, 1.0f, 1.0f ) : float4( 0.55f, 0.55f, 0.55f, 1.0f ) ;\n")],
        },
    },
    {
        "name": "10_shadow_strength", "base": "B1_3DAction_DepthShadow", "lesson": "10", "title": "課題 10-2: 影を濃くする",
        "edits": {
            "shaders/DirLight_DepthShadow_Step2PS.hlsl": [(
                "\t\tDefaultOutput.rgb *= 0.5f;\n",
                "\t\tDefaultOutput.rgb *= 0.2f;	// 【課題 10-2】影の所の明るさ。0.5(半分)→ 0.2 で濃くする\n")],
        },
    },
]


def main():
    shutil.rmtree(ANSWERS, ignore_errors=True)
    for a in ANSWER_LIST:
        src = os.path.join(SAMPLES, a["base"])
        dst = os.path.join(ANSWERS, a["name"])
        shutil.copytree(src, dst, ignore=shutil.ignore_patterns("sample.json", "README.md"))
        for rel, pairs in a["edits"].items():
            p = os.path.join(dst, rel)
            text = open(p, encoding="utf-8-sig").read().replace("\r\n", "\n")
            for old, new in pairs:
                if old not in text:
                    raise SystemExit(f"{a['name']}: {rel} に見つからない: {old[:60]!r}")
                text = text.replace(old, new, 1)
            with open(p, "w", encoding="utf-8-sig", newline="\r\n") as f:
                f.write(text)
        for asset in a.get("assets", []):
            shutil.copy(os.path.join(SDK_DATA, asset), dst)
        base_conf = json.load(open(os.path.join(src, "sample.json"), encoding="utf-8"))
        conf = {
            "title": a["title"],
            "lesson": a["lesson"],
            "base": a["base"],
            "assets": base_conf.get("assets", []) + a.get("assets", []),
            "frame": a.get("frame", base_conf.get("frame", 30)),
        }
        with open(os.path.join(dst, "answer.json"), "w", encoding="utf-8", newline="\n") as f:
            json.dump(conf, f, ensure_ascii=False, indent="\t")
            f.write("\n")
        print("ok", a["name"])


if __name__ == "__main__":
    main()
