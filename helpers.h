#pragma once

// ===============================================================
// ヘルパー関数ファイル (helpers.h)
// ===============================================================
//
// このファイルには、画面描画、ネットワーク接続、季節判定など、
// メインスケッチの処理を補助する具体的な関数がすべて含まれています。
//

// --- 外部グローバル変数の参照宣言 ---
// これらの変数の本体は、メインの.inoファイルに存在します。
extern LGFX_Sprite canvas;
extern WiFiClient espClient;
extern PubSubClient client;
extern float indoor_temp, indoor_hum, outdoor_temp, outdoor_hum;
extern int indoor_co2;
extern bool data_updated;
extern unsigned long last_indoor_update, last_outdoor_update;
extern const char *ssid, *password, *mqtt_server;
extern const int mqtt_port;
extern const char *indoor_topic, *outdoor_topic;


// ===============================================================
// Section: 描画関連関数
// ===============================================================

/**
 * @brief 指定された値に基づき、htop風のステータスゲージを描画します。
 * @param x ゲージ左端のX座標
 * @param y ゲージ上端のY座標
 * @param w ゲージ全体の横幅
 * @param h ゲージ全体の高さ
 * @param value 現在の値
 * @param min_val ゲージの最小値
 * @param max_val ゲージの最大値
 * @param label ゲージのラベル名 (例: "温度")
 */
void drawGauge(int x, int y, int w, int h, float value, float min_val, float max_val, const char* label) {
  canvas.setFont(&fonts::lgfxJapanGothic_20);
  canvas.setTextDatum(ML_DATUM);
  canvas.drawString(label, x, y + h / 2);

  int gauge_x = x + 70;
  int gauge_w = w - 140;
  canvas.drawRect(gauge_x, y, gauge_w, h, DARKGREY);

  float percentage = (value - min_val) / (max_val - min_val);
  if (percentage < 0) percentage = 0;
  if (percentage > 1) percentage = 1;

  int bar_w = percentage * (gauge_w - 4);
  canvas.fillRect(gauge_x + 2, y + 2, bar_w, h - 4, BLACK);

  canvas.setTextDatum(MR_DATUM);
  canvas.drawString(String(value, 0), x + w - 5, y + h / 2);
}

// ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼ 修正点 ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
// 季節情報の構造体に「説明」を追加し、ロジックを更新
/**
 * @brief 現在の日付から二十四節気を判定し、季節のメッセージを生成します。
 * @param current_month 現在の月
 * @param current_day 現在の日
 * @param seasonal_title 生成された季節のイベント名を格納する文字列 (例: 「今日は『夏至』です」)
 * @param seasonal_phrase 生成された季節の説明文を格納する文字列 (例: 「一年で最も昼が長い日」)
 */
void getSeasonalInfo(int current_month, int current_day, String& seasonal_title, String& seasonal_phrase) {
  struct SeasonalEvent {
    int month;
    int day;
    const char* name;
    const char* phrase;  // 説明文を追加
  };

  // 二十四節気のおおよその日付と説明文を定義
  SeasonalEvent events[] = {
    { 1, 6, "小寒", "寒の入り、寒さ厳しく" }, { 1, 20, "大寒", "一年で最も寒い季節" }, { 2, 4, "立春", "春の気配、立ち始める" }, { 2, 19, "雨水", "雪解け水がぬるむ頃" }, { 3, 6, "啓蟄", "土の中の虫も目覚める" }, { 3, 21, "春分", "昼と夜の長さが同じに" }, { 4, 5, "清明", "万物清々しく、明るい季節" }, { 4, 20, "穀雨", "春雨降りて百穀を潤す" }, { 5, 6, "立夏", "夏の気配、立ち始める" }, { 5, 21, "小満", "万物が満ち始める頃" }, { 6, 6, "芒種", "稲や麦など、種を蒔く季節" }, { 6, 21, "夏至", "昼が最も長くなる頃" }, { 7, 7, "小暑", "梅雨明け、本格的な夏へ" }, { 7, 23, "大暑", "一年で最も暑い季節" }, { 8, 8, "立秋", "秋の気配、立ち始める" }, { 8, 23, "処暑", "暑さが和らぐ季節" }, { 9, 8, "白露", "草花に朝露がつき始める" }, { 9, 23, "秋分", "昼と夜の長さが同じに" }, { 10, 8, "寒露", "秋が深まり、冷たい露が" }, { 10, 23, "霜降", "朝夕に霜が降り始める" }, { 11, 7, "立冬", "冬の気配、立ち始める" }, { 11, 22, "小雪", "雪が降り始める季節" }, { 12, 7, "大雪", "本格的な冬の到来" }, { 12, 22, "冬至", "夜が最も長くなる頃" }
  };

  int current_date_val = current_month * 100 + current_day;
  SeasonalEvent most_recent_event = { 12, 22, "冬至", "夜が最も長くなる頃" };

  // 今年のイベントで、今日以前のものを探す
  for (const auto& event : events) {
    if (event.month * 100 + event.day <= current_date_val) {
      most_recent_event = event;
    }
  }

  // 判定ロジック
  if (most_recent_event.month == current_month && most_recent_event.day == current_day) {
    seasonal_title = "今日は「" + String(most_recent_event.name) + "」です";
  } else {
    seasonal_title = String(most_recent_event.month) + "月" + String(most_recent_event.day) + "日は「" + String(most_recent_event.name) + "」でした";
  }
  seasonal_phrase = most_recent_event.phrase;
}
// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲ 修正点 ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲


