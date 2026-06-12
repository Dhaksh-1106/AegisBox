import math
import time
import json
import asyncio
from pathlib import Path
import random
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
from fastapi.middleware.cors import CORSMiddleware
import paho.mqtt.client as mqtt

FRONTEND_DIR = Path(__file__).parent.parent.resolve() / "frontend"

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

MQTT_BROKER = "test.mosquitto.org" 
MQTT_PORT = 1883
MQTT_TOPIC = "YOUR_TOPIC"

active_dashboard: WebSocket = None
impact_count = 0
loop = None 

def on_connect(client, userdata, flags, rc, properties=None):
    if rc == 0:
        print(f"\n[SUCCESS] Connected to MQTT Broker: {MQTT_BROKER}")
        client.subscribe(MQTT_TOPIC)
        print(f"[SUBSCRIBED] Intercepting live streams on: {MQTT_TOPIC}")
    else:
        print(f"\n[MQTT ERROR] Connection rejected by broker. Code: {rc}")

def on_message(client, userdata, msg):
    global impact_count, active_dashboard, loop
    try:
        payload_str = msg.payload.decode('utf-8')
        print(f"\n[Data Arrived] -> {payload_str}")
        
        raw_payload = json.loads(payload_str)
        
        temp = raw_payload.get("temperature", 25.0)
        hum = raw_payload.get("humidity", 50.0)
        ax = raw_payload.get("accel_x", 0.0)
        ay = raw_payload.get("accel_y", 0.0)
        az = raw_payload.get("accel_z", 1.0)
        tamper = raw_payload.get("tamper_status", False)
        dev_id = raw_payload.get("device_id", "Aegis-Box-001")
        
        gforce = math.sqrt(ax**2 + ay**2 + az**2)
        if gforce >= 2.0:
            impact_count += 1
            
        processed_payload = {
            "device_id": dev_id,
            "timestamp": raw_payload.get("timestamp", int(time.time())),
            "temperature": temp,
            "humidity": hum,
            "gforce": round(gforce, 2),
            "impact_count": impact_count,
            "ldr_lux": 280.0 if tamper else 1.5,
            "tamper": tamper,
            "uptime_s": int(time.process_time()), 
            "rssi": -60 
        }
        
        if active_dashboard and loop:
            asyncio.run_coroutine_threadsafe(
                active_dashboard.send_text(json.dumps(processed_payload)), 
                loop
            )
            
    except Exception as e:
        print(f"[ERROR] Ingestion calculation failure: {e}")

random_suffix = "".join(random.choices("abcdefghijklmnopqrstuvwxyz0123456789", k=6))
client_unique_id = f"AegisBox_Monitor_{random_suffix}"

mqtt_client = mqtt.Client(
    client_id=client_unique_id, 
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
    protocol=mqtt.MQTTv311
)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

@app.on_event("startup")
def startup_event():
    global loop
    loop = asyncio.get_event_loop()
    print(f"[INIT] Connecting backend to Mosquitto loop using ID: {client_unique_id}")
    mqtt_client.connect_async(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_start()

@app.on_event("shutdown")
def shutdown_event():
    mqtt_client.loop_stop()
    mqtt_client.disconnect()

@app.get("/")
async def get_dashboard():
    return HTMLResponse(content=open(f"{FRONTEND_DIR}/index.html", "r", encoding="utf-8").read(), status_code=200)

@app.websocket("/ws/dashboard")
async def websocket_endpoint(websocket: WebSocket):
    global active_dashboard
    await websocket.accept()
    active_dashboard = websocket
    print("[NET] Web UI linked to local live streams.")
    try:
        while True:
            await websocket.receive_text()
    except WebSocketDisconnect:
        active_dashboard = None
        print("[NET] Web UI detached cleanly.")

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)