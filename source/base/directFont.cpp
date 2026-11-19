#include "directFont.h"
#include <memory>
#include <unordered_map>

// 今回は1つのフォントのみ使用するため簡略化した実装になっている

using namespace Microsoft;


static constexpr const wchar_t* path = L"asset\\font\\851H-kktt_004.ttf";

std::wstring StringToWString(std::string oString);

static IDWriteFactory5* g_pDWriteFactory = nullptr;
static ID2D1RenderTarget* g_pRenderTarget = nullptr;
static ID2D1Factory* g_pD2DFactory = nullptr;
static IDXGISurface* g_pBackBuffer = nullptr;
static ID2D1SolidColorBrush* g_pFontBrush = nullptr;
static ID2D1SolidColorBrush* g_pOutlineBrush = nullptr;
static ID2D1SolidColorBrush* g_pShadowBrush = nullptr;
static IDWriteFontCollection1* g_pFontCollection = nullptr;
static std::wstring g_fontFamilyName = L"";
static std::unordered_map<float, IDWriteTextFormat*> g_fontSizeMap;

void DirectFontInitialize(IDXGISwapChain* swapChain,HWND hwnd)
{
	// 初期化処理
	HRESULT result = S_OK;

	// Direct2Dファクトリ情報の初期化
	result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗",L"Direct2Dファクトリ作成失敗" , MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// バックバッファの取得
	result = swapChain->GetBuffer(0, IID_PPV_ARGS(&g_pBackBuffer));
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗", L"バックバッファ取得失敗", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// dpiの設定
	UINT dpi = GetDpiForWindow(hwnd);
	FLOAT dpiX = dpi;
	FLOAT dpiY = dpi;

	// レンダーターゲットの作成
	D2D1_RENDER_TARGET_PROPERTIES props =
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, 
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);

	// サーフェスに描画するレンダーターゲットを作成
	result = g_pD2DFactory->CreateDxgiSurfaceRenderTarget(g_pBackBuffer, &props, &g_pRenderTarget);
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗", L"レンダーターゲット作成失敗", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// アンチエイリアシングモードの設定
	g_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

	// ブラシの作成
	result = g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &g_pFontBrush);
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗", L"ブラシ作成失敗", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// 影用ブラシの作成
	result = g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &g_pShadowBrush);
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗", L"影ブラシ作成失敗", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// アウトライン用ブラシの作成
	result = g_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &g_pOutlineBrush);
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗", L"アウトラインブラシ作成失敗", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	// IDWriteFactoryの作成
	result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory5), reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
	if (FAILED(result))
	{
		MessageBoxW(NULL, L"FontInit失敗", L"IDWriteFactory作成失敗", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	HRESULT hr = S_OK;
	WRL::ComPtr<IDWriteFontFile> pFontFile;
	hr = g_pDWriteFactory->CreateFontFileReference(path, nullptr, &pFontFile);
	if (SUCCEEDED(hr))
	{
		// FontSetBuilderの作成
		WRL::ComPtr<IDWriteFontSetBuilder1> pFontSetBuilder;
		result = g_pDWriteFactory->CreateFontSetBuilder(pFontSetBuilder.GetAddressOf());
		if (FAILED(result))
		{
			MessageBoxW(NULL, L"FontInit失敗", L"FontSetBuilder作成失敗", MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		// フォントファイルの追加
		hr = pFontSetBuilder->AddFontFile(pFontFile.Get());
		if (FAILED(hr))
		{
			MessageBoxW(NULL, L"フォントファイルの追加失敗", path, MB_ICONEXCLAMATION | MB_OK);
		}

		// フォントセットの作成
		WRL::ComPtr<IDWriteFontSet> pFontSet;
		hr = pFontSetBuilder->CreateFontSet(&pFontSet);
		if (FAILED(hr))
		{
			MessageBoxW(NULL, L"フォントセットの作成失敗", path, MB_ICONEXCLAMATION | MB_OK);
		}

		// フォントコレクションの作成
		hr = g_pDWriteFactory->CreateFontCollectionFromFontSet(pFontSet.Get(), &g_pFontCollection);
		if (FAILED(hr))
		{
			MessageBoxW(NULL, L"フォントコレクションの作成失敗", path, MB_ICONEXCLAMATION | MB_OK);
		}

		// フォント名の取得

		// フォントファミリの取得
		WRL::ComPtr<IDWriteFontFamily> pFontFamily;
		g_pFontCollection->GetFontFamily(0, &pFontFamily);

		// ローカライズされた文字列の取得
		WRL::ComPtr<IDWriteLocalizedStrings> pFamilyNames;
		pFontFamily.Get()->GetFamilyNames(&pFamilyNames);

		// ロケール名のインデックス取得
		UINT32 index = 0;
		BOOL exists = FALSE;
		hr = pFamilyNames->FindLocaleName(L"ja-jp", &index, &exists);
		if (exists == FALSE)
		{
			hr = pFamilyNames->FindLocaleName(L"en-us", &index, &exists);
		}
		if (exists == FALSE)
		{
			index = 0;
		}

		// 文字列取得
		UINT32 length = 0;
		pFamilyNames->GetStringLength(index, &length);

		// フォント名の格納
		g_fontFamilyName = (length + 1, L'\0');
		pFamilyNames->GetString(index, &g_fontFamilyName[0], length + 1);

		IDWriteTextFormat* pDefaultTextFormat = nullptr;

		// デフォルトテキストフォーマットの作成
		hr = g_pDWriteFactory->CreateTextFormat
		(
			g_fontFamilyName.c_str(),                     // フォント名
			g_pFontCollection,                  // フォントコレクション
			DWRITE_FONT_WEIGHT_NORMAL,              // 太さ
			DWRITE_FONT_STYLE_NORMAL,               // スタイル
			DWRITE_FONT_STRETCH_NORMAL,             // ストレッチ
			80.0f,                                  // フォントサイズ
			L"ja-jp",                               // ロケール
			&pDefaultTextFormat                     // テキストフォーマット
		);

		if (FAILED(hr))
		{
			MessageBoxW(NULL, L"デフォルトテキストフォーマットの作成失敗", path, MB_ICONEXCLAMATION | MB_OK);
		}

		g_fontSizeMap[80.0f] = pDefaultTextFormat;	// デフォルトサイズを登録
	}
	else
	{
		MessageBoxW(NULL, L"フォントファイルの読み込み失敗", path, MB_ICONEXCLAMATION | MB_OK);
	}
}

void DirectFontFinalize()
{
	SAFE_RELEASE(g_pDWriteFactory);
	SAFE_RELEASE(g_pRenderTarget);
	SAFE_RELEASE(g_pD2DFactory);
	SAFE_RELEASE(g_pBackBuffer);
	SAFE_RELEASE(g_pFontBrush);
	SAFE_RELEASE(g_pShadowBrush);
	SAFE_RELEASE(g_pOutlineBrush);
	SAFE_RELEASE(g_pFontCollection);
	for(auto fontPair : g_fontSizeMap)
	{
		SAFE_RELEASE(fontPair.second);
	}
}


//====================================
// 文字描画
// string：文字列
// pos：描画ポジション
// options：テキストの整形
//====================================
void WriteText(const std::string& text, Vector2 pos, Color textColor, float fontSize,
	int outlineWidth, Color outlineColor)
{
	HRESULT result = S_OK;

	// ターゲットサイズの取得
	D2D1_SIZE_F TargetSize = g_pRenderTarget->GetSize();
	// 描画の開始
	g_pRenderTarget->BeginDraw();

	// 文字列の変換
	std::wstring wstr = StringToWString(text);

	// 指定サイズのテキストフォーマットがあるか確認
	if (g_fontSizeMap.contains(fontSize) == false)
	{
		// 作成
		IDWriteTextFormat* pTextFormat = nullptr;
		result = g_pDWriteFactory->CreateTextFormat
		(
			g_fontFamilyName.c_str(),   // フォント名
			g_pFontCollection,			// フォントコレクション
			DWRITE_FONT_WEIGHT_NORMAL,  // 太さ
			DWRITE_FONT_STYLE_NORMAL,   // スタイル
			DWRITE_FONT_STRETCH_NORMAL, // ストレッチ
			fontSize,					// フォントサイズ
			L"ja-jp",                   // ロケール
			&pTextFormat				// テキストフォーマット
		);
		g_fontSizeMap[fontSize] = pTextFormat;
	}

	WRL::ComPtr<IDWriteTextLayout> pTextLayout;
	// テキストレイアウトを作成
	result = g_pDWriteFactory->CreateTextLayout(wstr.c_str(), wstr.size(), 
												g_fontSizeMap[fontSize],
												TargetSize.width, TargetSize.height, 
												pTextLayout.GetAddressOf());
	if (FAILED(result))
	{
		OutputDebugStringA("WriteText:テキストレイアウト作成失敗\n");
		return;
	}

	// 描画位置の確定
	D2D1_POINT_2F pounts;
	pounts.x = pos.x;
	pounts.y = pos.y;

	// アウトライン描画
	g_pOutlineBrush->SetColor(D2D1::ColorF
	(
		outlineColor.r,
		outlineColor.g,
		outlineColor.b,
		outlineColor.a
	));

	for (int y = -outlineWidth; y <= outlineWidth; y++)
	{
		for (int x = -outlineWidth; x <= outlineWidth; x++)
		{
			if (x == 0 && y == 0) continue; // 中心はスキップ
			D2D1_POINT_2F outlinePos;
			outlinePos.x = pounts.x + x;
			outlinePos.y = pounts.y + y;
			g_pRenderTarget->DrawTextLayout(outlinePos, pTextLayout.Get(), g_pOutlineBrush, D2D1_DRAW_TEXT_OPTIONS_NONE);
		}
	}
	// 描画処理
	g_pFontBrush->SetColor(D2D1::ColorF
	(
		textColor.r,
		textColor.g,
		textColor.b,
		textColor.a
	));

	g_pRenderTarget->DrawTextLayout(pounts, pTextLayout.Get(), g_pFontBrush, D2D1_DRAW_TEXT_OPTIONS_NONE);

	// 描画の終了
	result = g_pRenderTarget->EndDraw();
}

// stringをwstringへ変換する
std::wstring StringToWString(std::string oString)
{
	// SJIS → wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, (wchar_t*)NULL, 0);

	// バッファの取得
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];

	// SJIS → wstring
	MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2, iBufferSize);

	// stringの生成
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

	// バッファの破棄
	delete[] cpUCS2;

	// 変換結果を返す
	return(oRet);
}

IDWriteFactory5* GetDWriteFactory()
{
	return g_pDWriteFactory;
}
