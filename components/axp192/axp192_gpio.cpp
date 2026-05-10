#include "axp192_gpio.h"
#include "esphome/core/log.h"

namespace esphome {
namespace axp192 {

static const char *TAG = "axp192.gpio";

void AXP192GPIOPin::setup() {
    // Drive to initial state when configured as output
    if (this->flags_ & gpio::FLAG_OUTPUT) {
        this->digital_write(!this->inverted_);  // default HIGH (deasserted)
    }
}

void AXP192GPIOPin::pin_mode(gpio::Flags flags) {
    this->flags_ = flags;
    if (flags & gpio::FLAG_OUTPUT) {
        this->digital_write(!this->inverted_);
    }
}

bool AXP192GPIOPin::digital_read() {
    // AXP192 GPIOs are output-only in this implementation
    ESP_LOGW(TAG, "digital_read() not supported on AXP192 GPIO%u", this->pin_);
    return false;
}

void AXP192GPIOPin::digital_write(bool value) {
    bool actual = this->inverted_ ? !value : value;
    this->parent_->setGpioPin(this->pin_, actual ? 1 : 0);
}

std::string AXP192GPIOPin::dump_summary() const {
    char buf[32];
    snprintf(buf, sizeof(buf), "AXP192 GPIO%u", this->pin_);
    return std::string(buf);
}

}  // namespace axp192
}  // namespace esphome
