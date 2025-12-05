# iNaturalist API Contract

**API Base URL**: `https://api.inaturalist.org/v1/`  
**Authentication**: OAuth 2.0  
**Purpose**: Upload microscopy images and populate observation fields with measurement statistics

## Authentication Flow

### OAuth 2.0 Authorization Code Flow

**1. Authorization Request**

Direct user to iNaturalist authorization page:

```
GET https://www.inaturalist.org/oauth/authorize
  ?client_id={YOUR_CLIENT_ID}
  &redirect_uri=http://localhost:48934/callback
  &response_type=code
  &scope=write
```

**Parameters**:
- `client_id`: Obtained from iNaturalist app registration
- `redirect_uri`: Localhost callback (Qt handles with `QOAuthHttpServerReplyHandler`)
- `response_type`: Always `code` for authorization code flow
- `scope`: `write` (required for uploading photos and updating observations)

**2. Authorization Callback**

User logs in and authorizes → iNaturalist redirects to:

```
http://localhost:48934/callback?code={AUTHORIZATION_CODE}
```

**3. Token Exchange**

Exchange authorization code for access token:

```http
POST https://www.inaturalist.org/oauth/token
Content-Type: application/x-www-form-urlencoded

client_id={YOUR_CLIENT_ID}
&client_secret={YOUR_CLIENT_SECRET}
&code={AUTHORIZATION_CODE}
&redirect_uri=http://localhost:48934/callback
&grant_type=authorization_code
```

**Response**:
```json
{
  "access_token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "token_type": "Bearer",
  "expires_in": 7200,
  "refresh_token": "def502...",
  "created_at": 1733414400
}
```

**Storage**:
```cpp
QSettings settings;
settings.setValue("inaturalist/access_token", accessToken);
settings.setValue("inaturalist/token_expiration", 
                  QDateTime::currentDateTime().addSecs(expiresIn));
settings.setValue("inaturalist/refresh_token", refreshToken);
```

**4. Token Refresh** (when expired)

```http
POST https://www.inaturalist.org/oauth/token
Content-Type: application/x-www-form-urlencoded

client_id={YOUR_CLIENT_ID}
&client_secret={YOUR_CLIENT_SECRET}
&refresh_token={REFRESH_TOKEN}
&grant_type=refresh_token
```

---

## API Endpoints

### 1. Get Current User

Verify authentication and get user info.

**Request**:
```http
GET https://api.inaturalist.org/v1/users/me
Authorization: Bearer {ACCESS_TOKEN}
```

**Response**:
```json
{
  "results": [{
    "id": 12345,
    "login": "mycologist_jane",
    "name": "Jane Smith",
    "created_at": "2020-01-15T10:30:00Z"
  }]
}
```

---

### 2. Search Observations

Find existing observations to link to imaging session.

**Request**:
```http
GET https://api.inaturalist.org/v1/observations
  ?user_id=12345
  &taxon_name=Amanita
  &per_page=20
  &order_by=created_at
Authorization: Bearer {ACCESS_TOKEN}
```

**Query Parameters**:
- `user_id`: Filter by user (use current user ID)
- `taxon_name`: Filter by taxon (e.g., genus, species)
- `per_page`: Results per page (max 200)
- `order_by`: Sort order (`created_at`, `observed_on`, `species_guess`)

**Response**:
```json
{
  "total_results": 42,
  "page": 1,
  "per_page": 20,
  "results": [
    {
      "id": 987654,
      "species_guess": "Amanita muscaria",
      "taxon": {
        "id": 48716,
        "name": "Amanita muscaria",
        "rank": "species"
      },
      "observed_on": "2025-12-01",
      "place_guess": "Redwood Forest, CA",
      "photos": [
        {"id": 111, "url": "https://..."}
      ],
      "ofvs": []  // Observation field values
    }
  ]
}
```

**UI Integration**:
```cpp
// Display in QListWidget or QTableView
foreach (const QJsonObject& obs, results) {
    QString display = QString("%1 - %2 - %3")
        .arg(obs["id"].toInt())
        .arg(obs["species_guess"].toString())
        .arg(obs["observed_on"].toString());
    ui->observationList->addItem(display);
}
```

---

### 3. Create New Observation

Create observation from within μScope.

