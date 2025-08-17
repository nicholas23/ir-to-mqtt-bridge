# Xiaomi Fan IR Control via MQTT & Home Assistant

## 專案介紹

想用傳統的紅外線遙控器，來控制沒有遙控功能的小米智慧風扇嗎？本專案提供一個完整的 DIY 解決方案，讓您透過 **ESP8266**、**MQTT** 和 **Home Assistant**，將任何紅外線遙控器的訊號轉換成智慧家庭指令。

---

## 專案原理

專案的核心是建立一個「紅外線轉 MQTT 橋接器」，系統流程如下：

1.  **紅外線接收 (ESP8266):** 搭載紅外線接收模組的 ESP8266 裝置，持續監聽遙控器訊號。
2.  **訊號轉換 (ESP8266):** 接收到訊號後，解碼成十六進位碼 (例如 `0xDEADBEEF`)。
3.  **訊息傳輸 (MQTT):** 解碼後的代碼透過 MQTT 協議，發布到 Home Assistant 內的 MQTT 伺服器特定主題 (Topic) 上。
4.  **智慧控制 (Home Assistant):** Home Assistant 訂閱該主題，當收到指定代碼時，觸發預設的自動化指令，進而控制小米風扇。

---

## 硬體與軟體需求

### 硬體

*   **ESP8266** 開發板 (例如：Wemos D1 Mini, NodeMCU)
*   **紅外線接收模組**
*   杜邦線

### 軟體

*   **Home Assistant** 實例 (需已設定並啟用 MQTT 整合)
*   **Arduino IDE** 或其他相容開發環境
*   **`IRremoteESP8266`** 函式庫 (可透過 Arduino IDE 函式庫管理器安裝)

---

## 安裝與設定教學

### 步驟一：硬體連接與韌體燒錄

1.  **連接硬體：** 將 ESP8266 開發板與紅外線接收模組正確連接。
2.  **設定程式碼：** 在 Arduino IDE 中打開 `IrToMqtt.ino` 檔案。
3.  **填入您的資訊：** 修改檔案中的 Wi-Fi 網路名稱 (SSID)、密碼，以及 MQTT 伺服器設定。**注意：MQTT 伺服器位址通常是您的 Home Assistant IP 位址。**
4.  **燒錄韌體：** 使用 Arduino IDE 將程式碼上傳並燒錄到 ESP8266 裝置。

### 步驟二：取得紅外線代碼

燒錄完成後，您可以透過 **Arduino IDE 的序列埠監控視窗 (Serial Monitor)** 查看初步的紅外線代碼。

**重要提示：** 序列埠監控視窗中顯示的代碼，可能與 Home Assistant 最終收到的 MQTT 訊息**不完全相同**（例如大小寫或缺少前導零）。

**最可靠的方法是**，在 Home Assistant 中使用 **開發者工具 -> MQTT** 來監聽您的主題 (`your/mqtt/topic`)，並直接從那裡複製觸發的**酬載 (payload)**。

**請將您想使用的按鈕代碼 (payload) 記錄下來，下一步將會用到。**

### 步驟三：設定 Home Assistant

#### 3.1 前置作業：安裝小米整合 (Xiaomi Integration)

在建立自動化之前，請確保您的小米風扇已經可以被 Home Assistant 控制。若尚未設定，請依以下步驟操作：

1.  **安裝 Terminal & SSH 附加元件：** 進入 Home Assistant 的附加元件商店，安裝 "Terminal & SSH"。
2.  **安裝 HACS：** 啟動 Terminal，並執行以下指令安裝 HACS (Home Assistant Community Store)，若已安裝可略過。
    ```bash
    wget -q -O - https://install.hacs.xyz | bash -
    ```
3.  **安裝小米整合：** 進入 HACS 商店，搜尋並安裝 `Xiaomi Miot Auto` 這個社群整合。
4.  **設定整合：** 根據整合的說明，新增並連結您的小米帳號。完成後，您的小米設備應會出現在 Home Assistant 的實體清單中。

#### 3.2 建立自動化腳本

1.  在您的 Home Assistant 配置目錄中，找到並打開 `automations.yaml` 檔案。
2.  **注意：** `payload` 的值**必須**與您在**步驟二**從 MQTT 工具中監聽到的**原始酬載完全一致**。
3.  複製以下範例，並根據您記錄下的紅外線代碼，以及您風扇的實體 ID (`entity_id`) 進行修改。
4.  儲存檔案後，**重新啟動 Home Assistant** 以載入新的自動化設定。

```yaml
# 範例：使用紅外線遙控器開關風扇
- id: 'ir_remote_toggle_fan_1633888888'
  alias: 'IR Remote - Toggle Fan'
  trigger:
    - platform: mqtt
      topic: 'your/mqtt/topic' # 請填寫您在 .ino 檔中設定的主題
      payload: 'deadbeef' # 範例代碼，請替換成您從 MQTT 工具監聽到的確切酬載
  action:
    - service: fan.toggle # 使用 toggle 可以方便地切換開關狀態
      target:
        entity_id: fan.your_xiaomi_fan_entity_id # 請替換成您的風扇實體ID

# 範例：使用紅外線遙控器調整風速
# 如果一個代碼 (例如 0x03FFFFFF) 在 MQTT 中顯示為 3ffffff，請直接使用該字串
- id: 'ir_remote_set_fan_speed_1633888999'
  alias: 'IR Remote - Set Fan Speed'
  trigger:
    - platform: mqtt
      topic: 'your/mqtt/topic' # 請填寫您在 .ino 檔中設定的主題
      payload: '3ffffff' # 範例代碼，此為 0x03FFFFFF 的可能實際酬載
  action:
    - service: fan.set_percentage
      target:
        entity_id: fan.your_xiaomi_fan_entity_id # 請替換成您的風扇實體ID
      data:
        percentage: 30 # 設定為 30% 的風速
```
