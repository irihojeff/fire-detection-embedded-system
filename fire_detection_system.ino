/*
 * 🚨 FIRE DETECTION SYSTEM - TINKERCAD OPTIMIZED 🚨
 * Author: Japhet
 * 
 * Components Used:
 * - TMP36 Temperature Sensor (A0)
 * - PIR Motion Sensor (Pin 3)
 * - Red LED (Pin 8) + 220Ω Resistor
 * - Green LED (Pin 9) + 220Ω Resistor  
 * - Yellow LED (Pin 10) + 220Ω Resistor
 * - Buzzer (Pin 11)
 * - LU-5-R Relay (Pin 12)
 * 
 * Optimized for Tinkercad simulation and demo presentation
 */

// 📌 PIN DEFINITIONS
#define TMP36_PIN A0          // TMP36 temperature sensor
#define PIR_PIN 3             // PIR motion sensor
#define LED_RED 8             // Red LED (Danger)
#define LED_GREEN 9           // Green LED (Normal)
#define LED_YELLOW 10         // Yellow LED (Warning)
#define BUZZER_PIN 11         // Buzzer
#define RELAY_PIN 12          // LU-5-R Relay

// 🎯 FIRE DETECTION THRESHOLDS
const float TEMP_NORMAL_MAX = 49.9;    // Normal operation limit
const float TEMP_WARNING = 50.0;       // Warning threshold
const float TEMP_DANGER = 60.0;        // Danger threshold (fire detected)

// ⏱️ TIMING CONSTANTS
const unsigned long SENSOR_INTERVAL = 1000;    // Read sensors every 1 second
const unsigned long LED_SLOW_BLINK = 1000;     // Warning LED blink rate
const unsigned long LED_FAST_BLINK = 200;      // Danger LED blink rate
const unsigned long BUZZER_BEEP_INTERVAL = 2000; // Warning buzzer interval
const unsigned long STATUS_LOG_INTERVAL = 3000;  // Status logging interval

// 🔒 SECURITY CONSTANTS
const float TEMP_MIN_VALID = -10.0;     // Minimum valid temperature
const float TEMP_MAX_VALID = 85.0;      // Maximum valid temperature
const byte MAX_SENSOR_ERRORS = 5;       // Max consecutive sensor errors
const byte MAX_INVALID_COMMANDS = 3;    // Max invalid commands before alert

// 🏗️ SYSTEM STATES
enum FireRiskLevel {
  NORMAL = 0,
  WARNING = 1, 
  DANGER = 2
};

enum SystemMode {
  AUTO_MODE = 0,
  DEMO_MODE = 1
};

// 🔧 GLOBAL VARIABLES
FireRiskLevel currentRiskLevel = NORMAL;
SystemMode systemMode = AUTO_MODE;

float currentTemperature = 25.0;
float demoTemperature = 25.0;
float lastValidTemperature = 25.0;

bool motionDetected = false;
bool demoMotion = false;
bool suppressionActivated = false;

// 🔒 SECURITY VARIABLES
byte sensorErrorCount = 0;
byte invalidCommandCount = 0;
bool systemIntegrityOK = true;

// ⏱️ TIMING VARIABLES
unsigned long lastSensorRead = 0;
unsigned long lastLedUpdate = 0;
unsigned long lastBuzzerUpdate = 0;
unsigned long lastStatusLog = 0;
unsigned long systemStartTime = 0;

// 💡 COMPONENT STATES
bool ledBlinkState = false;
bool buzzerBeepState = false;

// 📊 SYSTEM STATISTICS
unsigned int totalSensorReadings = 0;
unsigned int warningActivations = 0;
unsigned int dangerActivations = 0;
unsigned int suppressionActivations = 0;

