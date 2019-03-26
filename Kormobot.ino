#include <Servo.h>

//#define DEBUG false

#define BTLED_PIN 13
#define LED01_PIN 4
#define SERVM_PIN 9
#define SERVS_PIN 7
#define IROBS_PIN 8
#define PIRMS_PIN 3
#define BUTTN_PIN 2

//CONSTANTS

// const unsigned long MILLIS_LED = 5000;
// const unsigned long MILLIS_LONG_NO_FEED = 20000;
// const unsigned long MILLIS_TOO_MUCH_NO_FEED = 40000;
// const unsigned long MOVEMENT_THRESHOLD = 5000;
// const unsigned long DECREASE_FOOD_THRESHOLD = 5000;

const unsigned long MILLIS_LED = 20000;
const unsigned long MILLIS_LONG_NO_FEED = 7200000; //2 hours
const unsigned long MILLIS_TOO_MUCH_NO_FEED = 43200000; //12 hrs
const unsigned long MOVEMENT_THRESHOLD = 10000;
const unsigned long DECREASE_FOOD_THRESHOLD = 40000;

const byte ANGLE_OPEN = 94;
const byte ANGLE_CLOSED = 4;
const byte ANGLE_FULL = 160;

const unsigned int SERIAL_SPEED = 9600;
const unsigned int OPEN_DELAY = 700;
const unsigned int MAX_FOOD = 2;
const unsigned int LOTS_OF_FOOD = 1;
const unsigned int A_BIT_OF_FOOD = 1;

Servo servoGate;
Servo servoMixer;

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
    servoGate.attach(SERVM_PIN);
    servoGate.write(ANGLE_CLOSED);

    servoMixer.attach(SERVS_PIN);
    servoMixer.write(ANGLE_CLOSED);

    delay(2000);

    servoGate.detach();
    servoMixer.detach();
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
        Serial.print("currentMilliseconds: ");
        Serial.println(currentMilliseconds);

        Serial.print("lastFoodCounterDecreased: ");
        Serial.println(lastFoodCounterDecreased);

        Serial.println("DecreaseFoodCounter!");
#endif 
        BlinkLed(BTLED_PIN, 1, 50);
        BlinkLed(BTLED_PIN, 3, 200);
        BlinkLed(BTLED_PIN, 1, 50);

        if (foodCounter > 0)
        {
            foodCounter--;
        }
        
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
    else if ((currentMilliseconds - lastFeed >= MILLIS_TOO_MUCH_NO_FEED) && foodCounter > MAX_FOOD)
    {
        foodCounter -= 2;
    }
}

void IndicateFoodCount()
{
    unsigned long currentMilliseconds = millis();
    if ((currentMilliseconds < lastLedIndicateMilliseconds) || (currentMilliseconds - lastLedIndicateMilliseconds >= MILLIS_LED))
    {
        lastLedIndicateMilliseconds = currentMilliseconds;
        if (foodCounter >= MAX_FOOD)
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
        BlinkLed(BTLED_PIN, 1, 200);

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

    servoMixer.attach(SERVS_PIN);
    servoGate.attach(SERVM_PIN);

    while (times > 0)
    {
        servoMixer.write(ANGLE_FULL);
        delay(OPEN_DELAY);
        servoMixer.write(ANGLE_CLOSED);
        delay(OPEN_DELAY);


        BlinkLed(BTLED_PIN, 1, 100);
        servoGate.write(ANGLE_OPEN);
        delay(OPEN_DELAY);

        servoGate.write(ANGLE_CLOSED);
        BlinkLed(BTLED_PIN, 1, 100);
        delay(300);

        times--;

        if(!manual)
        {
            foodCounter++;
        }
        
    }

    servoMixer.detach();
    servoGate.detach();
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
