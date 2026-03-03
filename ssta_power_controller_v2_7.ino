/*
 * SSTA Power Controller Firmware v2.5
 * 
 * Laboratory test instrument for SSTC/SSTD operations,
 * calibration, and data collection.
 * 
 * Changes in v2.5:
 * - Added JSON telemetry output for SSTC Controller app compatibility
 * - Added MODE:xxx command for direct mode setting
 * - Added PWR:xxx command for power level setting  
 * - Telemetry sent every second in JSON format
 * - Backward compatible with text commands
 * 
 * Hardware:
 * - Arduino Nano
 * - IBT-2 H-Bridge (BTS7960)
 * - 6x NTC Thermistors (10k, B=3435)
 * - Buck converter providing ~5.7V to H-bridge
 * 
 * Pin Assignments:
 * - A0: T1 (Primary control surface / spreader)
 * - A1: T2 (TEC cold side / TEC-vault interface)
 * - A2: T3 (TEC hot side / TEC-spreader interface) 
 * - A3: T4 (Vault center - in PCM)
 * - A4: T5 (Vault edge - in PCM)
 * - A5: T6 (Ambient reference)
 * - D7: R_EN (H-bridge enable)
 * - D8: L_EN (H-bridge enable)
 * - D9: RPWM (H-bridge PWM A)
 * - D10: LPWM (H-bridge PWM B)
 * 
 * Serial Protocol:
 * - Baud: 115200
 * - Commands IN: STOP, HOLD:xx, COOL:xxx, HEAT:xxx, MODE:xxx, PWR:xxx, TEMPS, RAW, STATUS
 * - Telemetry OUT: JSON every 1s when active
 */

// Pin definitions
const int RPWM = 9;
const int LPWM = 10;
const int R_EN = 7;
const int L_EN = 8;

// Thermistor pins
const int THERM_PINS[6] = {A0, A1, A2, A3, A4, A5};

// Thermistor parameters (calibrated for Garosa NTC B=3435)
const float SERIES_RESISTOR = 10000.0;
const float NOMINAL_RESISTANCE = 10000.0;
const float NOMINAL_TEMP = 25.0;
const float B_COEFFICIENT = 3435.0;

// Safety limits
const float MAX_TEMP = 45.0;      // T1 max - hard safety cutoff (silicone pad adds buffer)
const float MIN_TEMP = 0.0;       // T1 min - hard safety cutoff
const float MAX_T3_TEMP = 60.0;   // T3 max - hot side safety (internal, can be higher)
const unsigned long TIMEOUT_MS = 300000;  // 5 minute timeout (extended for profiles)

// PID parameters
float Kp = 15.0;
float Ki = 0.5;
float Kd = 5.0;

// PID state
float integral = 0;
float lastError = 0;
unsigned long lastPIDTime = 0;

// System state
enum State { IDLE, HOLDING, MANUAL_COOL, MANUAL_HEAT };
State currentState = IDLE;
float targetTemp = 25.0;
int manualPower = 0;
int currentPower = 0;  // Actual PWM being applied
unsigned long lastActivityTime = 0;
unsigned long lastTelemetryTime = 0;

// Temperature readings (stored globally for telemetry)
float temps[6] = {0, 0, 0, 0, 0, 0};
int rawADC[6] = {0, 0, 0, 0, 0, 0};

// Fault state
bool faultActive = false;
String faultReason = "";

void setup() {
  Serial.begin(115200);
  
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);
  
  // Enable H-bridge
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);
  
  // Start with TEC off
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  
  Serial.println(F("{\"event\":\"boot\",\"fw\":\"SSTA_PWR_v2.5\",\"channels\":6}"));
  
  lastActivityTime = millis();
}

float readThermistor(int pin, int index) {
  int raw = analogRead(pin);
  rawADC[index] = raw;
  
  if (raw <= 0 || raw >= 1023) return -999;
  
  float voltage = raw * 5.0 / 1023.0;
  float resistance = SERIES_RESISTOR * voltage / (5.0 - voltage);
  
  float steinhart = resistance / NOMINAL_RESISTANCE;
  steinhart = log(steinhart);
  steinhart /= B_COEFFICIENT;
  steinhart += 1.0 / (NOMINAL_TEMP + 273.15);
  steinhart = 1.0 / steinhart;
  steinhart -= 273.15;
  
  return steinhart;
}

void readAllTemps() {
  for (int i = 0; i < 6; i++) {
    temps[i] = readThermistor(THERM_PINS[i], i);
  }
}

void stopTEC() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
  currentState = IDLE;
  currentPower = 0;
  integral = 0;
  lastError = 0;
}

