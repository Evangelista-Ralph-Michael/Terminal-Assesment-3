🔧 How It Works

This Smart Energy Management System allows users to monitor energy usage, control appliances, and track air quality in real time through a web interface. Here's a breakdown of the workflow:

1.  Connect ESP32 to Wi-Fi
- Upload the provided Arduino code to the ESP32 board.
- Upon startup, the ESP32 connects to your local Wi-Fi network.
- It begins reading data from:
  - MQ2 Sensor (for smoke/LPG detection)
  - MQ135 Sensor (for air quality monitoring)

2. Real-Time Communication via Firebase
- Sensor values and appliance states are sent to Firebase Realtime Database
- The Firebase config is preloaded in the web app and Arduino sketch.
- All updates (sensor readings and device toggles) are instantly synced across devices.

 3. Web Dashboard Access
- Open the `index.html` file in any browser or deploy it on a local/server environment.
- The dashboard allows:
  - Toggling appliance (bedroom, kitchen, living room)
  - Turning sensors on/off
  - Viewing current sensor values
  - Monitoring energy usage every minute
  - Checking the estimated electricity bill

4.  Usage Logging and Billing
- The JavaScript frontend calculates:
  - Watt-minute values for each appliance.
  - Aggregates the values and converts them to kWh.
  - Estimates the electricity bill (default rate: ₱10.00 per kWh).

5. Air Quality Classification
- Sensor readings are categorized for user clarity:
  - MQ2: Safe, Alarming, Dangerous
  - MQ135: Excellent, Good, Moderate, Unhealthy, Very Unhealthy
- Warnings are displayed if values exceed safe thresholds.
