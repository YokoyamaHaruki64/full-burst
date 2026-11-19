#include <SDKDDKver.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <mmsystem.h>
#include <time.h>
#include "main.h"
#include "directx.h"
#include "utility.h"
#include "mouse.h"
#include "sound.h"
#include "shader.h"
#include "keyboard.h"
#include "input.h"
#include "texture.h"
#include "sprite.h"
#include "game.h"
#include "title.h"
#include "result.h"
#include "fade.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "pause.h"

#pragma comment (lib,"winmm.lib")

#define WINDOW_STYLE	WS_OVERLAPPEDWINDOW
//#define WINDOW_STYLE	WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX

//ウィンドウ情報
static constexpr char WINDOW_CLASS[] = "GameWindow";	//	メインウィンドウクラス名
static constexpr char TITLE[] = "FULL BURST!";		//タイトルバーのテキスト

static Scene g_scene = SCENE_GAME;
static bool g_cursorClip = true;

static bool g_isEndApplication = false;	//アプリケーション終了フラグ
static bool g_initialized = false;	//初期化フラグ

#ifdef _DEBUG
int g_CountFPS;		// FPSカウンタ
char g_DebugStr[2048] = "ウィンドウ表示";	//デバッグ文字表示用
#endif // DEBUG

//ウィンドウプロシージャ プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Initialize(HWND hwnd);
void Finalize();
void Update();
void Draw();


//これでWindowsアプリケーションのプログラムになる
//HINSTANCEはアプリケーションのハンドル
int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	srand((unsigned)time(NULL));

	//ウィンドウクラスの登録
	WNDCLASSEX wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;//関数ポインタ
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL;
	//wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	RegisterClassEx(&wcex);

	//クライアント領域のサイズを持った矩形、左からLeft,Top,Right,Bottom
	RECT window_rect = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT};
	//ウィンドウのスタイル
	DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	//指定したクライアント領域を確保するために新たな矩形座標を計算
	AdjustWindowRect(&window_rect, WINDOW_STYLE, FALSE);

	//新たなウィンドウの矩形座標から幅と高さを計算
	int window_w = window_rect.right - window_rect.left;
	int window_h = window_rect.bottom - window_rect.top;

	//メインウィンドウの作成
	//int wdSetting = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	HWND hWnd = CreateWindow(WINDOW_CLASS, TITLE, WINDOW_STYLE,
		150, 30, window_w, window_h, nullptr, nullptr, hInstance, nullptr);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	Initialize(hWnd);

	// 時間計測用
	DWORD dwExacLastTime;
	DWORD dwFPSLastTime;
	DWORD dwCurrentTime;
	DWORD dwFrameCount;

	// フレームカウント初期化
	timeBeginPeriod(1);// 分解能の設定
	dwExacLastTime = dwFPSLastTime = timeGetTime();	// システム時刻の取得(ミリ秒単位)
	dwCurrentTime = dwFrameCount = 0;

	//メッセージループ
	MSG msg;
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))//ウィンドウメッセージが来た場合の処理
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);	//WndProc呼び出し
		}
		else //ゲームの処理
		{
			dwCurrentTime = timeGetTime();

			if ((dwCurrentTime - dwFPSLastTime) >= 1000)// 1秒ごとに実行
			{
#ifdef _DEBUG
				g_CountFPS = dwFrameCount;	// 表示用変数にフレームカウントを格納,1秒ごとにこの処理が行われるのでFPSになる
#endif // DEBUG
				dwFPSLastTime = dwCurrentTime;
				dwFrameCount = 0;	// フレーム数をリセット
			}

			if ((dwCurrentTime - dwExacLastTime) >= (1000 / 60)) // 1/60秒ごとに実行
			{
				dwExacLastTime = dwCurrentTime;

#ifdef _DEBUG	// FPS表示
				wsprintf(g_DebugStr,TITLE);
				wsprintf(&g_DebugStr[strlen(g_DebugStr)],"FPS:%d",g_CountFPS);
				SetWindowText(hWnd,g_DebugStr);
#endif // DEBUG

				Update();
				Draw();

				if(g_isEndApplication)
				{
					DestroyWindow(hWnd);
				}

				dwFrameCount++;	// フレーム数を加算
			}


		}

	} while (msg.message != WM_QUIT);

	Finalize();

	return (int)msg.wParam;
}

