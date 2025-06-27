#pragma once

// ===============================================================
// アプリケーション設定テンプレート (config_template.h)
// ===============================================================
//
// このファイルは、`config.h`に記述すべき設定項目を示すための
// テンプレートです。
//
// このファイルを `config.h` という名前にコピーし、
// あなた自身の環境に合わせて以下の情報を編集してください。
//
// 注意: `config.h` ファイルは `.gitignore` によって
//        Gitの追跡対象から除外されています。
//

// --- Wi-Fi 接続設定 ---
// ご利用のWi-FiネットワークのSSID(名前)とパスワードを入力してください。
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- MQTT ブローカー設定 ---
// データを送受信するための中継サーバー(ブローカー)のIPアドレスとポート番号です。
const char* mqtt_server = "192.168.1.100"; // 例: あなたのMQTTブローカーのIPアドレス
const int mqtt_port = 1883;

// --- MQTT トピック設定 ---
// 室内外の温湿度データが、どの「チャンネル名(トピック)」で配信されているかを指定します。
const char* indoor_topic = "your/indoor/topic"; // 例: "living/environment"
const char* outdoor_topic = "your/outdoor/topic"; // 例: "balcony/weather"
