#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHTesp.h>

// Constants
#define DHT_PIN 15
#define LED_PIN 2

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 8883;

const char* topic_pub_temp = "/arun12vak/temp";
const char* topic_pub_hum = "/arun12vak/hum";
const char* topic_sub = "/ThinkIOT/Subscribe";

const char* ca_cert = R"(-----BEGIN CERTIFICATE-----
MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL
BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG
A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU
BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv
by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE
BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES
MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp
dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg
UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW
Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA
s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH
3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo
E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT
MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV
6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL
BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC
6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf
+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK
sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839
LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE
m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=
-----END CERTIFICATE-----)";

const char* client_cert = R"(-----BEGIN CERTIFICATE-----
MIIDuzCCAqOgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix
FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE
CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y
ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yNTAxMDExMjIz
NDVaFw0yNTA0MDExMjIzNDVaMIGUMQswCQYDVQQGEwJNQTEPMA0GA1UECAwGQWdh
ZGlyMQ8wDQYDVQQHDAZBZ2FkaXIxETAPBgNVBAoMCGlibiB6b2hyMQwwCgYDVQQL
DANmc2ExGjAYBgNVBAMMEXRlc3QubW9zcXVpdHRvIGMxMSYwJAYJKoZIhvcNAQkB
FhdoYW1kaS5iYWloaWNoQGdtYWlsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEP
ADCCAQoCggEBANaZTAz0mvWxisZ83wqjsVBGZG3sPgDZglP7mwulUgH4J6OtAR16
ylJonaGNUBAVzLxPZvXZRO278O22rJ9duLYRe99CT4O/iBxqnrPfN/YRC5Yy3ot4
BkAGlEbSVKIoEwwjU5Sj+f/qy4kcLt4GwcYffeKVdXly+xAjj4FGRmZujg8q578y
gBLAkWFrJ/CwsAFLQKF9XofnVYnI3zZzbGNro8z7442PtN1T440aYjXm+JOP0DcP
PvNzMd6w71A9A8r6OubeYA/kfMy0J4zUy57wUl8HYY1q1xlTgEmmi6YL0M1LHkhC
lNDrCe6Ann3Hy+CIaXdG1ZIuiPT0f9Bzo/cCAwEAAaMaMBgwCQYDVR0TBAIwADAL
BgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADggEBAEHhRBRb59N6VpI6AZ36lqq6
UjeaPZX8Zh2/VirDCcJRSOFmWt13ft8lTmXfWlTaFnUL9qY4+lgg3R2djs3m2Ifk
9iRg93oNXwCUfhxGG8/ZSYT+Y3rP1eET3Uk6JYF+zhpkOcUr8IuTkpb2+k9xdcnk
aiR+LRZ/QtmbqJ2Yw0vleCOx2mUsIOQjMex0lf5D0GT+94HlZOZFDLa8FTJUNCWX
zlZsmt649Z8QJJ6RzgIKW4JOVsB39wpQaIq4KhYKmXIRKYrzgqDqg9Nys5tbzzUA
oeG0h/z/2Z+I1HX4LeRfk8I2mha7kY9YRDEVq3ug7jA9L4iaKUbJoZIhuDKVVNI=
-----END CERTIFICATE-----)";

