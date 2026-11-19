#include "directX.h"
#include "shader.h"
#include "directFont.h"

#pragma comment (lib,"d3d11.lib")//D3D11.hには関数のプロトタイプ宣言しか入ってないため関数定義をここで読み出す(スタティックリンクライブラリ)

static constexpr UINT SHADOW_MAP_SIZE = 4096;

static ID3D11Device* g_Device = NULL;
static ID3D11DeviceContext* g_DeviceContext = NULL;
static IDXGISwapChain* g_SwapChain = NULL;
static ID3D11Texture2D* g_DepthStencilTexture = NULL;
static ID3D11RenderTargetView* g_RenderTargetView = NULL;

static ID3D11DepthStencilView* g_DepthStencilView = NULL;
static D3D11_VIEWPORT g_Viewport{};

static ID3D11RasterizerState* g_RasterizerState = NULL;
static ID3D11BlendState* g_BlendState = NULL;
static ID3D11BlendState* g_BlendState_add = NULL;
static ID3D11DepthStencilState* g_DepthStencilStateDepthDisable = NULL;
static ID3D11DepthStencilState* g_DepthStencilStateDepthEnable = NULL;
static ID3D11DepthStencilState* g_DepthStencilStateDepthWriteDisable = NULL;	// テストはオン
// Shadow Map関連
static ID3D11Texture2D* g_ShadowDepthTexture = NULL;
static ID3D11DepthStencilView* g_ShadowDSV = NULL;
static ID3D11ShaderResourceView* g_ShadowSRV = NULL;
static ID3D11RasterizerState* g_ShadowRS = NULL;
static D3D11_VIEWPORT g_ShadowViewport;

// リザルト用スナップショット
static ID3D11Texture2D* g_ResultTexture = NULL;
static ID3D11ShaderResourceView* g_ResultSRV = NULL;


Color g_clearColor = Color(1.0f, 0.0f, 1.0f, 1.0f);

