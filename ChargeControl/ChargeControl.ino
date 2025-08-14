 #include <stdlib.h>

const int8_t QtyBatteries = 5;  // кількість портів девайса для підключення батарей. Портом я називаю

const int8_t ConnectedBatteryPins[] = { 3, 5, 7, 10, 12 };        // піни які використовуються для вказання, який порт зараз підключений до зарядного присторою. LOW / HIGH
const int8_t BatteryChargedIndicatorPins[] = { 4, 6, 8, 9, 11 };  // піни для індикації батарей, які пройшли процес зарядки. Значенння: LOW,  HIGH
const int8_t BatteryVoltagePins[] = { A4, A3, A2, A1, A0 };       // піни з яких можна считати напругу кожної з підключених батарей. Значенння: 0 - 1023
const int8_t ChargerCurrentPin = 5;                               // через цей пін можна считати силу струму, якою зарядник зараз заряджає підключену батарею. Значенння: 0 - 1023

const int16_t VoltageLevelWhenBatteryPluged = 900;        // Рівень аналогового сигналу з піну батки, який означає, що батарея підключена до порту
const int16_t CurrentLevelAtWhichContinueCharging = 40;  // Рівень аналогового сигналу з піну струму від зарядки, який означає, що заряд ще продовжуємо

static void (*STATE)();


void setup() {
  Serial.begin(115200);
  for (byte i = 0; i < QtyBatteries; i++) {
    pinMode(ConnectedBatteryPins[i], OUTPUT);
    digitalWrite(ConnectedBatteryPins[i], LOW);
    pinMode(BatteryChargedIndicatorPins[i], OUTPUT);
    digitalWrite(BatteryChargedIndicatorPins[i], LOW);
  }
  STATE = &state01;
}

void loop() {
  updateIndicationByInnerState();
  (*STATE)();
  delay(200);
}

// Стани автомата

// state01. В цьому стані шукуаємо на яку батарею перемкнути вихід зарядного пристрою. Якщо пішов струм через зярядку то переходимо в state02 інакше залишаємося в цьому стані.
void state01() {
  for (int8_t i = 0; i < QtyBatteries; i++) {
    if (batteryVoltage(i) > VoltageLevelWhenBatteryPluged && batteryChargedIndicator(i) == false) {
      setConnectedBattery(i);
      break;
    }
  }

  if (currentOfCharging() > 0) {
    STATE = &state02;
  }
}

// state02. В цьому стані чекаємо поки батарея заряджається, коли струм упав нижче критичного рівня то інидкуємо що батарея пройшла цикл зарядду і переходимо в state01
void state02() {
  if (currentOfCharging() <= CurrentLevelAtWhichContinueCharging) {
    int8_t battery = connectedBattery();
    setBatteryChargedIndicator(battery, true);
    setConnectedBattery(-1);
    STATE = &state01;
  }
}

void updateIndicationByInnerState() {
  for (int8_t i = 0; i < QtyBatteries; i++) {
    int16_t voltage = batteryVoltage(i);
    Serial.print(voltage);
    Serial.print("\t");
    if (voltage < VoltageLevelWhenBatteryPluged) {
      setBatteryChargedIndicator(i, false);
    }
  }
  Serial.print(F("cur: "));
  Serial.print(currentOfCharging());
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
  if (battery >= QtyBatteries) {
    return;
  }
  bool wasSwitch = false;
  for (byte i = 0; i < QtyBatteries; i++) {
    if (digitalRead(ConnectedBatteryPins[i]) == HIGH) {
      wasSwitch = true;
    }
    digitalWrite(ConnectedBatteryPins[i], LOW);
  }
  if (battery >= 0) {
    if ( digitalRead(ConnectedBatteryPins[battery]) == LOW) {
       wasSwitch = true;
    }
    digitalWrite(ConnectedBatteryPins[battery], HIGH);
  }
  if (wasSwitch) {
    delay(3000);  // після будь-якого переключення батки треба зарядному дати час переключитися, підняти струм до потрібного рівня
  }
}

 int comparation_two_int_for_qsort(const void *a, const void *b) {
     return *(int*)a - *(int*)b;
 }

int16_t currentOfChargingRaw() {
  return (analogRead(ChargerCurrentPin) - 512) * 48.2;
}

int16_t currentOfCharging() {
  static int16_t epsilon = currentOfChargingRaw();
  const int8_t N = 11;
  int16_t mass[N];
  for ( int8_t i = 0; i < N; i++) {
    mass[i] = max(0, currentOfChargingRaw() - epsilon);
    delay(5);
  }
  qsort(mass, N, sizeof(int16_t), comparation_two_int_for_qsort);
  return mass[N/2+1];
}

bool batteryChargedIndicator(int8_t battery) {
  if (battery < 0 || battery >= QtyBatteries) {
    return false;
  }
  return digitalRead(BatteryChargedIndicatorPins[battery]) == HIGH;
}

void setBatteryChargedIndicator(int8_t battery, bool flag) {
  if (battery < 0 || battery >= QtyBatteries) {
    return;
  }
  digitalWrite(BatteryChargedIndicatorPins[battery], flag ? HIGH : LOW);
}

int16_t batteryVoltage(int8_t battery) {
  if (battery < 0 || battery >= QtyBatteries) {
    return 0;
  }
  return analogRead(BatteryVoltagePins[battery]);
}
