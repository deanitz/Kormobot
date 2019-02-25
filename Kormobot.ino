#include <Servo.h>

#define DEBUG true

#define BTLED_PIN 13
#define LED01_PIN 4
#define SERVO_PIN 9
#define IROBS_PIN 8
#define PIRMS_PIN 3
#define BUTTN_PIN 2

//CONSTANTS
const unsigned long MILLIS_DECOUNT = 5000;//1200000;
const unsigned long LONG_WAIT = 5000; //1200000;
const unsigned long SHORT_WAIT = 2000;

const byte ANGLE_OPEN = 94;
const byte ANGLE_CLOSED = 4;

const unsigned int SERIAL_SPEED = 9600;
const unsigned int OPEN_DELAY = 700;
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
}

void loop() 
{
    if(IsManualFeedButtonDown())
    {
        ThrowSomeFood(A_BIT_OF_FOOD, true);
        return;
    }

    ComputeTimeFromLastCountDecrease();

    if (IsOverFeed())
    {
        switchFeedMode = true;
        BlinkLed(BTLED_PIN, 7, 50);

#ifdef DEBUG
        Serial.println("long wait");
#endif
        delay(LONG_WAIT);
        return;
    }

    if(switchFeedMode)
    {
        isEatTime = !isEatTime;
        switchFeedMode = false;

#ifdef DEBUG
        Serial.print("isEatTime ");
        Serial.println(isEatTime);
#endif   

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

    IndicateFoodCount();
}

void IndicateFoodCount()
{
    digitalWrite(LED01_PIN, LOW);

    if ((foodCounter > LOTS_OF_FOOD) && (foodCounter < (MAX_FOOD / 2)))
    {
        BlinkLed(LED01_PIN, 1, 50);
    }
    else if (foodCounter >= (MAX_FOOD / 2))
    {
        BlinkLed(LED01_PIN, 3, 50);
    }
    else if (foodCounter >= MAX_FOOD)
    {
       digitalWrite(LED01_PIN, HIGH);
    }

#ifdef DEBUG
    Serial.println(foodCounter);
#endif 
    
}

bool IsManualFeedButtonDown()
{
    return (digitalRead(BUTTN_PIN) == HIGH);
}

bool CheckIR()
{
    int val = digitalRead(IROBS_PIN); 
    if (val == LOW)
    {
        BlinkLed(BTLED_PIN, 1, 50);

#ifdef DEBUG
        Serial.println("Obstacle detected");
#endif 

        return true;
    }

    return false;
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
      
        isMoveDetected = true;
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
        //myservo.write(ANGLE_OPEN);
        delay(OPEN_DELAY);

        //myservo.write(ANGLE_CLOSED);
        BlinkLed(BTLED_PIN, 1, 100);
        delay(300);

        times--;

        if(!manual)
        {
            foodCounter++;
        }
        
    }

    myservo.detach();
    
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
