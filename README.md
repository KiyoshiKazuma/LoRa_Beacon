# LoRa Beacon (XIAO ESP32S3)

このリポジトリは、XIAO ESP32S3 + E220 + NEO-M8N + SSD1315 を用いた
LoRaビーコン端末の設計・実装を管理します。

## 設計書
- [docs/design.md](docs/design.md)

## 開発環境

- エディター: Visual Studio Code
- 拡張機能: PlatformIO IDE (`platformio.platformio-ide`)
- ビルド環境: PlatformIO
- ターゲットボード: Seeed Studio XIAO ESP32S3 (`seeed_xiao_esp32s3`)
- プラットフォーム: Espressif 32 (`espressif32`)
- フレームワーク: Arduino

### セットアップ

1. Visual Studio Code に PlatformIO IDE 拡張機能をインストールします。
2. このリポジトリを Visual Studio Code で開きます。
3. PlatformIO が `platformio.ini` の設定に基づいてビルド環境を準備するのを待ちます。
4. XIAO ESP32S3 を USB で接続します。

### ビルドと書き込み

PlatformIO のターミナル、または PlatformIO Core が利用可能なターミナルで、リポジトリのルートから実行します。

```sh
# ビルド
pio run

# ボードへ書き込み
pio run --target upload

# シリアルモニター
pio device monitor
```

複数のシリアルポートがある場合は、対象ポートを指定してください（例: `pio run --target upload --upload-port COM3`、`pio device monitor --port COM3`）。ボードやフレームワークなどのプロジェクト設定は [platformio.ini](platformio.ini) を参照してください。

テストは PlatformIO のテスト機能を利用できます。テストを追加した後、`pio test` で実行します。

## 要点
- 2台間でGPS位置交換
- 相手方位・距離を表示
- 1ボタンUI（短押し/長押し）
- OFF/SEARCH/TRACK の状態遷移
- バッテリー駆動向け省電力設計
- 疎結合なレイヤード構成
