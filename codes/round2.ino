#include <Servo.h>

#define in1 6
#define in2 7
#define en 5

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

#define rasback 2
#define rasleft 3
#define rasright 4

Servo steering;

bool start = true;
bool inparking = true;

int distance;
int speed = 120;
int mid = 100;
int left = 75;
int right = 130;
int turns = 0;
int clockwise = 0;
int lefttimer = 0;
int righttimer = 0;
int left_obstacle_timer = 0;
int right_obstacle_timer = 0;

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
  
  pinMode(rasback, INPUT);
  pinMode(rasleft, INPUT);
  pinMode(rasright, INPUT);

  distance = 1000;

  delay(1000);
}


void loop() {
  // IDLE: wait for start button
  while(start){ 
    if(digitalRead(button) == HIGH){
      start = false;
    }
  } 
  
  //LEAVE PARKING

  // finish in section
  if(turns == 12){
    forward(speed, 1000);
    while(1){}
  }

  //turn (counter)clockwise
  turning();

  distance++;
  // Detect traffic signs and react
  front();

  if(lefttimer > 0){
    lefttimer--;
    steering.write(right);
    forward(speed, 30);
    return;
  }
  if(righttimer > 0){
    righttimer--;
    steering.write(left);
    forward(speed, 30);
    return;
  }

  if(digitalRead(rasleft) == HIGH){
    // passing from wrong side: go left
    lefttimer = righttimer = 0;
    
    while(digitalRead(rasleft) == HIGH || digitalRead(rasback) == HIGH){
      turning();
      front();
      steering.write(left);
      forward(speed, 30);
      distance++;
      lefttimer++;
    }
    lefttimer = lefttimer * 4 / 7;
    return;
  }
  if(digitalRead(rasright) == HIGH || digitalRead(rasback) == HIGH){
    // passing from wrong side: go right
    lefttimer = righttimer = 0;

    while(digitalRead(rasright) == HIGH){
      turning();
      front();
      steering.write(right);
      forward(speed, 30);
      distance++;
      righttimer++;
    }
    righttimer = righttimer * 4 / 7;
    return;
  } 

  // IR obstacles
  if(digitalRead(irleft) == LOW) {
    // obstacle on left, turn right
    steering.write(right);
    left_obstacle_timer++;
    if(left_obstacle_timer == 30){// car is stuck to the wall
      steering.write(left);
      delay(20);

      backward(speed, 800);
      steering.write(mid + 15);
      delay(100);
    }
  }
  else if(digitalRead(irright) == LOW){
    // obstacle on right, turn left
    steering.write(left);
    right_obstacle_timer++;
    if(right_obstacle_timer == 30){ // car is stuck to the wall
      steering.write(right);
      delay(20);

      backward(speed, 800);
      steering.write(mid - 11);
      delay(100);
    }
  }
  else{
    // no obstacle, go forward
    left_obstacle_timer = right_obstacle_timer = 0;
    steering.write(mid);
  }
  forward(speed, 30);
}


/////////////////////////

//obstacle in front
void front(){
  if(digitalRead(rasback) == HIGH){
    // obstacle in front: go back
    lefttimer = righttimer = 0;
    while(digitalRead(rasback) == HIGH){
      steering.write(mid);
      backward(speed, 30);
    }
    delay(100);
    backward(speed, 600);
  }
}


// line on map
void turning(){
  
  int d = direction();
  if(distance > 50 && d == 2 && clockwise != -1){
    clockwise = 1;
//    lefttimer = righttimer = 0;

    steering.write(right);
    delay(20);

    forward(120, 800);
    steering.write(left);
    delay(20);

    backward(120, 800);

    steering.write(right);
    delay(20);

    forward(120, 1400);
    steering.write(mid - 11);
    backward(speed + 40, 1200);

    turns++;
    distance = 0;
  }
  else if (distance > 50 && d == 1 && clockwise != 1){
    clockwise = -1;
  //  lefttimer = righttimer = 0;

    steering.write(left);
    delay(20);

    forward(120, 800);

    steering.write(right);
    delay(20);

    backward(120, 800);
    
    steering.write(left);
    delay(20);

    forward(120, 1400);
    steering.write(mid + 15);
    backward(speed + 40, 1200);

    turns++;
    distance = 0;
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




// void loop(){{
int direction() {

  digitalWrite(s2_pin,LOW);
  digitalWrite(s3_pin,HIGH);
  int blue  = pulseIn(out_pin, LOW);
 blue = map(blue, 25,70,255,0);


  digitalWrite(s2_pin,LOW);
  digitalWrite(s3_pin,LOW);
  int red = pulseIn(out_pin, LOW);
 red = map(red, 25,72,255,0);



  digitalWrite(s2_pin,HIGH);
  digitalWrite(s3_pin,HIGH);
  int green = pulseIn(out_pin, LOW);
 green = map(green, 30,90,255,0);


  // int sm = (red + green + blue);

  // red = red * 1000 / sm;
  // green = green * 1000 / sm;
  // blue = blue * 1000 / sm;

  // Serial.print(blue);
  // Serial.print(" ");
  
  // Serial.print(red);
  // Serial.print(" ");

  // Serial.print(green);
  // Serial.print(" ");

   Serial.println("");


  if(red < -8000){
    Serial.print(1); // blue line - counter clock - left
   Serial.print(" ");
    return 1;
  }
  else if(blue < -5000){
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


/*




*/