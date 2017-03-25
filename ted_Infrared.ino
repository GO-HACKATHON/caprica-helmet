int led = 13;
int tcrt;

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop(){
  //digitalWrite(led,LOW);
  tcrt = analogRead(A0);
  //Serial.println("LOW");
  if (tcrt<700){
    digitalWrite(led, HIGH);
    Serial.println("HIGH");
  }
  else {
    Serial.println("LOW");
    digitalWrite(led,LOW);
  }
  //analogWrite(led,tcrt/4);
}
