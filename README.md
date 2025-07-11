# Water Device Monitoring

This project is an IoT (Internet of Things) solution designed to monitor water-related devices using an ESP32 microcontroller. The system collects real-time data on both water quality and water level, and sends this information to a remote server for further analysis and monitoring.

## Features

- **Distance Measurement**: Uses an ultrasonic sensor to monitor water level or tank fill status.
- **Water Quality Monitoring**: Uses a TDS (Total Dissolved Solids) sensor to assess water quality.
- **WiFi Connectivity**: Connects to a specified WiFi network to enable data transmission.
- **Cloud Integration**: Sends sensor data to a remote server via HTTP POST requests in JSON format.
- **Change Detection**: Only transmits data when a significant change is detected in the sensor readings, optimizing network usage.
- **Visual Alerts**: Activates a red LED indicator when a significant event occurs and data is sent.

## How It Works

1. **Sensor Reading**: The ESP32 continuously reads data from the ultrasonic distance sensor and the TDS sensor.
2. **Change Detection**: Data is only sent when a significant change in water level or quality is detected.
3. **Data Transmission**: The ESP32 sends the data to a remote server endpoint using HTTP POST in JSON format.
4. **Visual Feedback**: A red LED blinks to indicate when data has been sent due to a significant event.

## Hardware Requirements

- ESP32 development board
- Ultrasonic distance sensor (e.g., HC-SR04 or compatible)
- TDS sensor
- Red LED
- Jumper wires and breadboard

## Software Requirements

- Arduino IDE or PlatformIO
- ESP32 board support installed
- WiFi credentials and API server endpoint

## Example Use Cases

- **Water Tank Monitoring**: Track the water level in reservoirs or tanks and get notified of significant changes.
- **Water Quality Monitoring**: Track the quality of water in real time and send alerts if contamination is detected.
- **Remote Data Logging**: Store water usage and quality data for later analysis.

## Getting Started

1. **Hardware Setup**: 
   - Connect the sensors and LED to the specified ESP32 pins.
   - Power up the ESP32 board.

2. **Software Setup**:
   - Configure your WiFi SSID, password, and server endpoint in the code.
   - Upload the code to your ESP32 using Arduino IDE or PlatformIO.

3. **Operation**:
   - The ESP32 will connect to WiFi, monitor the sensors, and send data to the server when significant changes are detected.

## Configuration

Edit the following lines in the source code to set your WiFi credentials and server details:
```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverName = "YOUR_SERVER_ENDPOINT";
const char* apiKey = "YOUR_API_KEY";
```

## Data Format

Data is sent to the server as a JSON object, for example:
```json
{
  "device_id": "esp32-01",
  "raw_tds": 1234,
  "raw_distance": 5678
}
```

## License

This project is intended for educational and prototyping purposes.

---

Feel free to modify and expand this project to suit your specific water monitoring needs!