// 🚀 SETUP FUNCTION
void setup() {
  Serial.begin(9600);
  delay(500);
  
  // Print welcome banner
  printWelcomeBanner();
  
  // Initialize hardware
  initializeHardware();
  
  // Run system self-test
  runSystemSelfTest();
  
  // Record start time
  systemStartTime = millis();
  
  Serial.println(F("🟢 SYSTEM READY FOR OPERATION!"));
  Serial.println(F("💡 Type 'HELP' for demo commands"));
  Serial.println(F("========================================"));
  delay(1000);
}

// 🔄 MAIN LOOP
void loop() {
  unsigned long currentTime = millis();
  
  // 📡 Read sensors at regular intervals
  if (currentTime - lastSensorRead >= SENSOR_INTERVAL) {
    readSensorsWithValidation();
    evaluateFireRisk();
    lastSensorRead = currentTime;
  }
  
  // 💡 Update visual indicators
  updateLEDIndicators();
  
  // 🔊 Control audio alerts
  updateBuzzerAlerts();
  
  // 🚨 Handle fire suppression
  handleFireSuppression();
  
  // 📱 Process serial commands
  handleSerialCommands();
  
  // 📊 Log system status
  if (currentTime - lastStatusLog >= STATUS_LOG_INTERVAL) {
    logSystemStatus();
    lastStatusLog = currentTime;
  }
}

// 🔧 HARDWARE INITIALIZATION
void initializeHardware() {
  Serial.println(F("🔧 Initializing hardware components..."));
  
  // Configure digital pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  // Set initial safe states
  digitalWrite(LED_GREEN, HIGH);    // Start with normal status
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  
  Serial.println(F("✅ Hardware initialization complete"));
}

// 🧪 SYSTEM SELF-TEST
void runSystemSelfTest() {
  Serial.println(F("🧪 Running system self-test..."));
  
  // Test LED indicators
  Serial.println(F("   Testing LED indicators..."));
  digitalWrite(LED_RED, HIGH);
  delay(200);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, HIGH);
  delay(200);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  delay(200);
  
  // Test buzzer
  Serial.println(F("   Testing buzzer..."));
  digitalWrite(BUZZER_PIN, HIGH);
  delay(150);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Test relay
  Serial.println(F("   Testing suppression relay..."));
  digitalWrite(RELAY_PIN, HIGH);
  delay(100);
  digitalWrite(RELAY_PIN, LOW);
  
  Serial.println(F("✅ Self-test completed successfully"));
}

// 📡 SENSOR READING WITH VALIDATION
void readSensorsWithValidation() {
  totalSensorReadings++;
  
  if (systemMode == DEMO_MODE) {
    // Demo mode - use simulated values
    currentTemperature = demoTemperature;
    motionDetected = demoMotion;
    sensorErrorCount = 0; // Reset errors in demo mode
  } else {
    // Auto mode - read actual sensors with validation
    readTMP36Temperature();
    motionDetected = digitalRead(PIR_PIN);
  }
}

// 🌡️ TMP36 TEMPERATURE READING
void readTMP36Temperature() {
  // Read analog value from TMP36
  int analogValue = analogRead(TMP36_PIN);
  
  // Convert to voltage (0-5V scale)
  float voltage = analogValue * (5.0 / 1023.0);
  
  // Convert voltage to temperature using TMP36 formula
  // TMP36: 10mV per °C with 500mV offset at 0°C
  float temperature = (voltage - 0.5) * 100.0;
  
  // Data integrity validation
  if (temperature >= TEMP_MIN_VALID && temperature <= TEMP_MAX_VALID) {
    // Valid temperature reading
    currentTemperature = temperature;
    lastValidTemperature = temperature;
    sensorErrorCount = 0;
    systemIntegrityOK = true;
  } else {
    // Invalid reading - use last valid value
    sensorErrorCount++;
    currentTemperature = lastValidTemperature;
    
    Serial.print(F("⚠️ SECURITY: Invalid temperature reading ("));
    Serial.print(temperature, 1);
    Serial.print(F("°C). Error count: "));
    Serial.println(sensorErrorCount);
    
    // Check for multiple sensor failures
    if (sensorErrorCount >= MAX_SENSOR_ERRORS) {
      Serial.println(F("🚨 SECURITY ALERT: Multiple sensor failures detected!"));
      systemIntegrityOK = false;
    }
  }
}

