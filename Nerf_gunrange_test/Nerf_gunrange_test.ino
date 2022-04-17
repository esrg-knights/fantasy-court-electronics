//Nerf gunrange test 7 April
//D12: Red led
//D11: Driver power enable (all)
//D7-2: Motor 6-1
//A7: Speed potmeter
//A5-3/D19-17: Dial switch 3-1  --> Used to select difficulty or such, could add more and use more pins. 3 pins = 4 settings 
//A1/D15: Green button
//A0/D14: Red button

//D13 10 9 8 A3 A2: Free
//D0 D1: Free but also serial comms to PC
int spd;
byte dial1,dial2,dial3,redbt,grnbt;

void setup() {
Serial.begin(9600);

pinMode(12, OUTPUT);
pinMode(11, OUTPUT);
pinMode(7, OUTPUT);
pinMode(6, OUTPUT);
pinMode(5, OUTPUT);
pinMode(4, OUTPUT);
pinMode(3, OUTPUT);
pinMode(2, OUTPUT);

pinMode(14, INPUT_PULLUP);
pinMode(15, INPUT_PULLUP);

pinMode(17, INPUT_PULLUP);
pinMode(18, INPUT_PULLUP);
pinMode(19, INPUT_PULLUP);
Serial.println("Passing butter, please standby");
delay(1000);
digitalWrite(11, 1);
}

void loop() {
  // put your main code here, to run repeatedly:
spd = analogRead(A7);
dial3 = digitalRead(17);
dial2 = digitalRead(18);
dial1 = digitalRead(19);
redbt=digitalRead(14);
grnbt=digitalRead(15);

Serial.println();
Serial.println(spd);
Serial.println(dial1);
Serial.println(dial2);
Serial.println(dial3);
Serial.println(redbt);
Serial.println(grnbt);
delay(1000+spd);
digitalWrite(12, 1);
digitalWrite(7, 1);
delay(1000+spd);
digitalWrite(12, 0);
digitalWrite(7, 0);
}
