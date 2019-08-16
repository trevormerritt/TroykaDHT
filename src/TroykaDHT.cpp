/****************************************************************************/
//  Function:       Cpp file for TroykaDHT
//  Hardware:       DHT11, DHT21, DHT22
//  Arduino IDE:    Arduino-1.8.2
//  Author:         Igor Dementiev
//  Date:           Feb 22,2018
//  Version:        v1.0.1
//  by www.amperka.ru
//  Modification:   Trevor Merritt - Add English Translation.
/****************************************************************************/

#include "TroykaDHT.h"

DHT::DHT(uint8_t pin, uint8_t type) {
    _pin = pin;
    _type = type;
}

void DHT::begin() {
}

int8_t DHT::read() {
    // data buffer
    // буффер данных
    uint8_t data[5];
    // data bit
    // бит данных
    uint8_t dataBit;
    // checksum
    // контрольнная сумма
    uint8_t checkSum;
    // очистка буффера
    // flush the buffer
    for (int i = 0; i < 5; i++)
        data[i] = 0;
    // push the bus to ground for 18ms
    // прижимаем шину к земле на 18 мс
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delay(18);
    // release the bus
    // отпускаем шину
    pinMode(_pin, INPUT_PULLUP);
    // check the sensor response.
    // проверяем реакцию датчика
    // for 20-40 us, the sensor clamps the bus to 0 and holds it for 80 us.
    // в течении 20-40 мкс датчик зажимает шину в 0 и держит 80 мкс 
    // then releases for 80 us (generates a presence signal)
    // потом отпускает на 80 мкс (формирует сигнал присутствия)
    if (pulseInLength(_pin, HIGH, 40) == 0) {
        _state = DHT_ERROR_NO_REPLY;
        return _state;
    }

    if (pulseInLength(_pin, LOW, 80) == 0) {
        _state = DHT_ERROR_NO_REPLY;
        return _state;
    }

    if (pulseInLength(_pin, HIGH, 80) == 0) {
        _state = DHT_ERROR_NO_REPLY;
        return _state;
    }

    // read 40 bits of data
    // считываем 40 бит данных
    // with the same start - clamping the bus for about 50us and a data bit.
    // с одинаковым стартом — зажатием шины около 50 мкс и битом данных
    // the "sensor" releases the bus in ~27us if it wants to transmit
    // датчик «отпускает» шину на ~27мкс если хочет передать «0», или на ~70мкс если хочет передать «1»
    for (int i = 0; i < 40; i++) {
        pulseInLength(_pin, LOW, 50);
        dataBit = pulseInLength(_pin, HIGH, 100);
        if (dataBit) {
            data[i / 8] <<= 1;
            data[i / 8] += dataBit > 45 ? 1 : 0;
        } else { 
            _state = DHT_ERROR_TIMEOUT;
            return _state;
        }
    }

    // calculate checksum
    // проверка контрольной суммы
    checkSum = data[0] + data[1] + data[2] + data[3];
    // verify checksum
    // проверка контрольнной суммы
    if (data[4] != checkSum) {
        _state = DHT_ERROR_CHECKSUM;
        return _state;
    }

    // write data
    // запись данных
    switch (_type) {
        case DHT11:
            _humidity = data[0];
            _temperatureC = data[3] & 0x80 ? (data[2] + (1 - (data[3] & 0x7F) * 0.1)) * -1 : (data[2] + (data[3] & 0x7F) * 0.1);
            _temperatureF = (_temperatureC * 9.0 / 5.0) + 32.0;
            _temperatureK = _temperatureC + CELSIUS_TO_KELVIN;
            break;
        case DHT21:
            _humidity = ((data[0] << 8) + data[1]) * 0.1;
            _temperatureC = (((data[2] & 0x7F) << 8) + data[3]) * (data[2] & 0x80 ? -0.1 : 0.1);
            _temperatureF = (_temperatureC * 9.0 / 5.0) + 32.0;
            _temperatureK = _temperatureC + CELSIUS_TO_KELVIN;
            break;
        case DHT22:
            _humidity = ((data[0] << 8) + data[1]) * 0.1;
            _temperatureC = (((data[2] & 0x7F) << 8) + data[3]) * (data[2] & 0x80 ? -0.1 : 0.1);
            _temperatureF = (_temperatureC * 9.0 / 5.0) + 32.0;
            _temperatureK = _temperatureC + CELSIUS_TO_KELVIN;
            break;
    }

    _state = DHT_OK;
    return _state;
}

unsigned long DHT::pulseInLength(uint8_t pin, bool state, unsigned long timeout) {
    unsigned long startMicros = micros();
    // wait for the end of the current heart rate
    // ожидание окончания текущего пульса
    while (digitalRead(pin) == state) {
        if (micros() - startMicros > timeout)
            return 0;
    }
    return micros() - startMicros;
}
