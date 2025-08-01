const int8_t QtyBatteries = 5;  // кількість портів девайса для підключення батарей. Портом я називаю

const int8_t ConnectedBatteryPins[] = { 3, 5, 7, 10, 12 };        // піни які використовуються для вказання, який порт зараз підключений до зарядного присторою. LOW / HIGH
const int8_t BatteryChargedIndicatorPins[] = { 4, 6, 8, 9, 11 };  // піни для індикації батарей, які пройшли процес зарядки. Значенння: LOW,  HIGH
const int8_t BatteryVoltagePins[] = { A4, A3, A2, A1, A0 };       // піни з яких можна считати напругу кожної з підключених батарей. Значенння: 0 - 1023
const int8_t ChargerCurrentPin = 5;                               // через цей пін можна считати силу струму, якою зарядник зараз заряджає підключену батарею. Значенння: 0 - 1023

const int16_t VoltageLevelWhenBatteryPluged = 900;        // Рівень аналогового сигналу з піну батки, який означає, що батарея підключена до порту
const int16_t CurrentLevelAtWhichContinueCharging = 300;  // Рівень аналогового сигналу з піну струму від зарядки, який означає, що заряд ще продовжуємо

static void (*STATE)();

int epsilon;


void setup() {
  Serial.begin(115200);
  for (byte i = 0; i < QtyBatteries; i++) {
    pinMode(ConnectedBatteryPins[i], OUTPUT);
    digitalWrite(ConnectedBatteryPins[i], LOW);
    pinMode(BatteryChargedIndicatorPins[i], OUTPUT);
    digitalWrite(BatteryChargedIndicatorPins[i], LOW);
    //pinMode(BatteryVoltagePins[i], INPUT);
  }
  //pinMode(ChargerCurrentPin, INPUT);
  epsilon = CurrentOfCharging();

  STATE = &state01;
}

void loop() {
  UpdateIndicationByInnerState();
  (*STATE)();
  delay(1200);
}

// Стани автомата

// state01. В цьому стані шукуаємо на яку батарею перемкнути вихід зарядного пристрою. Якщо пішов струм через зярядку то переходимо в state02 інакше залишаємося в цьому стані.
void state01() {

  bool batteryWasFound = false;
  for (int8_t i = 0; i < QtyBatteries; i++) {
    if (batteryVoltage(i) > VoltageLevelWhenBatteryPluged && batteryChargedIndicator(i) == false) {
      setConnectedBattery(i);
      batteryWasFound = true;
      break;
    }
  }
  if (batteryWasFound == false) {
    setConnectedBattery(-1);
  }
  // Serial.println(connectedBattery());
  // setConnectedBattery(1);
  // Serial.println(connectedBattery());


  if (CurrentOfCharging() > CurrentLevelAtWhichContinueCharging) {
    STATE = &state02;
  }
}

// state02. В цьому стані чекаємо поки батарея заряджається, коли струм упав нижче критичного рівня то інидкуємо що батарея пройшла цикл зарядду і переходимо в state01
void state02() {
  if (CurrentOfCharging() < CurrentLevelAtWhichContinueCharging) {
    int8_t battery = connectedBattery();
    setBatteryChargedIndicator(battery, true);
    STATE = &state01;
  }
}


void UpdateIndicationByInnerState() {
  for (int8_t i = 0; i < QtyBatteries; i++) {
    int16_t voltage = batteryVoltage(i);
    Serial.print(voltage);
    Serial.print("\t");
    if (voltage < VoltageLevelWhenBatteryPluged) {
      setBatteryChargedIndicator(i, false);
    }
  }
  Serial.print(F("cur: "));
  Serial.print(CurrentOfCharging());
  Serial.print(F("\tconn: "));
  Serial.print(connectedBattery());
  Serial.print(F("\tstate: "));
  Serial.print(STATE == &state01 ? 1 : 2);
  Serial.println("");
}

// Низькорівневі функцїї роботи з залізом

int8_t connectedBattery() {
  for (int8_t i = 0; i < QtyBatteries; i++) {
    if (digitalRead(ConnectedBatteryPins[i]) == HIGH) {
      return i;
    }
  }
  return -1;
}

void setConnectedBattery(int8_t battery) {
  // спочатку вимикаємо непотрібні, потім  включаємо потрібну
  if (battery >= QtyBatteries) {
    return;
  }
  for (byte i = 0; i < QtyBatteries; i++) {
    digitalWrite(ConnectedBatteryPins[i], LOW);
  }

  digitalWrite(ConnectedBatteryPins[battery], HIGH);

  // for (int8_t i = 0; i < QtyBatteries; i++) {
  //   if (i == battery) {
  //     digitalWrite(ConnectedBatteryPins[i], HIGH);
  //     break;
  //   }
  // }

  // for (int8_t i = 0; i < QtyBatteries; i++) {
  //   if (i != battery) {
  //     digitalWrite(ConnectedBatteryPins[i], LOW);
  //   }
  // }
}

int16_t CurrentOfCharging() {
  int tok;
  long sum = 0;
  float average;
  for (int i = 0; i < 1000; i++) {
    sum = sum + analogRead(ChargerCurrentPin);
  }
  average = (float)sum / 1000.0;
  //Serial.print(average);
  tok = int(((average - 512.0) * 48.2)) - epsilon;
  //Serial.println(tok);
  return tok;
}

bool batteryChargedIndicator(int8_t battery) {
  if (battery < 0 || battery >= QtyBatteries) {
    return false;
  }
  return digitalRead(BatteryChargedIndicatorPins[battery]) == HIGH;
}

void setBatteryChargedIndicator(int8_t battery, bool STATE) {
  if (battery < 0 || battery >= QtyBatteries) {
    return;
  }
  digitalWrite(BatteryChargedIndicatorPins[battery], STATE ? HIGH : LOW);
}

int16_t batteryVoltage(int8_t battery) {
  if (battery < 0 || battery >= QtyBatteries) {
    return 0;
  }
  return analogRead(BatteryVoltagePins[battery]);
}
