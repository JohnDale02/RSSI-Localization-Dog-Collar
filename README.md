# RSSI-Localization-Dog-Collar

Code for Dog Collar Localization system. Custom WIFI chip dog collar using RSSI localization with ESP8266 microcontroller network. NVIDIA Jetson used as a server to calculate and push estimated positional data to website. 

## Steps:
- 1) Install Mosquitto Broker on Server (NVIDIA Jetson in my case)
- 2) run the Mosquitto Broker in the background 
-       - mosquitto -d
- 3) Configure your ESP8266 nodes
        - Upload Node Code (Recieving BLE packets --> pubishing to MQTT broker)
- 4) Run the Web Server + MQTT Client code on the Server 
        - Recieve all node messages
        - Estimate location of Dog using RSSI triangulation
        - Update the Dog's location in OpenGl
        - Use Enscripten to convert OpenGl (C++) -> WebGL (Javascript)
        - Publish WebGL graphics to Web Server
       