**Request**:
```http
POST https://api.inaturalist.org/v1/observations
Authorization: Bearer {ACCESS_TOKEN}
Content-Type: application/json

{
  "observation": {
    "species_guess": "Amanita muscaria",
    "observed_on_string": "2025-12-05",
    "place_guess": "Lab",
    "description": "Microscopy imaging session"
  }
}
```

**Response**:
```json
{
  "id": 999999,
  "created_at": "2025-12-05T14:30:00Z",
  "species_guess": "Amanita muscaria",
  "user": {"id": 12345, "login": "mycologist_jane"}
}
```

---

### 4. Upload Photos to Observation

Upload microscopy images to linked observation.

**Request**:
```http
POST https://api.inaturalist.org/v1/observation_photos
Authorization: Bearer {ACCESS_TOKEN}
Content-Type: multipart/form-data

------WebKitFormBoundary
Content-Disposition: form-data; name="observation_photo[observation_id]"

999999
------WebKitFormBoundary
Content-Disposition: form-data; name="file"; filename="spore_image.jpg"
Content-Type: image/jpeg

{BINARY_IMAGE_DATA}
------WebKitFormBoundary--
```

**Qt Implementation**:
```cpp
QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

// Observation ID
QHttpPart observationIdPart;
observationIdPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant("form-data; name=\"observation_photo[observation_id]\""));
observationIdPart.setBody(QString::number(observationId).toUtf8());
multiPart->append(observationIdPart);

// Image file
QHttpPart imagePart;
imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                   QVariant("form-data; name=\"file\"; filename=\"" + filename + "\""));
QFile* file = new QFile(filePath);
file->open(QIODevice::ReadOnly);
imagePart.setBodyDevice(file);
file->setParent(multiPart);
multiPart->append(imagePart);

// Send request
QNetworkRequest request(QUrl("https://api.inaturalist.org/v1/observation_photos"));
request.setRawHeader("Authorization", "Bearer " + accessToken.toUtf8());
QNetworkReply* reply = networkManager->post(request, multiPart);
multiPart->setParent(reply);
```

**Response**:
```json
{
  "id": 222222,
  "photo": {
    "id": 333333,
    "large_url": "https://static.inaturalist.org/photos/333333/large.jpg",
    "medium_url": "https://static.inaturalist.org/photos/333333/medium.jpg"
  },
  "observation_id": 999999
}
```

---

### 5. Get Observation Fields

Retrieve available observation fields for populating statistics.

**Request**:
```http
GET https://api.inaturalist.org/v1/observation_fields
  ?q=spore
  &per_page=50
Authorization: Bearer {ACCESS_TOKEN}
```

**Response**:
```json
{
  "results": [
    {
      "id": 1234,
      "name": "Spore Length",
      "datatype": "text",
      "allowed_values": null,
      "description": "Length of spores in micrometers"
    },
    {
      "id": 1235,
      "name": "Spore Width",
      "datatype": "text",
      "allowed_values": null
    }
  ]
}
```

**UI Integration**:
```cpp
// Allow user to map measurement datasets to observation fields
// E.g., "Spore Lengths" dataset → "Spore Length" field
QMap<QString, int> fieldMappings;  // datasetName → observation_field_id
fieldMappings["Spore Lengths"] = 1234;
fieldMappings["Spore Widths"] = 1235;
```

---

### 6. Create Observation Field Value

Populate observation field with measurement statistics.

**Request**:
```http
POST https://api.inaturalist.org/v1/observation_field_values
Authorization: Bearer {ACCESS_TOKEN}
Content-Type: application/json

{
  "observation_field_value": {
    "observation_id": 999999,
    "observation_field_id": 1234,
    "value": "8.5µm ± 1.2µm (n=47, range: 6.2-10.8µm)"
  }
}
```

**Value Formatting**:
```cpp
QString formatStatistics(const MeasurementStatistics& stats, const QString& unit) {
    return QString("%1%2 ± %3%2 (n=%4, range: %5-%6%2)")
        .arg(stats.mean, 0, 'f', 1)
        .arg(unit)
        .arg(stats.standardDeviation, 0, 'f', 1)
        .arg(stats.count)
        .arg(stats.minimum, 0, 'f', 1)
        .arg(stats.maximum, 0, 'f', 1);
}
```

