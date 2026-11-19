/*==============================================================================

   シェーダー [shader.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------
複数シェーダ―読み込みのため改変 
	改変者:Haruki Yokoyama	Date:2026/01/03


==============================================================================*/
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "directx.h"
#include "debug_ostream.h"
#include <fstream>
#include "shader.h"
#include "light.h"


static ID3D11VertexShader* g_pVertexShader_2D = nullptr;
static ID3D11VertexShader* g_pVertexShader_3D = nullptr;
static ID3D11VertexShader* g_pInstanceVertexShader = nullptr;
static ID3D11VertexShader* g_pInstanceObj_VS = nullptr;
static ID3D11PixelShader* g_pPixelShader_2D = nullptr;
static ID3D11PixelShader* g_pPixelShader_3D = nullptr;

static ID3D11InputLayout* g_pInputLayout_2D = nullptr;
static ID3D11InputLayout* g_pInputLayout_3D = nullptr;
static ID3D11InputLayout* g_pInstanceInputLayout = nullptr;
static ID3D11InputLayout* g_pInstanceObj_InputLayout = nullptr;

static ID3D11Buffer* g_pVSConstantBuffer_2D = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer_3D = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer_Instance_Obj = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer_Instance = nullptr;

static ID3D11Buffer* g_pVSLightBuffer = nullptr;
static ID3D11Buffer* g_pVSLightBuffer_Instance = nullptr;
static ID3D11Buffer* g_pPSRimBuffer = nullptr;
static ID3D11Buffer* g_pPS_LightViewProjBuffer = nullptr;
static ID3D11Buffer* g_pPS_TimeBuffer = nullptr;

static ID3D11SamplerState* g_SamplerState = nullptr;
// Shadow Map用
static ID3D11VertexShader* g_pVertexShader_Shadow = nullptr;
static ID3D11InputLayout* g_pInputLayout_Shadow = nullptr;
static ID3D11Buffer* g_pVSConstantBuffer_Shadow = nullptr;
static ID3D11SamplerState* g_pShadowSamplerState = nullptr;

// 注意！初期化で外部から設定されるもの。Release不要。
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

bool ShaderLoad_2D();
bool ShaderLoad_3D();
bool ShaderLoad_Shadow();
bool ShaderLoad_Instance();
bool ShaderLoad_InstanceObj();

bool Shader_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	// デバイスとデバイスコンテキストのチェック
	if (!pDevice || !pContext) {
		hal::dout << "Shader_Initialize() : 与えられたデバイスかコンテキストが不正です" << std::endl;
		return false;
	}

	// デバイスとデバイスコンテキストの保存
	g_pDevice = pDevice;
	g_pContext = pContext;

	if(ShaderLoad_2D() == false)
	{
		hal::dout << "Shader_Initialize() : 2Dシェーダーの読み込みに失敗しました" << std::endl;
		return false;
	}

	if(ShaderLoad_3D() == false)
	{
		hal::dout << "Shader_Initialize() : 3Dシェーダーの読み込みに失敗しました" << std::endl;
		return false;
	}

	if(ShaderLoad_Shadow() == false)
	{
		hal::dout << "Shader_Initialize() : シャドウマップシェーダーの読み込みに失敗しました" << std::endl;
		return false;
	}

	if(ShaderLoad_Instance() == false)
	{
		hal::dout << "Shader_Initialize() : インスタンス描画シェーダーの読み込みに失敗しました" << std::endl;
		return false;
	}

	if(ShaderLoad_InstanceObj() == false)
	{
		hal::dout << "Shader_Initialize() : インスタンス描画シェーダーの読み込みに失敗しました" << std::endl;
		return false;
	}

	//サンプラーステート設定
	//サンプラー→テクスチャの貼り付け方
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	g_pDevice->CreateSamplerState(&samplerDesc,&g_SamplerState);

	g_pContext->PSSetSamplers(0,1,&g_SamplerState);

	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sd.BorderColor[0] = 1.0f;
	sd.BorderColor[1] = 1.0f;
	sd.BorderColor[2] = 1.0f;
	sd.BorderColor[3] = 1.0f;
	sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

	g_pDevice->CreateSamplerState(&sd, &g_pShadowSamplerState);
	g_pContext->PSSetSamplers(1, 1, &g_pShadowSamplerState);

	return true;
}