void DirectXInitialize(HWND hWnd)
{
	//デバイス、スワップチェイン、コンテキスト生成
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = WINDOW_WIDTH;
	sd.BufferDesc.Height = WINDOW_HEIGHT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;	//FPS(MAX)
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	//デバイス、スワップチェインの作成をしてくれる
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&g_SwapChain,
		&g_Device,
		&feature_level,
		&g_DeviceContext
	);

	//hrに戻り値が返ってくる(successとか、エラーならエラー)
	if (FAILED(hr)) {
		//メッセージボックスなどでエラー表示

		return;
	}

	//レンダーターゲットビュー生成
	ID3D11Texture2D* pBackBuffer = NULL;
	g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	g_Device->CreateRenderTargetView(pBackBuffer, NULL, &g_RenderTargetView);
	pBackBuffer->Release();

	//デプスステンシル用テクスチャ作成
	D3D11_TEXTURE2D_DESC td = {};
	td.Width = sd.BufferDesc.Width;
	td.Height = sd.BufferDesc.Height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// td.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	td.SampleDesc = sd.SampleDesc;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;
	g_Device->CreateTexture2D(&td, NULL, &g_DepthStencilTexture);

	//デプスステンシルターゲット作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
	dsvd.Format = td.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Flags = 0;
	g_Device->CreateDepthStencilView(g_DepthStencilTexture, &dsvd, &g_DepthStencilView);

	//デバイスコンテキストへレンダーターゲットビューとデプスステンシルビューを設定
	g_DeviceContext->OMSetRenderTargets(1, &g_RenderTargetView, g_DepthStencilView);

	//ビューポート設定
	
	g_Viewport.Width = (FLOAT)WINDOW_WIDTH;
	g_Viewport.Height = (FLOAT)WINDOW_HEIGHT;
	g_Viewport.MinDepth = 0.0f;
	g_Viewport.MaxDepth = 1.0f;
	g_Viewport.TopLeftX = 0;
	g_Viewport.TopLeftY = 0;
	g_DeviceContext->RSSetViewports(1, &g_Viewport);

	//ラスタライザステートの作成
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.DepthClipEnable = TRUE;
	rd.MultisampleEnable = FALSE;
	g_Device->CreateRasterizerState(&rd, &g_RasterizerState);

	//シャドウマップ用ラスタライザーステート作成
	// シャドウマップ専用
	rd.DepthBias = 1000;
	rd.SlopeScaledDepthBias = 1.0f;
	rd.DepthBiasClamp = 0.0f;
	g_Device->CreateRasterizerState(&rd, &g_ShadowRS);

	//デバイスコンテキストにラスタライザーステートを設定
	g_DeviceContext->RSSetState(g_RasterizerState);

	//ブレンドステート設定
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = FALSE;
	bd.IndependentBlendEnable = FALSE;
	bd.RenderTarget[0].BlendEnable = TRUE;

	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	g_Device->CreateBlendState(&bd, &g_BlendState);

	bd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	g_Device->CreateBlendState(&bd, &g_BlendState_add);

	g_DeviceContext->OMSetBlendState(g_BlendState, blend_factor, 0xffffffff);


	// 深度ステンシルステート設定
	D3D11_DEPTH_STENCIL_DESC dsd = {};
	dsd.DepthEnable = TRUE;
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthFunc = D3D11_COMPARISON_LESS;
	dsd.StencilEnable = FALSE;
	g_Device->CreateDepthStencilState(&dsd, &g_DepthStencilStateDepthEnable);

	// 深度書き込み無効ステート作成
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	g_Device->CreateDepthStencilState(&dsd, &g_DepthStencilStateDepthWriteDisable);

	// 深度テスト無効ステート作成
	dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsd.DepthEnable = FALSE;
	g_Device->CreateDepthStencilState(&dsd, &g_DepthStencilStateDepthDisable);

	g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthEnable, NULL);

	// シャドウマップ用デプステクスチャ作成
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = SHADOW_MAP_SIZE;
	shadowDesc.Height = SHADOW_MAP_SIZE;
	shadowDesc.MipLevels = 1;
	shadowDesc.ArraySize = 1;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	g_Device->CreateTexture2D(&shadowDesc, nullptr, &g_ShadowDepthTexture);

	// シャドウマップ用DSV作成
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc = {};
	shadowDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	g_Device->CreateDepthStencilView(
		g_ShadowDepthTexture,
		&shadowDSVDesc,
		&g_ShadowDSV
	);

	// シャドウマップ用SRV設定
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = 1;

	g_Device->CreateShaderResourceView(
		g_ShadowDepthTexture,
		&shadowSRVDesc,
		&g_ShadowSRV
	);

	// シャドウマップ用ビューポート設定
	g_ShadowViewport.Width = (FLOAT)SHADOW_MAP_SIZE;
	g_ShadowViewport.Height = (FLOAT)SHADOW_MAP_SIZE;
	g_ShadowViewport.MinDepth = 0.0f;
	g_ShadowViewport.MaxDepth = 1.0f;
	g_ShadowViewport.TopLeftX = 0;
	g_ShadowViewport.TopLeftY = 0;

	// リザルト用テクスチャ作成
	D3D11_TEXTURE2D_DESC resultDesc = {};
	resultDesc.Width = WINDOW_WIDTH;
	resultDesc.Height = WINDOW_HEIGHT;
	resultDesc.MipLevels = 1;
	resultDesc.ArraySize = 1;
	resultDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resultDesc.SampleDesc.Count = 1;
	resultDesc.Usage = D3D11_USAGE_DEFAULT;
	resultDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	g_Device->CreateTexture2D(&resultDesc, nullptr, &g_ResultTexture);

	// リザルト用SRV作成
	D3D11_SHADER_RESOURCE_VIEW_DESC resultSRVDesc = {};
	resultSRVDesc.Format = resultDesc.Format;
	resultSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resultSRVDesc.Texture2D.MipLevels = 1;
	g_Device->CreateShaderResourceView(g_ResultTexture,&resultSRVDesc,&g_ResultSRV);

	DirectFontInitialize(g_SwapChain, hWnd);
}

