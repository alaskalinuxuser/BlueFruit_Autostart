  #include <bluefruit.h>

  BLEUart bleuart;
  BLEDis bledis;    // DIS (Device Information Service) helper class instance

  
  // Function prototypes for packetparser.cpp
  uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
  float   parsefloat (uint8_t *buffer);
  void    printHex   (const uint8_t * data, const uint32_t numBytes);
  
  // Packet buffer
  extern uint8_t packetbuffer[];
  
  int brightness = 0;    // how bright the LED is
  int fadeAmount = 10;    // how many points to fade the LED by
  int crankTimer = -1; // Time spent cranking engine.
  int startTimer = -1; // Time spent between start and crank.
  int runTimer = -1; // Time to run before auto shutoff.
  int startAttempts = -1; // Number of start attempts.
  boolean runBool = false;
  
  void setup(void) {
  Serial.begin(115200);
  Serial.println(F("-------------------------------------------"));
  Serial.println(F("-Homemade autostart by the alaskalinuxuser-"));

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Bluefruit_AlaskaLinuxUser");
  
  // Configure and start the BLE Uart service
  bleuart.begin();

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Set up and start advertising
  startAdv();

  pinMode(17, OUTPUT); //19 is blue LED, 17 is red LED.
  pinMode(19, OUTPUT); //19 is blue LED, 17 is red LED.
  
  pinMode(16, OUTPUT); // 16 is the access/fuel pump on.
  pinMode(15, OUTPUT); // 15 is the cranking on.
  pinMode(7, OUTPUT); // 7 is the access/fuel pump off.
  pinMode(11, OUTPUT); // 11 is the cranking off.
  pinMode(30, INPUT); 
  // Pin 30 will be digital input (5vdc) that brakes were pressed.
  
  digitalWrite(16, LOW);
  digitalWrite(15, LOW);
  digitalWrite(7, LOW);
  digitalWrite(11, LOW);
  analogWrite(19, 0);


}
 
void startAdv(void) {
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void connect_callback(uint16_t conn_handle) {
  (void) conn_handle;
  Serial.println("Connected");
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason) {
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

void loop(void) {

  delay(250); // Want a 500 ms delay, but as a timing issue, I am
  // breaking this in half to iron out the timing on the circuit.
  
  analogWrite(17, brightness);
  digitalWrite(16, LOW);  // Conserve power, since these are 
  digitalWrite(15, LOW);  // Latching relays, they don't need
  digitalWrite(7, LOW);  // to be on all the time. In between
  digitalWrite(11, LOW);  // runs they will be off.

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 50) {
    fadeAmount = -fadeAmount;
  }
  // wait for 500 milliseconds (with two 250ms) to see the dimming effect
  delay(250);
  // To print the brightness // Serial.println(brightness);
  // This really does serve a purpose. This tells me there is power,
  // and that the program is running, as the brightness changes.
  
  // read the input on digital pin 30:
  int sensorValue = digitalRead(30);
  // print out the value you read:
  if (sensorValue == 1) {
     Serial.println("Pin 30 HIGH ");
     // Brakes have been pushed, shut down all autostart circuits.
     // This will not affect actual engine run circuits, just the autostart
     // ones. So the engine will stay running if the key is in and on.
     digitalWrite(16, LOW);
     digitalWrite(15, LOW);
     digitalWrite(7, HIGH);
     digitalWrite(11, HIGH);
     crankTimer = -1;
     startTimer = -1;
     runTimer = -1;
     startAttempts = -1;
     Serial.println(" Kill All - Brakes pressed. ");
  }

  // read the input on analog pin 3:
  int voltValue = analogRead(A3);
  float voltVoltage = voltValue * (3.6 / 1023.0);
  if (voltVoltage >= 0.9) {
      // It has enough voltage, it is running.
      Serial.print("Greater than 13v, Pin A3: ");Serial.println(voltVoltage);
      runBool = true;
  } else {
    // Not enough voltage, must not be running.
    runBool = false;
    Serial.print("Less than 13v, Pin A3: ");Serial.println(voltVoltage);
  }

  if (runBool) {
    // Since it is running, make sure we are not cranking!
    crankTimer = -1;
    startTimer = -1;
    digitalWrite(15, LOW);
    digitalWrite(11, HIGH);
  }

  if (runTimer == 0) {
        // Turn everything off from the runTimer.
        digitalWrite(16, LOW);
        digitalWrite(15, LOW);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        crankTimer = -1;
        startTimer = -1;
        runTimer = -1;
        startAttempts = -1;
        Serial.println(" Kill All - runTimer elapsed. ");
  } else if (runTimer > 0) {
    runTimer = runTimer -1;
    Serial.print (" runTimer "); Serial.println(runTimer);
  }

  if (startTimer == 0 && startAttempts > 0) {
    digitalWrite(11, LOW);
    digitalWrite(15, HIGH);
    crankTimer = 4;
    startTimer = -1;
    Serial.println (" crankTimer ON ");
  } else if (startTimer == 0 && startAttempts == 0) {
        // Turn everything off from to many attempts.
        digitalWrite(16, LOW);
        digitalWrite(15, LOW);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        crankTimer = -1;
        startTimer = -1;
        runTimer = -1;
        startAttempts = -1;
        Serial.println(" Kill All - Too many attempts. ");
  } else if (startTimer > 0) {
    startTimer = startTimer -1;
    Serial.print (" startTimer "); Serial.println(startTimer);
  }

  if (crankTimer == 0) {
    digitalWrite(15, LOW);
    digitalWrite(11, HIGH);
    if (runBool) {
      crankTimer = -1;
      startTimer = -1;
      startAttempts = -1;
      runTimer = 600; // 600 seconds = 10 minutes of run time before shutdown.
      Serial.println (" crankTimer OFF, runTimer started. ");
    } else {
      crankTimer = -1;
      startTimer = 5; // To start process again.
      runTimer = -1;
      startAttempts = startAttempts -1;
      Serial.println (" Not running, try again... "); Serial.println(startAttempts);
    }
  } else if (crankTimer > 0) {
    crankTimer = crankTimer -1;
    Serial.print (" crankTimer "); Serial.println(crankTimer);
  }
    

  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 500);
  if (len == 0) return;

  // Buttons
  if (packetbuffer[1] == 'B') {
    uint8_t buttnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    Serial.print ("Button "); Serial.print(buttnum);
    if (pressed) {
      Serial.println(" pressed");
      //bluart.print("pressed");
      analogWrite(19, 50);
    } else {
      Serial.println(" released");
      analogWrite(19, 0);
    }
    if (buttnum == 1) { 
      // Turn on vehicle, by turning on accessory, fuel pump,
      // and start timer to crank.
      digitalWrite(7, LOW);
      digitalWrite(16, HIGH);
      startTimer = 5; // Delay time before cranking.
      startAttempts = 3; // Number of tries to start.
    } else if (buttnum == 2) {
        // Turn everything off from the button.
        digitalWrite(16, LOW);
        digitalWrite(15, LOW);
        digitalWrite(7, HIGH);
        digitalWrite(11, HIGH);
        crankTimer = -1;
        startTimer = -1;
        runTimer = -1;
        startAttempts = -1;
        Serial.println(" Kill All - Button pressed. ");
    }
  }
    
}