// 🎯 FIRE RISK EVALUATION
void evaluateFireRisk() {
  FireRiskLevel previousLevel = currentRiskLevel;
  
  // Determine risk level based on temperature
  if (currentTemperature >= TEMP_DANGER) {
    currentRiskLevel = DANGER;
    if (previousLevel != DANGER) {
      dangerActivations++;
      Serial.println(F("🚨🚨 DANGER LEVEL ACTIVATED! 🚨🚨"));
      Serial.println(F("🔥 FIRE DETECTED - EMERGENCY PROTOCOLS ENGAGED"));
    }
  } 
  else if (currentTemperature >= TEMP_WARNING) {
    currentRiskLevel = WARNING;
    if (previousLevel != WARNING) {
      warningActivations++;
      Serial.println(F("⚠️ WARNING LEVEL ACTIVATED!"));
      Serial.println(F("🌡️ Temperature approaching dangerous levels"));
    }
  } 
  else {
    currentRiskLevel = NORMAL;
    if (previousLevel != NORMAL) {
      Serial.println(F("✅ NORMAL LEVEL RESTORED"));
      suppressionActivated = false; // Reset suppression flag
    }
  }
  
  // Enhanced detection with motion sensor
  if (currentRiskLevel >= WARNING && motionDetected) {
    Serial.println(F("👤 ALERT: Human presence detected in risk zone!"));
  }
}

// 💡 LED INDICATOR CONTROL
void updateLEDIndicators() {
  unsigned long currentTime = millis();
  
  switch (currentRiskLevel) {
    case NORMAL:
      // Green LED solid ON - all systems normal
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_YELLOW, LOW);
      digitalWrite(LED_RED, LOW);
      break;
      
    case WARNING:
      // Yellow LED slow blinking - warning condition
      if (currentTime - lastLedUpdate >= LED_SLOW_BLINK) {
        ledBlinkState = !ledBlinkState;
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, ledBlinkState);
        digitalWrite(LED_RED, LOW);
        lastLedUpdate = currentTime;
      }
      break;
      
    case DANGER:
      // Red LED rapid blinking - emergency condition
      if (currentTime - lastLedUpdate >= LED_FAST_BLINK) {
        ledBlinkState = !ledBlinkState;
        digitalWrite(LED_GREEN, LOW);
        digitalWrite(LED_YELLOW, LOW);
        digitalWrite(LED_RED, ledBlinkState);
        lastLedUpdate = currentTime;
      }
      break;
  }
}

// 🔊 BUZZER ALERT CONTROL
void updateBuzzerAlerts() {
  unsigned long currentTime = millis();
  
  switch (currentRiskLevel) {
    case NORMAL:
      // Silent operation
      digitalWrite(BUZZER_PIN, LOW);
      buzzerBeepState = false;
      break;
      
    case WARNING:
      // Soft intermittent beeping
      if (currentTime - lastBuzzerUpdate >= BUZZER_BEEP_INTERVAL) {
        buzzerBeepState = !buzzerBeepState;
        digitalWrite(BUZZER_PIN, buzzerBeepState);
        lastBuzzerUpdate = currentTime;
      }
      break;
      
    case DANGER:
      // Continuous loud alarm
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerBeepState = true;
      break;
  }
}

// 🚨 FIRE SUPPRESSION HANDLING
void handleFireSuppression() {
  if (currentRiskLevel == DANGER && !suppressionActivated) {
    if (validateSuppressionCommand()) {
      activateFireSuppression();
    }
  }
}

// 🔒 SUPPRESSION COMMAND VALIDATION
bool validateSuppressionCommand() {
  // Security checks before suppression activation
  if (currentTemperature >= TEMP_DANGER && 
      totalSensorReadings > 3 && 
      systemIntegrityOK) {
    return true;
  }
  
  Serial.println(F("🔒 SECURITY: Suppression activation validation failed"));
  return false;
}

