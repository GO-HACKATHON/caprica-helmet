#define tone 255

//PWM Pin arduino nano D3, D5, D6
#define speakerPin 3

void setup(){

}

void loop(){
	analogWrite(speakerPin, 255);
}