const char* client_key = R"(-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDWmUwM9Jr1sYrG
fN8Ko7FQRmRt7D4A2YJT+5sLpVIB+CejrQEdespSaJ2hjVAQFcy8T2b12UTtu/Dt
tqyfXbi2EXvfQk+Dv4gcap6z3zf2EQuWMt6LeAZABpRG0lSiKBMMI1OUo/n/6suJ
HC7eBsHGH33ilXV5cvsQI4+BRkZmbo4PKue/MoASwJFhayfwsLABS0ChfV6H51WJ
yN82c2xja6PM++ONj7TdU+ONGmI15viTj9A3Dz7zczHesO9QPQPK+jrm3mAP5HzM
tCeM1Mue8FJfB2GNatcZU4BJpoumC9DNSx5IQpTQ6wnugJ59x8vgiGl3RtWSLoj0
9H/Qc6P3AgMBAAECggEAZ249ZGUkptSqgV5AFh2tYXZ8AysA+2HaWeYD5YoJy58y
Y8YHqpC7IRsBFpNIimgnZH+UrVvJyBd0WO5ZpvoCA+bLYGDSeDqBPMj6stEcZMH0
ZrEf5/KyeHtzTeskFX/hJlGEgDjETt94uB3YTPTOwlH9V48XrrCHZ1DsYq9fURCH
rzV/bLwgphVXtA86NLR5CRpuq7oNRTCsF8PpANG4Gp9KTt7jQHdQ6HMDAYbflFBF
LXpUTO64+gQRlDRpepa/Grg5nhEkbSKgedFa6nKwaPC/UeRgTpO9Zm0aMqDDCuxl
RH5LGzlZjYQHf9uwRk9Z/2EO7X7eb6kFlfxh6hkx0QKBgQD3NAJSN+6SRLr/BQDu
74OtbjfzVgKzxVrEv035XyyAf12z4UzAPqHhSi4rw3gmtI5yW33CE3ShGR5AGkfc
DGUkNfk269gMNWHIrnbiYXnWWl23GgK51Z/y1RzfYPMSb5CoNOGDE5FJk3GNmDoS
xEQX8BJcouynMC1uLaZRW6wH/wKBgQDePEQw+xeBJx2eqyr69uTIseS13gEljQCh
GAZdGB234X+QcnhboeY4qEKuGSgzW853cDLpv08yKTrPczwfIVDabHpxrIia4Ukb
ewe/5AvPSNvWa35BRKA0dcsTUNI5mjRjPbvUp7eqencN30pYhRKXtH6nm04+YIR4
VoWkXbikCQKBgQCI6itm8keWh66yVEkiDVJ3GhavFbJFc0dEtVgwiAAT43c4i86A
o6/xIa7U2lyPw20p9XZ/qVrtZwYUJvop7EuQdLxlKNbuXBqeldKOq8JZcI97PFLK
LoF6c4KcTgwS5+vM3g0RFiNgBuPbsrZncoDsaTEiUbKVHt/qqCn34bg0bQKBgQDc
UOzdjh/IJ0ojAdgzQs6e9FUjw3ppirbj/ZhZdE4J/KDlR8ZwOTmuU4j/ZetHty0h
lXaz6rgNp3gpLzmcNwAb+k0NIpmuyccbWkXdg6v9jGJ82MYq2GjmeRyhAo/XETv8
YrgyYy8e8BfVBdeDmDFNel/Rs5LHHhJV5pjI0Sz6WQKBgFOWwlsW23UBG8H+y1Bh
xa/Co/YL8DLA6GzKtWmQiSy0DuDt7bHTlmHLMjPP0T/GJ8w8J5ggsmqNxbRQFS3D
wKB7j0cMruLPwZbWf75uvlERhwsb9IXsJJxsDi6mNXbKxRKxnBh75+Is4yFum3rw
eKgvHpMnv7BtTpq8FwzeMCy1
-----END PRIVATE KEY-----)";
// Global objects
WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);
DHTesp dht;

// Function prototypes
void connectWiFi();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishData();

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    dht.setup(DHT_PIN, DHTesp::DHT22);

    connectWiFi();

    // Configure secure client
    secureClient.setCACert(ca_cert);
    secureClient.setCertificate(client_cert);
    secureClient.setPrivateKey(client_key);

    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqttCallback);

    connectMQTT();
}

void loop() {
    if (!mqttClient.connected()) {
        connectMQTT();
    }

    mqttClient.loop();
    publishData();
    delay(2000);
}

void connectWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void connectMQTT() {
    Serial.println("Connecting to MQTT Broker...");
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP32Client")) {
            Serial.println("Connected to MQTT Broker");
            mqttClient.subscribe(topic_sub);
        } else {
            Serial.print("Failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);

    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println("Message: " + message);

    if (message == "toggle_led") {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        Serial.printf("LED toggled to %s\n", digitalRead(LED_PIN) ? "ON" : "OFF");
    }
}

void publishData() {
    TempAndHumidity data = dht.getTempAndHumidity();

    if (isnan(data.temperature) || isnan(data.humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    Serial.printf("Publishing Temp: %.2f°C, Humidity: %.2f%%\n", data.temperature, data.humidity);
    mqttClient.publish(topic_pub_temp, String(data.temperature).c_str());
    mqttClient.publish(topic_pub_hum, String(data.humidity).c_str());
}
