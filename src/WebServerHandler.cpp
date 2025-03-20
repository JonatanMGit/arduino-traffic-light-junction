#include <WiFiNINA.h>
#include "TrafficLightController.h"
#include <Arduino_LSM6DS3.h>

extern WiFiServer server;
extern TrafficLightState currentState; // Declared externally

// Returns the name of the current state for display.
const char *getStateName(TrafficLightState state)
{
    switch (state)
    {
    case MAIN_GREEN:
        return "MAIN_GREEN";
    case MAIN_YELLOW:
        return "MAIN_YELLOW";
    case ALL_RED:
        return "ALL_RED";
    case SIDE_RED_YELLOW:
        return "SIDE_RED_YELLOW";
    case SIDE_GREEN:
        return "SIDE_GREEN";
    case SIDE_YELLOW:
        return "SIDE_YELLOW";
    case MAIN_RED_YELLOW:
        return "MAIN_RED_YELLOW";
    case PEDESTRIAN_GREEN:
        return "PEDESTRIAN_GREEN";
    default:
        return "UNKNOWN";
    }
}

// Handles incoming web requests
void handleWebRequests()
{
    WiFiClient client = server.available();
    if (client)
    {
        String request = client.readStringUntil('\r');
        client.flush();

        // Serve gyroscope data: returns JSON with x, y, z values
        if (request.indexOf("/gyro") != -1)
        {
            float gx = 0, gy = 0, gz = 0;
            if (IMU.gyroscopeAvailable())
            {
                if (IMU.readGyroscope(gx, gy, gz))
                {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: application/json");
                    client.println("Connection: close");
                    client.println();
                    client.print("{\"x\":");
                    client.print(gx, 2);
                    client.print(",\"y\":");
                    client.print(gy, 2);
                    client.print(",\"z\":");
                    client.print(gz, 2);
                    client.println("}");
                }
                else
                {
                    client.println("HTTP/1.1 500 Internal Server Error");
                    client.println("Content-Type: text/plain");
                    client.println("Connection: close");
                    client.println();
                    client.println("Gyroscope read error");
                }
            }
            else
            {
                client.println("HTTP/1.1 503 Service Unavailable");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");
                client.println();
                client.println("Gyroscope not available");
            }
        }
        // Serve accelerometer data: returns JSON with x, y, z values
        else if (request.indexOf("/accel") != -1)
        {
            float ax = 0, ay = 0, az = 0;
            if (IMU.accelerationAvailable())
            {
                if (IMU.readAcceleration(ax, ay, az))
                {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: application/json");
                    client.println("Connection: close");
                    client.println();
                    client.print("{\"x\":");
                    client.print(ax, 2);
                    client.print(",\"y\":");
                    client.print(ay, 2);
                    client.print(",\"z\":");
                    client.print(az, 2);
                    client.println("}");
                }
                else
                {
                    client.println("HTTP/1.1 500 Internal Server Error");
                    client.println("Content-Type: text/plain");
                    client.println("Connection: close");
                    client.println();
                    client.println("Accelerometer read error");
                }
            }
            else
            {
                client.println("HTTP/1.1 503 Service Unavailable");
                client.println("Content-Type: text/plain");
                client.println("Connection: close");
                client.println();
                client.println("Accelerometer not available");
            }
        }
        // Serve current state
        else if (request.indexOf("/state") != -1)
        {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println(getStateName(currentState));
        }
        // Set new state via AJAX
        else if (request.indexOf("/set") != -1)
        {
            int idx = request.indexOf("state=");
            if (idx != -1)
            {
                int startIdx = idx + 6;                      // Start after "state="
                int endIdx = request.indexOf(' ', startIdx); // Find the space before HTTP/1.1
                if (endIdx == -1)
                {
                    endIdx = request.length(); // If no space found, take the rest of the string
                }
                String newState = request.substring(startIdx, endIdx);
                newState.trim(); // Remove any extra whitespace
                Serial.print("Setting state to: ");
                Serial.println(newState);
                setTrafficLightState(newState); // Function defined in TrafficLightController.h
            }
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("OK");
        }
        // Ignore favicon requests
        else if (request.indexOf("/favicon.ico") != -1)
        {
            client.stop();
            return;
        }
        // Serve main webpage with grid, state info, 3D gyro demo and raw sensor displays
        else
        {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("  <meta charset='UTF-8'>");
            client.println("  <title>Verkehrsampel Status</title>");
            client.println("  <style>");
            client.println("    body { font-family: Arial, sans-serif; background-color: #f0f0f0; margin: 0; padding: 20px; }");
            client.println("    h1, h2 { text-align: center; }");
            client.println("    .grid-container {");
            client.println("      display: grid;");
            client.println("      grid-template-columns: repeat(3, 1fr);");
            client.println("      grid-template-rows: repeat(3, 1fr);");
            client.println("      grid-template-areas: ");
            client.println("        \". . A3\"");
            client.println("        \". . .\"");
            client.println("        \"A1 P A2\";");
            client.println("      gap: 10px;");
            client.println("      max-width: 600px;");
            client.println("      margin: auto;");
            client.println("    }");
            client.println("    .grid-item {");
            client.println("      border: 2px solid #ccc;");
            client.println("      border-radius: 5px;");
            client.println("      display: flex;");
            client.println("      align-items: center;");
            client.println("      justify-content: center;");
            client.println("      font-size: 1.5em;");
            client.println("      padding: 20px;");
            client.println("      box-shadow: 2px 2px 5px rgba(0,0,0,0.1);");
            client.println("      color: white;");
            client.println("    }");
            client.println("    .red { background-color: red; }");
            client.println("    .yellow { background-color: yellow; color: black; }");
            client.println("    .green { background-color: green; }");
            client.println("    .red-yellow {");
            client.println("      background: linear-gradient(to bottom, red, yellow);");
            client.println("      color: black;");
            client.println("    }");
            // Style for the 3D gyroscope display
            client.println("    #gyroContainer { margin: 40px auto; width: 200px; height: 200px; background: #eee; perspective: 800px; display: none; }");
            client.println("    #gyroDisplay { width: 100%; height: 100%; background-color: #3498db; transform-style: preserve-3d; transition: transform 0.1s ease-out; }");
            // Style for raw sensor values
            client.println("    #rawData { text-align: center; margin-top: 20px; font-size: 1.2em; display: none; }");
            client.println("    .sensor-toggle { text-align: center; margin: 20px 0; }");
            client.println("    .sensor-toggle label { cursor: pointer; }");
            client.println("  </style>");
            client.println("  <meta name='color-scheme' content='light only'>");
            client.println("</head>");
            client.println("<body>");
            client.println("  <h1>Verkehrsampel Status</h1>");
            client.println("  <div class='grid-container'>");
            client.println("    <div id='A3' class='grid-item' style='grid-area: A3;'>Ampel 3</div>");
            client.println("    <div id='A1' class='grid-item' style='grid-area: A1;'>Ampel 1</div>");
            client.println("    <div id='A2' class='grid-item' style='grid-area: A2;'>Ampel 2</div>");
            client.println("    <div id='P' class='grid-item' style='grid-area: P;'>Fußgänger</div>");
            client.println("  </div>");
            client.println("  <p style='text-align:center; margin-top:20px;'>");
            client.println("    Aktueller Zustand: <span id='state'>Lädt...</span>");
            client.println("  </p>");
            // Dropdown for state selection and button to set the state
            client.println("  <div style='text-align:center; margin-top:20px;'>");
            client.println("    <select id='stateSelect'>");
            client.println("      <option value='MAIN_GREEN'>MAIN_GREEN</option>");
            client.println("      <option value='MAIN_YELLOW'>MAIN_YELLOW</option>");
            client.println("      <option value='ALL_RED'>ALL_RED</option>");
            client.println("      <option value='SIDE_RED_YELLOW'>SIDE_RED_YELLOW</option>");
            client.println("      <option value='SIDE_GREEN'>SIDE_GREEN</option>");
            client.println("      <option value='SIDE_YELLOW'>SIDE_YELLOW</option>");
            client.println("      <option value='MAIN_RED_YELLOW'>MAIN_RED_YELLOW</option>");
            client.println("      <option value='PEDESTRIAN_GREEN'>PEDESTRIAN_GREEN</option>");
            client.println("    </select>");
            client.println("    <button onclick='setState()'>Set State</button>");
            client.println("  </div>");

            // Add checkbox to toggle gyro display
            client.println("  <div class='sensor-toggle'>");
            client.println("    <label>");
            client.println("      <input type='checkbox' id='showGyroData' onclick='toggleGyroData()'>");
            client.println("      Zeige Extra");
            client.println("    </label>");
            client.println("  </div>");

            // 3D Gyroscope display element
            client.println("  <h2>Drehung</h2>");
            client.println("  <div id='gyroContainer'>");
            client.println("    <div id='gyroDisplay'></div>");
            client.println("  </div>");
            // Raw sensor values display
            client.println("  <div id='rawData'>");
            client.println("    <p id='gyroRaw'>Gyro Raw: Loading...</p>");
            client.println("    <p id='accelRaw'>Accel Raw: Loading...</p>");
            client.println("  </div>");
            // JavaScript for fetching state and sensor data
            client.println("  <script>");
            client.println("    function updateColors(state) {");
            client.println("      const colors = {");
            client.println("        MAIN_GREEN: { A1: 'green', A2: 'red', A3: 'green', P: 'red' },");
            client.println("        MAIN_YELLOW: { A1: 'yellow', A2: 'red', A3: 'yellow', P: 'red' },");
            client.println("        ALL_RED: { A1: 'red', A2: 'red', A3: 'red', P: 'red' },");
            client.println("        SIDE_RED_YELLOW: { A1: 'red', A2: 'red-yellow', A3: 'red', P: 'red' },");
            client.println("        SIDE_GREEN: { A1: 'red', A2: 'green', A3: 'red', P: 'red' },");
            client.println("        SIDE_YELLOW: { A1: 'red', A2: 'yellow', A3: 'red', P: 'red' },");
            client.println("        MAIN_RED_YELLOW: { A1: 'red-yellow', A2: 'red', A3: 'red-yellow', P: 'red' },");
            client.println("        PEDESTRIAN_GREEN: { A1: 'red', A2: 'red', A3: 'red', P: 'green' }");
            client.println("      };");
            client.println("      const colorMap = colors[state] || { A1: 'red', A2: 'red', A3: 'red', P: 'red' };");
            client.println("      document.getElementById('A1').className = 'grid-item ' + colorMap.A1;");
            client.println("      document.getElementById('A2').className = 'grid-item ' + colorMap.A2;");
            client.println("      document.getElementById('A3').className = 'grid-item ' + colorMap.A3;");
            client.println("      document.getElementById('P').className  = 'grid-item ' + colorMap.P;");
            client.println("    }");
            client.println("    async function fetchState() {");
            client.println("      const response = await fetch('/state');");
            client.println("      let state = await response.text();");
            client.println("      state = state.trim();");
            client.println("      document.getElementById('state').innerText = 'Aktueller Zustand: ' + state;");
            client.println("      updateColors(state);");
            client.println("    }");
            client.println("    async function fetchGyro() {");
            client.println("      try {");
            client.println("        const response = await fetch('/gyro');");
            client.println("        if(response.ok) {");
            client.println("          const data = await response.json();");
            client.println("          // Update the 3D rotation based on gyro values");
            client.println("          document.getElementById('gyroDisplay').style.transform = ");
            client.println("            `rotateX(${data.x}deg) rotateY(${data.y}deg) rotateZ(${data.z}deg)`;");
            client.println("          // Update raw gyro display");
            client.println("          document.getElementById('gyroRaw').innerText = ");
            client.println("            `Gyro Raw: x=${data.x.toFixed(2)} dps, y=${data.y.toFixed(2)} dps, z=${data.z.toFixed(2)} dps`;");
            client.println("        }");
            client.println("      } catch (error) {");
            client.println("        console.error('Error fetching gyro data:', error);");
            client.println("      }");
            client.println("    }");
            client.println("    async function fetchAccel() {");
            client.println("      try {");
            client.println("        const response = await fetch('/accel');");
            client.println("        if(response.ok) {");
            client.println("          const data = await response.json();");
            client.println("          // Update raw acceleration display");
            client.println("          document.getElementById('accelRaw').innerText = ");
            client.println("            `Accel Raw: x=${data.x.toFixed(2)} g, y=${data.y.toFixed(2)} g, z=${data.z.toFixed(2)} g`;");
            client.println("        }");
            client.println("      } catch (error) {");
            client.println("        console.error('Error fetching accel data:', error);");
            client.println("      }");
            client.println("    }");
            client.println("    async function setState() {");
            client.println("      const select = document.getElementById('stateSelect');");
            client.println("      const newState = select.value;");
            client.println("      await fetch(`/set?state=${newState}`);");
            client.println("      fetchState();");
            client.println("    }");

            client.println("    // Sensor data intervals");
            client.println("    let gyroInterval = null;");
            client.println("    let accelInterval = null;");

            client.println("    function toggleGyroData() {");
            client.println("      const checked = document.getElementById('showGyroData').checked;");
            client.println("      const gyroContainer = document.getElementById('gyroContainer');");
            client.println("      const rawData = document.getElementById('rawData');");
            client.println("      ");
            client.println("      // Toggle visibility");
            client.println("      gyroContainer.style.display = checked ? 'block' : 'none';");
            client.println("      rawData.style.display = checked ? 'block' : 'none';");
            client.println("      ");
            client.println("      // Toggle data fetching");
            client.println("      if (checked) {");
            client.println("        // Initial fetch to show data immediately");
            client.println("        fetchGyro();");
            client.println("        fetchAccel();");
            client.println("        // Start intervals");
            client.println("        gyroInterval = setInterval(fetchGyro, 1000);");
            client.println("        accelInterval = setInterval(fetchAccel, 1000);");
            client.println("      } else {");
            client.println("        // Clear intervals");
            client.println("        clearInterval(gyroInterval);");
            client.println("        clearInterval(accelInterval);");
            client.println("        gyroInterval = null;");
            client.println("        accelInterval = null;");
            client.println("      }");
            client.println("    }");

            client.println("    setInterval(fetchState, 500);");
            client.println("  </script>");
            client.println("</body>");
            client.println("</html>");
        }
        client.stop();
    }
}
