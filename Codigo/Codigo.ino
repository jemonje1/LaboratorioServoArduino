
#include <Servo.h>
#include <Stepper.h>

const int VRxPin = A0;
const int VRyPin = A1;
const int SWPin = 2;
const int vibracionPin = 3;
const int ledPin = 4;
const int servoPin = 7;

const int stepsPerRevolution = 2048;

const int cambiosNecesarios = 8;
const unsigned long intervaloVibracion = 100;

Servo myServo;
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

volatile bool bloqueado = false;
volatile unsigned long ultimoCambio = 0;
volatile int cambios = 0;
volatile bool monitoreando = false;

bool botonAnterior = HIGH;
unsigned long ultimoBoton = 0;

void detectarVibracion() {
  if (!monitoreando || bloqueado) return;

  unsigned long ahora = millis();

  if (ahora - ultimoCambio > intervaloVibracion) {
    cambios = 0;
  }

  ultimoCambio = ahora;
  cambios++;

  if (cambios >= cambiosNecesarios) {
    bloqueado = true;
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(SWPin, INPUT_PULLUP);
  pinMode(vibracionPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);

  myServo.attach(servoPin);
  myServo.write(90);

  myStepper.setSpeed(10);

  attachInterrupt(
    digitalPinToInterrupt(vibracionPin),
    detectarVibracion,
    CHANGE
  );

  delay(500);

  noInterrupts();
  cambios = 0;
  ultimoCambio = millis();
  monitoreando = true;
  interrupts();
}

void loop() {
  bool botonActual = digitalRead(SWPin);

  if (bloqueado) {
    digitalWrite(ledPin, HIGH);

    if (botonAnterior == HIGH &&
        botonActual == LOW &&
        millis() - ultimoBoton > 200) {

      noInterrupts();
      bloqueado = false;
      cambios = 0;
      ultimoCambio = millis();
      monitoreando = false;
      interrupts();

      digitalWrite(ledPin, LOW);
      ultimoBoton = millis();

      delay(300);

      noInterrupts();
      cambios = 0;
      ultimoCambio = millis();
      monitoreando = true;
      interrupts();
    }

    botonAnterior = botonActual;
    return;
  }

  digitalWrite(ledPin, LOW);
  botonAnterior = botonActual;

  int xValue = analogRead(VRxPin);
  int yValue = analogRead(VRyPin);

  int angulo = map(xValue, 0, 1023, 0, 180);
  angulo = constrain(angulo, 0, 180);

  myServo.write(angulo);

  int distanciaY = abs(yValue - 512);

  if (distanciaY >= 70) {
    int velocidad = map(distanciaY, 70, 512, 3, 15);
    velocidad = constrain(velocidad, 3, 15);

    myStepper.setSpeed(velocidad);

    if (!bloqueado) {
      if (yValue > 582) {
        myStepper.step(1);
      }
      else if (yValue < 442) {
        myStepper.step(-1);
      }
    }
  }
}
