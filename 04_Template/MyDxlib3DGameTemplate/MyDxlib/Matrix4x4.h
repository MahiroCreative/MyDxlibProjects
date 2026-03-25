#pragma once
#pragma once
#include "DxLib.h"
#include <string>

struct Matrix4x4
{
    // DxLibに合わせて [行][列] の順（m[row][column]）
    float m[4][4] = { 0 };

	/* staticメンバ変数宣言 */
    static const Matrix4x4 Identity;// 単位行列

    /*メンバ関数 */
    // DxLibの MATRIX 型に変換
    MATRIX ToDxMatrix() const {
        MATRIX res;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                res.m[i][j] = m[i][j];
            }
        }
        return res;
    }

    /* 文字列変換 */
    std::string ToString() const {
        std::string str = "";
        for (int i = 0; i < 4; ++i) {
            str += std::to_string(m[i][0]) + ", " + std::to_string(m[i][1]) + ", "
                + std::to_string(m[i][2]) + ", " + std::to_string(m[i][3]) + "\n";
        }
        return str;
    }
};

/* staticメンバ変数の実装 */
inline const Matrix4x4 Matrix4x4::Identity = { {
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 }
} };