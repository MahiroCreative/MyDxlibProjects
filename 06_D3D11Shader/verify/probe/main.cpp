// 仕様書の検証プログラム。
// DxLib が自作シェーダー用にセットする定数バッファの中身を、検証用シェーダー(verify/shaders)で
// 浮動小数点の画面に書き出して読み戻し、DxLib の関数で取れる値と比べる。
// 結果: results.txt(OK/NG の判定)、dump.txt(場面ごとの定数バッファの中身をフィールド名つきで)
#include "DxLib.h"
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// DxLib のソースパッケージの定数バッファ定義(C++ の構造体としても使える)
#include "DxShader_DataType_D3D11.h"
#include "DxShader_PS_D3D11.h"
#include "DxShader_VS_D3D11.h"

static_assert(sizeof(DX_D3D11_CONST_BUFFER_COMMON) == 54 * 16, "b0 common");
static_assert(sizeof(DX_D3D11_VS_CONST_BUFFER_BASE) == 16 * 16, "vs b1");
static_assert(sizeof(DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX) == 18 * 16, "vs b2");
static_assert(sizeof(DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX) == 162 * 16, "vs b3");
static_assert(sizeof(DX_D3D11_PS_CONST_BUFFER_BASE) == 5 * 16, "ps b1");
static_assert(sizeof(DX_D3D11_PS_CONST_BUFFER_SHADOWMAP) == 6 * 16, "ps b2");

static const int DUMP_WIDTH = 1024;

static FILE* g_res = nullptr;
static FILE* g_dump = nullptr;
static int g_ng = 0;
static int g_rt = -1;
static int g_vsDump[5];
static int g_vsDumpPS;
static int g_psDump[5];
static int g_clipVS;
static int g_hlsliVS;
static int g_hlsliPS;

// ---------------------------------------------------------------------------
// 判定

static void Result(bool ok, const char* name, const std::string& detail)
{
	if (!ok)
	{
		g_ng++;
	}
	fprintf(g_res, "%s %s %s\n", ok ? "OK" : "NG", name, detail.c_str());
}

static bool Near(float a, float b, float eps = 1e-4f)
{
	return std::fabs(a - b) <= eps * (1.0f + std::fabs(b));
}

static std::string F4(const float* v)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "(%g, %g, %g, %g)", v[0], v[1], v[2], v[3]);
	return buf;
}

// ---------------------------------------------------------------------------
// 定数バッファの読み出し

// 描画先を検証用の画面にする。SetDrawScreen はカメラの設定を初期化するので、場面の設定より先に呼ぶ
static void UseDumpTarget()
{
	SetDrawScreen(g_rt);
	SetBackgroundColor(0, 0, 0);
}

// 描画先の x = 0..count-1 の画素から 32 ビットを復元する。描かれなかった画素(B が印の 0.5 でない)があれば false
static bool ReadBack(uint32_t* out, int count)
{
	for (int i = 0; i < count; i++)
	{
		const COLOR_F c = GetPixelF(i, 0);
		if (c.b != 0.5f)
		{
			return false;
		}
		out[i] = (uint32_t)c.r | ((uint32_t)c.g << 16);
	}
	return true;
}

// 頂点シェーダーの定数バッファ(b0〜b3)を読む。DrawPrimitive3DToShader で点を 1 個ずつ描く
static bool DumpVSWith(int vs, void* out, int bytes)
{
	const int count = bytes / 4;
	std::vector<VERTEX3DSHADER> v(count);
	memset(v.data(), 0, sizeof(VERTEX3DSHADER) * count);
	for (int i = 0; i < count; i++)
	{
		v[i].pos = VGet((float)i, 0.0f, 0.0f);
	}
	ClearDrawScreen();
	SetUseVertexShader(vs);
	SetUsePixelShader(g_vsDumpPS);
	DrawPrimitive3DToShader(v.data(), count, DX_PRIMTYPE_POINTLIST);
	RenderVertex();
	return ReadBack((uint32_t*)out, count);
}

static bool DumpVS(int slot, void* out, int bytes)
{
	return DumpVSWith(g_vsDump[slot], out, bytes);
}

// ピクセルシェーダーの定数バッファ(b0〜b2)を読む。2D の描画(use3D=false)か 3D の描画(true)で板を描く
static bool DumpPSWith(int ps, void* out, int bytes, bool use3D)
{
	const int count = bytes / 4;
	ClearDrawScreen();
	SetUsePixelShader(ps);
	if (use3D)
	{
		VERTEX3DSHADER v[6];
		memset(v, 0, sizeof(v));
		const float px[6] = { -1, 1, -1, 1, 1, -1 };
		const float py[6] = { 1, 1, -1, 1, -1, -1 };
		for (int i = 0; i < 6; i++)
		{
			v[i].pos = VGet(px[i], py[i], 0.0f);
		}
		SetUseVertexShader(g_clipVS);
		DrawPrimitive3DToShader(v, 6, DX_PRIMTYPE_TRIANGLELIST);
	}
	else
	{
		VERTEX2DSHADER v[6];
		memset(v, 0, sizeof(v));
		const float px[6] = { 0, (float)DUMP_WIDTH, 0, (float)DUMP_WIDTH, (float)DUMP_WIDTH, 0 };
		const float py[6] = { 0, 0, 1, 0, 1, 1 };
		for (int i = 0; i < 6; i++)
		{
			v[i].pos = VGet(px[i], py[i], 0.0f);
			v[i].rhw = 1.0f;
			v[i].dif = GetColorU8(255, 255, 255, 255);
		}
		DrawPrimitive2DToShader(v, 6, DX_PRIMTYPE_TRIANGLELIST);
	}
	RenderVertex();
	return ReadBack((uint32_t*)out, count);
}

static bool DumpPS(int slot, void* out, int bytes, bool use3D)
{
	return DumpPSWith(g_psDump[slot], out, bytes, use3D);
}

// ---------------------------------------------------------------------------
// 中身の書き出し(dump.txt)

static void PF(const char* name, const float* v, int n)
{
	fprintf(g_dump, "  %-40s", name);
	for (int i = 0; i < n; i++)
	{
		fprintf(g_dump, " %12.6g", v[i]);
	}
	fprintf(g_dump, "\n");
}

