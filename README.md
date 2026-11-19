# # FULL BURST!!

Direct3D 11 と C++ で制作した Windows 向けの 3D ゲームです。プレイヤーとボスを中心に、レーザー攻撃、カメラ、制限時間、UI を実装しています。

- `source/boss.cpp` と `source/B_*.cpp` にボスとレーザー攻撃の処理があります。
- `source/base/` にモデル読み込み、テクスチャ、シェーダーなどの基盤があり、HLSL のソースは `shader/` にあります。

Windows と Visual Studio の C++ 環境（MSVC v143）で `full-burst.sln` を開きます。元の構成では、プロジェクト直下の `assimp/` と `imgui/`、`include/base/` の DirectXTex ヘッダー、`library/` のリンク用ライブラリが必要です。実行には Assimp の DLL、`asset/` の素材、`shader/build/` のコンパイル済みシェーダーも必要です。外部依存物と素材は Git の対象から外しています。

## コミット履歴について

就職活動で制作物を紹介するため、制作後に Git リポジトリとして整理しました。そのため、制作中の変更はコミット履歴に残っておらず、コミット数も少なくなっています。
