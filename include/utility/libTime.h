#pragma once
#include <vector>
#include <chrono>

// タイマークラス
// static変数
// deltaTime : シーン始まってからのフレーム
// totalTime : シーン始まってからのフレーム
// sceneTime : ゲーム始まってからのフレーム
// オブジェクトのInitializeでSetIntervalか、
// オブジェクトのコンストラクタ内、イニシャライザでコンストラクタを書くことで
// 自由にtrueを返す間隔を設定できる
// インスタンスはnewで作らずクラス変数として持たせること！
class Time
{
    static std::chrono::high_resolution_clock::time_point lastFrame;
    static std::vector<Time*>* instancies;
public:
    static float deltaTime; // フレーム間の時間
    static float sceneTime; // シーン始まってからの時間
    static float totalTime; // ゲーム始まってからの時間
    float instanceTime;     // インスタンス生成からの時間

    // ゲーム起動時の一回だけ呼ぶ
    static void InitializeTime();
    // sceneの始まりに呼ぶ
    static void InitializeSceneTime();
    // ゲームの最後に呼ぶ
    static void FinalizeTime();
    // 毎フレーム呼ぶ
    static void UpdateTime();

    // インスタンス用タイマー機能(引数の時間ごとにtrueを返す)
    // (デフォルト:3秒)
    Time(float interval = 3.0f);
    ~Time();

    // タイマーをリセットし、経過時間を0に戻す。
    // 次のカウントは現在時刻から再スタート。
    void Reset() { m_elapsed = 0.0f; }

    // 強制的にタイマーを「完了」状態として扱い、
    // 呼び出し時にタイマーをリセットして再スタートする。
    bool FinishAndReset()
    {
        m_elapsed = 0.0f;
        return true;
    }

    // タイマーの間隔（インターバル時間）を再設定する。
    // 単位:秒
    void SetInterval(float interval) { m_interval = interval; }

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

    // 経過時間を取得する
    float GetElapsed() const { return m_elapsed; }

	// インターバルの取得
	float GetInterval() const { return m_interval; }

private:
    float m_interval;
    float m_elapsed;


};