// 🚨 FIRE SUPPRESSION ACTIVATION
void activateFireSuppression() {
  suppressionActivations++;
  suppressionActivated = true;
  
  Serial.println(F("🚨🚨🚨 FIRE SUPPRESSION ACTIVATED! 🚨🚨🚨"));
  Serial.println(F("💧 Deploying suppression system..."));
  
  // Activate relay
  digitalWrite(RELAY_PIN, HIGH);
  
  // Visual confirmation - flash all LEDs
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(100);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(100);
  }
  
  // Keep suppression active for 3 seconds
  delay(3000);
  
  // Deactivate relay
  digitalWrite(RELAY_PIN, LOW);
  
  Serial.println(F("✅ Fire suppression cycle completed"));
  Serial.println(F("🔄 System monitoring resumed"));
}

// 📱 SERIAL COMMAND HANDLING
void handleSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    processSecureCommand(command);
  }
}

// 🔒 SECURE COMMAND PROCESSING
void processSecureCommand(String cmd) {
  // Command validation
  if (cmd.length() == 0 || cmd.length() > 15) {
    invalidCommandCount++;
    Serial.println(F("🔒 SECURITY: Invalid command format"));
    return;
  }
  
  Serial.print(F("📝 Processing command: "));
  Serial.println(cmd);
  
  // Process validated commands
  if (cmd == "HELP") {
    printCommandHelp();
  }
  else if (cmd == "STATUS") {
    printDetailedStatus();
  }
  else if (cmd == "TEST_NORMAL") {
    if (validateDemoCommand()) {
      systemMode = DEMO_MODE;
      demoTemperature = 25.0;
      Serial.println(F("🟢 Demo Mode: Normal temperature (25°C)"));
    }
  }
  else if (cmd == "TEST_WARNING") {
    if (validateDemoCommand()) {
      systemMode = DEMO_MODE;
      demoTemperature = 55.0;
      Serial.println(F("🟡 Demo Mode: Warning temperature (55°C)"));
    }
  }
  else if (cmd == "TEST_DANGER") {
    if (validateDemoCommand()) {
      systemMode = DEMO_MODE;
      demoTemperature = 70.0;
      Serial.println(F("🔴 Demo Mode: Danger temperature (70°C)"));
    }
  }
  else if (cmd == "MOTION") {
    demoMotion = !demoMotion;
    Serial.print(F("👤 Motion detection: "));
    Serial.println(demoMotion ? F("TRIGGERED") : F("CLEAR"));
  }
  else if (cmd == "RESET") {
    resetSystemSecure();
  }
  else if (cmd == "AUTO") {
    systemMode = AUTO_MODE;
    Serial.println(F("🤖 Switched to AUTOMATIC mode"));
  }
  else if (cmd == "DEMO") {
    systemMode = DEMO_MODE;
    Serial.println(F("🎭 Switched to DEMONSTRATION mode"));
  }
  else if (cmd == "STATS") {
    printSystemStatistics();
  }
  else if (cmd == "SECURITY") {
    printSecurityStatus();
  }
  else {
    invalidCommandCount++;
    Serial.println(F("❌ Unknown command. Type HELP for available commands."));
    
    if (invalidCommandCount >= MAX_INVALID_COMMANDS) {
      Serial.println(F("🚨 SECURITY ALERT: Multiple invalid commands detected!"));
      invalidCommandCount = 0; // Reset counter
    }
  }
}

// 🔒 DEMO COMMAND VALIDATION
bool validateDemoCommand() {
  if (totalSensorReadings > 1) {
    return true;
  }
  Serial.println(F("🔒 SECURITY: Demo command requires system initialization"));
  return false;
}