**Response**:
```json
{
  "id": 555555,
  "observation_id": 999999,
  "observation_field_id": 1234,
  "value": "8.5µm ± 1.2µm (n=47, range: 6.2-10.8µm)",
  "created_at": "2025-12-05T14:35:00Z"
}
```

---

## Error Handling

### HTTP Status Codes

| Code | Meaning | Handling |
|------|---------|----------|
| 200 | Success | Process response |
| 401 | Unauthorized (token expired) | Refresh token or re-authenticate |
| 403 | Forbidden | Check permissions/scope |
| 404 | Observation not found | Notify user, offer to unlink |
| 422 | Validation error | Display error details to user |
| 429 | Rate limit exceeded | Retry with exponential backoff |
| 500 | Server error | Retry 3 times, then fail gracefully |

### Error Response Format

```json
{
  "error": "Unauthorized",
  "status": 401
}
```

**Qt Error Handling**:
```cpp
void handleNetworkReply(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (statusCode == 401) {
            // Token expired
            emit authenticationExpired();
            refreshAccessToken();
        } else if (statusCode == 404) {
            emit observationNotFound();
            unlinkSession();
        }
    } else {
        // Network error
        emit networkError(reply->errorString());
    }
}
```

---

## Upload Queue & Retry Logic

### Queue Management

**Data Structure**:
```cpp
struct UploadQueueItem {
    QString filePath;
    int observationId;
    int retryCount;
    QDateTime queuedAt;
};

QList<UploadQueueItem> uploadQueue;
```

**Storage**:
```cpp
QSettings settings;
settings.beginWriteArray("inaturalist/upload_queue");
for (int i = 0; i < uploadQueue.size(); ++i) {
    settings.setArrayIndex(i);
    settings.setValue("filePath", uploadQueue[i].filePath);
    settings.setValue("observationId", uploadQueue[i].observationId);
    settings.setValue("retryCount", uploadQueue[i].retryCount);
}
settings.endArray();
```

### Retry Strategy

1. **On Network Error**: Retry immediately (up to 3 times)
2. **On Server Error (5xx)**: Retry with exponential backoff (1s, 2s, 4s, 8s, 16s)
3. **On Rate Limit (429)**: Wait for `Retry-After` header duration
4. **On Client Error (4xx)**: Fail permanently, notify user

**Implementation**:
```cpp
void retryUpload(UploadQueueItem& item) {
    if (item.retryCount >= 3) {
        emit uploadFailed(item.filePath, "Maximum retries exceeded");
        uploadQueue.removeOne(item);
        return;
    }
    
    int delay = qPow(2, item.retryCount) * 1000;  // Exponential backoff
    QTimer::singleShot(delay, this, [this, item]() mutable {
        item.retryCount++;
        uploadImage(item);
    });
}
```

---

## Rate Limiting

**iNaturalist Rate Limits**:
- 100 requests per minute per user
- 10,000 requests per day per user

**Client-Side Rate Limiting**:
```cpp
class RateLimiter {
public:
    bool canMakeRequest() {
        QDateTime now = QDateTime::currentDateTime();
        m_requests.removeIf([&now](const QDateTime& dt) {
            return dt.secsTo(now) > 60;  // Remove requests older than 1 minute
        });
        
        if (m_requests.size() >= 90) {  // Leave buffer (90 < 100)
            return false;
        }
        
        m_requests.append(now);
        return true;
    }
    
private:
    QList<QDateTime> m_requests;
};
```

---

## Testing & Validation

### Manual Test Scenarios

1. **OAuth Flow**: Authenticate, verify token saved, refresh token on expiration
2. **Search Observations**: Search for user's observations, display in UI
3. **Create Observation**: Create new observation, verify returned ID
4. **Upload Image**: Upload microscopy image, verify appears in observation
5. **Populate Field**: Map dataset to observation field, verify value set correctly
6. **Network Failure**: Disconnect internet during upload, verify queuing and retry
7. **Token Expiration**: Wait for token expiration (or mock), verify auto-refresh

### API Response Validation

```cpp
bool validateObservationResponse(const QJsonObject& response) {
    return response.contains("id") &&
           response["id"].toInt() > 0 &&
           response.contains("created_at");
}
```

---

**Contract Status**: ✅ COMPLETE - iNaturalist API integration defined with OAuth, photo upload, and observation field population
