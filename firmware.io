/* Variables */

int ButtonStates[] = {0,0,0,0,0};
int LastStates[] = {0,0,0,0,0};
int gambiarra = 0;
String msg = "";

void CheckPushButton(const int ButtonNumber, const int i, int *Bstates, int *Lstates, int *gamb) {
    int buttonState = digitalRead(ButtonNumber);

    if(buttonState != Lstates[i]) {

        if(buttonState == HIGH) {
            Bstates[i] = HIGH;
            (*gamb) = 1;
        } else {
            Bstates[i] = LOW;
        }
        delay(50);
    }
    Lstates[i] = buttonState;
}

void setup() {
    pinMode(12, INPUT);
    pinMode(11, INPUT);
    pinMode(10, INPUT);
    pinMode(9, INPUT);
    pinMode(8, INPUT);
    Serial.begin(9600);
}

void loop() {

    CheckPushButton(8, 0, ButtonStates, LastStates, &gambiarra);
    CheckPushButton(9, 1, ButtonStates, LastStates, &gambiarra);
    CheckPushButton(10, 2, ButtonStates, LastStates, &gambiarra);
    CheckPushButton(11, 3, ButtonStates, LastStates, &gambiarra);
    CheckPushButton(12, 4, ButtonStates, LastStates, &gambiarra);

    if(gambiarra == 1) {
        for(int i = 0; i < 5; i++) {
            if(ButtonStates[i] == HIGH){
              msg += '1';
            } else {
              msg += '0';
            }
        }
        Serial.print(msg);
        msg = "";
        gambiarra = 0;
    }

}