static void PI(const char* name, const int* v, int n)
{
	fprintf(g_dump, "  %-40s", name);
	for (int i = 0; i < n; i++)
	{
		fprintf(g_dump, " %12d", v[i]);
	}
	fprintf(g_dump, "\n");
}

static void PrintCommon(const DX_D3D11_CONST_BUFFER_COMMON& c)
{
	char n[64];
	for (int i = 0; i < DX_D3D11_COMMON_CONST_LIGHT_NUM; i++)
	{
		const DX_D3D11_CONST_LIGHT& l = c.Light[i];
		snprintf(n, sizeof(n), "Light[%d].Type", i);
		PI(n, &l.Type, 1);
		snprintf(n, sizeof(n), "Light[%d].Position,RangePow2", i);
		PF(n, l.Position, 4);
		snprintf(n, sizeof(n), "Light[%d].Direction,FallOff", i);
		PF(n, l.Direction, 4);
		snprintf(n, sizeof(n), "Light[%d].Diffuse,SpotParam0", i);
		PF(n, l.Diffuse, 4);
		snprintf(n, sizeof(n), "Light[%d].Specular,SpotParam1", i);
		PF(n, l.Specular, 4);
		snprintf(n, sizeof(n), "Light[%d].Ambient", i);
		PF(n, l.Ambient, 4);
		snprintf(n, sizeof(n), "Light[%d].Attenuation0,1,2", i);
		PF(n, &l.Attenuation0, 3);
	}
	PF("Material.Diffuse", c.Material.Diffuse, 4);
	PF("Material.Specular", c.Material.Specular, 4);
	PF("Material.Ambient_Emissive", c.Material.Ambient_Emissive, 4);
	PF("Material.Power,TypeParam0,1,2", &c.Material.Power, 4);
	const DX_D3D11_VS_CONST_FOG* fogs[2] = { &c.Fog, &c.VerticalFog };
	const char* fogNames[2] = { "Fog", "VerticalFog" };
	for (int i = 0; i < 2; i++)
	{
		snprintf(n, sizeof(n), "%s.Mode", fogNames[i]);
		PI(n, &fogs[i]->Mode, 1);
		snprintf(n, sizeof(n), "%s.LinearAdd,LinearDiv,Density,DensityStart", fogNames[i]);
		PF(n, &fogs[i]->LinearAdd, 4);
		snprintf(n, sizeof(n), "%s.E", fogNames[i]);
		PF(n, &fogs[i]->E, 1);
		snprintf(n, sizeof(n), "%s.Color", fogNames[i]);
		PF(n, fogs[i]->Color, 4);
	}
}

static void PrintVSBase(const DX_D3D11_VS_CONST_BUFFER_BASE& b)
{
	char n[64];
	for (int i = 0; i < 4; i++)
	{
		snprintf(n, sizeof(n), "AntiViewportMatrix[%d]", i);
		PF(n, b.AntiViewportMatrix[i], 4);
	}
	for (int i = 0; i < 4; i++)
	{
		snprintf(n, sizeof(n), "ProjectionMatrix[%d]", i);
		PF(n, b.ProjectionMatrix[i], 4);
	}
	for (int i = 0; i < 3; i++)
	{
		snprintf(n, sizeof(n), "ViewMatrix[%d]", i);
		PF(n, b.ViewMatrix[i], 4);
	}
	for (int i = 0; i < 3; i++)
	{
		snprintf(n, sizeof(n), "LocalWorldMatrix[%d]", i);
		PF(n, b.LocalWorldMatrix[i], 4);
	}
	PF("ToonOutLineSize", b.ToonOutLineSize, 4);
	PF("DiffuseSource,SpecularSource,MulSpecularColor", &b.DiffuseSource, 3);
}

static void PrintVSOther(const DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX& o)
{
	char n[64];
	for (int m = 0; m < 3; m++)
	{
		for (int i = 0; i < 4; i++)
		{
			snprintf(n, sizeof(n), "ShadowMapLightViewProjectionMatrix[%d][%d]", m, i);
			PF(n, o.ShadowMapLightViewProjectionMatrix[m][i], 4);
		}
	}
	for (int m = 0; m < DX_D3D11_VS_CONST_TEXTURE_MATRIX_NUM; m++)
	{
		for (int i = 0; i < 2; i++)
		{
			snprintf(n, sizeof(n), "TextureMatrix[%d][%d]", m, i);
			PF(n, o.TextureMatrix[m][i], 4);
		}
	}
}

static void PrintVSLocalWorld(const DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX& w, int rows)
{
	char n[64];
	for (int i = 0; i < rows; i++)
	{
		snprintf(n, sizeof(n), "Matrix[%d]", i);
		PF(n, w.Matrix[i], 4);
	}
}

static void PrintPSBase(const DX_D3D11_PS_CONST_BUFFER_BASE& b)
{
	PF("FactorColor", b.FactorColor, 4);
	PF("MulAlphaColor,AlphaTestRef", &b.MulAlphaColor, 2);
	PI("AlphaTestCmpMode,NoLightAngleAtten,UseHalfLambert", &b.AlphaTestCmpMode, 3);
	PF("IgnoreTextureColor", b.IgnoreTextureColor, 4);
	PF("DrawAddColor", b.DrawAddColor, 4);
}

static void PrintPSShadow(const DX_D3D11_PS_CONST_BUFFER_SHADOWMAP& s)
{
	char n[64];
	for (int i = 0; i < 3; i++)
	{
		snprintf(n, sizeof(n), "Data[%d].AdjustDepth,Gradation,EnL0,EnL1", i);
		PF(n, &s.Data[i].AdjustDepth, 4);
		snprintf(n, sizeof(n), "Data[%d].Enable_Light2", i);
		PF(n, &s.Data[i].Enable_Light2, 1);
	}
}