void setTEC(int power, bool cooling) {
  power = constrain(power, 0, 255);
  currentPower = power;
  if (cooling) {
    analogWrite(RPWM, 0);
    analogWrite(LPWM, power);
  } else {
    analogWrite(RPWM, power);
    analogWrite(LPWM, 0);
  }
}

const char* getModeName() {
  switch(currentState) {
    case IDLE: return "OFF";
    case HOLDING: 
      if (temps[0] > targetTemp) return "COOL";
      else return "HEAT";
    case MANUAL_COOL: return "COOL";
    case MANUAL_HEAT: return "HEAT";
    default: return "OFF";
  }
}

void sendTelemetry() {
  // JSON format for SSTC Controller app
  // Note: App expects ADC values for t1-t5, we send actual temps
  // Modified processData in app will handle this
  
  Serial.print(F("{\"t1\":"));
  Serial.print(temps[0], 1);
  Serial.print(F(",\"t2\":"));
  Serial.print(temps[1], 1);
  Serial.print(F(",\"t3\":"));
  Serial.print(temps[2], 1);
  Serial.print(F(",\"t4\":"));
  Serial.print(temps[3], 1);
  Serial.print(F(",\"t5\":"));
  Serial.print(temps[4], 1);
  Serial.print(F(",\"t6\":"));
  Serial.print(temps[5], 1);
  Serial.print(F(",\"mode\":\""));
  Serial.print(getModeName());
  Serial.print(F("\",\"pwm\":"));
  Serial.print(currentPower);
  Serial.print(F(",\"target\":"));
  if (currentState == HOLDING) {
    Serial.print(targetTemp, 1);
  } else {
    Serial.print(F("null"));
  }
  Serial.print(F(",\"state\":\""));
  Serial.print(currentState == IDLE ? "IDLE" : "ACTIVE");
  Serial.print(F("\""));
  if (faultActive) {
    Serial.print(F(",\"fault\":\""));
    Serial.print(faultReason);
    Serial.print(F("\""));
  }
  Serial.println(F("}"));
}

void runPID() {
  unsigned long now = millis();
  float dt = (now - lastPIDTime) / 1000.0;
  
  if (dt < 0.1) return;  // 10Hz max
  lastPIDTime = now;
  
  float error = targetTemp - temps[0];
  
  integral += error * dt;
  integral = constrain(integral, -100, 100);  // Anti-windup
  
  float derivative = (error - lastError) / dt;
  lastError = error;
  
  float output = Kp * error + Ki * integral + Kd * derivative;
  
  int power = abs((int)output);
  power = constrain(power, 0, 255);
  
  // Deadband
  if (power < 5) power = 0;
  
  bool cooling = (output < 0);
  setTEC(power, cooling);
}

void runManual(bool cooling) {
  setTEC(manualPower, cooling);
}

bool checkSafety() {
  // T1 over temp
  if (temps[0] > MAX_TEMP) {
    faultActive = true;
    faultReason = "T1_OVER";
    return false;
  }
  
  // T1 under temp
  if (temps[0] < MIN_TEMP) {
    faultActive = true;
    faultReason = "T1_UNDER";
    return false;
  }
  
  // T3 over temp (hot side)
  if (temps[2] > MAX_T3_TEMP) {
    faultActive = true;
    faultReason = "T3_OVER";
    return false;
  }
  
  // Timeout
  if (currentState != IDLE && (millis() - lastActivityTime > TIMEOUT_MS)) {
    faultActive = true;
    faultReason = "TIMEOUT";
    return false;
  }
  
  return true;
}

