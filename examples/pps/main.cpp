#include "pd_sink.h"
#include <algorithm>

using namespace usb_pd;

// PPS might be able to deliver less than 5V, but
// below 4V, the LDO on the board might cut out.
constexpr int cutout_voltage = 4000;

mcu_hal usb_pd::hal;
pd_sink power_sink;

int voltage = 5000;
const source_capability* pps_cap = nullptr;

// Find the PPS capability
// (take the last one if multiple ones are available)
const source_capability* find_pps_cap() {
    int index = 0;
    for (int i = 0; i < power_sink.num_source_caps; i++) {
        if (power_sink.source_caps[i].supply_type == pd_supply_type::pps)
            index = i;
    }
    return &power_sink.source_caps[index];
}

// called when the USB PD controller triggers an event
void sink_callback(callback_event event) {

    if (event == callback_event::source_caps_changed) {
        // source has announced its capabilities; sink must request voltage
        pps_cap = find_pps_cap();
        voltage = std::max((int)pps_cap->min_voltage, cutout_voltage);
        power_sink.request_power(voltage);
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

        // If button has been pressed, increase voltage by 100mV
        if (hal.has_button_been_pressed() && pps_cap != nullptr) {
            voltage += 100;
            if (voltage > pps_cap->voltage)
                voltage = std::max((int)pps_cap->min_voltage, cutout_voltage);
            power_sink.request_power(voltage);
        }

        // set LED: green for desired voltage, red otherwise
        hal.set_led(power_sink.active_voltage == voltage ? color::green : color::red);
    }
}