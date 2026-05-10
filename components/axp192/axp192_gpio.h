#pragma once

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"
#include "axp192.h"

namespace esphome {
namespace axp192 {

class AXP192GPIOPin : public GPIOPin, public Parented<AXP192> {
public:
    void setup() override;
    void pin_mode(gpio::Flags flags) override;
    gpio::Flags get_flags() const override { return this->flags_; }
    bool digital_read() override;
    void digital_write(bool value) override;
    std::string dump_summary() const override;

    void set_pin(uint8_t pin) { this->pin_ = pin; }
    void set_inverted(bool inverted) { this->inverted_ = inverted; }
    void set_flags(gpio::Flags flags) { this->flags_ = flags; }

protected:
    uint8_t pin_{0};
    bool inverted_{false};
    gpio::Flags flags_{gpio::FLAG_NONE};
};

}  // namespace axp192
}  // namespace esphome