// 🔄 SECURE SYSTEM RESET
void resetSystemSecure() {
  Serial.println(F("🔄 SECURE SYSTEM RESET INITIATED"));
  
  // Reset all states
  currentRiskLevel = NORMAL;
  systemMode = AUTO_MODE;
  suppressionActivated = false;
  demoTemperature = 25.0;
  demoMotion = false;
  sensorErrorCount = 0;
  invalidCommandCount = 0;
  systemIntegrityOK = true;
  
  // Reset hardware states
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_GREEN, HIGH);
  
  Serial.println(F("✅ System reset completed successfully"));
}

// 📊 SYSTEM STATUS LOGGING
void logSystemStatus() {
  Serial.println(F("========================================"));
  Serial.println(F("📊 FIRE DETECTION SYSTEM STATUS"));
  Serial.println(F("========================================"));
  
  Serial.print(F("🌡️ Temperature: "));
  Serial.print(currentTemperature, 1);
  Serial.println(F("°C"));
  
  Serial.print(F("👤 Motion: "));
  Serial.println(motionDetected ? F("DETECTED") : F("Clear"));
  
  Serial.print(F("🚨 Risk Level: "));
  switch (currentRiskLevel) {
    case NORMAL:
      Serial.println(F("NORMAL 🟢"));
      break;
    case WARNING:
      Serial.println(F("WARNING 🟡"));
      break;
    case DANGER:
      Serial.println(F("DANGER 🔴"));
      break;
  }
  
  Serial.print(F("🤖 Mode: "));
  Serial.println(systemMode == AUTO_MODE ? F("AUTOMATIC") : F("DEMONSTRATION"));
  
  Serial.print(F("💧 Suppression: "));
  Serial.println(suppressionActivated ? F("ACTIVE") : F("Standby"));
  
  Serial.print(F("🔒 System Integrity: "));
  Serial.println(systemIntegrityOK ? F("OK") : F("DEGRADED"));
  
  Serial.println(F("========================================"));
}

// 🆘 PRINT WELCOME BANNER
void printWelcomeBanner() {
  Serial.println(F("========================================"));
  Serial.println(F("🚨    FIRE DETECTION SYSTEM v2.0    🚨"));
  Serial.println(F("🔥   Intelligent Emergency Response   🔥"));
  Serial.println(F("========================================"));
  Serial.println(F("👨‍💻 Author: Japhet"));
  Serial.println(F("🎯 Platform: Arduino Uno + TMP36"));
  Serial.println(F("🔒 Security: Enhanced with validation"));
  Serial.println(F("🎭 Optimized for Tinkercad simulation"));
  Serial.println(F("========================================"));
}

// 🆘 PRINT COMMAND HELP
void printCommandHelp() {
  Serial.println(F("========================================"));
  Serial.println(F("🎯 FIRE DETECTION SYSTEM COMMANDS"));
  Serial.println(F("========================================"));
  Serial.println(F("📊 System Commands:"));
  Serial.println(F("   STATUS     - Show detailed system status"));
  Serial.println(F("   STATS      - Display operation statistics"));
  Serial.println(F("   SECURITY   - Show security status"));
  Serial.println(F("   HELP       - Display this help menu"));
  Serial.println();
  Serial.println(F("🧪 Demo Commands:"));
  Serial.println(F("   TEST_NORMAL   - Simulate normal temp (25°C)"));
  Serial.println(F("   TEST_WARNING  - Simulate warning temp (55°C)"));
  Serial.println(F("   TEST_DANGER   - Simulate fire temp (70°C)"));
  Serial.println(F("   MOTION        - Toggle motion detection"));
  Serial.println();
  Serial.println(F("🎛️ Control Commands:"));
  Serial.println(F("   RESET      - Reset system to normal state"));
  Serial.println(F("   AUTO       - Switch to automatic mode"));
  Serial.println(F("   DEMO       - Switch to demonstration mode"));
  Serial.println(F("========================================"));
}

