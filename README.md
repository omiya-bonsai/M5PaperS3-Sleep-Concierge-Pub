# 快眠環境コンシェルジュ for M5PaperS3


## 1. プロジェクト概要

このプロジェクトは、M5Stack社のE-Ink搭載デバイス「M5PaperS3」を使用して、個人の睡眠環境を最適化することを目的とした「快眠環境コンシェルジュ」を開発するものです。

MQTTプロトコルを通じてリアルタイムで室内外の環境データ（温度、湿度、CO₂濃度）を取得し、独自の快眠アルゴリズムと日本の季節（二十四節気）に基づいて、ユーザーに最適なエアコンの稼働モードを提案します。

さらに、デバイス自身の状態（バッテリー残量や通信状況）も表示することで、信頼性の高いパートナーとして機能します。

## 2. 主な機能

* **リアルタイム環境モニタリング**: 室内外の温度、湿度、CO₂濃度をMQTTで取得し、ディスプレイに表示します。
* **インテリジェントな快眠アドバイス**: 取得したデータと季節情報に基づき、ユーザーの快適な睡眠をサポートするための最適なエアコン設定（冷房・暖房・除湿）を提案します。
* **環境ステータスゲージ**: 現在の室内環境が、快適な範囲に対してどのレベルにあるのかを、視覚的に分かりやすいゲージで表示します。
* **季節情報の表示**: 日本の美しい「二十四節気」に基づき、季節の節目とその意味をディスプレイに表示し、日々の生活に彩りを加えます。
* **システム自己診断**: バッテリー残量、MQTTサーバーとの接続状態、センサーデータの鮮度など、デバイス自身の状態を常に表示し、高い信頼性を確保します。

## 3. プロジェクトの構成

このプロジェクトは、責務を明確にするため、以下の3つのファイルに分割されています。

1.  **`M5PaperS3_MqttEnvMonitor.ino`**:
    * プログラム全体の流れを制御する、中心的なメインスケッチです。
    * `setup()`と`loop()`関数のみを記述し、高い可読性を維持しています。

2.  **`config.h`**:
    * Wi-FiのSSIDやパスワード、MQTTサーバーのアドレスなど、ユーザー個人の環境設定を格納するファイルです。
    * このファイルを編集することで、あなたの環境に合わせた設定が可能です。メインコードを共有する際、このファイルを含めないことで、個人情報を安全に保つことができます。

3.  **`helpers.h`**:
    * 画面レイアウトの描画、ネットワークへの接続、快眠アルゴリズム、季節の判定など、具体的な処理を行う補助関数をすべて集約したファイルです。
    * 機能ごとの修正や改善は、主にこのファイルを編集することで行います。

## 4. 準備するもの

### ハードウェア

* M5PaperS3
* 室内環境センサー (CO₂、温度、湿度を測定し、MQTTでデータを送信できるもの。例: M5StickC Plus + ENV III Unit / SHT30 Unit)
* 屋外環境センサー (温度、湿度を測定し、MQTTでデータを送信できるもの。例: M5StampS3 + ENV IV Unit)
* MQTTブローカー (例: ローカルネットワーク上のRaspberry Piで動作するMosquittoなど)

### ソフトウェア

* Arduino IDE
* M5Stackボードマネージャ
* 以下のArduinoライブラリ:
    * `M5Unified` by M5Stack
    * `PubSubClient` by Nick O'Leary
    * `ArduinoJson` by Benoit Blanchon

## 5. セットアップ手順

1.  **ライブラリのインストール**:
    * Arduino IDEのライブラリマネージャを開き、上記の3つのライブラリを検索してインストールします。

2.  **ファイルの配置**:
    * `M5PaperS3_MqttEnvMonitor.ino`、`config.h`、`helpers.h`の3つのファイルを、同じフォルダ内に保存します。

3.  **個人設定の編集**:
    * `config.h`ファイルを開き、あなたの環境に合わせて`ssid`、`password`、`mqtt_server`などの情報を編集します。

4.  **書き込み**:
    * Arduino IDEで`M5PaperS3_MqttEnvMonitor.ino`を開き、ボード設定を「M5PaperS3」にして、スケッチをデバイスに書き込みます。

## 6. 今後の展望

* 屋外の天気予報APIと連携し、数時間後の天気に基づいた予見的なエアコン制御提案。
* タッチスクリーンを活用した、表示モードの切り替え機能（詳細データモード、シンプルモードなど）。
* 睡眠時間中のデータをSDカードに記録し、睡眠の質を長期的に分析する機能。
