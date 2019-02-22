#include <Servo.h>

#define SERVO_PIN 9
#define IROBS_PIN 8
#define PIRMS_PIN 3
#define BUTTN_PIN 2

const int OPEN_DELAY = 700;
const byte ANGLE_OPEN = 94;
const byte ANGLE_CLOSED = 4;

Servo myservo;

void setup()
{
    Serial.begin (9600);
    pinMode(IROBS_PIN, INPUT);
    pinMode(PIRMS_PIN, INPUT);
    pinMode(BUTTN_PIN, INPUT);
    myservo.attach(SERVO_PIN);
    myservo.write(0);
}



void loop() 
{

    if (digitalRead(BUTTN_PIN) == HIGH) 
    {
        ThrowSomeFood(3);
    }
    
    CheckMotion();
    CheckIR();

    Serial.println("++++++++++++++++++++");

    
    delay(200);
}

void CheckIR()
{
    int val = digitalRead(IROBS_PIN); 
    if (val == LOW)
    {
        Serial.println("Obstacle detected");
    }
}

void CheckMotion()
{
    int val = digitalRead(PIRMS_PIN); 
    if (val != LOW)
    {
        Serial.println("Motion detected");
    }
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