void Shader_Finalize()
{
	SAFE_RELEASE(g_pVertexShader_3D);
	SAFE_RELEASE(g_pVertexShader_2D);
	SAFE_RELEASE(g_pInstanceVertexShader);
	SAFE_RELEASE(g_pInstanceObj_VS);
	SAFE_RELEASE(g_pPixelShader_3D);
	SAFE_RELEASE(g_pPixelShader_2D);

	SAFE_RELEASE(g_pInputLayout_3D);
	SAFE_RELEASE(g_pInputLayout_2D);
	SAFE_RELEASE(g_pInstanceObj_InputLayout);
	SAFE_RELEASE(g_pInstanceInputLayout);

	SAFE_RELEASE(g_pVSConstantBuffer_3D);
	SAFE_RELEASE(g_pVSConstantBuffer_2D);
	SAFE_RELEASE(g_pVSConstantBuffer_Instance);
	SAFE_RELEASE(g_pVSConstantBuffer_Instance_Obj);

	SAFE_RELEASE(g_pVSLightBuffer);
	SAFE_RELEASE(g_pVSLightBuffer_Instance);
	SAFE_RELEASE(g_pPSRimBuffer);
	SAFE_RELEASE(g_pPS_LightViewProjBuffer);
	SAFE_RELEASE(g_pPS_TimeBuffer);

	SAFE_RELEASE(g_SamplerState);

	// Shadow Map用
	SAFE_RELEASE(g_pVertexShader_Shadow);
	SAFE_RELEASE(g_pInputLayout_Shadow);
	SAFE_RELEASE(g_pVSConstantBuffer_Shadow);
	SAFE_RELEASE(g_pShadowSamplerState);
}

void Shader_SetMatrix_3D(const Matrix& matrix, bool isInstance)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose[2];

	// XMFLOAT4X4とXMMATRIXはメモリ配置が異なるため、直接代入できない
	// そのため、行列を転置してXMFLOAT4X4に格納する
	// 転置→ 行と列を入れ替えること
	// GPUへの転送はXMFLOAT4X4の方が高速。

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose[0], XMMatrixTranspose(matrix.matrix));
	XMStoreFloat4x4(&transpose[1], XMMatrixTranspose(matrix.world));

	// 定数バッファに行列をセット
	if(isInstance)
	{
		g_pContext->UpdateSubresource(g_pVSConstantBuffer_Instance_Obj, 0, nullptr, &transpose[0], 0, 0);
	}
	else
	{
		g_pContext->UpdateSubresource(g_pVSConstantBuffer_3D, 0, nullptr, transpose, 0, 0);
	}

	return;
}

void Shader_SetMatrix_2D(const XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer_2D, 0, nullptr, &transpose, 0, 0);
}

void Shader_SetLight(const Light& light, bool isInstance)
{
	// 定数バッファにライト情報をセット
	if(isInstance)
	{
		g_pContext->UpdateSubresource(g_pVSLightBuffer_Instance, 0, nullptr, &light, 0, 0);
	}
	else
	{
		g_pContext->UpdateSubresource(g_pVSLightBuffer, 0, nullptr, &light, 0, 0);
	}
}

void Shader_SetRimlight(const Rimlight& rimlight)
{
	// 定数バッファにリムライト情報をセット
	g_pContext->UpdateSubresource(g_pPSRimBuffer, 0, nullptr, &rimlight, 0, 0);
}

void Shader_SetTime(float time)
{
	// 定数バッファに時間情報をセット
	g_pContext->UpdateSubresource(g_pPS_TimeBuffer, 0, nullptr, &time, 0, 0);
}

