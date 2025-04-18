#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <BH1750.h>
#include <esp_now.h>
#include <WiFi.h>

#define DHTPIN 4      // Chân của DHT11
#define DHTTYPE DHT11 // Loại cảm biến DHT11

DHT dht(DHTPIN, DHTTYPE);      // Khởi tạo DHT11
BH1750 lightMeter;             // Khởi tạo cảm biến BH1750

typedef struct struct_message {
  float temp;
  float hum;
  float lux;
} struct_message;

struct_message sensorData;

esp_now_peer_info_t peerInfo;
uint8_t broadcastAddress[] = {0x94, 0x54, 0xc5, 0xa9, 0x36, 0x08};

bool dhtAvailable = false;
bool bh1750Available = false;


// Gọi khi gửi xong
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println("===== GỬI DỮ LIỆU =====");
  delay(200);
  Serial.print("Trạng thái gửi: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Thành công" : "Thất bại");
  delay(200);
  Serial.print("Nhiệt độ: "); Serial.print(sensorData.temp); Serial.println(" °C");
  delay(200);
  Serial.print("Độ ẩm: "); Serial.print(sensorData.hum); Serial.println(" %");
  delay(200);
  Serial.print("Ánh sáng: "); Serial.print(sensorData.lux); Serial.println(" lux");
  delay(200);
  Serial.println("========================\n");
  delay(300);
}

// Gọi khi nhận dữ liệu
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  struct_message receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  Serial.println("===== NHẬN DỮ LIỆU =====");
  delay(200);
  Serial.print("Từ địa chỉ MAC: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 0x10) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  delay(200);
  Serial.print("Nhiệt độ: "); Serial.print(receivedData.temp); Serial.println(" °C");
  delay(200);
  Serial.print("Độ ẩm: "); Serial.print(receivedData.hum); Serial.println(" %");
  delay(200);
  Serial.print("Ánh sáng: "); Serial.print(receivedData.lux); Serial.println(" lux");
  delay(200);
  Serial.println("========================\n");
  delay(300);
}


void setup() {
  Serial.begin(9600);
  dht.begin();
  Wire.begin();

  // Kiểm tra DHT11
  float testTemp = dht.readTemperature();
  float testHum = dht.readHumidity();
  if (!isnan(testTemp) && !isnan(testHum)) {
    dhtAvailable = true;
  } else {
    Serial.println("⚠️ Không tìm thấy cảm biến DHT11. Gửi giá trị mặc định.");
  }

  // Kiểm tra BH1750
  if (lightMeter.begin()) {
    bh1750Available = true;
  } else {
    Serial.println("⚠️ Không tìm thấy cảm biến BH1750. Gửi giá trị mặc định.");
  }

  // Khởi động ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("❌ Lỗi khởi tạo ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Thêm peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("❌ Không thể thêm peer");
    return;
  }
}

void loop() {
  // Đọc DHT11
  if (dhtAvailable) {
    sensorData.temp = dht.readTemperature();
    sensorData.hum = dht.readHumidity();
    if (isnan(sensorData.temp)) sensorData.temp = 0;
    if (isnan(sensorData.hum)) sensorData.hum = 0;
  } else {
    sensorData.temp = 0;
    sensorData.hum = 0;
  }

  // Đọc BH1750 nếu có
  if (bh1750Available) {
    float lux = lightMeter.readLightLevel();
    sensorData.lux = lux >= 0 ? lux : 0;
  } else {
    sensorData.lux = 0;
  }

  // Gửi dữ liệu
  esp_now_send(broadcastAddress, (uint8_t *)&sensorData, sizeof(sensorData));
  delay(1000);
}
