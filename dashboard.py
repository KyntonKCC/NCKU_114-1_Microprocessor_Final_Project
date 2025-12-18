import streamlit as st
import serial
import socket
import threading
import json
import time
import pandas as pd
from collections import deque
from datetime import datetime

# --- CONFIGURATION ---
SERIAL_PORT = 'COM3'   # Update if needed
BAUD_RATE = 9600       # Ensure this matches your MCU
TCP_IP = '127.0.0.1'
TCP_PORT = 5000        
MAX_POINTS = 50        # Keep last 50 points (approx 12.5s history at 0.25s interval)

# --- BACKGROUND SERVICE CLASS ---
class SerialProxyService:
    """
    Handles Serial I/O and TCP Forwarding in background threads.
    Persists across Streamlit re-runs using st.cache_resource.
    """
    def __init__(self):
        self.running = True
        self.data_buffer = deque(maxlen=MAX_POINTS)
        self.client_socket = None
        self.client_addr = None
        
        # 1. Connect to Serial
        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
            self.connection_status = f"✅ Connected to {SERIAL_PORT}"
        except Exception as e:
            self.connection_status = f"❌ Serial Error: {e}"
            self.running = False
            return

        # 2. Start TCP Server (for PuTTY/External monitors)
        try:
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((TCP_IP, TCP_PORT))
            self.server_socket.listen(1)
            self.server_socket.settimeout(1.0)
        except Exception as e:
            self.connection_status = f"❌ TCP Error: {e}"
            self.running = False
            return

        # 3. Start Threads
        self.t_serial = threading.Thread(target=self._serial_loop, daemon=True)
        self.t_tcp = threading.Thread(target=self._tcp_server_loop, daemon=True)
        
        self.t_serial.start()
        self.t_tcp.start()

    def _serial_loop(self):
        """Reads Serial -> Updates Buffer AND Forwards to PuTTY"""
        while self.running:
            try:
                if self.ser.in_waiting:
                    # Read raw bytes
                    raw_data = self.ser.readline()
                    
                    # A. Forward to PuTTY (if connected)
                    if self.client_socket:
                        try:
                            self.client_socket.sendall(raw_data)
                        except:
                            self.client_socket = None 

                    # B. Parse for Dashboard
                    try:
                        text_data = raw_data.decode('utf-8').strip()
                        # Expected format: {"angle":30,"temp":1023,"status":"FIRE"}
                        payload = json.loads(text_data)
                        
                        # Validate keys exist before appending
                        if all(k in payload for k in ("angle", "temp", "status")):
                            # Add timestamp for X-axis if needed, or just use index
                            payload['timestamp'] = datetime.now().strftime("%H:%M:%S")
                            payload['raw'] = text_data
                            self.data_buffer.append(payload)
                            
                    except json.JSONDecodeError:
                        pass # partial line or noise
                    except Exception as e:
                        print(f"Parse Error: {e}")
                        
            except Exception as e:
                print(f"Serial Loop Error: {e}")
                self.running = False
                break
            time.sleep(0.05) # Check slightly faster than data rate

    def _tcp_server_loop(self):
        """Listens for PuTTY connections -> Forwards Input to Serial"""
        while self.running:
            try:
                if not self.client_socket:
                    try:
                        client, addr = self.server_socket.accept()
                        self.client_socket = client
                        self.client_addr = addr
                    except socket.timeout:
                        continue
                else:
                    try:
                        self.client_socket.settimeout(0.1)
                        data = self.client_socket.recv(1024)
                        if not data:
                            self.client_socket = None
                        else:
                            self.ser.write(data) 
                    except socket.timeout:
                        pass
                    except Exception:
                        self.client_socket = None
            except Exception:
                time.sleep(1)

    def stop(self):
        self.running = False
        if self.client_socket: self.client_socket.close()
        if hasattr(self, 'server_socket'): self.server_socket.close()
        if hasattr(self, 'ser') and self.ser.is_open: self.ser.close()

# --- STREAMLIT SINGLETON ---
@st.cache_resource
def get_service():
    return SerialProxyService()

# --- FRONTEND UI ---
st.set_page_config(page_title="Fire Detection Hub", layout="wide", page_icon="🔥")

# CSS for status indicators
st.markdown("""
<style>
    .metric-card {
        background-color: #f0f2f6;
        border-radius: 10px;
        padding: 15px;
        text-align: center;
    }
</style>
""", unsafe_allow_html=True)

st.title("🔥 Fire Detection System")

service = get_service()

# Sidebar
st.sidebar.header("Connection Status")
st.sidebar.info(service.connection_status)
if service.client_socket:
    st.sidebar.success(f"PuTTY: Linked {service.client_addr}")
else:
    st.sidebar.warning(f"PuTTY: Idle ({TCP_PORT})")

if st.sidebar.button("🔴 STOP SYSTEM"):
    service.stop()
    st.sidebar.error("System Halted.")

# Layout Containers
status_container = st.container()
metrics_container = st.container()
charts_container = st.container()

# Placeholders for live updates
with status_container:
    status_alert = st.empty()

with metrics_container:
    m1, m2, m3 = st.columns(3)
    with m1: metric_temp = st.empty()
    with m2: metric_angle = st.empty()
    with m3: metric_status = st.empty()

with charts_container:
    c1, c2 = st.columns(2)
    with c1: 
        st.caption("Temperature History (ADC)")
        chart_temp = st.empty()
    with c2: 
        st.caption("Sensor Rotation (Angle)")
        chart_angle = st.empty()

# --- LIVE UI UPDATE LOOP ---
while service.running:
    # Snapshot current buffer
    data = list(service.data_buffer)
    
    if data:
        last_reading = data[-1]
        
        # 1. Update Status Header
        current_status = last_reading.get("status", "UNKNOWN")
        if current_status == "FIRE":
            status_alert.error("🚨 CRITICAL ALERT: FIRE DETECTED 🚨")
        elif current_status == "SAFE":
            status_alert.success("✅ System Status: NORMAL")
        else:
            status_alert.warning(f"⚠️ System Status: {current_status}")

        # 2. Update Key Metrics
        metric_temp.metric("Flame Sensor (ADC)", last_reading["temp"], border=True)
        metric_angle.metric("Servo Angle", f"{last_reading['angle']}°", border=True)
        metric_status.metric("Classification", current_status, border=True)

        st.caption(f"Raw UART: `{last_reading.get('raw', '')}`")

        # 3. Update Charts
        df = pd.DataFrame(data)
        
        # Temperature Line Chart
        chart_temp.line_chart(df[["temp"]], height=250)
        
        # Angle Area Chart (Visualizes sweep better)
        chart_angle.area_chart(df[["angle"]], color="#FF4B4B", height=250)

    time.sleep(0.1) # UI Refresh Rate (decoupled from data rate)