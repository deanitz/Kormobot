#include <Servo.h>

#define DEBUG true

#define BTLED_PIN 13
#define LED01_PIN 4
#define SERVO_PIN 9
#define IROBS_PIN 8
#define PIRMS_PIN 3
#define BUTTN_PIN 2

//CONSTANTS

#ifdef DEBUG
const unsigned long MILLIS_LED = 2000;
const unsigned long MILLIS_LONG_NO_FEED = 20000;
const unsigned long MOVEMENT_THRESHOLD = 5000;
const unsigned long DECREASE_FOOD_THRESHOLD = 5000;
#else
const unsigned long MILLIS_LED = 5000;
const unsigned long MILLIS_LONG_NO_FEED = 1800000; //30 minutes
const unsigned long MOVEMENT_THRESHOLD = 10000;
const unsigned long DECREASE_FOOD_THRESHOLD = 20000;
#endif 



const byte ANGLE_OPEN = 94;
const byte ANGLE_CLOSED = 4;

const unsigned int SERIAL_SPEED = 9600;
const unsigned int OPEN_DELAY = 700;
const unsigned int MAX_FOOD = 20;
const unsigned int LOTS_OF_FOOD = 2;
const unsigned int A_BIT_OF_FOOD = 1;

Servo myservo;

//VARIABLES
unsigned long lastMilliseconds = 0;
unsigned long lastLedIndicateMilliseconds = 0;
unsigned long lastFeed = 0;
unsigned long lastMovementDetected = 0;
unsigned long lastObstacleDetected = 0;
unsigned long lastFoodCounterDecreased = 0;
unsigned long lastMoveEnded = 0;

byte foodCounter = 1;

void setup()
{
        
#ifdef DEBUG
    Serial.begin(SERIAL_SPEED);
#endif 
   
    pinMode(BTLED_PIN, OUTPUT);
    pinMode(LED01_PIN, OUTPUT);
    pinMode(IROBS_PIN, INPUT);
    pinMode(PIRMS_PIN, INPUT);
    pinMode(BUTTN_PIN, INPUT);
    myservo.attach(SERVO_PIN);
    myservo.write(ANGLE_CLOSED);

    delay(2000);
}

void loop() 
{
    IndicateFoodCount();

    if(IsManualFeedButtonDown())
    {
        ThrowSomeFood(A_BIT_OF_FOOD, true);
        return;
    }

    CheckIR();
    CheckMotion();

    if (IsMoveInProgress())
    {
        return;
    }

    FeedByTime();
}

void DecreaseFoodCounter()
{
    unsigned long currentMilliseconds = millis();
if ((currentMilliseconds < lastFoodCounterDecreased) || (currentMilliseconds - lastFoodCounterDecreased >= DECREASE_FOOD_THRESHOLD))
    {
#ifdef DEBUG
        Serial.println("DecreaseFoodCounter!");
#endif 
        BlinkLed(BTLED_PIN, 1, 50);
        BlinkLed(BTLED_PIN, 1, 200);
        BlinkLed(BTLED_PIN, 2, 50);

        foodCounter--;
        lastFoodCounterDecreased = currentMilliseconds;
    }
}

void FeedByTime()
{
    unsigned long currentMilliseconds = millis();
    if (((currentMilliseconds < lastFeed) || (currentMilliseconds - lastFeed >= MILLIS_LONG_NO_FEED))
        && foodCounter < MAX_FOOD)
    {
#ifdef DEBUG
        Serial.println("FeedByTime!");
#endif 
        ThrowSomeFood(LOTS_OF_FOOD, false);
    }
}

void IndicateFoodCount()
{
    unsigned long currentMilliseconds = millis();
    if ((currentMilliseconds < lastLedIndicateMilliseconds) || (currentMilliseconds - lastLedIndicateMilliseconds >= MILLIS_LED))
    {
        lastLedIndicateMilliseconds = currentMilliseconds;
        if (foodCounter > MAX_FOOD)
        {
            digitalWrite(LED01_PIN, HIGH);
        }
        else if (foodCounter >= (MAX_FOOD / 2))
        {
            BlinkLed(LED01_PIN, 3, 100);
        }
        else if ((foodCounter > LOTS_OF_FOOD) && (foodCounter < (MAX_FOOD / 2)))
        {
            BlinkLed(LED01_PIN, 1, 100);
        }
        else
        {
            digitalWrite(LED01_PIN, LOW);
        }

#ifdef DEBUG
         Serial.println(foodCounter);
#endif 

    }
}

bool IsManualFeedButtonDown()
{
    return (digitalRead(BUTTN_PIN) == HIGH);
}

bool IsMoveInProgress()
{
    unsigned long currentMilliseconds = millis();
    return (((currentMilliseconds - lastMovementDetected) < MOVEMENT_THRESHOLD) 
            || ((currentMilliseconds - lastObstacleDetected) < MOVEMENT_THRESHOLD));
}

void SetMovementDetected()
{
    lastMovementDetected = millis();
}

void CheckIR()
{
    int val = digitalRead(IROBS_PIN); 
    if (val == LOW)
    {
        DecreaseFoodCounter();
        BlinkLed(BTLED_PIN, 1, 50);

#ifdef DEBUG
        Serial.println("Obstacle detected");
#endif 

        SetMovementDetected();
        lastObstacleDetected = millis();
    }
}

void CheckMotion()
{
    int val = digitalRead(PIRMS_PIN); 
    if (val != LOW)
    {
        BlinkLed(BTLED_PIN, 3, 50);
        
#ifdef DEBUG
        Serial.println("Motion detected");
#endif 
      
        SetMovementDetected();
    }
}

void ThrowSomeFood(int times, bool manual)
{
    if (times <= 0)
        return;

    myservo.attach(SERVO_PIN);

    while (times > 0)
    {
        BlinkLed(BTLED_PIN, 1, 100);
        myservo.write(ANGLE_OPEN);
        delay(OPEN_DELAY);

        myservo.write(ANGLE_CLOSED);
        BlinkLed(BTLED_PIN, 1, 100);
        delay(300);

        times--;

        if(!manual)
        {
            foodCounter++;
        }
        
    }

    myservo.detach();
    lastFeed = millis();
    
}

void BlinkLed(byte pin, int times, int delayMs)
{
    for(int i = 0; i < times; i++)
    {
        digitalWrite(pin, HIGH);
        delay(delayMs);
        digitalWrite(pin, LOW);
        delay(delayMs);
    }
}