/**
 * @brief 画面全体のレイアウトと情報を描画するメイン関数
 */
void drawScreen() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  canvas.fillSprite(WHITE);
  canvas.setTextColor(BLACK);

  const int margin = 50;

  // Block 1: Indoor Environment
  canvas.setTextDatum(TL_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_28);
  canvas.drawString("室内環境", margin, 30);
  canvas.setFont(&fonts::lgfxJapanGothic_40);

  canvas.setTextDatum(TL_DATUM);
  canvas.drawString(String(indoor_temp, 1) + "℃", margin, 65);
  canvas.setTextDatum(TC_DATUM);
  canvas.drawString(String(indoor_hum, 1) + "%", M5.Display.width() / 2, 65);
  canvas.setTextDatum(TR_DATUM);
  canvas.drawString(String(indoor_co2) + "ppm", M5.Display.width() - margin, 65);

  // Block 2: Sleep Advice
  canvas.setTextDatum(TL_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_28);
  canvas.drawString("快眠アドバイス", margin, 130);

  String mode = "---", setting = "---";
  int month = timeinfo.tm_mon + 1;
  if (indoor_temp > 0) {
    if (indoor_co2 > 1000) {
      mode = "換気";
      setting = "CO2濃度が高いため、空気の入れ替えを";
    } else if (month >= 6 && month <= 9) {  // 夏
      if (indoor_temp >= 28.0 && indoor_hum > 60) {
        mode = "冷房";
        setting = "設定26℃ | 湿度も下げます";
      } else if (indoor_hum > 60) {
        mode = "除湿";
        setting = "目標湿度 55% | 快適な眠りのために";
      } else if (indoor_temp >= 28.0) {
        mode = "冷房";
        setting = "設定27℃ | 涼しく眠れます";
      } else {
        mode = "快適";
        setting = "最高の睡眠環境です";
      }
    } else if (month == 12 || month <= 3) {  // 冬
      if (indoor_temp < 18.0) {
        mode = "暖房";
        setting = "設定20℃で身体を温めます";
      } else if (indoor_hum < 45) {
        mode = "暖房 + 加湿";
        setting = "加湿器で喉と肌の乾燥を防ぎます";
      } else {
        mode = "快適";
        setting = "冬の理想的な睡眠環境です";
      }
    } else {  // 春・秋
      if (indoor_temp >= 25.0) {
        mode = "冷房";
        setting = "設定27℃で室温を調整";
      } else if (indoor_temp < 18.0) {
        mode = "暖房";
        setting = "設定20℃で寝冷えを防止";
      } else {
        mode = "快適";
        setting = "過ごしやすい季節です";
      }
    }
  }
  canvas.setFont(&fonts::lgfxJapanGothic_40);
  canvas.drawString(mode, margin, 165);
  canvas.setFont(&fonts::lgfxJapanGothic_24);
  canvas.drawString(setting, margin, 215);

  canvas.drawFastHLine(margin, 255, M5.Display.width() - (margin * 2), LIGHTGREY);

  // Block 3: Environmental Gauges
  int gauge_y = 280;
  int gauge_h = 28;
  int gauge_w = M5.Display.width() - (margin * 2);

  drawGauge(margin, gauge_y, gauge_w, gauge_h, indoor_temp, 15, 30, "温度");
  drawGauge(margin, gauge_y + gauge_h + 15, gauge_w, gauge_h, indoor_hum, 40, 70, "湿度");
  drawGauge(margin, gauge_y + 2 * (gauge_h + 15), gauge_w, gauge_h, indoor_co2, 400, 1500, "CO2");

  canvas.drawFastHLine(margin, 410, M5.Display.width() - (margin * 2), LIGHTGREY);

  // ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼ 修正点 ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
  // Block 4: Seasonal Information
  String seasonal_title, seasonal_phrase;
  getSeasonalInfo(month, timeinfo.tm_mday, seasonal_title, seasonal_phrase);

  canvas.setTextDatum(TC_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_28);
  canvas.drawString(seasonal_title, M5.Display.width() / 2, 445);
  canvas.setFont(&fonts::lgfxJapanGothic_20);
  canvas.drawString(seasonal_phrase, M5.Display.width() / 2, 480);
  // ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲ 修正点 ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

  // Block 5: System Status
  const int bottom_y = M5.Display.height() - 30;
  canvas.drawFastHLine(0, bottom_y - 25, M5.Display.width(), LIGHTGREY);

  canvas.setFont(&fonts::lgfxJapanGothic_20);
  canvas.setTextDatum(ML_DATUM);
  canvas.drawString("BAT: " + String(M5.Power.getBatteryLevel()) + "%", margin, bottom_y);

  canvas.setTextDatum(MC_DATUM);
  canvas.drawString(client.connected() ? "MQTT: 接続中" : "MQTT: 切断", M5.Display.width() / 2, bottom_y);

  canvas.setTextDatum(MR_DATUM);
  unsigned long now = millis();
  String data_status = String("室内:") + ((now - last_indoor_update < 600000) ? "OK" : "NG")
                       + " 屋外:" + ((now - last_outdoor_update < 600000) ? "OK" : "NG");
  canvas.drawString(data_status, M5.Display.width() - margin, bottom_y);

  canvas.pushSprite(0, 0);
}


