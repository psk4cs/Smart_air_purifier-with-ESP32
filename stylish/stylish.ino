#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

// Define sensor and actuator pins
#define DHTPIN 15          // Pin connected to the DHT sensor on ESP32
#define DHTTYPE DHT11      // DHT11 sensor type
#define MQ135_PIN 34       // Pin connected to the MQ135 sensor
#define RELAY_PIN 4        // Pin connected to the relay
#define BUZZER_PIN 5       // Pin connected to the buzzer

// Initialize sensors
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char* ssid = "Infinix";      // Your WiFi SSID
const char* password = "80708091"; // Your WiFi password

int airQuality = 0;
float temperature = 0;
int threshold = 250;               // Set your pollution threshold for relay and buzzer
bool relayState = false;           // Store the current state of the relay
bool buzzerState = false;          // Store the current state of the buzzer

// Function Prototypes
void handleRoot();
void handleData();
void handleToggleRelay();
void handleToggleBuzzer();
void handleLocation();

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);    // Air purifier off initially
  digitalWrite(BUZZER_PIN, LOW);   // Buzzer off initially
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/toggle", handleToggleRelay);
  server.on("/toggleBuzzer", handleToggleBuzzer);
  server.on("/location", handleLocation);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Read sensor data
  airQuality = analogRead(MQ135_PIN);   // Read air quality
  temperature = dht.readTemperature();  // Read temperature

  // Automatically control relay and buzzer based on air quality
  if (airQuality > threshold) {
    digitalWrite(RELAY_PIN, HIGH);      // Turn on the air purifier
    digitalWrite(BUZZER_PIN, HIGH);     // Turn on the buzzer
    relayState = true;
    buzzerState = true;
  } else {
    digitalWrite(RELAY_PIN, LOW);       // Turn off the air purifier
    digitalWrite(BUZZER_PIN, LOW);      // Turn off the buzzer
    relayState = false;
    buzzerState = false;
  }

  server.handleClient();
}

// Function to handle the root page
void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Air Quality Monitor</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background-color: #f0f2f5;
      margin: 0;
      padding: 0;
    }
    .header {
      background-color: #007BFF;
      color: white;
      padding: 20px;
      text-align: center;
    }
    .container {
      margin: 20px;
    }
    .status {
      font-size: 1.5em;
      margin-bottom: 20px;
    }
    .buttons {
      margin-bottom: 20px;
    }
    button {
      padding: 15px 25px;
      font-size: 16px;
      margin: 5px;
      cursor: pointer;
      border: none;
      border-radius: 5px;
      color: white;
    }
    #relayButton {
      background-color: #28a745;
    }
    #buzzerButton {
      background-color: #dc3545;
    }
    #mapLink {
      display: block;
      margin-top: 20px;
      font-size: 1.2em;
      color: #007BFF;
      text-decoration: none;
    }
    #mapLink:hover {
      text-decoration: underline;
    }
    .info {
      margin-top: 40px;
      font-size: 1em;
      color: #333;
      text-align: center;
    }
    .info h2 {
      margin-bottom: 10px;
    }
    .info p {
      margin: 5px 0;
    }
    #chartContainer {
      width: 100%;
      max-width: 600px;
      margin: 0 auto;
    }
    canvas {
      width: 100% !important;
      max-width: 600px;
      height: auto !important;
    }
  </style>
</head>
<body>
  <div class="header">
    <h1>Air Pollution and Temperature Monitoring</h1>
  </div>
  <div class="container">
    <div id="status" class="status">Loading data...</div>
    <div class="buttons">
      <button id="relayButton" onclick="toggleRelay()">Toggle Air Purifier</button>
      <button id="buzzerButton" onclick="toggleBuzzer()">Toggle Buzzer</button>
    </div>
    <div id="chartContainer">
      <canvas id="chart"></canvas>
    </div>
    <a id="mapLink" href="#" target="_blank">View Device Location on Map</a>
    <div class="info">
      <h2>Student Information</h2>
      <p><strong>Name:</strong> Priyanshu Saxena</p>
      <p><strong>College:</strong> Dayalbagh Educational Institute</p>
      <p><strong>Course:</strong> B.Voc IoT</p>
    </div>
  </div>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    let chart;
    let latitude;
    let longitude;
    const threshold = )=====" + String(threshold) + R"=====(;

    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('status').innerHTML = 'Temperature: ' + data.temperature + ' °C | Air Quality: ' + data.airQuality;
          if (data.airQuality > threshold) {
            alert('Warning: High Pollution Detected!');
          }
          updateChart(data);
        });
    }

    function toggleRelay() {
      fetch('/toggle')
        .then(response => response.text())
        .then(state => {
          alert('Air Purifier is now ' + state);
        });
    }

    function toggleBuzzer() {
      fetch('/toggleBuzzer')
        .then(response => response.text())
        .then(state => {
          alert('Buzzer is now ' + state);
        });
    }

    function updateChart(data) {
      if (!chart) {
        const ctx = document.getElementById('chart').getContext('2d');
        chart = new Chart(ctx, {
          type: 'bar',
          data: {
            labels: ['Temperature (°C)', 'Air Quality'],
            datasets: [{
              label: 'Sensor Data',
              data: [data.temperature, data.airQuality],
              backgroundColor: ['rgba(40, 167, 69, 0.2)', 'rgba(220, 53, 69, 0.2)'],
              borderColor: ['rgba(40, 167, 69, 1)', 'rgba(220, 53, 69, 1)'],
              borderWidth: 2
            }]
          },
          options: {
            scales: {
              y: {
                beginAtZero: true
              }
            },
            responsive: true,
            maintainAspectRatio: false
          }
        });
      } else {
        chart.data.datasets[0].data = [data.temperature, data.airQuality];
        chart.update();
      }
    }

    function getLocation() {
      if (navigator.geolocation) {
        navigator.geolocation.getCurrentPosition(position => {
          latitude = position.coords.latitude;
          longitude = position.coords.longitude;
          const mapLink = document.getElementById('mapLink');
          mapLink.href = `https://www.google.com/maps?q=${latitude},${longitude}`;
          fetch(`/location?lat=${latitude}&lon=${longitude}`);
        });
      } else {
        alert('Geolocation is not supported by this browser.');
      }
    }

    setInterval(fetchData, 5000);  // Fetch data every 5 seconds
    getLocation();
  </script>
</body>
</html>
  )=====";

  server.send(200, "text/html", html);
}

// Function to serve sensor data
void handleData() {
  String json = "{\"temperature\": " + String(temperature) + ", \"airQuality\": " + String(airQuality) + "}";
  server.send(200, "application/json", json);
}

// Function to toggle relay state
void handleToggleRelay() {
  relayState = !relayState;
  digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  server.send(200, "text/plain", relayState ? "ON" : "OFF");
}

// Function to toggle buzzer state
void handleToggleBuzzer() {
  buzzerState = !buzzerState;
  digitalWrite(BUZZER_PIN, buzzerState ? HIGH : LOW);
  server.send(200, "text/plain", buzzerState ? "ON" : "OFF");
}

// Function to handle location updates
void handleLocation() {
  if (server.hasArg("lat") && server.hasArg("lon")) {
    String latitude = server.arg("lat");
    String longitude = server.arg("lon");
    Serial.println("Location updated: " + latitude + ", " + longitude);
    server.send(200, "text/plain", "Location updated");
  } else {
    server.send(400, "text/plain", "Invalid parameters");
  }
}
