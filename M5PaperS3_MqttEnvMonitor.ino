// ===============================================================
//
//          Sleep Environment Concierge for M5PaperS3
//
// ---------------------------------------------------------------
//
//                          メインスケッチ
//
// ---------------------------------------------------------------
//
// このプログラムは、あなたの快適な睡眠をサポートするために開発された
// 「快眠環境コンシェルジュ」です。
// M5PaperS3デバイス上で動作し、MQTTプロトコルを通じて室内外の
// 環境センサーからデータ（温度、湿度、CO2濃度）をリアルタイムで取得します。
//
// 取得したデータと日本の季節（二十四節気）に基づき、内蔵された
// 快眠アルゴリズムがエアコンの最適な稼働モードを判断し、
// ディスプレイに表示します。
//
// さらに、デバイス自身のバッテリー状態や通信状況も表示することで、
// システム全体の信頼性を確保します。
//
// --- プロジェクトの構成ファイル ---
// 1. M5PaperS3_QuadrantMonitor_JP.ino (このファイル)
//    - プログラム全体の流れを制御する、最も中心的なファイル。
//      setup()とloop()関数のみを記述し、高い可読性を維持します。
//
// 2. config.h
//    - Wi-FiのSSIDやパスワードなど、あなた個人の設定情報を
//      安全に管理するためのファイルです。
//
// 3. helpers.h
//    - 画面レイアウトの描画、ネットワークへの接続、快眠アルゴリズム、
//      季節の判定など、具体的な処理を行う補助関数をすべて
//      集約したファイルです。
//
// ===============================================================


// --- ライブラリと設定ファイルの読み込み ---
#include <M5Unified.h>     // M5Stack製品を統合的に扱うための必須ライブラリ
#include <WiFi.h>          // Wi-Fi機能を利用するためのライブラリ
#include <PubSubClient.h>  // MQTT通信機能を利用するためのライブラリ
#include <ArduinoJson.h>   // MQTTで受信したJSON形式のデータを解析するためのライブラリ
#include "config.h"        // Wi-FiやMQTTの個人設定を記述したファイルを読み込む


// --- グローバル変数の定義 ---
// これらの変数は、このプロジェクト内の複数のファイル (helpers.hなど) から
// 参照されるため、グローバルスコープで定義されています。

// センサーデータ格納用
float indoor_temp = -99.9;   // 室内温度
float indoor_hum = -99.9;    // 室内湿度
int indoor_co2 = -1;         // 室内CO2濃度
float outdoor_temp = -99.9;  // 屋外温度
float outdoor_hum = -99.9;   // 屋外湿度

// データ更新管理用
bool data_updated = false;              // 新しいデータを受信したかどうかのフラグ
unsigned long last_indoor_update = 0;   // 室内データを最後に受信した時刻
unsigned long last_outdoor_update = 0;  // 屋外データを最後に受信した時刻

// --- オブジェクトのインスタンス化 ---
// 各ライブラリの機能を使うためのオブジェクトを作成します。
WiFiClient espClient;             // Wi-Fi通信の基盤となるクライアント
PubSubClient client(espClient);   // MQTT通信を行うためのクライアント
LGFX_Sprite canvas(&M5.Display);  // M5PaperS3の画面に描画するためのバッファ(キャンバス)


// --- ヘルパー関数の読み込み ---
// 描画やネットワークなどの具体的な処理は、このファイルに集約されています。
#include "helpers.h"


/**
 * @brief 初期設定
 * この関数は、デバイスの電源投入時やリセット時に一度だけ実行されます。
 * デバイスの基本的な準備と、ネットワークへの接続を行います。
 */
void setup() {
  // M5Stackデバイスのハードウェアを初期化
  auto cfg = M5.config();
  M5.begin(cfg);

  // 画面描画用のバッファ(Sprite)を作成
  // これにより、ちらつきのないスムーズな画面更新が可能になります。
  canvas.setColorDepth(1);  // 1-bit (白黒) モードに設定
  canvas.createSprite(M5.Display.width(), M5.Display.height());

  // 起動メッセージを画面に表示
  canvas.setTextDatum(MC_DATUM);  // 文字の基準点を中央に設定
  canvas.setFont(&fonts::lgfxJapanGothic_24);
  canvas.drawString("Sleep Environment Concierge", M5.Display.width() / 2, M5.Display.height() / 2 - 20);
  canvas.drawString("起動中...", M5.Display.width() / 2, M5.Display.height() / 2 + 20);
  canvas.pushSprite(0, 0);  // バッファの内容を実際の画面に転送
  delay(1500);              // メッセージを視認させるための小休止

  // --- ネットワーク関連のセットアップ ---
  setup_wifi();  // Wi-Fiに接続 (詳細はhelpers.h)

  // MQTTクライアントの設定
  client.setServer(mqtt_server, mqtt_port);  // ブローカーのアドレスとポートを設定
  client.setCallback(callback);              // メッセージ受信時の処理関数を登録

  // --- 時刻合わせ (NTP) ---
  // 正確な季節判定と更新時刻の表示のため、インターネット経由で時刻を取得します。
  configTime(9 * 3600, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
}


/**
 * @brief メインループ
 * この関数内の処理は、setup()が完了した後、デバイスの電源が切れるまで
 * 無限に繰り返し実行され続けます。
 */
void loop() {
  M5.update();  // ボタン入力など、M5Stackデバイスの状態を常に更新

  // MQTTブローカーとの接続状態を確認し、もし切断されていれば再接続を試みる
  if (!client.connected()) {
    reconnect();
    data_updated = true;  // 再接続後は、最新のステータスを表示するため画面を強制更新
  }
  client.loop();  // MQTTのメッセージ受信などを処理

  // 定期的な画面更新
  static unsigned long last_refresh = 0;
  // データが更新された、または最後の更新から5分以上経過した場合に画面を再描画
  if (data_updated || (millis() - last_refresh > 300000)) {
    Serial.println("Data updated or periodic refresh. Redrawing screen...");
    drawScreen();             // 画面描画のメイン処理 (詳細はhelpers.h)
    data_updated = false;     // 更新フラグをリセット
    last_refresh = millis();  // 最終更新時刻を記録
  }

  delay(100);  // CPU負荷を軽減し、システムを安定させるための短い待機時間
}