void DirectXFinalaize(void)
{
	SAFE_RELEASE(g_DepthStencilStateDepthDisable);
	SAFE_RELEASE(g_DepthStencilStateDepthEnable);
	SAFE_RELEASE(g_DepthStencilStateDepthWriteDisable);
	SAFE_RELEASE(g_BlendState);
	SAFE_RELEASE(g_RasterizerState);

	SAFE_RELEASE(g_DepthStencilView);
	SAFE_RELEASE(g_DepthStencilTexture);
	SAFE_RELEASE(g_RenderTargetView);
	SAFE_RELEASE(g_SwapChain);
	SAFE_RELEASE(g_DeviceContext);
	SAFE_RELEASE(g_Device);

	SAFE_RELEASE(g_ShadowSRV);
	SAFE_RELEASE(g_ShadowDSV);
	SAFE_RELEASE(g_ShadowDepthTexture);

	SAFE_RELEASE(g_ResultSRV);
	SAFE_RELEASE(g_ResultTexture);

	DirectFontFinalize();
}

void ShadowBegin()
{
	IsDepthEnable(true);
	// レンダーターゲットなし（深度のみ）
	ID3D11RenderTargetView* nullRTV = nullptr;
	g_DeviceContext->OMSetRenderTargets(0, &nullRTV, g_ShadowDSV);

	g_DeviceContext->ClearDepthStencilView(
		g_ShadowDSV,
		D3D11_CLEAR_DEPTH,
		1.0f,
		0
	);

	g_DeviceContext->RSSetViewports(1, &g_ShadowViewport);
	g_DeviceContext->RSSetState(g_ShadowRS);

	Shader_Begin_Shadow();
	
}

void ShadowEnd()
{
	Shader_End_Shadow();
	// 通常のレンダーターゲットに戻す
	g_DeviceContext->OMSetRenderTargets(
		1,
		&g_RenderTargetView,
		g_DepthStencilView
	);

	g_DeviceContext->RSSetState(g_RasterizerState);
	// ビューポートも戻す
	g_DeviceContext->RSSetViewports(1, &g_Viewport);


}

ID3D11Device* DirectXGetDevice()
{
	return g_Device;
}

ID3D11DeviceContext* DirectXGetDeviceContext()
{
	return g_DeviceContext;
}

ID3D11ShaderResourceView* const* GetShadowMapSRV()
{
	return &g_ShadowSRV;
}

//バックバッファクリア
void Clear()
{
	//	バックバッファクリア
	float clear_color[4] = { g_clearColor.r, g_clearColor.g, g_clearColor.b,1.0f};
	g_DeviceContext->ClearRenderTargetView(g_RenderTargetView,clear_color);
	g_DeviceContext->ClearDepthStencilView(g_DepthStencilView,D3D11_CLEAR_DEPTH,1.0f,0);
}

//プレゼント
void Present()
{
	//バックバッファとフロントバッファの切り替え
	g_SwapChain->Present(1,0);
}

void SetClearColor(Color color)
{
	g_clearColor = color;
}

void SetBlendState(BLEND_STATE state)
{
	float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	switch (state)
	{
	case NOT_ALPHA:
		g_DeviceContext->OMSetBlendState(NULL, blend_factor, 0xffffffff);
		break;

	case ALPHABLEND:
		g_DeviceContext->OMSetBlendState(g_BlendState, blend_factor, 0xffffffff);
		break;
	case ADD:
		g_DeviceContext->OMSetBlendState(g_BlendState_add, blend_factor, 0xffffffff);
		break;
	default:
		break;
	}
}

void IsDepthEnable(bool enable)
{
	if (enable)
	{
		g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthEnable, NULL);
	}
	else
	{
		g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthDisable, NULL);
	}
}

void IsDepthWriteEnable(bool enable)
{
	if (enable)
	{
		g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthEnable, NULL);
	}
	else
	{
		g_DeviceContext->OMSetDepthStencilState(g_DepthStencilStateDepthWriteDisable, NULL);
	}
}

void CreateResultSnapshot()
{

	ID3D11Texture2D* backBuffer = nullptr;
	// バックバッファを取得
	g_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	// リザルトテクスチャにコピー
	g_DeviceContext->CopyResource(g_ResultTexture, backBuffer);

	SAFE_RELEASE(backBuffer);
}

ID3D11ShaderResourceView* const* GetResultSnapshotSRV()
{
	return &g_ResultSRV;
}

