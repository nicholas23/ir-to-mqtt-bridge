# Xiaomi Fan IR Control via MQTT & Home Assistant

## 專案介紹

你有一個沒有紅外線遙控功能的小米風扇，但你又想用傳統遙控器來控制它嗎？這個專案提供了一個完整的 DIY 解決方案，讓你透過 **ESP8266**、**MQTT** 和 **Home Assistant**，將任何紅外線遙控器的訊號轉換成智慧家庭指令，進而控制小米風扇。

---

## 專案原理

專案的核心思想是建立一個「紅外線轉 MQTT 橋接器」。整個系統流程如下：

1.  **紅外線接收 (ESP8266):** 一個搭載紅外線接收模組的 ESP8266 裝置，會持續監聽紅外線遙控器發出的訊號。
2.  **訊號轉換 (ESP8266):** 裝置接收到訊號後，會將其解碼成一串十六進位碼（例如 `0xDEADBEEF`）。
3.  **訊息傳輸 (MQTT):** 解碼後的十六進位碼會被傳送到你的 Home Assistant 內建的 MQTT 伺服器，並發布到一個特定的主題（Topic）上。
4.  **智慧控制 (Home Assistant):** Home Assistant 訂閱了這個 MQTT 主題。當它收到一個指定的紅外線碼時，就會觸發一個預設的自動化指令，去控制你已經設定好的小米風扇。

---

## 硬體與軟體需求

### 硬體

* **ESP8266** 開發板（例如：Wemos D1 Mini、NodeMCU 等）
* **紅外線接收模組**
* 杜邦線

### 軟體

* **Home Assistant** 實例 (需要開啟 MQTT 整合功能)
* **Arduino IDE** 或其他相容開發環境
* **`IRremoteESP8266` 函式庫** (透過 Arduino IDE 的函式庫管理器安裝)

---

## 安裝與設定教學

### 步驟 1: 設定 ESP8266

1.  將 ESP8266 裝置與紅外線接收模組連接。
2.  在你的 Arduino IDE 中，打開 `IrToMqtt.ino` 檔案。
3.  修改檔案中的 Wi-Fi 和 MQTT 伺服器設定，填入你的網路資訊。**請注意，MQTT 伺服器位址應為你的 Home Assistant IP 位址。**
4.  使用 Arduino IDE 將程式碼燒錄到 ESP8266 裝置。

### 步驟 2: 取得紅外線代碼

在你將程式碼燒錄到 ESP8266 後，打開 **Arduino IDE 的序列埠監控視窗 (Serial Monitor)**。當你對著紅外線接收器按下遙控器按鈕時，你應該能在監控視窗中看到對應的十六進位代碼。

**請記錄下這些代碼，它們在下一步的 Home Assistant 設定中會用到。**

### 步驟 3: 設定 Home Assistant 自動化

1.  在你的 Home Assistant 配置目錄中，打開 `automations.yaml` 檔案。
2.  複製以下範例，並根據你記錄下的紅外線代碼和你的小米風扇實體名稱 (`entity_id`) 進行修改。
3.  **重新啟動 Home Assistant**，以套用新的自動化設定。

```yaml
# 範例：使用紅外線遙控器開關風扇
- id: 'ir_remote_turn_on_fan'
  alias: 'IR Remote Turn On Fan'
  trigger:
    - platform: mqtt
      topic: '/webduino/irrecv'
      payload: '0x你的開關代碼'
  action:
    - service: fan.turn_on
      target:
        entity_id: fan.your_xiaomi_fan_entity_id
# 範例：使用紅外線遙控器改變風速
- id: 'ir_remote_change_fan_speed'
  alias: 'IR Remote Change Fan Speed'
  trigger:
    - platform: mqtt
      topic: 'ir_remote/signal'
      payload: '0x你的風速代碼'
  action:
    - service: fan.set_speed
      data:
        entity_id: fan.your_xiaomi_fan_entity_id
        speed: 'high'
```
