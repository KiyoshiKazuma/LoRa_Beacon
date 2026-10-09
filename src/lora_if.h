#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/// UART接続したLoRaモジュールの初期化、設定、送受信を行うインターフェース。
/// begin()を呼び出してから各通信関数を使用する。
class LoRa
{
public:
    /**
     * @brief UART接続したLoRaモジュールのインターフェースを生成する。
     * @param uartNum 使用するHardwareSerialのUART番号。
     * @param rxPin LoRaモジュールから受信するUART RXピン。
     * @param txPin LoRaモジュールへ送信するUART TXピン。
     * @param m0Pin LoRaモジュールの動作モード選択M0ピン。
     * @param m1Pin LoRaモジュールの動作モード選択M1ピン。
    * 戻り値はありません（コンストラクター）。
     */
    LoRa(
        uint8_t uartNum,
        uint16_t rxPin,
        uint16_t txPin,
        uint16_t m0Pin,
        uint16_t m1Pin);

    /**
     * @brief UARTを終了し、内部の排他資源を解放する。
    * 引数および戻り値はありません。
     */
    ~LoRa();

    /**
     * @brief LoRaインターフェースのコピー生成を禁止する。
     * @param other コピー元オブジェクト。
     * コピーは禁止されているため、この関数は使用できない。
     */
    LoRa(const LoRa &) = delete;

    /**
     * @brief LoRaインターフェースのコピー代入を禁止する。
     * @param other 代入元オブジェクト。
     * コピー代入は禁止されているため、この関数は使用できない。
     */
    LoRa &operator=(const LoRa &) = delete;

    /**
     * @brief GPIO、排他ロック、UARTを初期化して設定モードで起動する。
     * @param baudRate UART通信速度。省略時は9600 baud。
     * @param serialConfig UARTの通信形式。省略時はSERIAL_8N1。
     * @return 初期化成功時はtrue、排他ロックを作成できない場合はfalse。
     * @note 通信関数を呼び出す前に実行する。複数タスクからの同時呼び出しは想定しない。
     */
    bool begin(uint32_t baudRate = 9600, uint32_t serialConfig = SERIAL_8N1);

    /**
     * @brief 設定モードに切り替え、設定データをUART送信する。
     * @param config 送信する設定コマンドの先頭アドレス。
     * @param length 送信する設定データのバイト数。
     * @return 指定された全バイトを送信できた場合はtrue、それ以外はfalse。
     * @note 初期化前、configがnullptr、またはlengthが0の場合はfalseを返す。
     *       モード切替から送信完了まで他の通信関数は待機する。
     */
    bool writeConfig(const uint8_t *config, size_t length);

    /**
     * @brief 設定モードを維持して設定応答をUARTから受信する。
     * @param buffer 受信データを書き込むバッファ。
     * @param length 受信する最大バイト数。
     * @return 実際に受信したバイト数。初期化前、bufferがnullptr、または
     *         lengthが0の場合は0。
     * @note 指定長を受信するかUARTのタイムアウトになるまで待機する。
     */
    size_t receiveConfigResponse(uint8_t *buffer, size_t length);

    /**
     * @brief 通常モードに切り替え、メッセージをUART送信する。
     * @param message 送信するメッセージの先頭アドレス。
     * @param length 送信するメッセージのバイト数。
     * @return 指定された全バイトを送信できた場合はtrue、それ以外はfalse。
     * @note 初期化前、messageがnullptr、またはlengthが0の場合はfalseを返す。
     *       モード切替から送信完了まで他の通信関数は待機する。
     */
    bool sendMessage(const uint8_t *message, size_t length);

    /**
     * @brief 通常モードに切り替え、UARTからメッセージを受信する。
     * @param buffer 受信データを書き込むバッファ。
     * @param length 受信する最大バイト数。
     * @return 実際に受信したバイト数。初期化前、bufferがnullptr、または
     *         lengthが0の場合は0。
     * @note 指定長を受信するかUARTのタイムアウトになるまで待機する。
     *       受信中は共通ロックを保持するため、他の通信関数も待機する。
     */
    size_t receiveMessage(uint8_t *buffer, size_t length);

    /**
     * @brief 通常モードで現在UARTにあるデータを受信する。
     * @param buffer 受信データを書き込むバッファ。
     * @param length 受信する最大バイト数。
     * @return 実際に受信したバイト数。データがない場合や引数が無効な場合は0。
     * @note データがない場合は待機せずに戻る。
     */
    size_t receiveAvailable(uint8_t *buffer, size_t length);

private:
    HardwareSerial loraSerial;
    uint16_t u2_pin_rx;
    uint16_t u2_pin_tx;
    uint16_t u2_pin_m0;
    uint16_t u2_pin_m1;

    // モード切替とUART処理を一括して排他する共通ロック。
    SemaphoreHandle_t ioMutex;

    // 初期化後のGPIOモードを記録する。
    bool isConfigMode;
    bool initialized;

    /**
     * @brief M0/M1をHIGHにして設定モードへ切り替える。
    * 引数および戻り値はありません。
     */
    void set_configumode(void);

    /**
     * @brief M0/M1をLOWにして通常モードへ切り替える。
    * 引数および戻り値はありません。
     */
    void set_transmitmode(void);
};

#endif // LORA_DRIVER_H