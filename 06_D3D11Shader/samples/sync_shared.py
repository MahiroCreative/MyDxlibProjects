r"""_shared の .hlsli を、各サンプルの shaders/ に複写する(サンプルのフォルダだけで拡張機能から開けるように)。
あわせて、サンプルのソース(.cpp / .h / .hlsl / .hlsli)を BOM 付き UTF-8・CRLF にそろえる(拡張機能の既定と同じ)。

使い方: python samples/sync_shared.py
"""
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SHARED = os.path.join(HERE, "_shared")
SOURCE_EXTS = (".cpp", ".h", ".hlsl", ".hlsli")


def normalize(path, text=None):
    if text is None:
        text = open(path, encoding="utf-8-sig").read()
    text = text.replace("\r\n", "\n")
    with open(path, "w", encoding="utf-8-sig", newline="\r\n") as f:
        f.write(text)


def main():
    shared = {f: open(os.path.join(SHARED, f), encoding="utf-8-sig").read() for f in os.listdir(SHARED) if f.endswith(".hlsli")}
    for f, text in shared.items():
        normalize(os.path.join(SHARED, f), text)
    for name in sorted(os.listdir(HERE)):
        sample = os.path.join(HERE, name)
        if not os.path.exists(os.path.join(sample, "sample.json")):
            continue
        shaders = os.path.join(sample, "shaders")
        os.makedirs(shaders, exist_ok=True)
        for f, text in shared.items():
            normalize(os.path.join(shaders, f), text)
        for base, _, files in os.walk(sample):
            for f in files:
                if f.endswith(SOURCE_EXTS):
                    normalize(os.path.join(base, f))
        print(f"[sync] {name}")


if __name__ == "__main__":
    main()