// 場面ごとに全バッファを読んで dump.txt に書く
struct AllBuffers
{
	DX_D3D11_CONST_BUFFER_COMMON vsCommon;
	DX_D3D11_VS_CONST_BUFFER_BASE vsBase;
	DX_D3D11_VS_CONST_BUFFER_OTHERMATRIX vsOther;
	DX_D3D11_VS_CONST_BUFFER_LOCALWORLDMATRIX vsLocalWorld;
	DX_D3D11_CONST_BUFFER_COMMON psCommon2D, psCommon3D;
	DX_D3D11_PS_CONST_BUFFER_BASE psBase2D, psBase3D;
	DX_D3D11_PS_CONST_BUFFER_SHADOWMAP psShadow3D;
	bool ok[9];
};

static void DumpAll(const char* scene, AllBuffers& a)
{
	a.ok[0] = DumpVS(0, &a.vsCommon, sizeof(a.vsCommon));
	a.ok[1] = DumpVS(1, &a.vsBase, sizeof(a.vsBase));
	a.ok[2] = DumpVS(2, &a.vsOther, sizeof(a.vsOther));
	a.ok[3] = DumpVS(3, &a.vsLocalWorld, sizeof(a.vsLocalWorld));
	a.ok[4] = DumpPS(0, &a.psCommon3D, sizeof(a.psCommon3D), true);
	a.ok[5] = DumpPS(1, &a.psBase3D, sizeof(a.psBase3D), true);
	a.ok[6] = DumpPS(2, &a.psShadow3D, sizeof(a.psShadow3D), true);
	a.ok[7] = DumpPS(0, &a.psCommon2D, sizeof(a.psCommon2D), false);
	a.ok[8] = DumpPS(1, &a.psBase2D, sizeof(a.psBase2D), false);

	fprintf(g_dump, "==== %s\n", scene);
	fprintf(g_dump, "-- VS b0 common (DrawPrimitive3DToShader)\n");
	PrintCommon(a.vsCommon);
	fprintf(g_dump, "-- VS b1 base\n");
	PrintVSBase(a.vsBase);
	fprintf(g_dump, "-- VS b2 other matrix\n");
	PrintVSOther(a.vsOther);
	fprintf(g_dump, "-- VS b3 local world matrix (first 6 rows)\n");
	PrintVSLocalWorld(a.vsLocalWorld, 6);
	fprintf(g_dump, "-- PS b1 base (3D)\n");
	PrintPSBase(a.psBase3D);
	fprintf(g_dump, "-- PS b1 base (2D)\n");
	PrintPSBase(a.psBase2D);
	fprintf(g_dump, "-- PS b2 shadow map (3D)\n");
	PrintPSShadow(a.psShadow3D);
	fprintf(g_dump, "-- PS b0 common: same as VS b0? 3D=%d 2D=%d\n",
		memcmp(&a.psCommon3D, &a.vsCommon, sizeof(a.vsCommon)) == 0, memcmp(&a.psCommon2D, &a.vsCommon, sizeof(a.vsCommon)) == 0);
	fflush(g_dump);

	std::string d;
	for (int i = 0; i < 9; i++)
	{
		d += a.ok[i] ? "1" : "0";
	}
	Result(std::string(d) == "111111111", (std::string("readback/") + scene).c_str(), "all pixels drawn=" + d);
}

// ---------------------------------------------------------------------------
// 場面

// 行列 m(DxLib の MATRIX は行ベクトル × 行列)と、定数バッファの float4 の行 rows[] を比べる。
// transposed=true なら rows[i] = m の i 列目(転置)とみなす
static bool SameMatrix(const float (*rows)[4], int n, const MATRIX& m, bool transposed)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			const float e = transposed ? m.m[j][i] : m.m[i][j];
			if (!Near(rows[i][j], e))
			{
				return false;
			}
		}
	}
	return true;
}

static std::string MatStr(const float (*rows)[4], int n)
{
	std::string s;
	for (int i = 0; i < n; i++)
	{
		s += F4(rows[i]);
	}
	return s;
}

static std::string MatStr(const MATRIX& m)
{
	std::string s;
	for (int i = 0; i < 4; i++)
	{
		s += F4(m.m[i]);
	}
	return s;
}

static void SceneCamera()
{
	UseDumpTarget();
	SetCameraNearFar(1.0f, 1000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(10.0f, 50.0f, -100.0f), VGet(0.0f, 0.0f, 0.0f));
	const MATRIX world = MMult(MGetRotY(0.5f), MGetTranslate(VGet(1.0f, 2.0f, 3.0f)));
	SetTransformToWorld(&world);

	AllBuffers a;
	DumpAll("camera", a);

	const MATRIX view = GetCameraViewMatrix();
	const MATRIX proj = GetCameraProjectionMatrix();
	Result(SameMatrix(a.vsBase.ViewMatrix, 3, view, true), "VS.b1.ViewMatrix=transpose(GetCameraViewMatrix) rows 0-2",
		MatStr(a.vsBase.ViewMatrix, 3) + " view=" + MatStr(view));
	Result(SameMatrix(a.vsBase.ProjectionMatrix, 4, proj, true), "VS.b1.ProjectionMatrix=transpose(GetCameraProjectionMatrix)",
		MatStr(a.vsBase.ProjectionMatrix, 4) + " proj=" + MatStr(proj));
	Result(SameMatrix(a.vsBase.LocalWorldMatrix, 3, world, true), "VS.b1.LocalWorldMatrix=transpose(SetTransformToWorld) rows 0-2",
		MatStr(a.vsBase.LocalWorldMatrix, 3) + " world=" + MatStr(world));
}

