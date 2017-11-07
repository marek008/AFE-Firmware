#include "AFE-Relay.h"

AFERelay::AFERelay(){};

AFERelay::AFERelay(uint8_t id) { begin(id); }

void AFERelay::begin(uint8_t id) {
  MQTT MQTTConfiguration;
  MQTTConfiguration = Data.getMQTTConfiguration();
  _id = id;
  RelayConfiguration = Data.getRelayConfiguration(_id);
  pinMode(RelayConfiguration.gpio, OUTPUT);

  sprintf(mqttTopic, "%s%s/", MQTTConfiguration.topic, RelayConfiguration.name);

  Serial << endl << "INFO: Relay #" << id << " has been initialized";
  Serial << endl << "INFO: Relay #" << id << " MQTT Topic: " << mqttTopic;
}

const char *AFERelay::getMQTTTopic() { return RelayConfiguration.mqttTopic; }

byte AFERelay::get() {
  return digitalRead(RelayConfiguration.gpio) == HIGH ? RELAY_ON : RELAY_OFF;
}

/* Set relay to ON */
void AFERelay::on() {
  if (get() == RELAY_OFF) {
    digitalWrite(RelayConfiguration.gpio, HIGH);
    if (RelayConfiguration.timeToOff >
        0) { // Start counter if relay should be automatically turned off
      turnOffCounter = millis();
    }
    Data.saveRelayState(_id, RELAY_ON);
    Serial << endl << "INFO: Relay set to ON";
  }
}

/* Set relay to OFF */
void AFERelay::off() {
  if (get() == RELAY_ON) {
    digitalWrite(RelayConfiguration.gpio, LOW);
    Data.saveRelayState(_id, RELAY_OFF);
    Serial << endl << "INFO: Relay set to OFF";
  }
}

/* Toggle relay */
void AFERelay::toggle() {
  if (digitalRead(RelayConfiguration.gpio) == LOW) {
    on();
  } else {
    off();
  }
}

void AFERelay::setRelayAfterRestoringPower() {
  setRelayAfterRestore(RelayConfiguration.statePowerOn);
}

boolean AFERelay::setRelayAfterRestoringMQTTConnection() {
  if (RelayConfiguration.stateMQTTConnected ==
      5) { // request state from MQTT Broker
    return false;
  } else {
    setRelayAfterRestore(RelayConfiguration.stateMQTTConnected);
    return true;
  }
}

void AFERelay::setRelayAfterRestore(uint8_t option) {
  Serial << endl << "DEBUG: Setting relay state";
  Serial << endl
         << " - Current state=" << get() << endl
         << " - Saved=" << Data.getRelayState(_id) << endl
         << " - Option=" << option;
  if (option == 1) {
    off();
  } else if (option == 2) {
    on();
  } else if (option == 3) {
    Data.getRelayState(_id) == RELAY_ON ? on() : off();
  } else if (option == 4) {
    Data.getRelayState(_id) == RELAY_ON ? off() : on();
  }
  Serial << endl << " - New:" << Data.getRelayState(_id);
}

boolean AFERelay::autoTurnOff() {
  if (RelayConfiguration.timeToOff > 0 && get() == RELAY_ON &&
      millis() - turnOffCounter >= RelayConfiguration.timeToOff * 1000) {
    Serial << endl
           << "INFO: Automatically relay switched off after "
           << (millis() - turnOffCounter) / 1000 << "sek.";
    off();
    return true;
  } else {
    return false;
  }
}
