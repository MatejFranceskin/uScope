# RTSP Streaming Protocol Contract

**Protocol**: RTSP (Real-Time Streaming Protocol) + RTP (Real-Time Transport Protocol)  
**Purpose**: Teacher-to-student classroom collaboration video streaming with annotation sync

## Architecture

```
Teacher (Server)                                Student (Client)
┌────────────────────┐                          ┌────────────────────┐
│ Qt Camera          │                          │ Qt Media Player    │
│    ↓               │                          │    ↑               │
│ GStreamer Pipeline │                          │ GStreamer Pipeline │
│ appsrc → x264enc   │                          │ rtspsrc → decoder  │
│    ↓               │                          │    ↑               │
│ rtph264pay         │                          │ rtph264depay       │
│    ↓               │    RTSP/RTP (UDP)        │    ↑               │
│ rtspsink ◄─────────┼──────────────────────────┼─►  rtspsrc         │
└────────────────────┘                          └────────────────────┘
         │                                               ↑
         │ UDP Broadcast (Service Discovery)            │
         └───────────────────────────────────────────────┘
```

## Service Discovery

### UDP Broadcast Message (Teacher → Students)

**Format**: JSON over UDP on port 5353 (mDNS fallback)  
**Broadcast Interval**: Every 2 seconds while streaming active  
**Payload**:

```json
{
  "type": "uScope.rtsp.announce",
  "version": "1.0",
  "sessionId": "uuid-v4-session-id",
  "teacherName": "Dr. Smith",
  "teacherDeviceId": "mac-address-or-uuid",
  "rtspUrl": "rtsp://192.168.1.100:8554/uuid-v4-session-id",
  "resolution": {"width": 1280, "height": 720},
  "frameRate": 30,
  "requiresAuth": false,
  "authToken": null,
  "timestamp": "2025-12-05T14:30:00Z"
}
```

**Client Behavior**:
1. Listen on UDP port 5353
2. Parse incoming JSON messages
3. Filter by `type == "uScope.rtsp.announce"`
4. Display available streams in UI (teacher name, timestamp)
5. Connect to selected `rtspUrl`

**mDNS/Bonjour Fallback** (for multi-subnet networks):
- Service Type: `_uscope-rtsp._tcp`
- Service Name: `{teacherName} - μScope`
- TXT Records: `sessionId`, `rtspUrl`, `resolution`, `frameRate`

---

## RTSP Stream Specification

### Video Stream

**Container**: RTSP with H.264 over RTP  
**Codec**: H.264/AVC (Baseline Profile 3.1)  
**Resolution**: 720p (1280×720) default, configurable  
**Frame Rate**: 30 fps default, configurable  
**Bitrate**: 3000-5000 kbps (adaptive based on network)  
**Transport**: RTP over UDP (port 5004-5005)  
**Latency Target**: <500ms end-to-end

### GStreamer Pipeline (Teacher)

```bash
appsrc name=source format=time is-live=true \
! videoconvert \
! x264enc tune=zerolatency bitrate=4000 speed-preset=ultrafast \
! rtph264pay config-interval=1 pt=96 \
! udpsink host={client_ip} port=5004
```

**C++ Integration**:
```cpp
QGst::PipelinePtr pipeline = QGst::Pipeline::create();
QGst::ElementPtr appsrc = QGst::ElementFactory::make("appsrc");
// Configure appsrc with QVideoFrame data from Qt camera
// Connect signals for buffer pushing
```

### GStreamer Pipeline (Student)

```bash
rtspsrc location=rtsp://192.168.1.100:8554/{sessionId} latency=100 \
! rtph264depay \
! h264parse \
! avdec_h264 \
! videoconvert \
! appsink name=sink emit-signals=true
```

---

## Annotation Synchronization

### Embedded Metadata Track

**Method 1: Custom RTP Payload** (Preferred)

Embed annotations in custom RTP payload type (PT=97) sent alongside video (PT=96).

