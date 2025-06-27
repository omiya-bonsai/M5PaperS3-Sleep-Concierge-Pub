#pragma once

// ===============================================================
// このファイルには、描画、ネットワーク、季節判定などの
// 補助的な関数がすべて含まれています。
// ===============================================================

// 外部グローバル変数の参照宣言
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

// ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼ 修正点 ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
// カレンダーの描画サイズとフォントを拡大
/**
 * @brief 月間カレンダーを描画します。
 */
void drawCalendar(int tx, int ty, int year, int month, int day) {
  const char* week_days[] = { "日", "月", "火", "水", "木", "金", "土" };
  int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
    days_in_month[1] = 29;  // うるう年
  }

  struct tm timeinfo_first = { 0 };
  timeinfo_first.tm_year = year - 1900;
  timeinfo_first.tm_mon = month - 1;
  timeinfo_first.tm_mday = 1;
  mktime(&timeinfo_first);
  int first_day_of_week = timeinfo_first.tm_wday;

  const int cell_width = 50;   // セルの幅を拡大
  const int cell_height = 45;  // セルの高さを拡大

  // カレンダーのヘッダーを描画
  canvas.setFont(&fonts::lgfxJapanGothic_32);
  canvas.setTextDatum(TC_DATUM);
  canvas.drawString(String(year) + "年 " + String(month) + "月", tx + (cell_width * 7) / 2, ty);

  ty += 50;
  canvas.setFont(&fonts::lgfxJapanGothic_24);
  for (int i = 0; i < 7; i++) {
    canvas.drawString(week_days[i], tx + i * cell_width + cell_width / 2, ty);
  }

  // 日付を描画
  ty += 45;
  int current_day = 1;
  for (int row = 0; row < 6; row++) {
    for (int col = 0; col < 7; col++) {
      if ((row == 0 && col < first_day_of_week) || current_day > days_in_month[month - 1]) {
        // 月の範囲外の日付は描画しない
      } else {
        int day_x = tx + col * cell_width + cell_width / 2;
        int day_y = ty + row * cell_height + cell_height / 2;

        // 今日の日付をハイライト
        if (current_day == day) {
          canvas.fillRect(day_x - (cell_width / 2), day_y - (cell_height / 2), cell_width - 2, cell_height - 2, BLACK);
          canvas.setTextColor(WHITE);
          canvas.drawString(String(current_day), day_x, day_y);
          canvas.setTextColor(BLACK);
        } else {
          canvas.drawString(String(current_day), day_x, day_y);
        }
        current_day++;
      }
    }
  }
}

/**
 * @brief 現在の日付から二十四節気を判定し、季節のメッセージを生成します。
 */
