# rola beacon 設計書

## 1. 概要
本システムは 2 台の端末間で GPS 位置情報を LoRa で交換し、各端末の画面に相手端末の方位と距離を表示する。

- MCU: XIAO ESP32S3
- LoRa: E220-900T22S(JP)-EV2 (UART)
- GPS: NEO-M8N (UART)
- Display: SSD1315 128x64 (I2C)
- Battery: 18650
- UI: タクトスイッチ 1 個

---

## 2. 全体状態遷移

### 2.1 全体状態
- `OFF`: Deep Sleep（最省電力）
- `SEARCH`: LoRa 途絶/探索（省電力運用）
- `TRACK`: LoRa 通信中/追尾（高更新）

### 2.2 状態遷移条件
- `OFF -> SEARCH`: ボタン長押し（1.5 秒以上）
- `SEARCH -> TRACK`: 受信成立（例: 5 秒以内に 2 回受信）
- `TRACK -> SEARCH`: リンクタイムアウト（例: 最終受信から 8 秒超）
- `SEARCH/TRACK -> OFF`: ボタン長押し

### 2.3 代表タイマ定数（初期値）
- 長押し判定: 1500 ms
- 短押し判定: 50〜600 ms
- 画面自動消灯: 30000 ms
- SEARCH送信周期: 15000 ms
- TRACK送信周期: 1000 ms
- リンクタイムアウト: 8000 ms

---

## 3. モジュール別状態

### 3.1 MCU (ESP32S3)
- `MCU_SLEEP`（OFF）
- `MCU_ACTIVE_LOW`（SEARCH）
- `MCU_ACTIVE_HIGH`（TRACK）

### 3.2 LoRa (E220)
- `LORA_OFF`
- `LORA_RX_WINDOW`（SEARCH）
- `LORA_TX_BEACON`
- `LORA_LINKED`（TRACK）

### 3.3 GPS (NEO-M8N)
- `GPS_OFF`
- `GPS_ACQUIRE`
- `GPS_FIX`
- `GPS_INTERMIT`（SEARCH向け間欠）

### 3.4 Display (SSD1315)
- `DISP_OFF`
- `DISP_DIM`
- `DISP_ON`
- ページ: `COMPASS / DETAIL / GPS / SYS`

### 3.5 Button（1タクト）
- `BTN_SHORT`: 画面遷移、消灯解除
- `BTN_LONG`: ON/OFF切替
- （任意）`BTN_DOUBLE`: 輝度切替など

---

## 4. 1ボタンUI仕様

- 短押し（50〜600ms）
  - 画面ページ循環: `COMPASS -> DETAIL -> GPS -> SYS -> ...`
  - 画面消灯中は点灯のみ
- 長押し（>=1.5s）
  - OFF時: 起動（SEARCHへ）
  - SEARCH/TRACK時: OFF遷移（Deep Sleep）
- 無操作30秒で画面消灯
- 受信イベントまたは短押しで再点灯

---

## 5. 省電力方針

### 5.1 OFF
- ESP32S3: Deep Sleep
- GPS/LoRa/OLED: 停止（可能なら電源断）

### 5.2 SEARCH（省電力優先）
- LoRa送信周期を長くする（例: 15秒）
- LoRa受信は窓受信または低Duty
- GPSは間欠動作（例: ON 10〜20秒 / OFF 40〜50秒）
- 画面は低更新・自動消灯

### 5.3 TRACK（追尾優先）
- LoRa送信周期短縮（例: 1秒）
- GPS連続測位
- 表示更新高速化（200〜500ms）

---

## 6. ソフトウェア構成（疎結合重視）

```
src/
  app/
    App.cpp
    App.h
    StateMachine.cpp
    StateMachine.h
    Events.h
    Types.h

  domain/
    BeaconModel.h
    BeaconMath.cpp
    BeaconMath.h
    Policies.cpp
    Policies.h

  drivers/
    gps/
      IGps.h
      NeoM8nUart.cpp
      NeoM8nUart.h
    lora/
      IRadio.h
      E220Uart.cpp
      E220Uart.h
    display/
      IDisplay.h
      Ssd1315I2c.cpp
      Ssd1315I2c.h
    button/
      IButton.h
      GpioButton.cpp
      GpioButton.h
    power/
      IPower.h
      Esp32Power.cpp
      Esp32Power.h
    battery/
      IBattery.h
      AdcBattery.cpp
      AdcBattery.h
    time/
      IClock.h
      EspClock.cpp
      EspClock.h

  services/
    TelemetryService.cpp
    TelemetryService.h
    LinkService.cpp
    LinkService.h
    UiService.cpp
    UiService.h
    PowerService.cpp
    PowerService.h

  infra/
    EventBus.h
    RingQueue.h
    Config.h
    Logger.h

  main.cpp
```

---

## 7. 依存関係ルール

依存は一方向に限定する。

- `app -> services -> domain`
- `services -> drivers(interfaces)`
- `drivers(impl) -> platform libraries`

ルール:
- domain はハード依存を持たない
- driver同士は直接依存しない
- app/services は具象実装ではなく interface 経由で利用する

---

## 8. イベント駆動方針

モジュール間は直接呼び出しではなくイベントで疎結合化する。

代表イベント:
- `EvTick1s`
- `EvButtonShort`, `EvButtonLong`
- `EvGpsUpdated`
- `EvRadioPacketRx`
- `EvLinkUp`, `EvLinkDown`
- `EvDisplayTimeout`
- `EvBatteryLow`

StateMachine は「状態遷移判定と遷移アクション」に責務を限定し、UART/I2C詳細を持たない。

---

## 9. 通信データ（MVP）

固定長バイナリ例:
- `node_id` (1B)
- `seq` (2B)
- `lat_e7` (4B)
- `lon_e7` (4B)
- `fix` (1B)
- `sats` (1B)
- `crc16` (2B)

約15Bで軽量。送信時間短縮に有利。

---

## 10. 実装優先順

1. OLED表示確認
2. GPS受信確認
3. LoRa送受信確認（固定文字列）
4. GPS値をLoRa送受信
5. 距離/方位計算統合
6. 状態遷移/省電力制御統合
7. 実機電流測定で定数最適化
