from flask import Flask, request, jsonify  # Flask web framework primitives for routes and JSON responses [14]
from flask_sqlalchemy import SQLAlchemy     # Flask extension that integrates SQLAlchemy ORM [14]
from datetime import datetime               # Timestamp for event logging [14]

app = Flask(__name__)                       # Create Flask application instance [14]
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///smartfan.db"  # SQLite DB file in project directory [14]
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False             # Disable track mod overhead [14]
db = SQLAlchemy(app)                        # Bind SQLAlchemy to the app [14]

class Event(db.Model):                      # ORM model for events table [14]
    id = db.Column(db.Integer, primary_key=True)               # PK auto-increment [14]
    detected = db.Column(db.Boolean, nullable=False)           # Person detected flag [14]
    state = db.Column(db.String(8), nullable=False)            # 'on' or 'off' [14]
    ts = db.Column(db.DateTime, default=datetime.utcnow)       # Event timestamp [14]

class State(db.Model):                      # Tiny table to hold current desired_state [14]
    id = db.Column(db.Integer, primary_key=True)               # Always 1-row table [14]
    desired = db.Column(db.String(8), nullable=False)          # 'on' or 'off' [14]

def init_db():
    with app.app_context():
        db.create_all()
        if State.query.get(1) is None:
            db.session.add(State(id=1, desired="off"))
            db.session.commit()

@app.get("/api/desired_state")              # Endpoint polled by ESP32 [14]
def get_desired_state():
    s = State.query.get(1)                  # Fetch single state row [14]
    return jsonify({"desired": s.desired})  # Return JSON desired state [14]

@app.post("/api/set_state")                 # Endpoint set by detector service [14]
def set_state():
    data = request.get_json(force=True)     # Parse JSON body safely [14]
    new_state = data.get("desired", "off")  # Default off if missing [14]
    detected = bool(data.get("detected", False))  # Whether person seen [14]
    s = State.query.get(1)                  # Load the single state row [14]
    s.desired = new_state                   # Update desired state [14]
    db.session.add(Event(detected=detected, state=new_state))  # Log event row [14]
    db.session.commit()                     # Commit changes [14]
    return jsonify({"ok": True, "desired": s.desired})  # Ack response [14]

@app.get("/api/events")                     # Fetch recent events for dashboard/logs [14]
def list_events():
    rows = Event.query.order_by(Event.ts.desc()).limit(50).all()  # Latest 50 events [14]
    return jsonify([
        {"id": r.id, "detected": r.detected, "state": r.state, "ts": r.ts.isoformat()}
        for r in rows
    ])                                       # JSON array response [14]

if __name__ == "__main__":                   # Dev entrypoint [14]
    init_db()                                # Initialize DB and default state before running app
    app.run(host="0.0.0.0", port=8080, debug=True)  # Listen on all interfaces [14]