void Shader_Begin_3D(bool isInstance)
{
	if (isInstance)
	{
		// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
		g_pContext->VSSetShader(g_pInstanceObj_VS, nullptr, 0);
		g_pContext->PSSetShader(g_pPixelShader_3D, nullptr, 0);

		// 頂点レイアウトを描画パイプラインに設定
		g_pContext->IASetInputLayout(g_pInstanceObj_InputLayout);

		// 定数バッファを描画パイプラインに設定
		g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer_Instance_Obj);
		// ライト用定数バッファをセット
		g_pContext->VSSetConstantBuffers(1, 1, &g_pVSLightBuffer_Instance);
	}
	else
	{
		// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
		g_pContext->VSSetShader(g_pVertexShader_3D, nullptr, 0);
		g_pContext->PSSetShader(g_pPixelShader_3D, nullptr, 0);

		// 頂点レイアウトを描画パイプラインに設定
		g_pContext->IASetInputLayout(g_pInputLayout_3D);

		// 定数バッファを描画パイプラインに設定
		g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer_3D);
		// ライト用定数バッファをセット
		g_pContext->VSSetConstantBuffers(1, 1, &g_pVSLightBuffer);
		// 時間用定数バッファをセット
		g_pContext->PSSetConstantBuffers(2, 1, &g_pPS_TimeBuffer);
	}
		// リムライト用定数バッファをセット
		g_pContext->PSSetConstantBuffers(0, 1, &g_pPSRimBuffer);
		// ライトビュー射影行列用定数バッファをセット
		g_pContext->PSSetConstantBuffers(1, 1, &g_pPS_LightViewProjBuffer);

		// シャドウマップ用シェーダーリソースビューをセット
		g_pContext->PSSetShaderResources(1, 1, GetShadowMapSRV());
		g_pContext->PSSetSamplers(1, 1, &g_pShadowSamplerState);
}

void Shader_Begin_2D()
{
	// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader_2D, nullptr, 0);
	g_pContext->PSSetShader(g_pPixelShader_2D, nullptr, 0);
	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout_2D);
	// 定数バッファを描画パイプラインに設定
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer_2D);


}

void Shader_Begin_Shadow()
{
	// 頂点シェーダーとピクセルシェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pVertexShader_Shadow, nullptr, 0);
	g_pContext->PSSetShader(nullptr, nullptr, 0); // ピクセルシェーダーは使わない
	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInputLayout_Shadow);
	// 定数バッファを描画パイプラインに設定
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer_Shadow);
}

void Shader_End_Shadow()
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;

	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(GetLightViewProj()));
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pPS_LightViewProjBuffer, 0, nullptr, &transpose, 0, 0);

}

void InstanceShader_Begin()
{
	// 頂点シェーダーを描画パイプラインに設定
	g_pContext->VSSetShader(g_pInstanceVertexShader, nullptr, 0);
	// ピクセルシェーダーを描画パイプラインに設定
	g_pContext->PSSetShader(g_pPixelShader_2D, nullptr, 0);

	// 頂点レイアウトを描画パイプラインに設定
	g_pContext->IASetInputLayout(g_pInstanceInputLayout);
	// 定数バッファを描画パイプラインに設定
	g_pContext->VSSetConstantBuffers(0, 1, &g_pVSConstantBuffer_Instance);
}

void InstanceShader_SetMatrix(const XMMATRIX& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose;
	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose, XMMatrixTranspose(matrix));
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer_Instance, 0, nullptr, &transpose, 0, 0);
}

void Shader_SetMatrix_Shadow(const Matrix& matrix)
{
	// 定数バッファ格納用行列の構造体を定義
	XMFLOAT4X4 transpose[2];
	// 行列を転置して定数バッファ格納用行列に変換
	XMStoreFloat4x4(&transpose[0], XMMatrixTranspose(matrix.world));
	XMStoreFloat4x4(&transpose[1], XMMatrixTranspose(matrix.matrix));
	// 定数バッファに行列をセット
	g_pContext->UpdateSubresource(g_pVSConstantBuffer_Shadow, 0, nullptr, transpose, 0, 0);
}

