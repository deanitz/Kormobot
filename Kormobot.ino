#include <Servo.h>

#define SERVO_PIN 9
#define IROBS_PIN 8
#define PIRMS_PIN 3
#define BUTTN_PIN 2

//CONSTANTS
const unsigned int SERIAL_SPEED = 9600;
const unsigned int OPEN_DELAY = 700;
const unsigned long LONG_WAIT = 1200000;
const unsigned long SHORT_WAIT = 5000;
const byte ANGLE_OPEN = 94;
const byte ANGLE_CLOSED = 4;
const unsigned long MILLIS_DECOUNT = 1200000;
const unsigned int MAX_FOOD = 12;
const unsigned int LOTS_OF_FOOD = 3;
const unsigned int A_BIT_OF_FOOD = 1;

Servo myservo;

//VARIABLES
unsigned long lastMilliseconds = 0;
byte foodCounter = 0;
bool isMoveDetected = false;
bool isEatTime = true;

void setup()
{
    Serial.begin(SERIAL_SPEED);
    pinMode(IROBS_PIN, INPUT);
    pinMode(PIRMS_PIN, INPUT);
    pinMode(BUTTN_PIN, INPUT);
    myservo.attach(SERVO_PIN);
    myservo.write(ANGLE_CLOSED);
}

void loop() 
{
    ComputeTimeFromLastCountDecrease();

    if (IsOverFeed())
    {
        delay(LONG_WAIT);
        return;
    }

    if (CheckIR())
    {
        isMoveDetected = true;
        delay(SHORT_WAIT);
        return;
    }

    CheckMotion();
    if(isMoveDetected)
    {
        isMoveDetected = false;
        ThrowSomeFood(LOTS_OF_FOOD);
        delay(SHORT_WAIT);
        return;
    }

    if(CheckManualFeedButton())
    {
        ThrowSomeFood(A_BIT_OF_FOOD);
        return;
    }
    else
    {
        delay(SHORT_WAIT);
    }

    Serial.println("++++++++++++++++++++");
    delay(100);
}

bool IsOverFeed()
{
    return foodCounter > MAX_FOOD;
}

void ComputeTimeFromLastCountDecrease()
{
    unsigned long currentMilliseconds = millis();

    // 1st condition is a dirty solution for long millis rollover
    if ((currentMilliseconds < lastMilliseconds) || (currentMilliseconds - lastMilliseconds >= MILLIS_DECOUNT))
    {
        if (foodCounter > 0)
        {
            foodCounter--;
        }
        
        lastMilliseconds = currentMilliseconds;
    }
}

bool CheckManualFeedButton()
{
    return (digitalRead(BUTTN_PIN) == HIGH);
}

bool CheckIR()
{
    int val = digitalRead(IROBS_PIN); 
    if (val == LOW)
    {
        Serial.println("Obstacle detected");
        return true;
    }

    return false;
}

bool CheckMotion()
{
    int val = digitalRead(PIRMS_PIN); 
    if (val != LOW)
    {
        Serial.println("Motion detected");
        return true;
    }

    return false;
}

void ThrowSomeFood(int times)
{
    if (times <= 0)
        return;

    while (times > 0)
    {
        myservo.write(ANGLE_OPEN);
        Serial.println("open");

        delay(OPEN_DELAY);

        myservo.write(ANGLE_CLOSED);
        Serial.println("closed");

        delay(500);

        times--;
    }
    
}