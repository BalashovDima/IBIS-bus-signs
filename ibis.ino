void IBIS_init() {
    Serial.begin(1200, SERIAL_7E2);
    while (!Serial);
}