bool ShaderLoad_3D()
{
	HRESULT hr; // 戻り値格納用

	// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shader\\build\\shaderVertex3D.cso", std::ios::binary);

	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\shaderVertex3D.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end); // ファイルポインタを末尾に移動
	std::streamsize filesize = ifs_vs.tellg(); // ファイルポインタの位置を取得（つまりファイルサイズ）
	ifs_vs.seekg(0, std::ios::beg); // ファイルポインタを先頭に戻す

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];

	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close(); // ファイルを閉じる

	// 頂点シェーダーの作成
	hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader_3D);

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}

	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

	// 頂点レイアウトの作成
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout_3D);

	delete[] vsbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}


	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Matrix); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_3D);

	buffer_desc.ByteWidth = sizeof(Light); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSLightBuffer);

	// 事前コンパイル済みピクセルシェーダーの読み込み
	std::ifstream ifs_ps("shader\\build\\shaderPixel3D.cso", std::ios::binary);
	if (!ifs_ps) {
		MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\shaderPixel3D.cso", "エラー", MB_OK);
		return false;
	}

	ifs_ps.seekg(0, std::ios::end);
	filesize = ifs_ps.tellg();
	ifs_ps.seekg(0, std::ios::beg);

	unsigned char* psbinary_pointer = new unsigned char[filesize];
	ifs_ps.read((char*)psbinary_pointer, filesize);
	ifs_ps.close();

	// ピクセルシェーダーの作成
	hr = g_pDevice->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader_3D);

	delete[] psbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : ピクセルシェーダーの作成に失敗しました" << std::endl;
		return false;
	}

	// 頂点シェーダー用定数バッファの作成
	buffer_desc.ByteWidth = sizeof(Rimlight); // バッファのサイズ
	hr = g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPSRimBuffer);

	if(FAILED(hr))
	{
		hal::dout << "Shader_Initialize() : リムライト用定数バッファの作成に失敗しました" << std::endl;
		return false;
	}

	buffer_desc.ByteWidth = sizeof(XMMATRIX); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPS_LightViewProjBuffer);

	buffer_desc.ByteWidth = sizeof(float) * 4; // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pPS_TimeBuffer);

	return true;
}

bool ShaderLoad_2D()
{
	// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shader\\build\\shaderVertex2D.cso", std::ios::binary);
	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\nshader_vertex_2d.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end); // ファイルポインタを末尾に移動

	std::streamsize filesize = ifs_vs.tellg();
	ifs_vs.seekg(0, std::ios::beg); // ファイルポインタを先頭に戻す

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];
	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close(); // ファイルを閉じる

	// 頂点シェーダーの作成
	HRESULT hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader_2D);
	if (FAILED(hr)) {
		hal::dout << "ShaderLoad_2D() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}

	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得

	// 頂点レイアウトの作成
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout_2D);

	delete[] vsbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}


	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMMATRIX); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ

	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_2D);

	// 事前コンパイル済みピクセルシェーダーの読み込み
	std::ifstream ifs_ps("shader\\build\\shaderPixel2D.cso", std::ios::binary);
	if (!ifs_ps) {
		MessageBox(nullptr, "ピクセルシェーダーの読み込みに失敗しました\n\nshader_pixel_2d.cso", "エラー", MB_OK);
		return false;
	}

	ifs_ps.seekg(0, std::ios::end);
	filesize = ifs_ps.tellg();
	ifs_ps.seekg(0, std::ios::beg);

	unsigned char* psbinary_pointer = new unsigned char[filesize];
	ifs_ps.read((char*)psbinary_pointer, filesize);
	ifs_ps.close();

	// ピクセルシェーダーの作成
	hr = g_pDevice->CreatePixelShader(psbinary_pointer, filesize, nullptr, &g_pPixelShader_2D);

	delete[] psbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : ピクセルシェーダーの作成に失敗しました" << std::endl;
		return false;
	}

	return true;

}