// ライト: 標準ライト(ディレクショナル)+ ライトハンドル 4 個(うち 1 個は無効)
static void SceneLights()
{
	UseDumpTarget();
	SetCameraNearFar(1.0f, 1000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(10.0f, 50.0f, -100.0f), VGet(0.0f, 0.0f, 0.0f));
	SetGlobalAmbientLight(GetColorF(0.1f, 0.2f, 0.3f, 1.0f));
	SetLightDirection(VGet(0.3f, -1.0f, 0.5f));
	SetLightDifColor(GetColorF(0.9f, 0.8f, 0.7f, 1.0f));
	SetLightSpcColor(GetColorF(0.6f, 0.5f, 0.4f, 1.0f));
	SetLightAmbColor(GetColorF(0.3f, 0.2f, 0.1f, 1.0f));
	const int point = CreatePointLightHandle(VGet(10.0f, 20.0f, 30.0f), 200.0f, 0.5f, 0.01f, 0.001f);
	const int spot = CreateSpotLightHandle(VGet(-10.0f, 40.0f, 0.0f), VGet(0.0f, -1.0f, 0.2f), 1.0f, 0.5f, 300.0f, 0.2f, 0.02f, 0.0f);
	const int off = CreatePointLightHandle(VGet(0.0f, 0.0f, 0.0f), 100.0f, 1.0f, 0.0f, 0.0f);
	SetLightEnableHandle(off, FALSE);
	const int dir2 = CreateDirLightHandle(VGet(-1.0f, 0.0f, 0.0f));
	SetLightDifColorHandle(dir2, GetColorF(0.0f, 0.5f, 1.0f, 1.0f));

	MATERIALPARAM mat;
	mat.Diffuse = GetColorF(0.8f, 0.8f, 0.8f, 1.0f);
	mat.Ambient = GetColorF(0.5f, 0.4f, 0.3f, 1.0f);
	mat.Specular = GetColorF(0.8f, 0.8f, 0.8f, 0.0f);
	mat.Emissive = GetColorF(0.0f, 0.0f, 0.0f, 0.0f);
	mat.Power = 20.0f;
	SetMaterialParam(mat);

	AllBuffers a;
	DumpAll("lights", a);
	const DX_D3D11_CONST_BUFFER_COMMON& c = a.vsCommon;
	const MATRIX view = GetCameraViewMatrix();

	// 並び: 有効なライトだけを ディレクショナル → スポット → ポイント の順に詰める(無効にしたポイントライトは入らない)
	const int types[6] = { c.Light[0].Type, c.Light[1].Type, c.Light[2].Type, c.Light[3].Type, c.Light[4].Type, c.Light[5].Type };
	char buf[256];
	snprintf(buf, sizeof(buf), "types=%d,%d,%d,%d,%d,%d", types[0], types[1], types[2], types[3], types[4], types[5]);
	Result(types[0] == DX_LIGHTTYPE_DIRECTIONAL && types[1] == DX_LIGHTTYPE_DIRECTIONAL && types[2] == DX_LIGHTTYPE_SPOT && types[3] == DX_LIGHTTYPE_POINT &&
			   types[4] == 0 && types[5] == 0,
		"Light.order=enabled only, directional->spot->point", buf);

	// 標準ライト(Light[0]): 方向はビュー空間、正規化済み
	const VECTOR dir0 = VTransformSR(VNorm(VGet(0.3f, -1.0f, 0.5f)), view);
	Result(Near(c.Light[0].Direction[0], dir0.x) && Near(c.Light[0].Direction[1], dir0.y) && Near(c.Light[0].Direction[2], dir0.z),
		"Light.Direction=view space, normalized", F4(c.Light[0].Direction) + " expected=" + std::to_string(dir0.x) + "," + std::to_string(dir0.y) + "," + std::to_string(dir0.z));
	Result(Near(c.Light[0].Diffuse[0], 0.9f) && Near(c.Light[0].Diffuse[1], 0.8f) && Near(c.Light[0].Diffuse[2], 0.7f) &&
			   Near(c.Light[0].Specular[0], 0.6f) && Near(c.Light[0].Specular[1], 0.5f) && Near(c.Light[0].Specular[2], 0.4f),
		"Light.Diffuse,Specular=SetLightDifColor,SetLightSpcColor", F4(c.Light[0].Diffuse) + F4(c.Light[0].Specular));
	// アンビエント = ライトのアンビエント × マテリアルのアンビエント
	Result(Near(c.Light[0].Ambient[0], 0.3f * 0.5f) && Near(c.Light[0].Ambient[1], 0.2f * 0.4f) && Near(c.Light[0].Ambient[2], 0.1f * 0.3f),
		"Light.Ambient=light ambient*material ambient", F4(c.Light[0].Ambient));
	// ディレクショナルの既定: 有効距離 2000、減衰0 = 0.1、Phi = Theta = 2.0
	Result(Near(c.Light[0].RangePow2, 2000.0f * 2000.0f) && Near(c.Light[0].Attenuation0, 0.1f) && Near(c.Light[0].SpotParam0, cosf(1.0f)) && Near(c.Light[0].SpotParam1, 100000.0f),
		"Light.defaults(range 2000, atten0 0.1, spot0 cos(1), spot1 100000)", F4(c.Light[0].Position) + " att0=" + std::to_string(c.Light[0].Attenuation0));

	// スポットライト(Light[2]): 位置・方向はビュー空間、SpotParam0 = cos(外側の角度/2)、SpotParam1 = 1/(cos(内側/2) - cos(外側/2))
	// CreateSpotLightHandle は方向を正規化せずに保存する(CreateDirLightHandle・SetLightDirectionHandle は正規化する)
	const VECTOR sp = VTransform(VGet(-10.0f, 40.0f, 0.0f), view);
	const VECTOR sd = VTransformSR(VGet(0.0f, -1.0f, 0.2f), view);
	const DX_D3D11_CONST_LIGHT& s = c.Light[2];
	Result(Near(s.Position[0], sp.x) && Near(s.Position[1], sp.y) && Near(s.Position[2], sp.z) && Near(s.Direction[0], sd.x) && Near(s.Direction[1], sd.y) &&
			   Near(s.Direction[2], sd.z),
		"Light.Spot Position,Direction=view space (direction NOT normalized by CreateSpotLightHandle)", F4(s.Position) + F4(s.Direction));
	Result(Near(s.RangePow2, 300.0f * 300.0f) && Near(s.SpotParam0, cosf(0.5f)) && Near(s.SpotParam1, 1.0f / (cosf(0.25f) - cosf(0.5f))) &&
			   Near(s.Attenuation0, 0.2f) && Near(s.Attenuation1, 0.02f) && Near(s.Attenuation2, 0.0f) && Near(s.FallOff, 1.0f),
		"Light.Spot RangePow2,SpotParam0,SpotParam1,Attenuation,FallOff", F4(s.Diffuse) + F4(s.Specular) + " range2=" + std::to_string(s.RangePow2));

	// ポイントライト(Light[3])
	const VECTOR pp = VTransform(VGet(10.0f, 20.0f, 30.0f), view);
	const DX_D3D11_CONST_LIGHT& p = c.Light[3];
	Result(Near(p.Position[0], pp.x) && Near(p.Position[1], pp.y) && Near(p.Position[2], pp.z) && Near(p.RangePow2, 200.0f * 200.0f) &&
			   Near(p.Attenuation0, 0.5f) && Near(p.Attenuation1, 0.01f) && Near(p.Attenuation2, 0.001f),
		"Light.Point Position=view space,RangePow2,Attenuation", F4(p.Position));

	// ピクセルシェーダーの b0 は頂点シェーダーの b0 と同じ中身
	Result(memcmp(&a.psCommon3D, &a.vsCommon, sizeof(a.vsCommon)) == 0, "PS.b0=VS.b0 (same buffer)", "");

	DeleteLightHandleAll();
	SetGlobalAmbientLight(GetColorF(0.0f, 0.0f, 0.0f, 0.0f));
	SetLightDirection(VGet(1.0f, -1.0f, 1.0f));
	SetLightDifColor(GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	SetLightSpcColor(GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
	SetLightAmbColor(GetColorF(0.0f, 0.0f, 0.0f, 0.0f));
	(void)point;
	(void)spot;
}

// マテリアルとフォグ
static void SceneMaterialFog()
{
	UseDumpTarget();
	SetCameraNearFar(1.0f, 1000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(10.0f, 50.0f, -100.0f), VGet(0.0f, 0.0f, 0.0f));
	SetGlobalAmbientLight(GetColorF(0.1f, 0.2f, 0.3f, 1.0f));
	MATERIALPARAM m;
	m.Diffuse = GetColorF(0.9f, 0.8f, 0.7f, 0.6f);
	m.Ambient = GetColorF(0.5f, 0.4f, 0.3f, 1.0f);
	m.Specular = GetColorF(0.2f, 0.3f, 0.4f, 0.5f);
	m.Emissive = GetColorF(0.05f, 0.06f, 0.07f, 0.0f);
	m.Power = 12.0f;
	SetMaterialParam(m);
	SetFogEnable(TRUE);
	SetFogMode(DX_FOGMODE_LINEAR);
	SetFogColor(10, 20, 30);
	SetFogStartEnd(100.0f, 500.0f);

	AllBuffers a;
	DumpAll("material+fog(linear)", a);
	const DX_D3D11_CONST_BUFFER_COMMON& c = a.vsCommon;

	Result(Near(c.Material.Diffuse[0], 0.9f) && Near(c.Material.Diffuse[3], 0.6f) && Near(c.Material.Specular[0], 0.2f) && Near(c.Material.Specular[3], 0.5f) &&
			   Near(c.Material.Power, 12.0f),
		"Material.Diffuse,Specular,Power=SetMaterialParam", F4(c.Material.Diffuse) + F4(c.Material.Specular));
	// エミッシブ + マテリアルのアンビエント × グローバルアンビエント
	Result(Near(c.Material.Ambient_Emissive[0], 0.05f + 0.5f * 0.1f) && Near(c.Material.Ambient_Emissive[1], 0.06f + 0.4f * 0.2f) &&
			   Near(c.Material.Ambient_Emissive[2], 0.07f + 0.3f * 0.3f) && Near(c.Material.Ambient_Emissive[3], 0.0f + 1.0f * 1.0f),
		"Material.Ambient_Emissive=emissive+material ambient*global ambient", F4(c.Material.Ambient_Emissive));
	Result(c.Fog.Mode == DX_FOGMODE_LINEAR && Near(c.Fog.LinearAdd, 500.0f / 400.0f) && Near(c.Fog.LinearDiv, -1.0f / 400.0f) && Near(c.Fog.E, 2.71828183f),
		"Fog.Mode,LinearAdd=end/(end-start),LinearDiv=-1/(end-start),E", "mode=" + std::to_string(c.Fog.Mode));
	Result(Near(c.Fog.Color[0], 10 / 255.0f) && Near(c.Fog.Color[1], 20 / 255.0f) && Near(c.Fog.Color[2], 30 / 255.0f) && Near(c.Fog.Color[3], 1.0f),
		"Fog.Color=SetFogColor/255,a=1", F4(c.Fog.Color));

	// Fog.Mode は描画の履歴で変わる: 2D の描画でフォグが内部で切られた後に SetFogDensity すると NONE(0)が入る
	SetFogMode(DX_FOGMODE_EXP);
	SetFogDensity(0.25f);
	DumpAll("fog(exp) after 2D draw", a);
	Result(a.vsCommon.Fog.Mode == DX_FOGMODE_NONE && Near(a.vsCommon.Fog.Density, 0.25f), "Fog.Mode unreliable (EXP set, but NONE after a 2D draw)",
		"mode=" + std::to_string(a.vsCommon.Fog.Mode));

	SetFogEnable(FALSE);
	SetGlobalAmbientLight(GetColorF(0.0f, 0.0f, 0.0f, 0.0f));
	MATERIALPARAM d;
	d.Diffuse = GetColorF(0.8f, 0.8f, 0.8f, 1.0f);
	d.Ambient = GetColorF(0.0f, 0.0f, 0.0f, 0.0f);
	d.Specular = GetColorF(0.8f, 0.8f, 0.8f, 0.0f);
	d.Emissive = GetColorF(0.0f, 0.0f, 0.0f, 0.0f);
	d.Power = 20.0f;
	SetMaterialParam(d);

	// ライトを消した後も、使われていたスロットの Type は残る
	UseDumpTarget();
	DumpAll("after DeleteLightHandleAll", a);
	Result(a.vsCommon.Light[1].Type == DX_LIGHTTYPE_DIRECTIONAL && a.vsCommon.Light[2].Type == DX_LIGHTTYPE_SPOT && a.vsCommon.Light[3].Type == DX_LIGHTTYPE_POINT,
		"Light.Type of unused slots keeps old value (Type!=0 does not mean enabled)",
		"types=" + std::to_string(a.vsCommon.Light[1].Type) + "," + std::to_string(a.vsCommon.Light[2].Type) + "," + std::to_string(a.vsCommon.Light[3].Type));
}

// ブレンド・明るさ・アルファテスト(PS b1)
static void SceneBlend()
{
	UseDumpTarget();
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	SetDrawBright(200, 100, 50);
	SetDrawAlphaTest(DX_CMP_GREATER, 100);
	SetDrawAddColor(10, 20, 30);
	SetUseHalfLambertLighting(TRUE);
	SetUseLightAngleAttenuation(FALSE);
	AllBuffers a;
	DumpAll("blend(alpha 128)+bright(200,100,50)+alphatest(greater 100)+addcolor+halflambert", a);

	// ブレンドの値(128)や明るさは FactorColor に入らない(自作シェーダーの描画ではこの値は更新されない)
	Result(a.psBase3D.FactorColor[3] != 128 / 255.0f && a.psBase2D.FactorColor[3] != 128 / 255.0f, "PS.b1.FactorColor not updated by SetDrawBlendMode (ToShader)",
		F4(a.psBase3D.FactorColor));
	Result(a.psBase3D.AlphaTestCmpMode == DX_CMP_GREATER && Near(a.psBase3D.AlphaTestRef, 100 / 255.0f), "PS.b1.AlphaTestCmpMode,AlphaTestRef=SetDrawAlphaTest/255",
		std::to_string(a.psBase3D.AlphaTestCmpMode) + " " + std::to_string(a.psBase3D.AlphaTestRef));
	Result(Near(a.psBase3D.DrawAddColor[0], 10 / 255.0f) && Near(a.psBase3D.DrawAddColor[1], 20 / 255.0f) && Near(a.psBase3D.DrawAddColor[2], 30 / 255.0f) &&
			   a.psBase3D.DrawAddColor[3] == 0.0f,
		"PS.b1.DrawAddColor=SetDrawAddColor/255,a=0", F4(a.psBase3D.DrawAddColor));
	Result(a.psBase3D.UseHalfLambert == 1 && a.psBase3D.NoLightAngleAttenuation == 1, "PS.b1.UseHalfLambert,NoLightAngleAttenuation=1 when set",
		std::to_string(a.psBase3D.UseHalfLambert) + "," + std::to_string(a.psBase3D.NoLightAngleAttenuation));

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
	SetDrawBright(255, 255, 255);
	SetDrawAlphaTest(-1, 0);
	SetDrawAddColor(0, 0, 0);
	SetUseHalfLambertLighting(FALSE);
	SetUseLightAngleAttenuation(TRUE);
	DumpAll("reset", a);
	// アルファテストの既定: 比較値 -1/255、比較方法 GREATER(= 常に通る)
	Result(a.psBase3D.AlphaTestCmpMode == DX_CMP_GREATER && Near(a.psBase3D.AlphaTestRef, -1 / 255.0f) && a.psBase3D.UseHalfLambert == 0 &&
			   a.psBase3D.NoLightAngleAttenuation == 0,
		"PS.b1 defaults(AlphaTest GREATER -1/255, HalfLambert 0, NoLightAngleAttenuation 0)", std::to_string(a.psBase3D.AlphaTestRef));
}

// 既定値: シャドウマップ未使用、テクスチャ座標変換なし、スペキュラ・頂点カラーの使用
static void SceneDefaults()
{
	UseDumpTarget();
	SetCameraNearFar(1.0f, 1000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 0.0f, -100.0f), VGet(0.0f, 0.0f, 0.0f));
	AllBuffers a;
	DumpAll("defaults", a);

	bool zero = true;
	for (int m = 0; m < 3; m++)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				zero = zero && a.vsOther.ShadowMapLightViewProjectionMatrix[m][i][j] == 0.0f;
			}
		}
	}
	Result(zero, "VS.b2.ShadowMapLightViewProjectionMatrix=0 when no shadow map has been used", "");
	const DX_D3D11_PS_CONST_SHADOWMAP& s = a.psShadow3D.Data[0];
	Result(s.AdjustDepth == 1.0f && s.GradationParam == 0.0f && s.Enable_Light0 == 1.0f && s.Enable_Light1 == 1.0f && s.Enable_Light2 == 1.0f,
		"PS.b2 unused: AdjustDepth=1, Gradation=0, Enable_LightN=1 (1 = NOT shadowed)", "");
	const float(*t)[2][4] = a.vsOther.TextureMatrix;
	Result(t[0][0][0] == 1 && t[0][0][1] == 0 && t[0][0][2] == 0 && t[0][0][3] == 0 && t[0][1][0] == 0 && t[0][1][1] == 1 && t[0][1][2] == 0 && t[0][1][3] == 0 &&
			   t[1][0][0] == 0 && t[1][1][1] == 0 && t[2][0][0] == 0 && t[2][1][1] == 0,
		"VS.b2.TextureMatrix[0]=identity rows, [1],[2]=0", "");
	Result(a.vsBase.DiffuseSource == 1.0f && a.vsBase.SpecularSource == 1.0f && a.vsBase.MulSpecularColor == 1.0f,
		"VS.b1.DiffuseSource,SpecularSource,MulSpecularColor defaults=1", "");
	Result(a.vsBase.ToonOutLineSize[0] == 0.0f, "VS.b1.ToonOutLineSize=0 outside MV1 toon outline", "");

	// テクスチャ座標変換行列: TextureMatrix[0][0], [0][1] に行列の 0 列目・1 列目が入る
	const MATRIX tm = MMult(MGetScale(VGet(2.0f, 3.0f, 1.0f)), MGetTranslate(VGet(0.25f, 0.5f, 0.0f)));
	SetTextureAddressTransformMatrix(tm);
	SetMaterialUseVertDifColor(FALSE);
	SetUseSpecular(FALSE);
	DumpAll("texture transform + material dif + no specular", a);
	Result(SameMatrix(a.vsOther.TextureMatrix[0], 2, tm, true), "VS.b2.TextureMatrix[0]=columns 0,1 of SetTextureAddressTransformMatrix",
		MatStr(a.vsOther.TextureMatrix[0], 2));
	Result(a.vsBase.DiffuseSource == 0.0f && a.vsBase.MulSpecularColor == 0.0f, "VS.b1.DiffuseSource=0 (SetMaterialUseVertDifColor FALSE), MulSpecularColor=0 (SetUseSpecular FALSE)",
		std::to_string(a.vsBase.DiffuseSource) + "," + std::to_string(a.vsBase.MulSpecularColor));
	SetTextureAddressTransformMatrix(MGetIdent());
	ResetTextureAddressTransform();
	SetMaterialUseVertDifColor(TRUE);
	SetUseSpecular(TRUE);
}

