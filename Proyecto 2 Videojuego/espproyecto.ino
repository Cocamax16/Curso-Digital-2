#include <Arduino.h>
#include <Bluepad32.h>

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

const int DEADZONE = 120;

String lastMove = "";
bool lastJumpState = false;
bool lastCircleState = false;
bool lastSquareState = false;
bool lastTriangleState = false;

HardwareSerial NucleoSerial(2);

void onConnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      myControllers[i] = ctl;
      Serial.printf("Control conectado en slot %d\n", i);
      return;
    }
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      myControllers[i] = nullptr;
      Serial.printf("Control desconectado del slot %d\n", i);
      lastMove = "";
      lastJumpState = false;
      lastCircleState = false;
      lastSquareState = false;
      lastTriangleState = false;
      return;
    }
  }
}

void printCurrentAction(ControllerPtr ctl) {
  int x = ctl->axisX();
  int y = ctl->axisY();
  uint8_t dpad = ctl->dpad();

  Serial.printf("X=%d Y=%d DPAD=0x%02X a=%d b=%d x=%d y=%d\n",
                x, y, dpad, ctl->a(), ctl->b(), ctl->x(), ctl->y());

  // Botones del PS4 en Bluepad32 normalmente:
  // a() = cruz
  // b() = circulo
  // x() = cuadrado
  // y() = triangulo

  bool jumpNow     = ctl->a(); // cruz
  bool circleNow   = ctl->b(); // circulo
  bool squareNow   = ctl->x(); // cuadrado
  bool triangleNow = ctl->y(); // triangulo

  // Salto
  if (jumpNow && !lastJumpState) {
    Serial.println("SALTO");
    NucleoSerial.write('J');
    NucleoSerial.write('\n');
  }
  lastJumpState = jumpNow;

  // Circulo
  if (circleNow && !lastCircleState) {
    Serial.println("CIRCULO");
    NucleoSerial.write('C');
    NucleoSerial.write('\n');
  }
  lastCircleState = circleNow;

  // Cuadrado
  if (squareNow && !lastSquareState) {
    Serial.println("CUADRADO");
    NucleoSerial.write('Q');
    NucleoSerial.write('\n');
  }
  lastSquareState = squareNow;

  // Triangulo
  if (triangleNow && !lastTriangleState) {
    Serial.println("TRIANGULO");
    NucleoSerial.write('T');
    NucleoSerial.write('\n');
  }
  lastTriangleState = triangleNow;

  // Direcciones activas por cruceta o joystick
  bool up    = (dpad & DPAD_UP)    || (y < -DEADZONE);
  bool down  = (dpad & DPAD_DOWN)  || (y >  DEADZONE);
  bool left  = (dpad & DPAD_LEFT)  || (x < -DEADZONE);
  bool right = (dpad & DPAD_RIGHT) || (x >  DEADZONE);

  String currentMove;

  if (up && !down) {
    currentMove = "F";
  }
  else if (down && !up) {
    currentMove = "B";
  }
  else if (left && !right) {
    currentMove = "L";
  }
  else if (right && !left) {
    currentMove = "R";
  }
  else {
    currentMove = "S";
  }

  if (currentMove != lastMove) {
    Serial.printf("Movimiento: %s\n", currentMove.c_str());
    NucleoSerial.write(currentMove[0]);
    NucleoSerial.write('\n');
    lastMove = currentMove;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // RX=22, TX=21
  NucleoSerial.begin(115200, SERIAL_8N1, 22, 21);

  BP32.setup(&onConnectedController, &onDisconnectedController);

  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("MAC BT del ESP32: %02X:%02X:%02X:%02X:%02X:%02X\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  Serial.println("Pon el control PS4 en modo emparejamiento: SHARE + PS");
}

void loop() {
  BP32.update();

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    ControllerPtr ctl = myControllers[i];
    if (ctl && ctl->isConnected() && ctl->isGamepad()) {
      printCurrentAction(ctl);
    }
  }

  delay(20);
}