void getSeasonalInfo(int current_month, int current_day, String& seasonal_title, String& seasonal_phrase) {
  struct SeasonalEvent {
    int month;
    int day;
    const char* name;
    const char* phrase;
  };
  SeasonalEvent events[] = {
    { 1, 6, "小寒", "寒の入り、寒さ厳しく" }, { 1, 20, "大寒", "一年で最も寒い季節" }, { 2, 4, "立春", "春の気配、立ち始める" }, { 2, 19, "雨水", "雪解け水がぬるむ頃" }, { 3, 6, "啓蟄", "土の中の虫も目覚める" }, { 3, 21, "春分", "昼と夜の長さが同じに" }, { 4, 5, "清明", "万物清々しく、明るい季節" }, { 4, 20, "穀雨", "春雨降りて百穀を潤す" }, { 5, 6, "立夏", "夏の気配、立ち始める" }, { 5, 21, "小満", "万物が満ち始める頃" }, { 6, 6, "芒種", "稲や麦など、種を蒔く季節" }, { 6, 21, "夏至", "昼が最も長くなる頃" }, { 7, 7, "小暑", "梅雨明け、本格的な夏へ" }, { 7, 23, "大暑", "一年で最も暑い季節" }, { 8, 8, "立秋", "秋の気配、立ち始める" }, { 8, 23, "処暑", "暑さが和らぐ季節" }, { 9, 8, "白露", "草花に朝露がつき始める" }, { 9, 23, "秋分", "昼と夜の長さが同じに" }, { 10, 8, "寒露", "秋が深まり、冷たい露が" }, { 10, 23, "霜降", "朝夕に霜が降り始める" }, { 11, 7, "立冬", "冬の気配、立ち始める" }, { 11, 22, "小雪", "雪が降り始める季節" }, { 12, 7, "大雪", "本格的な冬の到来" }, { 12, 22, "冬至", "夜が最も長くなる頃" }
  };
  int current_date_val = current_month * 100 + current_day;
  SeasonalEvent most_recent_event = { 12, 22, "冬至", "夜が最も長くなる頃" };
  for (const auto& event : events) {
    if (event.month * 100 + event.day <= current_date_val) {
      most_recent_event = event;
    }
  }
  if (most_recent_event.month == current_month && most_recent_event.day == current_day) {
    seasonal_title = "今日は「" + String(most_recent_event.name) + "」";
  } else {
    seasonal_title = "直近の節目: " + String(most_recent_event.name);
  }
  seasonal_phrase = most_recent_event.phrase;
}

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

  const int H_MARGIN = 50;
  const int V_MARGIN = 40;
  int current_y = V_MARGIN;

  // Block 1: Indoor Environment
  canvas.setTextDatum(TL_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_32);
  canvas.drawString("室内環境", H_MARGIN, current_y);

  current_y += 45;
  canvas.setFont(&fonts::lgfxJapanGothic_40);
  canvas.setTextDatum(TL_DATUM);
  canvas.drawString(String(indoor_temp, 1) + "℃", H_MARGIN, current_y);
  canvas.setTextDatum(TC_DATUM);
  canvas.drawString(String(indoor_hum, 1) + "%", M5.Display.width() / 2, current_y);
  canvas.setTextDatum(TR_DATUM);
  canvas.drawString(String(indoor_co2) + "ppm", M5.Display.width() - H_MARGIN, current_y);

  // Block 2: Sleep Advice
  current_y += 90;
  canvas.setTextDatum(TL_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_32);
  canvas.drawString("快眠アドバイス", H_MARGIN, current_y);

  String mode = "---", setting = "---";
  int month = timeinfo.tm_mon + 1;
  if (indoor_temp > 0) {
    if (indoor_co2 > 1000) {
      mode = "換気";
      setting = "CO2濃度が高いため、空気の入れ替えを";
    } else if (month >= 6 && month <= 9) {
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
    } else if (month == 12 || month <= 3) {
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
    } else {
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
  current_y += 45;
  canvas.setFont(&fonts::lgfxJapanGothic_40);
  canvas.drawString(mode, H_MARGIN, current_y);
  current_y += 55;
  canvas.setFont(&fonts::lgfxJapanGothic_24);
  canvas.drawString(setting, H_MARGIN, current_y);

  current_y += 45;
  canvas.drawFastHLine(H_MARGIN, current_y, M5.Display.width() - (H_MARGIN * 2), LIGHTGREY);

  // Block 3: Calendar & Seasonal Information
  current_y += 30;
  int calendar_x = (M5.Display.width() - (50 * 7)) / 2;
  drawCalendar(calendar_x, current_y, timeinfo.tm_year + 1900, month, timeinfo.tm_mday);

  current_y += 370;  // カレンダーの高さ分を確保
  String seasonal_title, seasonal_phrase;
  getSeasonalInfo(month, timeinfo.tm_mday, seasonal_title, seasonal_phrase);
  canvas.setTextDatum(TC_DATUM);
  canvas.setFont(&fonts::lgfxJapanGothic_24);
  canvas.drawString(seasonal_title + " | " + seasonal_phrase, M5.Display.width() / 2, current_y);

  // Block 4: System Status (Footer)
  const int footer_y = M5.Display.height() - 65;
  canvas.drawFastHLine(0, footer_y - 15, M5.Display.width(), LIGHTGREY);

  canvas.setFont(&fonts::lgfxJapanGothic_24);
  canvas.setTextDatum(TC_DATUM);
  String line1_text = "BAT: " + String(M5.Power.getBatteryLevel()) + "%"
                      + "    |    "
                      + (client.connected() ? "MQTT: 接続中" : "MQTT: 切断");
  canvas.drawString(line1_text, M5.Display.width() / 2, footer_y);

  String line2_text = "室内: " + String((millis() - last_indoor_update < 600000) ? "OK" : "NG")
                      + "    |    "
                      + "屋外: " + String((millis() - last_outdoor_update < 600000) ? "OK" : "NG");
  canvas.drawString(line2_text, M5.Display.width() / 2, footer_y + 35);
  // ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲ 最終レイアウト修正 ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

  canvas.pushSprite(0, 0);
}


// ===============================================================
// Section: ネットワーク関連関数
// ===============================================================

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
