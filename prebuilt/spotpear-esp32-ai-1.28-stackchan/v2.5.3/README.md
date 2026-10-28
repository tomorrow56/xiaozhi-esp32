# SpotPear StackChan 表示・リップシンク版 FW

Xiaozhi **v2.5.3** / ESP-IDF **v6.1**。
対象: **SpotPear ESP32-AI-1.28 v1.0（MAX98357A + NP0557）**、
ESP32-S3 / Flash 16 MB / Octal PSRAM 8 MB。
英語 UI / `wn9_hiesp`（Hi ESP）。

[表示・ビルド手順とライセンス](../../../main/boards/spotpear/spotpear-esp32-ai-1.28/STACKCHAN.md)

## ファイル

- `merged-binary.bin`: 全体イメージ。書き込み先 **0x0**。
- `xiaozhi.bin`: アプリ単体。このパーティション構成では **0x20000**。
- `bootloader/`、`partition_table/`、`ota_data_initial.bin`、`generated_assets.bin`:
  個別書き込み用。`flash_args` に書き込み位置を収録。
- `sdkconfig`、`dependencies.lock`、`build-info.json`: 設定とソースの識別情報。
- `SHA256SUMS`: 配布ファイルの SHA-256。

## 書き込み

Python と esptool を用意し、`PORT` を実際のシリアルポートに置き換えて、
このディレクトリで実行します。

```sh
python -m esptool --chip esp32s3 -p PORT write-flash --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 merged-binary.bin
```

または個別イメージを使用します。

```sh
python -m esptool --chip esp32s3 -p PORT write-flash @flash_args
```

既存設定を保持する必要がある場合は、事前に Flash をバックアップしてください。
配布ファイルに NVS ダンプや個体固有情報は含めていません。

## 検証状況

ESP-IDF v6.1 でのコンパイル・リンク、パーティション容量、結合イメージ生成を確認。
ホストテスト全体と、合成 PCM による無音・音量・ミュート・失効・停止・
時刻カウンター周回のリップシンクテストを実行しています。
ホストテストは 143 件で成功（依存ライブラリ不足による 3 件のスキップを含む）。
通常版の再ビルドも成功しました。具体的な結果は `build-info.json` に記録しています。

初回の StackChan 版は 2026-10-28 に書き込み、ユーザーが動作 OK と確認しました。
この更新では happy 系の表情でも目を 20 × 20 px の丸い形に保ちます。
まばたきと眠い表情は残しています。**目の形の修正後の見え方は確認待ちです。**

目の修正版はアプリ領域のみ実機へ書き込み、検証成功・StackChan/音声初期化・
Wi-Fi 再接続・待機状態への移行まで確認しています。見え方はユーザーの確認待ちです。