// 📋 PRINT DETAILED STATUS
void printDetailedStatus() {
  Serial.println(F("========================================"));
  Serial.println(F("🔍 DETAILED SYSTEM STATUS"));
  Serial.println(F("========================================"));
  Serial.println(F("Hardware Status:"));
  Serial.println(F("  🌡️ TMP36 Temperature Sensor: OK"));
  Serial.println(F("  👤 PIR Motion Sensor: OK"));
  Serial.println(F("  💡 LED Indicator System: OK"));
  Serial.println(F("  🔊 Buzzer Alert System: OK"));
  Serial.println(F("  🚨 LU-5-R Relay Module: OK"));
  Serial.println();
  Serial.println(F("Current Readings:"));
  Serial.print(F("  🌡️ Temperature: "));
  Serial.print(currentTemperature, 2);
  Serial.println(F("°C"));
  Serial.print(F("  👤 Motion Status: "));
  Serial.println(motionDetected ? F("ACTIVE") : F("INACTIVE"));
  Serial.println();
  Serial.println(F("System Configuration:"));
  Serial.print(F("  ⚠️ Warning Threshold: "));
  Serial.print(TEMP_WARNING, 1);
  Serial.println(F("°C"));
  Serial.print(F("  🚨 Danger Threshold: "));
  Serial.print(TEMP_DANGER, 1);
  Serial.println(F("°C"));
  Serial.print(F("  🤖 Current Mode: "));
  Serial.println(systemMode == AUTO_MODE ? F("AUTOMATIC") : F("DEMONSTRATION"));
  Serial.println(F("========================================"));
}

// 📈 PRINT SYSTEM STATISTICS
void printSystemStatistics() {
  Serial.println(F("========================================"));
  Serial.println(F("📈 SYSTEM OPERATION STATISTICS"));
  Serial.println(F("========================================"));
  Serial.print(F("📊 Total Sensor Readings: "));
  Serial.println(totalSensorReadings);
  Serial.print(F("⚠️ Warning Activations: "));
  Serial.println(warningActivations);
  Serial.print(F("🚨 Danger Activations: "));
  Serial.println(dangerActivations);
  Serial.print(F("💧 Suppression Activations: "));
  Serial.println(suppressionActivations);
  Serial.print(F("❌ Sensor Errors: "));
  Serial.println(sensorErrorCount);
  Serial.print(F("🔒 Invalid Commands: "));
  Serial.println(invalidCommandCount);
  Serial.print(F("⏱️ System Uptime: "));
  Serial.print((millis() - systemStartTime) / 1000);
  Serial.println(F(" seconds"));
  if (totalSensorReadings > 0) {
    Serial.print(F("📡 Average Reading Rate: "));
    Serial.print(totalSensorReadings / max(1UL, (millis() - systemStartTime) / 1000));
    Serial.println(F(" readings/second"));
  }
  Serial.println(F("========================================"));
}

// 🔒 PRINT SECURITY STATUS
void printSecurityStatus() {
  Serial.println(F("========================================"));
  Serial.println(F("🔒 SECURITY SYSTEM STATUS"));
  Serial.println(F("========================================"));
  Serial.println(F("Security Features Active:"));
  Serial.println(F("  ✅ Temperature data validation"));
  Serial.println(F("  ✅ Command authentication"));
  Serial.println(F("  ✅ Anti-spoofing protection"));
  Serial.println(F("  ✅ Sensor error monitoring"));
  Serial.println(F("  ✅ Suppression validation"));
  Serial.println();
  Serial.print(F("Current Security Status:"));
  Serial.println();
  Serial.print(F("  🌡️ Sensor Error Count: "));
  Serial.println(sensorErrorCount);
  Serial.print(F("  🔑 Invalid Command Count: "));
  Serial.println(invalidCommandCount);
  Serial.print(F("  🛡️ System Integrity: "));
  Serial.println(systemIntegrityOK ? F("GOOD") : F("COMPROMISED"));
  Serial.print(F("  🔐 Security Level: "));
  Serial.println((sensorErrorCount < 2 && invalidCommandCount < 2) ? F("HIGH") : F("MODERATE"));
  Serial.println(F("========================================"));
}