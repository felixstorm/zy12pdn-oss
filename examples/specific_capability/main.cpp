#include "pd_sink.h"

using namespace usb_pd;

mcu_hal usb_pd::hal;
pd_sink power_sink;

int voltage = 5000;
const source_capability* pps_cap = nullptr;

// called when the USB PD controller triggers an event
void sink_callback(callback_event event) {

    if (event == callback_event::source_caps_changed) {
        // source has announced its capabilities; sink must request voltage
        // search capability with 20V and 2A
        for (int i = 0; i < power_sink.num_source_caps; i++) {
            auto cap = &power_sink.source_caps[i];
            if (cap->min_voltage <= 20000 && cap->voltage >= 20000 && cap->max_current >= 2000) {
                power_sink.request_power_from_capability(i, 20000, 2000);
                return;
            }
        }

        // no matching voltage found; select 5V
        power_sink.request_power(5000);

    } else {
        // set LED color
        if (power_sink.active_voltage == 20000)
            hal.set_led(color::blue);
        else
            hal.set_led(color::red, 500, 500);
    }
}

int main() {
    // initialize the board
    hal.init();
    hal.set_led(color::red, 500, 500);

    // initialize PD power sink
    power_sink.set_event_callback(sink_callback);
    power_sink.init();

    // loop and poll
    while (true) {
        hal.poll();
        power_sink.poll();
    }
}