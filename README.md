# LoRa Beacon (XIAO ESP32S3)

このリポジトリは、XIAO ESP32S3 + E220 + NEO-M8N + SSD1315 を用いた
LoRaビーコン端末の設計・実装を管理します。

## 設計書
- [docs/design.md](docs/design.md)

## 要点
- 2台間でGPS位置交換
- 相手方位・距離を表示
- 1ボタンUI（短押し/長押し）
- OFF/SEARCH/TRACK の状態遷移
- バッテリー駆動向け省電力設計
- 疎結合なレイヤード構成
