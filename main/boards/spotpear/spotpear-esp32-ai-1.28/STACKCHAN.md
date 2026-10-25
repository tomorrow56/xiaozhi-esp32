# StackChan 表示・リップシンク版

SpotPear ESP32-AI-1.28 v1.0 用の別ビルドです。通常の絵文字を、
StackChan を参考にした丸い目と口の顔に置き換えます。
ブランチ: `spotpear-stackchan-lipsync`。

## 機能

- 240 × 240 の丸型画面に LVGL の図形で描画。画像や M5Stack BSP の追加は不要です。
- まばたきと小さな視線移動、happy / sad / angry / sleepy / surprised /
  thinking 系の表情に対応。他の表情名は通常の顔として表示します。
- **スピーカーへ送る再生 PCM** の RMS と設定音量から口の開きを計算します。
  20 ms 単位で計測し、25 FPS で口を更新します。マイク入力には反応しません。
- 無音判定、音量ゼロ、出力停止、更新途絶時はリップシンクの開口を解除します。
  驚きの表情では、無音でも表情として小さな丸い口を表示します。
- Wi-Fi 設定、状態表示、通知、会話字幕は既存 UI を利用します。
  プレビュー画像の表示中は顔を隠し、終了後に戻します。
- MAX98357A / NP0557 の GPIO、I2S フォーマット、サンプルレートは通常版と同じです。

これは音量に基づく開閉で、音素に応じた口形の生成ではありません。
I2S DMA の先行書き込みを考慮した約 60 ms の保持時間を使いますが、
音と表示の厳密な同期や実機での見え方は、書き込み後に確認が必要です。

## ビルド

ESP-IDF v6.1 を有効にして、リポジトリのルートで実行します。

```sh
python scripts/build.py spotpear/spotpear-esp32-ai-1.28 --name spotpear-esp32-ai-1.28-stackchan
```

日本語 UI にする場合は `--language ja-JP` を追加します。
公開ビルドは従来版と同じ英語 UI / `wn9_hiesp`（Hi ESP）です。

通常の絵文字表示に戻すビルド:

```sh
python scripts/build.py spotpear/spotpear-esp32-ai-1.28 --name spotpear-esp32-ai-1.28
```

`config.json` の新規バリアントが `CONFIG_SPOTPEAR_STACKCHAN_FACE=y` と
`CONFIG_USE_DEFAULT_MESSAGE_STYLE=y` を設定します。
メニュー設定では Xiaozhi Assistant 内の StackChan 項目を使います。
この機能は本ボードと通常メッセージ表示に限定しています。
ハードウェアの reported type は同じであり、OTA サーバーがこの表示バリアントを
自動的に選択する保証はありません。別の FW 更新で通常表示に戻ることがあります。

## 実装

- `stackchan_face.cc/.h`: `SpiLcdDisplay` を継承した描画と、
  `NoAudioCodecSimplex` を継承した再生 PCM の観測。
- `stackchan_lipsync.h`: 音量を反映した RMS、無音しきい値、失効処理。
  atomic 値で音声タスクと LVGL タスク間を受け渡し、音声タスクから UI を操作しません。
- 既存ボードの生成部分だけで表示・コーデックの型を切り替えます。
  共通の音声処理・プロトコル・アプリケーション処理は変更していません。
- 既存 CMake のボードディレクトリ自動列挙で `.cc` を取り込むため、
  CMake のソース登録追加は不要です。

## 検証

ビルド済みファイルと検証結果は
[StackChan 版 FW](../../../../prebuilt/spotpear-esp32-ai-1.28-stackchan/v2.5.3/) に記載します。
通常版のスピーカー・マイクの実機確認結果は、この表示変更版の実機確認を意味しません。
実機では起動・Wi-Fi 設定・会話字幕・まばたき・表情切り替え、
発話時の口の開閉、無音・ミュート・会話中断時の口の復帰、
スピーカー・マイク・再接続を確認してください。

## 参考・ライセンス

[M5Stack StackChan](https://github.com/m5stack/StackChan) の default avatar の
目と口の比率・表情の考え方を参考に、LVGL 用として実装しました。
参照コミット: `eff0d6b5109663faec8f1d98848b4b0e5246bbfc`。
対象は `firmware/main/stackchan/avatar/skins/default/` の
`default.cpp`、`eyes.cpp`、`mouth.cpp` です。

参照元の MIT ライセンスと著作権表示は
[STACKCHAN-LICENSE.txt](STACKCHAN-LICENSE.txt) に同梱しています。
Xiaozhi および各依存ライブラリにも、それぞれのライセンスが適用されます。
