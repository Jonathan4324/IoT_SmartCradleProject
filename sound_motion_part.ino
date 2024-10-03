#define BLYNK_TEMPLATE_ID "TMPL20H7M36co"
#define BLYNK_TEMPLATE_NAME "Sound and Motion"
#define BLYNK_AUTH_TOKEN "YKTKA8mYE1xtAfUw9iN6xgEiLMfLynef" 

// when we use blynk for project, related ID, NAME and AUTH_TOKEN must be written first.

#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <Servo.h> //   Servo library

// Ultrasonic sensor pins
const int trigPin = D5;  // Trigger pin
const int echoPin = D6;  // Echo pin

// WiFi credentials
char ssid[] = "GUSTO_209";   
char pass[] = "Gusto@123";  

// Initialize the LCD with I2C address 0x27 (this may vary; check your LCD's I2C address)
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 LCD display

// Define the pins
const int sensorPin = A0;    // Sound sensor connected to analog pin A0
const int buzzerPin = D4;    // Buzzer connected to digital pin D4
int sensorValue = 0;         // Variable to store the sensor value

// Define the servo pin
#define SERVO_PIN D7  // Pin connected to the servo motor (MG996R) //board

// Create a Servo object
Servo myServo;


bool soundEventLogged = false;  // To avoid logging the sound event multiple times
bool motionEventLogged = false;  // To avoid logging the motion event multiple times
bool servoActive = false;      // Track the state of the servo motor

BLYNK_WRITE(V0) {
  int position = param.asInt();
  myServo.write(position);// Get value from the Blynk switch (0 or 1)
  Blynk.virtualWrite(V1, position);

}


void cradle() {
//you begin your own personal code for servo here
  int pos;

  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(0);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 0; pos >= 180; pos -= 1) { // goes from 180 degrees to 0 degrees
    myServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(1);                       // waits 15ms for the servo to reach the position
  }
}

BLYNK_WRITE(V2) 
{
  int pinValue = param.asInt();
  if (pinValue == 1) {    // if Button sends 1
    cradle();             // start the function cradle
    Blynk.run(); // Run rest of show in-between waiting for this loop to repeat or quit.
    int pinValue = 0;  // Set V3 status to 0 to quit, unless button is still pushed (as per below)
    Blynk.syncVirtual(V2); // ...Then force BLYNK_WRITE(V3) function check of button status to determine if repeating or done.
  }
}

void setup() {
  // Start serial communication
  Serial.begin(115200);  // ESP8266 typically uses a higher baud rate

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();   // Turn on the LCD backlight

  // Initialize the buzzer pin as OUTPUT
  pinMode(buzzerPin, OUTPUT);
  //noTone(buzzerPin, LOW);  // Make sure the buzzer is off initially

  // Initialize ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize the servo motor
  myServo.attach(SERVO_PIN, 544, 2400); // guessing last properties wll increase the speed of the fan
  myServo.write(0);  // Start with the servo at 0 degrees (OFF position)

  // Display a startup message on the LCD
  lcd.setCursor(0, 0);
  lcd.print("Sound Sensor Init");
  delay(2000);  // Display for 2 seconds
  lcd.clear();
}



// Function to handle Blynk switch control for the servo motor


void loop() {
  // Run Blynk routine
  Blynk.run();

  // Read the analog value from the sound sensor
  sensorValue = analogRead(sensorPin);

  // Print the sensor value to the Serial Monitor (for debugging)
  Serial.print("Sound Sensor Value: ");
  Serial.println(sensorValue);

  // Send the sensor value to Blynk (Virtual Pin V0)
  Blynk.virtualWrite(V0, sensorValue);

  // Display the sensor value on the LCD
  lcd.setCursor(0, 0);
  lcd.print("Sound: ");
  lcd.print(sensorValue);

  // Check for loud sound and activate buzzer if sound exceeds threshold
  if (sensorValue > 50) {  // Example threshold for sound detection, adjust as needed
    Serial.println("Loud sound detected!");

    // Display "Loud Noise" on the LCD
    lcd.setCursor(0, 1);  // Move to the second line
    lcd.print("Noise Detected");

    // Turn the buzzer on
    tone(buzzerPin, 1000, 5000);

    // Log event to Blynk if not already logged
    if (!soundEventLogged) {
      Blynk.logEvent("sound_and_motion", "Baby is crying");  // Log event for loud noise
      soundEventLogged = true;  // Set to true to prevent multiple logs
    }

    // Activate the servo automatically
    myServo.write(180);  // Move the servo to 90 degrees (ON position)
    servoActive = true;

  } else {
    Serial.println("No significant sound.");

    // Display "Quiet" on the LCD
    lcd.setCursor(0, 1);  // Move to the second line
    lcd.print("Quiet        "); // Extra spaces to clear any leftover characters

    // Turn the buzzer off
    noTone(buzzerPin);

    // Reset the sound event flag
    soundEventLogged = false;
  }

  // Ultrasonic sensor logic for detecting motion
  long duration, distance;
  
  // Trigger the ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo pin and calculate distance
  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.034) / 2;  // Calculate distance in cm

  // Print the distance to the Serial Monitor (for debugging)
  Serial.print("Distance: ");
  Serial.println(distance);

  // Check if distance is below a threshold (indicating movement)
  if (distance > 0 && distance < 50) {  // Example threshold for motion detection, adjust as needed
    Serial.println("Motion detected!");

    // Display "Motion Detected" on the LCD
    lcd.setCursor(0, 1);
    lcd.print("Motion Detected");

    // Turn the buzzer on
    tone(buzzerPin, 1000, 5000);

    // Log event to Blynk if not already logged
    if (!motionEventLogged) {
      Blynk.logEvent("ultra_sonic_sensor", "Baby is awake");  // Log event for motion detection
      motionEventLogged = true;  // Set to true to prevent multiple logs
    }

    // Activate the servo automatically
    myServo.write(180);  // Move the servo to 90 degrees (ON position)
    servoActive = true;

  } else {
    Serial.println("No motion.");

    // Reset the motion event flag
    motionEventLogged = false;
    noTone(buzzerPin);
  }

  // Deactivate the servo if no sound or motion is detected
  if (!soundEventLogged && !motionEventLogged && servoActive) {
    myServo.write(0);  // Move the servo back to 0 degrees (OFF position)
    servoActive = false;
    Serial.println("Servo OFF (No Sound or Motion).");
  }

  // Delay between readings for stability
  delay(1000);
}