// 2D の描画では自作の頂点シェーダーが無視される
static void Scene2DIgnoresUserVS()
{
	UseDumpTarget();
	DX_D3D11_PS_CONST_BUFFER_BASE b;
	SetUseVertexShader(g_clipVS);	// 使われたら、画素座標(0〜1024)を射影空間の座標として扱うので 1 画素しか描かれない
	const bool all = DumpPS(1, &b, sizeof(b), false);
	Result(all, "2D ToShader ignores user vertex shader (built-in VS_Shader2D is used)", all ? "all pixels drawn" : "not drawn");
	SetUseVertexShader(-1);
}

// 自作の定数バッファ(b4)
static void SceneUserConstantBuffer()
{
	UseDumpTarget();
	Result(CreateShaderConstantBuffer(20) == -1, "CreateShaderConstantBuffer(size not multiple of 16) fails", "");
	const int cb = CreateShaderConstantBuffer(sizeof(float) * 16);
	float* f = (float*)GetBufferShaderConstantBuffer(cb);
	for (int i = 0; i < 16; i++)
	{
		f[i] = (float)(i + 1) * 0.5f;
	}
	UpdateShaderConstantBuffer(cb);
	SetShaderConstantBuffer(cb, DX_SHADERTYPE_VERTEX, 4);
	SetShaderConstantBuffer(cb, DX_SHADERTYPE_PIXEL, 4);
	float vs[16] = {}, ps[16] = {};
	const bool okV = DumpVS(4, vs, sizeof(vs));
	const bool okP = DumpPS(4, ps, sizeof(ps), true);
	Result(okV && okP && memcmp(vs, f, sizeof(vs)) == 0 && memcmp(ps, f, sizeof(ps)) == 0, "user constant buffer at b4 (VS and PS)", F4(vs) + F4(ps));
	DeleteShaderConstantBuffer(cb);
}

