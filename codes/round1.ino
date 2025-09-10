#include <Servo.h>

#define in1 6
#define in2 7
#define en 5

// free pins: 2 3 4

#define irleft A1
#define irright A0
#define irback A2

#define servoPin 9

#define button	8

#define s0_pin 11
#define s1_pin 12
#define s2_pin 13
#define s3_pin A3
#define out_pin 10

// #define raspred 6
// #define raspgreen 7



Servo steering;


bool start = true;
bool inparking = true;
int lastvalue = 5;
int speed = 120;
int mid = 100;
int left = 75;
int right = 130;
int turns = 0;
int finish_timer = 0;
int clockwise = 0;

uint16_t distance_middle = 0;
uint16_t distance_left = 0;
uint16_t distance_right = 0;

void setup() {
  Serial.begin(9600);
  steering.attach(servoPin);
  steering.write(mid);


  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(en, OUTPUT);
  
  pinMode(s0_pin, OUTPUT);
  pinMode(s1_pin, OUTPUT);
  pinMode(s2_pin, OUTPUT);
  pinMode(s3_pin, OUTPUT);
  pinMode(out_pin, INPUT);
  digitalWrite(s0_pin,HIGH);
  digitalWrite(s1_pin,LOW);

  pinMode(button, INPUT_PULLUP);

  //left 40
  //right 120

  delay(1000);
}


void loop() {
  while(start){
    if(digitalRead(button) == HIGH){
      start = false;
    }
  }
  if(turns == 12){
    forward(speed, 1000);
    while(1){}
  }
  int d = direction();
  if(d == 2 && clockwise != -1){
    clockwise = 1;
    steering.write(right);
    forward(speed, 800);
    steering.write(mid);

    backward(speed, 800);
    delay(100);

    steering.write(right);
    forward(speed, 1500);
    steering.write(mid);
    turns++;
  }
  else if (d == 1 && clockwise != 1){
    clockwise = -1;
    steering.write(left);
    forward(speed, 800);
    steering.write(mid);

    backward(speed, 800);
    delay(100);
    
    steering.write(left);
    forward(speed, 1500);
    steering.write(mid);
    turns++;
  }
  else{
    forward(speed,30);
  }
}




//leave parking
void leave_parking(){
  while(digitalRead(irleft) == LOW && digitalRead(irright) == LOW){
    backward(speed, 100);
  }
  if(digitalRead(irleft) == LOW) clockwise=1;
  else clockwise=-1;
  int direction = left + right;
  if(clockwise == -1){
    direction -= left;
  }
  else{
    direction -= right;
  }
  steering.write(direction);
  forward(speed, 500);
  direction = left + right - direction; // opposite direciton
  steering.write(direction);
  backward(speed, 700);  
  direction = left + right - direction; // opposite direciton
  steering.write(direction);
  backward(speed, 700);
  steering.write(mid);
  inparking = false;
}

void forward(uint8_t _speed, uint16_t _delay) {
  analogWrite(en, _speed);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  delay(_delay);
  Stop();
}

void backward(uint8_t _speed, uint16_t _delay) {
  analogWrite(en, _speed);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  delay(_delay);
  Stop();
}


void Stop() {
  digitalWrite(en, LOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}


////////////////////////




// void loop(){
int direction() {

  digitalWrite(s2_pin,LOW);
  digitalWrite(s3_pin,LOW);
  int red = pulseIn(out_pin, LOW);
 red = map(red, 25,72,255,0);

  //Serial.print(red);
  Serial.print(" ");


  digitalWrite(s2_pin,HIGH);
  digitalWrite(s3_pin,HIGH);
  int green = pulseIn(out_pin, LOW);
 green = map(green, 30,90,255,0);

  //Serial.print(green);
  Serial.print(" ");


  digitalWrite(s2_pin,LOW);
  digitalWrite(s3_pin,HIGH);
  int blue  = pulseIn(out_pin, LOW);
 blue = map(blue, 25,70,255,0);

  //Serial.print(blue);
  Serial.print(" ");


  Serial.println("");

  if(red < -3000){
    Serial.print(1); // blue line - counter clock - left
    Serial.print(" ");
    return 1;
  }
  else if(blue < -2000){
    Serial.print(2); // orange - clock - right
    Serial.print(" ");
    return 2;
  }
  else{
    Serial.print(0); // white
    Serial.print(" ");
    return 0;
  }
}