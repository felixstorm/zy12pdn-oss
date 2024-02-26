#include "pd_sink.h"

using namespace usb_pd;

// voltage in mV
const int desired_voltage = 18000;

mcu_hal usb_pd::hal;
pd_sink power_sink;

// called when the USB PD controller triggers an event
void sink_callback(callback_event event) {

    if (event == callback_event::source_caps_changed) {
        // source has announced its capabilities; sink must request voltage
        int res = power_sink.request_power(desired_voltage);

        if (res == -1) {
            // desired voltage is not available, request 5V instead
            power_sink.request_power(5000);
        }
    }
}

int main() {
    // initialize the board
    hal.init();
    hal.set_led(color::red);

    // initialize PD power sink
    power_sink.set_event_callback(sink_callback);
    power_sink.init();

    // loop and poll
    while (true) {
        hal.poll();
        power_sink.poll();

        // set LED: green for desired voltage, red otherwise
        hal.set_led(power_sink.active_voltage == desired_voltage ? color::green : color::red);
    }
}