void processCommand(String cmd) {
  cmd.trim();
  cmd.toUpperCase();
  
  lastActivityTime = millis();
  faultActive = false;
  faultReason = "";
  
  if (cmd == "STOP") {
    stopTEC();
    Serial.println(F("{\"ack\":\"STOP\"}"));
  }
  else if (cmd.startsWith("HOLD:")) {
    float temp = cmd.substring(5).toFloat();
    if (temp >= MIN_TEMP && temp <= MAX_TEMP) {
      targetTemp = temp;
      currentState = HOLDING;
      integral = 0;
      lastError = 0;
      Serial.print(F("{\"ack\":\"HOLD\",\"target\":"));
      Serial.print(targetTemp, 1);
      Serial.println(F("}"));
    } else {
      Serial.println(F("{\"error\":\"INVALID_TEMP\"}"));
    }
  }
  else if (cmd.startsWith("COOL:")) {
    int power = cmd.substring(5).toInt();
    if (power >= 0 && power <= 255) {
      manualPower = power;
      currentState = MANUAL_COOL;
      Serial.print(F("{\"ack\":\"COOL\",\"pwm\":"));
      Serial.print(manualPower);
      Serial.println(F("}"));
    }
  }
  else if (cmd.startsWith("HEAT:")) {
    int power = cmd.substring(5).toInt();
    if (power >= 0 && power <= 255) {
      manualPower = power;
      currentState = MANUAL_HEAT;
      Serial.print(F("{\"ack\":\"HEAT\",\"pwm\":"));
      Serial.print(manualPower);
      Serial.println(F("}"));
    }
  }
  else if (cmd.startsWith("MODE:")) {
    String mode = cmd.substring(5);
    mode.trim();
    if (mode == "OFF" || mode == "STOP") {
      stopTEC();
      Serial.println(F("{\"ack\":\"MODE\",\"mode\":\"OFF\"}"));
    } else if (mode == "COOL") {
      currentState = MANUAL_COOL;
      Serial.println(F("{\"ack\":\"MODE\",\"mode\":\"COOL\"}"));
    } else if (mode == "HEAT") {
      currentState = MANUAL_HEAT;
      Serial.println(F("{\"ack\":\"MODE\",\"mode\":\"HEAT\"}"));
    }
  }
  else if (cmd.startsWith("PWR:")) {
    int power = cmd.substring(4).toInt();
    if (power >= 0 && power <= 255) {
      manualPower = power;
      // Apply immediately if in manual mode
      if (currentState == MANUAL_COOL || currentState == MANUAL_HEAT) {
        setTEC(manualPower, currentState == MANUAL_COOL);
      }
      Serial.print(F("{\"ack\":\"PWR\",\"pwm\":"));
      Serial.print(manualPower);
      Serial.println(F("}"));
    }
  }
  else if (cmd == "TEMPS") {
    // Human readable format
    Serial.print(F("T1:"));
    Serial.print(temps[0], 1);
    Serial.print(F(" T2:"));
    Serial.print(temps[1], 1);
    Serial.print(F(" T3:"));
    Serial.print(temps[2], 1);
    Serial.print(F(" T4:"));
    Serial.print(temps[3], 1);
    Serial.print(F(" T5:"));
    Serial.print(temps[4], 1);
    Serial.print(F(" T6:"));
    Serial.println(temps[5], 1);
  }
  else if (cmd == "RAW") {
    for (int i = 0; i < 6; i++) {
      Serial.print(F("A"));
      Serial.print(i);
      Serial.print(F(":"));
      Serial.print(rawADC[i]);
      Serial.print(F(" "));
    }
    Serial.println();
  }
  else if (cmd == "STATUS") {
    sendTelemetry();
  }
  else if (cmd.startsWith("KP:")) {
    Kp = cmd.substring(3).toFloat();
    Serial.print(F("{\"ack\":\"KP\",\"val\":"));
    Serial.print(Kp);
    Serial.println(F("}"));
  }
  else if (cmd.startsWith("KI:")) {
    Ki = cmd.substring(3).toFloat();
    Serial.print(F("{\"ack\":\"KI\",\"val\":"));
    Serial.print(Ki);
    Serial.println(F("}"));
  }
  else if (cmd.startsWith("KD:")) {
    Kd = cmd.substring(3).toFloat();
    Serial.print(F("{\"ack\":\"KD\",\"val\":"));
    Serial.print(Kd);
    Serial.println(F("}"));
  }
  else if (cmd == "PING") {
    Serial.println(F("{\"pong\":true,\"fw\":\"SSTA_PWR_v2.5\"}"));
  }
  else {
    Serial.println(F("{\"error\":\"UNKNOWN_CMD\"}"));
  }
}

void loop() {
  // Read temperatures
  readAllTemps();
  
  // Check serial input
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    processCommand(cmd);
  }
  
  // Safety check
  if (!checkSafety()) {
    stopTEC();
    sendTelemetry();  // Send fault status
  }
  
  // Run current mode
  switch (currentState) {
    case HOLDING:
      runPID();
      break;
    case MANUAL_COOL:
      runManual(true);
      break;
    case MANUAL_HEAT:
      runManual(false);
      break;
    case IDLE:
    default:
      break;
  }
  
  // Send telemetry every second when active
  unsigned long now = millis();
  if (now - lastTelemetryTime >= 1000) {
    lastTelemetryTime = now;
    if (currentState != IDLE) {
      sendTelemetry();
    }
  }
  
  delay(50);  // 20Hz loop
}
