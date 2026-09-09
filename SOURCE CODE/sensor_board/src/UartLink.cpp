#include "UartLink.h"

void UartLink::begin() {
  Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

static uint8_t calcChecksum(const SteerCommand& cmd) {
  uint8_t sum = 0;
  const uint8_t* p = (const uint8_t*)&cmd;
  for (size_t i = 0; i < sizeof(SteerCommand) - 1; i++) sum ^= p[i];
  return sum;
}

void UartLink::sendCommand(const DriveCommand& cmd) {
  int16_t base = (int16_t)(cmd.speedPct * 255.0f / 100.0f);
  int16_t left = base, right = base;

  if (cmd.steer == Steer::LEFT)  left  = (int16_t)(base * TURN_SPEED_SCALE);
  if (cmd.steer == Steer::RIGHT) right = (int16_t)(base * TURN_SPEED_SCALE);

  SteerCommand out{ left, right, 0 };
  out.checksum = calcChecksum(out);
  Serial2.write((uint8_t*)&out, sizeof(out));
}

bool UartLink::pollEncoderReport(long& leftDelta, long& rightDelta) {
  while (Serial2.available()) {
    char c = (char)Serial2.read();
    if (c == '\n') {
      // Expect: "E,<left>,<right>"
      if (rxBuffer_.startsWith("E,")) {
        int firstComma = rxBuffer_.indexOf(',', 2);
        if (firstComma > 0) {
          leftDelta  = rxBuffer_.substring(2, firstComma).toInt();
          rightDelta = rxBuffer_.substring(firstComma + 1).toInt();
          rxBuffer_ = "";
          return true;
        }
      }
      rxBuffer_ = "";
    } else if (c != '\r') {
      rxBuffer_ += c;
    }
  }
  return false;
}