// MV1 の描画の後: FactorColor は MV1 の不透明度で上書きされ、その後の自作シェーダーの描画にも残る。b3 にボーンの行列が残る
static void SceneAfterMV1()
{
	const int model = MV1LoadModel("DxChara.x");
	Result(model != -1, "load DxChara.x", "");
	if (model == -1)
	{
		return;
	}
	UseDumpTarget();
	SetCameraNearFar(1.0f, 5000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 800.0f, -1500.0f), VGet(0.0f, 500.0f, 0.0f));
	MV1SetOpacityRate(model, 0.5f);
	MV1SetPosition(model, VGet(100.0f, 0.0f, 0.0f));
	MV1SetRotationXYZ(model, VGet(0.0f, 0.5f, 0.0f));
	MV1DrawModel(model);
	const MATRIX modelWorld = MV1GetMatrix(model);
	AllBuffers a;
	DumpAll("after MV1DrawModel (opacity 0.5, skinned)", a);
	Result(Near(a.psBase3D.FactorColor[0], 1.0f) && Near(a.psBase3D.FactorColor[3], 0.5f), "PS.b1.FactorColor=(1,1,1,opacity) set by MV1 draw, stays for later ToShader draws",
		F4(a.psBase3D.FactorColor));
	// アニメーションなし(作ったときの姿勢)なので、どのボーンの行列も「モデルのワールド行列」そのものになる。
	// スキンメッシュでは、モデルの位置・回転は b3 のボーン行列に入っている(b1 の LocalWorldMatrix ではない)
	Result(SameMatrix(&a.vsLocalWorld.Matrix[0], 3, modelWorld, true) && SameMatrix(&a.vsLocalWorld.Matrix[3], 3, modelWorld, true),
		"VS.b3.Matrix[bone*3+0..2]=transposed bone matrix incl. model world (skinned MV1 draw)",
		MatStr(&a.vsLocalWorld.Matrix[0], 3) + " model=" + MatStr(modelWorld));
	MV1DeleteModel(model);
}

