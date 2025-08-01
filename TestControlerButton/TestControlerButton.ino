const int8_t QtyBatteries = 5;  // кількість портів девайса для підключення батарей. Портом я називаю

const int8_t ConnectedBatteryPins[] = { 3, 5, 7, 10, 12 };        // піни які використовуються для вказання, який порт зараз підключений до зарядного присторою. LOW / HIGH
const int8_t BatteryChargedIndicatorPins[] = { 4, 6, 8, 9, 11 };  // піни для індикації батарей, які пройшли процес зарядки. Значенння: LOW,  HIGH
const int8_t BatteryVoltagePins[] = { A4, A3, A2, A1, A0 };       // піни з яких можна считати напругу кожної з підключених батарей. Значенння: 0 - 1023
const int8_t ChargerCurrentPin = 5;                               // через цей пін можна считати силу струму, якою зарядник зараз заряджає підключену батарею. Значенння: 0 - 1023

int val;
byte n;
int epsilon;

void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
  for (byte i = 0; i < QtyBatteries; i++) {
    pinMode(ConnectedBatteryPins[i], OUTPUT);
    digitalWrite(ConnectedBatteryPins[i], LOW);
    pinMode(BatteryChargedIndicatorPins[i], OUTPUT);
    digitalWrite(BatteryChargedIndicatorPins[i], LOW);
    epsilon = Curent();
  }
}


void loop() {

  if (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '0') n=0;
    if (c == '1') n=1;
    if (c == '2') n=2;
    if (c == '3') n=3;
    if (c == '4') n=4;
    if (c == '9') n=9;
    
    SelectCh(n);

    Serial.println();
    Serial.print("Select Chanel ");
    Serial.println(n);
  }

  Serial.print(" Ch# ");
  Serial.print(n);
  Serial.print(" = ");
  Serial.print(analogRead(BatteryVoltagePins[n]));
  Serial.print('\t');


  Serial.print(" I_sens= ");
  Serial.println(Curent());
  delay(1000);
}


int Curent() {  // return curent of battery in mA
  long sum = 0;
  float average;
  for (int i = 0; i < 1000; i++) {
    sum = sum + analogRead(ChargerCurrentPin);
  }
  average = (float)sum / 1000.0;
  //Serial.print(average);
  return int(((average - 512.0) * 48.2))-epsilon;
}

void SelectCh(byte chn) {  // 0 викл все,  1...5 вкл
  for (byte i = 0; i < QtyBatteries; i++) {
    digitalWrite(ConnectedBatteryPins[i], LOW);
  }
  if (chn == 9) return;
  //delay(200);
  digitalWrite(ConnectedBatteryPins[chn], HIGH);
 
}