bool ShaderLoad_Shadow()
{
	HRESULT hr; // 戻り値格納用
	// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shader\\build\\shadowVS.cso", std::ios::binary);
	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\shaderVS.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end);
	std::streamsize filesize = ifs_vs.tellg(); 
	ifs_vs.seekg(0, std::ios::beg); 

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];
	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close(); // ファイルを閉じる

	// 頂点シェーダーの作成
	hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pVertexShader_Shadow);
	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; 
		return false;
	}

	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得
	// 頂点レイアウトの作成
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInputLayout_Shadow);
	delete[] vsbinary_pointer; // バイナリデータのバッファを解放

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}
	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Matrix); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_Shadow);

	return true;
}

bool ShaderLoad_Instance()
{
	HRESULT hr; // 戻り値格納用
	// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shader\\build\\instancingVS.cso", std::ios::binary);
	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\instancingVS.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end); 
	std::streamsize filesize = ifs_vs.tellg();
	ifs_vs.seekg(0, std::ios::beg);

	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];
	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む

	ifs_vs.close();

	// 頂点シェーダーの作成
	hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pInstanceVertexShader);
	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}

	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		// インスタンスデータ
		{ "INSTANCE_POS",	  0, DXGI_FORMAT_R32G32B32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_SCALE",   0, DXGI_FORMAT_R32_FLOAT,			1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_ROT",	  0, DXGI_FORMAT_R32G32B32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_COLOR",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};
	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得
	// 頂点レイアウトの作成
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInstanceInputLayout);
	delete[] vsbinary_pointer; // バイナリデータのバッファを解放
	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}

	/*
	分かれる場合
	stream0 [pos][texcoord][pos][texcoord]...
	stream1 [instance_pos][instance_scale][instance_rot][instance_color][instance_pos][instance_scale][instance_rot][instance_color]...

	*/
	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMMATRIX); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_Instance);

	return true;
}

bool ShaderLoad_InstanceObj()
{
	HRESULT hr; // 戻り値格納用
	// 事前コンパイル済み頂点シェーダーの読み込み
	std::ifstream ifs_vs("shader\\build\\InstancingVS_obj.cso", std::ios::binary);
	if (!ifs_vs) {
		MessageBox(nullptr, "頂点シェーダーの読み込みに失敗しました\n\instancingVS_obj.cso", "エラー", MB_OK);
		return false;
	}

	// ファイルサイズを取得
	ifs_vs.seekg(0, std::ios::end);
	std::streamsize filesize = ifs_vs.tellg();
	ifs_vs.seekg(0, std::ios::beg);
	// バイナリデータを格納するためのバッファを確保
	unsigned char* vsbinary_pointer = new unsigned char[filesize];
	ifs_vs.read((char*)vsbinary_pointer, filesize); // バイナリデータを読み込む
	ifs_vs.close();
	// 頂点シェーダーの作成
	hr = g_pDevice->CreateVertexShader(vsbinary_pointer, filesize, nullptr, &g_pInstanceObj_VS);
	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点シェーダーの作成に失敗しました" << std::endl;
		delete[] vsbinary_pointer; // メモリリークしないようにバイナリデータのバッファを解放
		return false;
	}

	// 頂点レイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		// セマンティクス, インデックス, フォーマット, ストリーム, オフセット, 入力スロットクラス, スロットステップレート 
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		// インスタンスデータ
		{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};

	UINT num_elements = ARRAYSIZE(layout); // 配列の要素数を取得
	// 頂点レイアウトの作成
	hr = g_pDevice->CreateInputLayout(layout, num_elements, vsbinary_pointer, filesize, &g_pInstanceObj_InputLayout);
	delete[] vsbinary_pointer; // バイナリデータのバッファを解放
	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : 頂点レイアウトの作成に失敗しました" << std::endl;
		return false;
	}

	// 頂点シェーダー用定数バッファの作成
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(XMMATRIX); // バッファのサイズ
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; // バインドフラグ
	hr = g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSConstantBuffer_Instance_Obj);

	if (FAILED(hr)) {
		hal::dout << "Shader_Initialize() : インスタンス描画用定数バッファの作成に失敗しました" << std::endl;
		return false;
	}

	buffer_desc.ByteWidth = sizeof(Light); // バッファのサイズ
	g_pDevice->CreateBuffer(&buffer_desc, nullptr, &g_pVSLightBuffer_Instance);

	return true;
}