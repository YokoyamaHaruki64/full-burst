#pragma once
#include <vector>

// フレームタイマークラス
// static変数
// totalFrame : シーン始まってからのフレーム
// sceneFrame : ゲーム始まってからのフレーム
// オブジェクトのInitializeでSetIntervalか、
// オブジェクトのコンストラクタ内、イニシャライザでコンストラクタを書くことで
// 自由にtrueを返す間隔を設定できる
// インスタンスはnewで作らずクラス変数として持たせること！
class FrameTime
{
    static std::vector<FrameTime*>* instancies;
public:
    static int sceneFrame; // シーン始まってからのフレーム
    static int totalFrame; // ゲーム始まってからのフレーム
    int instanceFrame;     // インスタンス生成からのフレーム

    // ゲーム起動時の一回だけ呼ぶ
    static void InitializeFrameTime();
    // sceneの始まりに呼ぶ
    static void InitializeSceneFrameTime();
    // ゲームの最後に呼ぶ
    static void FinalizeFrameTime();
    // 毎フレーム呼ぶ
    static void UpdateFrameTime();

    // インスタンス用タイマー機能(引数の時間ごとにtrueを返す)(デフォルト:180F)
    FrameTime(int interval = 180);
    ~FrameTime();

    // タイマーをリセットし、経過時間を0に戻す。
    // 次のカウントは現在時刻から再スタート。
    void Reset() { m_elapsed = 0; }

    // 強制的にタイマーを「完了」状態として扱い、
    // 呼び出し時にタイマーをリセットして再スタートする。
    bool FinishAndReset()
    {
        m_elapsed = 0;
        return true;
    }

    // タイマーの間隔（インターバル時間）を再設定する。
    // 単位:秒
    void SetInterval(int interval) { m_interval = interval; }

    // タイマーが指定したインターバルに達したかを判定する。
    // 経過時間がインターバル以上ならtrueを返し、
    // 超過分を保持したまま経過時間をリセットして再スタートする。
    bool IsFinished()
    {
        if (m_elapsed >= m_interval)
        {
            m_elapsed -= m_interval;
            return true;
        }
        return false;
    }

	// タイマーの間隔（インターバル時間）を取得する
	int GetInterval() const { return m_interval; }

	// 経過時間を取得する
	int GetElapsed() const { return m_elapsed; }
private:
    int m_interval;
    int m_elapsed;
};