// ===============================================================
// Section: ネットワーク関連関数
// ===============================================================

/**
 * @brief Wi-Fiネットワークに接続します。
 */
void setup_wifi() {
  delay(10);
  canvas.fillSprite(WHITE);
  canvas.setTextDatum(MC_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_24);
  canvas.drawString("WiFiに接続中...", M5.Display.width() / 2, M5.Display.height() / 2);
  canvas.pushSprite(0, 0);

  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20) {
    delay(500);
    Serial.print(".");
    count++;
  }
}

/**
 * @brief MQTTメッセージを受信した際の処理（コールバック）
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.println(message);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  if (strcmp(topic, indoor_topic) == 0) {
    indoor_temp = doc["temp"];
    indoor_hum = doc["hum"];
    indoor_co2 = doc["co2"];
    last_indoor_update = millis();
    data_updated = true;
  } else if (strcmp(topic, outdoor_topic) == 0) {
    outdoor_temp = doc["temperature"];
    outdoor_hum = doc["humidity"];
    last_outdoor_update = millis();
    data_updated = true;
  }
}

/**
 * @brief MQTTブローカーに再接続します。
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    canvas.fillSprite(WHITE);
    canvas.setTextDatum(MC_DATUM);
    canvas.setFont(&fonts::lgfxJapanGothic_24);
    canvas.drawString("MQTT Brokerに接続中...", M5.Display.width() / 2, M5.Display.height() / 2 - 20);
    canvas.drawString(mqtt_server, M5.Display.width() / 2, M5.Display.height() / 2 + 20);
    canvas.pushSprite(0, 0);

    String clientId = "M5PaperS3-Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(indoor_topic);
      client.subscribe(outdoor_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
