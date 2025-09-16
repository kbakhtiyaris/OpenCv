# detector.py
import cv2                           # OpenCV main module [3]
import time                          # Timing for FPS/intervals [3]
import json                          # Encode JSON for HTTP [14]
import urllib.request                # Simple POST to Flask [14]
import numpy as np                   # Array ops for color/labels [3]

# Config: choose one video source
USE_WEBCAM = False                    # Toggle between webcam and IP stream [3]
CAP_URL = "http://192.168.1.106:8080/video"  # VideoCapture arg [3]

# Paths to MobileNet-SSD v2 COCO model (download beforehand)
PB = "frozen_inference_graph.pb"     # TensorFlow frozen graph weights [3]
PBTXT = "ssd_mobilenet_v2_coco_2018_03_29.pbtxt.txt"  # Graph config [3]
LABELS = "object_detection_classes_coco.txt"  # Text file with COCO class names [3]

# Flask server base URL (Ubuntu VM on LAN)
SERVER = "http://127.0.0.1:8080"  # Replace with VM IP (bridged networking) [27]

# Decision parameters
ON_THRESH = 0.5                      # Confidence threshold for 'person' [3]
FRAMES_TO_ON = 5                     # Require N consecutive detections to turn on [3]
OFF_GRACE_SEC = 8                    # Turn off after this many seconds without person [3]

# Load class labels
with open(LABELS, "r") as f:         # Read COCO class names [3]
    class_names = [c.strip() for c in f.readlines()]  # List of labels [3]

# Load DNN
net = cv2.dnn.readNet(model=PB, config=PBTXT, framework="TensorFlow")  # Load SSD model [3]

cap = cv2.VideoCapture(CAP_URL)      # Open camera stream [3]
assert cap.isOpened(), "Camera open failed"  # Basic guard [3]

# State tracking
consec = 0                           # Consecutive frames with person [3]
last_seen = 0                        # Timestamp when last person seen [3]
current_state = "off"                # Cache to avoid redundant posts [14]

def post_state(desired, detected):
    """POST desired state to Flask and log event."""  # Docstring [14]
    data = json.dumps({"desired": desired, "detected": bool(detected)}).encode("utf-8")  # JSON body [14]
    req = urllib.request.Request(f"{SERVER}/api/set_state", data=data, headers={"Content-Type": "application/json"})  # HTTP POST [14]
    with urllib.request.urlopen(req, timeout=3) as resp:  # Send request [14]
        resp.read()  # Drain response [14]

while True:
    ok, frame = cap.read()           # Read a frame [3]
    if not ok:
        break                        # End if stream stops [3]

    (H, W) = frame.shape[:2]         # Image dims [3]
    blob = cv2.dnn.blobFromImage(image=frame, size=(300, 300), mean=(104, 117, 123), swapRB=True)  # Preprocess to 300x300 RGB mean-sub [3]
    net.setInput(blob)               # Set blob [3]
    output = net.forward()           # Forward pass detections [3]

    person_present = False           # Flag for this frame [3]
    for det in output[0, 0, :, :]:   # Iterate detections [3]
        conf = float(det[2])         # Confidence [3]
        if conf < ON_THRESH:         # Skip low confidence [3]
            continue
        class_id = int(det[1])       # Class index [3]
        label = class_names[class_id - 1] if 0 < class_id <= len(class_names) else "unknown"  # COCO mapping [3]
        if label == "person":        # Only care about people [3]
            person_present = True    # Mark as present [3]
            # Draw box for visualization (optional)
            box_x = int(det[3] * W); box_y = int(det[4] * H)  # Top-left [3]
            box_w = int(det[5] * W); box_h = int(det[6] * H)  # Bottom-right raw [3]
            cv2.rectangle(frame, (box_x, box_y), (box_w, box_h), (0, 255, 0), 2)  # Draw rect [3]
            cv2.putText(frame, f"{label}:{conf:.2f}", (box_x, max(0, box_y-5)), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0,255,0), 2)  # Put label [3]

    now = time.time()                # Current time [3]
    if person_present:
        consec += 1                  # Count consecutive positives [3]
        last_seen = now              # Update last seen [3]
        if consec >= FRAMES_TO_ON and current_state != "on":  # Debounce ON [3][14]
            post_state("on", True)   # Tell Flask: desired on [14]
            current_state = "on"     # Cache [14]
    else:
        consec = 0                   # Reset streak [3]
        if current_state == "on" and (now - last_seen) > OFF_GRACE_SEC:  # Debounce OFF [3]
            post_state("off", False) # Tell Flask: desired off [14]
            current_state = "off"    # Cache [14]

    cv2.putText(frame, f"state:{current_state}", (10, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0,255,255), 2)  # HUD [3]
    cv2.imshow("Smart Fan Detector", frame)  # Show for debugging [3]
    if cv2.waitKey(1) & 0xFF == ord('q'):    # Quit on q [3]
        break

cap.release(); cv2.destroyAllWindows()       # Cleanup [3]