//ウィンドウプロシージャ
//メッセージ入力があった時に勝手に呼ばれる
//uMsgはメッセージの種類
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//  ImGuiのメッセージ処理
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return 0;

	switch (uMsg)
	{
	case WM_DESTROY:		//ウィンドウ破棄メッセージ
		PostQuitMessage(0);	//WM_QUITメッセージの送信
		break;


	case WM_KEYDOWN:
		//if (wParam == VK_ESCAPE)
		//{
		//	SendMessage(hWnd, WM_CLOSE, 0, 0);
		//}
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(uMsg,wParam,lParam);
		break;

	case WM_CLOSE:
	{
		// カーソルを確実に表示
		Mouse_SetVisible(true);

		int id = MessageBox(hWnd, "終了しますか？", "Information", MB_YESNO | MB_ICONASTERISK | MB_DEFBUTTON2);

		switch (id)
		{
		case IDYES:
			g_isEndApplication = true;
			break;
		case IDNO:
			// キャンセル
			break;
		default:
			break;
		}

		// カーソルを元に戻す（非表示）
		Mouse_SetVisible(false);
	}

	case WM_ACTIVATEAPP:
	{
		BOOL fActive = (BOOL)wParam;
		if (fActive && g_initialized)
		{
		}
		else
		{
			// アプリが非アクティブになった
			g_cursorClip = false;
		}
	}
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse_ProcessMessage(uMsg, wParam, lParam);
		break;
	default:
		//通常のメッセージ処理
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

void Initialize(HWND hWnd)
{
	//COM初期化
	Mouse_Initialize(hWnd);

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	Keyboard_Initialize();
	Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);
	Mouse_SetVisible(false);
	InitSound(hWnd);


	Time::InitializeTime();
	FrameTime::InitializeFrameTime();
	DirectXInitialize(hWnd);
	Shader_Initialize(DirectXGetDevice(), DirectXGetDeviceContext());
	TextureInitialize(DirectXGetDevice());

	// ImGui初期化
	//  バージョンの確認
#ifdef _DEBUG

	IMGUI_CHECKVERSION();
	//  コンテキストの作成
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // キーボードによるナビゲーションの有効化
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // コントローラーによるナビゲーションの有効化

	//  Win32用の初期化
	ImGui_ImplWin32_Init(hWnd);
	//  DirectX11用の初期化
	ImGui_ImplDX11_Init(DirectXGetDevice(), DirectXGetDeviceContext());

#endif // _DEBUG

	InitRandom();
	SpriteInitialize();
	FadeInitialize();

	g_scene = SCENE_NONE;

	SetScene(SCENE_TITLE);

	g_initialized = true;
}

void Finalize()
{
	SetScene(SCENE_NONE);

	FadeFinalize();
#ifdef _DEBUG
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext(); 
#endif // _DEBUG

	Mouse_Finalize();
	TextureFinalize();
	Shader_Finalize();
	DirectXFinalaize();
	UninitSound();
}

void Update()
{
	Mouse_Update();

#ifdef _DEBUG

	// ImGuiフレーム開始
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#endif // DEBUG

	switch (g_scene)
	{
	case SCENE_TITLE:
		TitleUpdate();
		break;

	case SCENE_GAME:
		GameUpdate();
		break;

	case SCENE_RESULT:
		ResultUpdate();
		break;

	default:
		break;
	}

	Time::UpdateTime();
	FrameTime::UpdateFrameTime();
	FadeUpdate();
	Keyboard_Update();

	if (IsGamePaused())
	{
		// ゲームが一時停止中の場合はカーソルを表示
		Mouse_SetVisible(true);
		Mouse_SetMode(MOUSE_POSITION_MODE_ABSOLUTE);
	}
	else
	{
		// ゲームが一時停止中でない場合はカーソルを非表示
		Mouse_SetVisible(false);
		Mouse_SetMode(MOUSE_POSITION_MODE_RELATIVE);
	}

}

void Draw()
{
	Clear();

	switch (g_scene)
	{
	case SCENE_TITLE:
		TitleDraw();
		break;
	case SCENE_GAME:
		GameDraw();
		break;
	case SCENE_RESULT:
		ResultDraw();
		break;
	default:
		break;
	}

	FadeDraw();
#ifdef _DEBUG
	//  ImGuiの描画処理
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif // DEBUG
	Present();

}

void SetScene(Scene next)
{
	switch (g_scene)
	{
	case SCENE_TITLE:
		TitleFinalize();
		break;
	case SCENE_GAME:
		GameFinalize();
		break;
	case SCENE_RESULT:
		ResultFinalize();
		break;
	default:
		break;
	}

	g_scene = next;
	Time::InitializeSceneTime();
	FrameTime::InitializeSceneFrameTime();

	switch (g_scene)
	{
	case SCENE_TITLE:
		TitleInitialize();
		break;
	case SCENE_GAME:
		GameInitialize();
		break;
	case SCENE_RESULT:
		ResultInitialize();
		break;
	default:
		break;
	}

}

const Scene GetScene()
{
	return g_scene;
}

void EndApplication()
{
	g_isEndApplication = true;
}