**RTP Packet Structure**:
```
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|V=2|P|X|  CC   |M|     PT=97   |       Sequence Number         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Timestamp                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                             SSRC                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      JSON Annotation Payload                  |
|                              ...                              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**JSON Payload Example**:
```json
{
  "action": "add",
  "annotation": {
    "id": 42,
    "type": "arrow",
    "position": {"x": 640, "y": 360},
    "points": [{"x": 640, "y": 360}, {"x": 800, "y": 400}],
    "color": "#FF0000",
    "lineWidth": 3
  },
  "timestamp": 1234567890
}
```

**Method 2: RTSP Metadata Track** (Alternative)

Use RTSP `application/x-metadata` track (RFC 4566 extension).

**SDP Description**:
```
m=application 0 RTP/AVP 98
a=rtpmap:98 x-metadata
a=fmtp:98 format=json
```

**Annotation Update Events**:
- `add`: New annotation created
- `update`: Existing annotation modified
- `delete`: Annotation removed (by ID)

### Differential Updates

To minimize bandwidth, only send changed annotations:

```json
{
  "action": "update",
  "changes": [
    {"id": 42, "position": {"x": 650, "y": 370}},
    {"id": 43, "color": "#00FF00"}
  ],
  "timestamp": 1234567891
}
```

---

## Connection Management

### Student Connection Flow

1. **Discovery**: Student receives UDP broadcast or mDNS announcement
2. **RTSP DESCRIBE**: Student sends DESCRIBE request to rtspUrl
   ```
   DESCRIBE rtsp://192.168.1.100:8554/{sessionId} RTSP/1.0
   CSeq: 1
   User-Agent: uScope-Student/1.0
   Accept: application/sdp
   ```

3. **RTSP SETUP**: Student sets up RTP transport
   ```
   SETUP rtsp://192.168.1.100:8554/{sessionId}/track0 RTSP/1.0
   CSeq: 2
   Transport: RTP/AVP;unicast;client_port=5004-5005
   ```

4. **RTSP PLAY**: Student starts playback
   ```
   PLAY rtsp://192.168.1.100:8554/{sessionId} RTSP/1.0
   CSeq: 3
   Range: npt=0.000-
   ```

5. **Authentication** (if `requiresAuth == true`):
   ```
   DESCRIBE rtsp://192.168.1.100:8554/{sessionId} RTSP/1.0
   CSeq: 1
   Authorization: Bearer {authToken}
   ```

### Server Response to Connection

Teacher's RTSP server emits Qt signal:
```cpp
emit studentConnected(deviceId, deviceName);
```

Teacher UI updates connection count and displays student list.

### Disconnection Handling

**Graceful Disconnect**:
```
TEARDOWN rtsp://192.168.1.100:8554/{sessionId} RTSP/1.0
CSeq: 10
```

**Teacher Stops Streaming**:
- Send RTSP 404 Not Found to all students
- Students emit signal: `streamingStopped()`
- Students display disconnection notification

**Network Interruption**:
- Student detects missing RTP packets (timeout 5 seconds)
- Attempt reconnection with exponential backoff (1s, 2s, 4s, 8s, max 30s)
- Display connection quality indicator: "Reconnecting..."

---

## Adaptive Bitrate Streaming

### Quality Levels

| Quality | Resolution | Frame Rate | Bitrate | Bandwidth Required |
|---------|-----------|------------|---------|-------------------|
| High    | 1280×720  | 30 fps     | 5000 kbps | ~6 Mbps         |
| Medium  | 1280×720  | 30 fps     | 3000 kbps | ~4 Mbps         |
| Low     | 960×540   | 20 fps     | 1500 kbps | ~2 Mbps         |
| Minimal | 640×360   | 15 fps     | 800 kbps  | ~1 Mbps         |

### Bitrate Adaptation Algorithm

**Server-Side** (Teacher):
1. Monitor RTP packet loss rate from RTCP Receiver Reports
2. If packet loss > 5%, decrease bitrate by one level
3. If packet loss < 1% for 10 seconds, increase bitrate by one level
4. Emit signal: `networkQualityChanged(quality)`

**Client-Side** (Student):
1. Monitor RTP jitter buffer (GStreamer `rtpjitterbuffer`)
2. If jitter > 100ms, request lower bitrate via RTSP SET_PARAMETER
3. Emit signal: `connectionQualityChanged(latency, quality)`

---

## Network Quality Indicators

### Metrics

| Metric | Calculation | Quality Threshold |
|--------|-------------|------------------|
| Latency | End-to-end delay (RTP timestamp - local time) | <500ms target |
| Jitter | Variation in packet arrival time | <50ms excellent, <100ms good |
| Packet Loss | % of RTP packets not received | <1% excellent, <5% acceptable |
| Bandwidth | Bitrate × 1.2 (overhead) | Measured via RTCP |

### Quality String Mapping

```cpp
QString getQualityString(int latency, double packetLoss) {
    if (latency < 100 && packetLoss < 0.01) return "excellent";
    if (latency < 300 && packetLoss < 0.05) return "good";
    if (latency < 500 && packetLoss < 0.10) return "fair";
    return "poor";
}
```

---

## Error Handling

### Connection Errors

| Error | RTSP Code | Client Behavior |
|-------|-----------|----------------|
| Stream not found | 404 | Display "Stream ended" notification |
| Authentication failed | 401 | Prompt for password |
| Connection limit reached | 503 | Display "Server full, try again later" |
| Network timeout | - | Auto-reconnect with backoff |

### Recovery Strategies

1. **Temporary Packet Loss**: Buffer with `rtpjitterbuffer` (200ms max)
2. **Sustained Packet Loss**: Request keyframe via RTCP PLI (Picture Loss Indication)
3. **Complete Disconnection**: Retry connection every 5 seconds for 1 minute, then give up

---

## Security Considerations

### Optional Authentication

**Token-Based Auth**:
1. Teacher generates UUID token when enabling classroom mode
2. Token shared out-of-band (displayed on screen, QR code)
3. Students include token in RTSP Authorization header
4. Server validates token before accepting connection

**Implementation**:
```cpp
void RTSPStreamingController::setAuthToken(const QString& token) {
    m_authToken = token;
    m_requiresAuth = !token.isEmpty();
}

bool validateClient(const QString& providedToken) {
    return !m_requiresAuth || providedToken == m_authToken;
}
```

### Network Restrictions

- Server binds to specific interface (not 0.0.0.0) to avoid external access
- Firewall rules: Allow UDP 5353 (discovery), TCP 8554 (RTSP), UDP 5004-5005 (RTP) on LAN only

---

## Testing & Validation

### Manual Test Scenarios

1. **Basic Streaming**: Teacher starts, student connects, verify video appears
2. **Annotation Sync**: Teacher draws arrow, verify appears on student within 500ms
3. **Multiple Students**: Connect 30 students, verify all receive stream
4. **Connection Limit**: Connect 31st student, verify rejection with 503
5. **Network Interruption**: Disconnect student's WiFi for 10s, reconnect, verify auto-reconnect
6. **Graceful Shutdown**: Teacher stops streaming, verify students notified

### Performance Benchmarks

- Latency: <500ms on 1 Gbps LAN (measure with timestamp watermark in video)
- Annotation Sync: <500ms (measure with signal timing)
- Bandwidth per Student: ~4 Mbps at 720p/30fps
- CPU Usage: <30% on teacher's Core i5 with 30 students

---

**Contract Status**: ✅ COMPLETE - RTSP streaming protocol defined with service discovery, annotation sync, and adaptive bitrate
