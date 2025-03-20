#include <WiFiNINA.h>
#include "TrafficLightController.h"

extern WiFiServer server;
extern TrafficLightState currentState; // Declare the current state as external

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

        // Serve current state
        if (request.indexOf("/state") != -1)
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
                newState.trim(); // remove any extra whitespace
                Serial.print("Setting state to: ");
                Serial.println(newState);
                setTrafficLightState(newState); // function defined in TrafficLightController.h
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
        // Serve main webpage with the grid, current state and dropdown control
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
            client.println("    h1 { text-align: center; }");
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
            client.println("      document.getElementById('A1').className = `grid-item ${colorMap.A1}`;");
            client.println("      document.getElementById('A2').className = `grid-item ${colorMap.A2}`;");
            client.println("      document.getElementById('A3').className = `grid-item ${colorMap.A3}`;");
            client.println("      document.getElementById('P').className = `grid-item ${colorMap.P}`;");
            client.println("    }");
            client.println("    async function fetchState() {");
            client.println("      const response = await fetch('/state');");
            client.println("      let state = await response.text();");
            client.println("      state = state.trim();");
            client.println("      document.getElementById('state').innerText = `Aktueller Zustand: ${state}`;");
            client.println("      updateColors(state);");
            client.println("    }");
            client.println("    async function setState() {");
            client.println("      const select = document.getElementById('stateSelect');");
            client.println("      const newState = select.value;");
            client.println("      await fetch(`/set?state=${newState}`);");
            client.println("      fetchState();");
            client.println("    }");
            client.println("    setInterval(fetchState, 500);");
            client.println("  </script>");
            client.println("</body>");
            client.println("</html>");
        }
        client.stop();
    }
}