<img width="1366" height="662" alt="VirtualBox_ubuntu_24_16_09_2025_16_33_37" src="https://github.com/user-attachments/assets/2c970ea0-c5fa-4560-983f-1fdd45d7d535" /># Smart Fan with ESP32, OpenCV, and Flask

![WhatsApp Image 2025-09-16 at 18 13 44_9d492521](https://github.com/user-attachments/assets/532ecb88-4eb1-4716-b2e2-e703d213de02)

<img width="1366" height="662" alt="VirtualBox_ubuntu_24_16_09_2025_18_10_21" src="https://github.com/user-attachments/assets/3cdd126d-82ec-4c83-9c02-e7f4c847c488" />

![WhatsApp Image 2025-09-16 at 18 13 39_a372496a](https://github.com/user-attachments/assets/0d89d59c-45b0-47f3-8bd5-363c8cb0297e)

This project turns a **regular fan** into a **smart AI-powered fan**.

💡 The fan automatically turns **ON** when a person is detected in front of the camera, and turns **OFF** when nobody is present.

It combines:

* **OpenCV** (for real-time person detection via webcam or IP camera)
* **Flask + SQLite** (backend server to hold the fan state: `"on"` / `"off"`)
* **ESP32 + Relay Module** (to physically switch DC power of the fan)

---

## 🔧 Features

* Detects people using **OpenCV DNN** (TensorFlow frozen graph).
* Exposes a **REST API** with Flask to share the desired fan state.
* Stores logs of detections in **SQLite** for later analysis.
* ESP32 polls the Flask API and switches the **relay** accordingly.
* Safe DC wiring with optocoupler relay isolation.

---

## 🖥️ System Architecture

```
[Webcam/IP Camera] ---> [Ubuntu + OpenCV + Flask + SQLite] ---> [REST API]
                                                                |
                                                                v
                                                       [ESP32 + Relay] ---> [Fan ON/OFF]
```

---

## ⚡ Hardware Required

* ESP32 board (3.3V logic)
* 5V Relay Module 
* DC Fan (DC 5V / 10V depending on Fan type)
* Jumper wires + breadboard / terminal block
* USB power adapter for ESP32
* Optional: Fuse + insulated junction box (for safety)

---

## 📂 Project Structure

```
/fan/
│── app.py                # Flask backend (API + detection)
│── detect.py             # OpenCV detection loop
│── frozen_inference_graph.pb   # TensorFlow detection model (too large for GitHub)
│── ssd_mobilenet_v3.pbtxt      # Config file
│── database.db           # SQLite database (created automatically)
│── esp32_fan_control.ino # ESP32 firmware
│── README.md             # This file
```

---

## 📥 Model Download

The pre-trained TensorFlow model `frozen_inference_graph.pb` is too large for GitHub.

👉 **Download it manually here**:
[TensorFlow Model Zoo - SSD MobileNet V3](http://download.tensorflow.org/models/object_detection/ssd_mobilenet_v3_large_coco_2020_01_14.tar.gz)

Steps:

1. Download and extract the archive.
2. Copy `frozen_inference_graph.pb` into the same folder.

   ```
   /smart-fan/models/frozen_inference_graph.pb
   ```

---

## ⚙️ Setup Instructions

### 1. Ubuntu / Linux (Server side)

```bash
# Clone the repo
git clone https://github.com/your-username/fan.git
cd fan

# Create venv
python3 -m venv venv
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

Run Flask + Detection:

```bash
python app.py
```

The server runs at: `http://<your-ubuntu-ip>:8080`

---

### 2. ESP32 (Firmware)

1. Open `esp32_fan_control.ino` in Arduino IDE.
2. Update your **Wi-Fi SSID and password** in the code.
3. Update your **serverBase** IP to match the Ubuntu machine.
4. Upload to ESP32.

---

### 3. Wiring (Relay + Fan)

```
 WALL AC            RELAY               FAN
  (220V)          +-------+           +------+
   L  ----------->| COM   |----NO---->|  L   |
   N  ------------------------------->|  N   |
   E  ------------------------------->|  E   |
                 +-------+
                 |  IN   | <--- GPIO (ESP32)
                 | VCC   | <--- 5V
                 | GND   | <--- GND
                 +-------+
```

⚠️ **Warning**: Mains AC is dangerous.

* Always enclose relay + wiring in an insulated box.
* Use a fuse for extra protection.
* Never touch AC lines directly.

---

## 🧪 Testing

1. Start the Flask server.
2. Place a person in front of the camera.
3. Check API endpoint:

   ```bash
   curl http://<your-ubuntu-ip>:8080/api/desired_state
   ```

   Should return:

   ```json
   { "desired": "on" }
   ```
4. ESP32 polls this API and switches relay accordingly.

---

## 🛠️ Future Improvements

* Add a web dashboard to view detection logs.
* Support multiple fans/devices.
* Use MQTT instead of REST API for faster communication.
* Add mobile app notifications.

---






## 📜 License

MIT License – feel free to use and modify.

---


🛠️ Notes

opencv-python and opencv-contrib-python both included (so you have full OpenCV features like DNN).

tensorflow gives you the environment to run the SSD MobileNet v3 frozen graph.

sqlalchemy for managing SQLite more cleanly (even if you’re using raw SQLite, it’s good for extending later).

flask-cors in case you want to connect from different frontends (React, mobile, etc.).

gunicorn lets you deploy Flask more “production style” later.

pip install -r requirements.txt
