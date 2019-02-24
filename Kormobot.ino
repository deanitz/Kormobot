#include <Servo.h>

#define BTLED_PIN 13
#define SERVO_PIN 9
#define IROBS_PIN 8
#define PIRMS_PIN 3
#define BUTTN_PIN 2

//CONSTANTS
const unsigned int SERIAL_SPEED = 9600;
const unsigned int OPEN_DELAY = 700;
const unsigned long LONG_WAIT = 5000; //1200000;
const unsigned long SHORT_WAIT = 2000;
const byte ANGLE_OPEN = 94;
const byte ANGLE_CLOSED = 4;
const unsigned long MILLIS_DECOUNT = 5000;//1200000;
const unsigned int MAX_FOOD = 20;
const unsigned int MAX_FOOD_RELAX = 4;
const unsigned int LOTS_OF_FOOD = 3;
const unsigned int A_BIT_OF_FOOD = 1;

Servo myservo;

//VARIABLES
unsigned long lastMilliseconds = 0;
byte foodCounter = 1;
bool isMoveDetected = false;
bool isEatTime = true;
bool switchFeedMode = false;

void setup()
{
    Serial.begin(SERIAL_SPEED);
    pinMode(BTLED_PIN, OUTPUT);
    pinMode(IROBS_PIN, INPUT);
    pinMode(PIRMS_PIN, INPUT);
    pinMode(BUTTN_PIN, INPUT);
    myservo.attach(SERVO_PIN);
    myservo.write(ANGLE_CLOSED);
}

void loop() 
{
    if(CheckManualFeedButton())
    {
        ThrowSomeFood(A_BIT_OF_FOOD, true);
        return;
    }

    ComputeTimeFromLastCountDecrease();

    if (IsOverFeed())
    {
        switchFeedMode = true;
        for(int i = 0; i < 10; i++)
        {
            digitalWrite(BTLED_PIN, HIGH);
            delay(50);
            digitalWrite(BTLED_PIN, LOW);
            delay(50);
        }
        Serial.println("long wait");
        delay(LONG_WAIT);
        return;
    }

    if(switchFeedMode)
    {
        isEatTime = !isEatTime;
        switchFeedMode = false;
        Serial.print("isEatTime ");
        Serial.println(isEatTime);
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
        ThrowSomeFood(LOTS_OF_FOOD, false);
        delay(SHORT_WAIT);
        return;
    }

    if(foodCounter <= 0)
    {
        foodCounter = 0;
        ThrowSomeFood(LOTS_OF_FOOD, false);
    }
}

bool IsOverFeed()
{
    return foodCounter > (isEatTime ? MAX_FOOD : MAX_FOOD_RELAX);
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
    Serial.println(foodCounter);
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
        for(int i = 0; i < 1; i++)
        {
            digitalWrite(BTLED_PIN, HIGH);
            delay(50);
            digitalWrite(BTLED_PIN, LOW);
            delay(50);
        }
        Serial.println("Obstacle detected");
        return true;
    }

    return false;
}

void CheckMotion()
{
    int val = digitalRead(PIRMS_PIN); 
    if (val != LOW)
    {
        for(int i = 0; i < 3; i++)
        {
            digitalWrite(BTLED_PIN, HIGH);
            delay(50);
            digitalWrite(BTLED_PIN, LOW);
            delay(50);
        }
        Serial.println("Motion detected");
        isMoveDetected = true;
    }
}

void ThrowSomeFood(int times, bool manual)
{
    if (times <= 0)
        return;

    myservo.attach(SERVO_PIN);

    for(int i = 0; i < 3; i++)
    {
        digitalWrite(BTLED_PIN, HIGH);
        delay(100);
        digitalWrite(BTLED_PIN, LOW);
        delay(50);
    }

    while (times > 0)
    {
       // myservo.write(ANGLE_OPEN);
        //Serial.println("open");

        delay(OPEN_DELAY);

       // myservo.write(ANGLE_CLOSED);
        //Serial.println("closed");

        delay(500);

        times--;

        if(!manual)
        {
            foodCounter++;
        }
        
    }

    myservo.detach();
    
}
