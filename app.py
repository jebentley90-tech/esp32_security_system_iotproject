import os
import json
from datetime import datetime
from flask import Flask, render_template, jsonify, request
from pymongo import MongoClient, DESCENDING # Added DESCENDING
import paho.mqtt.client as mqtt
from dotenv import load_dotenv

load_dotenv()
app = Flask(__name__)

MONGODB_URI = os.getenv("MONGODB_URI")
MQTT_BROKER = "broker.emqx.io"
MQTT_PORT = 1883
DATA_TOPIC = "joshandjoe/finalproject/esp32wroom1/system/status"
CMD_TOPIC = "joshandjoe/finalproject/esp32wroom1/system/cmd"

mongo_client = MongoClient(MONGODB_URI)
db = mongo_client["hawk_protection_system"]
collection = db["monitoring"]

def on_connect(client, userdata, flags, reason_code, properties=None):
    client.subscribe(DATA_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        record = {
            "device_id": payload.get("device_id", "unknown-device"),
            "server_time": datetime.now(),
            "event": payload.get("event"),
            "mode": payload.get("mode"),
            "pir": payload.get("pir"),
            "sonar_cm": payload.get("sonar_cm"),
            "sonar_presence": payload.get("sonar_presence"),
            "presence": payload.get("presence"),
            "actuators": payload.get("actuators"),
            "device_ts": payload.get("ts")
        }
        collection.insert_one(record)
    except Exception as e:
        print("Error:", e)

mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
mqtt_client.loop_start()

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/api/status")
def api_status():
    # 1. Get the last 20 records (better for charts)
    # 2. Sort by server_time (now works perfectly because it's a Date object)
    docs = list(collection.find({}, {"_id": 0}).sort("server_time", DESCENDING).limit(20))

    if not docs:
        return jsonify({"device_state": "offline", "history": []})

    # 3. Format the Date objects back into Strings for the Frontend
    for doc in docs:
        if isinstance(doc["server_time"], datetime):
            doc["server_time"] = doc["server_time"].strftime("%I:%M:%S %p")

    return jsonify({
        "last_update": docs[0].get("server_time"),
        "current_mode": docs[0].get("mode"),
        "history": docs
    })

@app.route("/send", methods=["POST"])
def send_command():
    # Supports "ON", "OFF", and now "AUTO"
    cmd = request.form["cmd"].upper() 
    
    payload = {
        "command": cmd,
        "timestamp": datetime.now().strftime("%m/%d/%Y %I:%M:%S %p"),
        "source": "flask_dashboard"
    }

    mqtt_client.publish(CMD_TOPIC, json.dumps(payload))
    return "OK"

if __name__ == "__main__":
    app.run(debug=True)
    
    
    