// samples/_shared の DxLibVS.hlsli・DxLibPS.hlsli の宣言が、DxLib の実際の並びと一致しているか。
// 宣言のフィールド名で読んだ値と、生の中身の決まった位置(float4 の番号)を、ビット単位で比べる
static void SceneHlsli()
{
	const int model = MV1LoadModel("DxChara.x");
	UseDumpTarget();
	SetCameraNearFar(1.0f, 5000.0f);
	SetCameraPositionAndTarget_UpVecY(VGet(30.0f, 800.0f, -1500.0f), VGet(0.0f, 500.0f, 0.0f));
	MV1SetOpacityRate(model, 0.25f);
	MV1DrawModel(model);	// b3 と FactorColor に値を入れる
	for (int i = 0; i < 5; i++)
	{
		CreatePointLightHandle(VGet(10.0f * i, 20.0f, 30.0f), 100.0f + i, 0.1f * i + 0.3f, 0.01f * i + 0.02f, 0.001f * i + 0.003f);
	}
	SetFogEnable(TRUE);
	SetFogColor(40, 50, 60);
	SetFogStartEnd(10.0f, 900.0f);
	SetVerticalFogEnable(TRUE);
	SetVerticalFogStartEnd(5.0f, 300.0f);
	SetVerticalFogDensity(0.5f, 7.0f);
	SetTextureAddressTransformMatrix(MMult(MGetScale(VGet(2.0f, 3.0f, 1.0f)), MGetTranslate(VGet(0.25f, 0.5f, 0.0f))));
	const MATRIX world = MMult(MGetRotX(0.3f), MGetTranslate(VGet(4.0f, 5.0f, 6.0f)));
	SetTransformToWorld(&world);
	SetDrawAddColor(11, 22, 33);
	SetDrawAlphaTest(DX_CMP_LESS, 77);
	SetUseHalfLambertLighting(TRUE);

	AllBuffers a;
	DumpAll("hlsli layout check", a);
	uint32_t vsField[13 * 4] = {}, psField[7 * 4] = {};
	const bool okV = DumpVSWith(g_hlsliVS, vsField, sizeof(vsField));
	const bool okP = DumpPSWith(g_hlsliPS, psField, sizeof(psField), true);

	// (バッファ, float4 の番号): HlsliVS.hlsl / HlsliPS.hlsl の Field() の順
	const uint32_t* vsBuf[4] = { (const uint32_t*)&a.vsCommon, (const uint32_t*)&a.vsBase, (const uint32_t*)&a.vsOther, (const uint32_t*)&a.vsLocalWorld };
	const int vsMap[13][2] = { { 0, 23 }, { 0, 41 }, { 0, 35 }, { 0, 45 }, { 0, 49 }, { 0, 51 }, { 1, 7 }, { 1, 10 }, { 1, 13 }, { 1, 15 }, { 2, 11 }, { 2, 13 }, { 3, 161 } };
	const uint32_t* psBuf[3] = { (const uint32_t*)&a.psCommon3D, (const uint32_t*)&a.psBase3D, (const uint32_t*)&a.psShadow3D };
	const int psMap[7][2] = { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 4 }, { 2, 3 }, { 2, 4 }, { 0, 32 } };
	std::string bad;
	int nonzero = 0;
	for (int f = 0; f < 13; f++)
	{
		for (int c = 0; c < 4; c++)
		{
			const uint32_t e = vsBuf[vsMap[f][0]][vsMap[f][1] * 4 + c];
			nonzero += e != 0;
			if (vsField[f * 4 + c] != e)
			{
				bad += " VS" + std::to_string(f) + "." + std::to_string(c);
			}
		}
	}
	for (int f = 0; f < 7; f++)
	{
		for (int c = 0; c < 4; c++)
		{
			const uint32_t e = psBuf[psMap[f][0]][psMap[f][1] * 4 + c];
			nonzero += e != 0;
			if (psField[f * 4 + c] != e)
			{
				bad += " PS" + std::to_string(f) + "." + std::to_string(c);
			}
		}
	}
	Result(okV && okP && bad.empty(), "samples/_shared DxLibVS.hlsli/DxLibPS.hlsli layout matches DxLib (20 fields, bitwise)",
		"mismatch:" + (bad.empty() ? std::string(" none") : bad) + " nonzero components=" + std::to_string(nonzero) + "/80");

	DeleteLightHandleAll();
	SetFogEnable(FALSE);
	SetVerticalFogEnable(FALSE);
	SetTextureAddressTransformMatrix(MGetIdent());
	SetDrawAddColor(0, 0, 0);
	SetDrawAlphaTest(-1, 0);
	SetUseHalfLambertLighting(FALSE);
	MV1DeleteModel(model);
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	ChangeWindowMode(TRUE);
	SetGraphMode(640, 480, 32);
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	SetOutApplicationLogValidFlag(FALSE);
	if (DxLib_Init() == -1)
	{
		return -1;
	}
	fopen_s(&g_res, "results.txt", "w");
	fopen_s(&g_dump, "dump.txt", "w");
	Result(GetUseDirect3DVersion() == DX_DIRECT3D_11, "direct3d11", "version=" + std::to_string(GetUseDirect3DVersion()));

	// 32 ビット浮動小数点 RGBA の描画先(幅 DUMP_WIDTH、高さ 1)
	SetDrawValidFloatTypeGraphCreateFlag(TRUE);
	SetCreateDrawValidGraphChannelNum(4);
	SetCreateGraphChannelBitDepth(32);
	g_rt = MakeScreen(DUMP_WIDTH, 1, FALSE);
	SetDrawValidFloatTypeGraphCreateFlag(FALSE);
	SetCreateDrawValidGraphChannelNum(0);
	SetCreateGraphChannelBitDepth(0);
	Result(g_rt != -1, "float-target", "handle=" + std::to_string(g_rt));

	// b4 は自作の定数バッファの確認用
	const char* slots[5] = { "b0", "b1", "b2", "b3", "b4" };
	bool loaded = true;
	for (int i = 0; i < 5; i++)
	{
		g_vsDump[i] = LoadVertexShader((std::string("shaders/DumpVS_") + slots[i] + ".vso").c_str());
		loaded = loaded && g_vsDump[i] != -1;
	}
	for (int i = 0; i < 5; i++)
	{
		g_psDump[i] = i == 3 ? -1 : LoadPixelShader((std::string("shaders/DumpPS_") + slots[i] + ".pso").c_str());
		loaded = loaded && (i == 3 || g_psDump[i] != -1);
	}
	g_vsDumpPS = LoadPixelShader("shaders/DumpVS_PS.pso");
	g_clipVS = LoadVertexShader("shaders/ClipVS.vso");
	g_hlsliVS = LoadVertexShader("shaders/HlsliVS.vso");
	g_hlsliPS = LoadPixelShader("shaders/HlsliPS.pso");
	loaded = loaded && g_vsDumpPS != -1 && g_clipVS != -1 && g_hlsliVS != -1 && g_hlsliPS != -1;
	Result(loaded, "load-shaders", "");

	if (g_rt != -1 && loaded)
	{
		SceneCamera();
		SceneLights();
		SceneMaterialFog();
		SceneBlend();
		SceneDefaults();
		Scene2DIgnoresUserVS();
		SceneUserConstantBuffer();
		SceneAfterMV1();
		SceneHlsli();
	}

	fprintf(g_res, "NG count=%d\n", g_ng);
	fclose(g_res);
	fclose(g_dump);
	DxLib_End();
	return 0;
}
