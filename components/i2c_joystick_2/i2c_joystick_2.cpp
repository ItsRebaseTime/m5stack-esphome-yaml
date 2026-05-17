#include "i2c_joystick_2.h"

#include <cstdio>
#include <cstdlib>
#include "esphome/core/log.h"

namespace esphome {
namespace i2c_joystick_2 {

static const char *const TAG = "i2c_joystick_2";

static const uint8_t JOYSTICK2_ADC_VALUE_12BITS_REG = 0x00;
static const uint8_t JOYSTICK2_ADC_VALUE_8BITS_REG = 0x10;
static const uint8_t JOYSTICK2_BUTTON_REG = 0x20;
static const uint8_t JOYSTICK2_OFFSET_ADC_VALUE_12BITS_REG = 0x50;
static const uint8_t JOYSTICK2_OFFSET_ADC_VALUE_8BITS_REG = 0x60;
static const uint8_t JOYSTICK2_FIRMWARE_VERSION_REG = 0xFE;
static const uint8_t JOYSTICK2_I2C_ADDRESS_REG = 0xFF;
static const uint8_t JOYSTICK2_LIGHT_B = 0x30;
static const uint8_t JOYSTICK2_LIGHT_G = 0x31;
static const uint8_t JOYSTICK2_LIGHT_R = 0x32;

void I2CJoystick2Component::setup() {
  ESP_LOGD(TAG, "Setting up I2C Joystick2...");
  // Read firmware version
  if ( !read_u8_(JOYSTICK2_FIRMWARE_VERSION_REG, &this->firmware_version_) ) {
    ESP_LOGW(TAG, "Failed to read firmware version from Joystick2");
    return;
  }

}

bool I2CJoystick2Component::read_axis_value(uint8_t axis, uint8_t mode, float *value) {
  if (value == nullptr) {
    return false;
  }

  if (mode == MODE_ADC_16BIT) {
    uint16_t raw = 0;
    uint8_t reg = (axis == AXIS_X) ? JOYSTICK2_ADC_VALUE_12BITS_REG : (JOYSTICK2_ADC_VALUE_12BITS_REG + 2);
    if (!this->read_le_u16_(reg, &raw)) {
      return false;
    }
    *value = static_cast<float>(raw);
    return true;
  }

  if (mode == MODE_ADC_8BIT) {
    uint8_t raw = 0;
    uint8_t reg = (axis == AXIS_X) ? JOYSTICK2_ADC_VALUE_8BITS_REG : (JOYSTICK2_ADC_VALUE_8BITS_REG + 1);
    if (!this->read_u8_(reg, &raw)) {
      return false;
    }
    *value = static_cast<float>(raw);
    return true;
  }

  if (mode == MODE_OFFSET_8BIT) {
    int8_t mapped = 0;
    uint8_t reg = (axis == AXIS_X) ? JOYSTICK2_OFFSET_ADC_VALUE_8BITS_REG
                                          : (JOYSTICK2_OFFSET_ADC_VALUE_8BITS_REG + 1);
    if (!this->read_i8_(reg, &mapped)) {
      return false;
    }
    *value = static_cast<float>(mapped);
    return true;
  }

  if (mode == MODE_OFFSET_12BIT) {
    int16_t mapped = 0;
    uint8_t reg = (axis == AXIS_X) ? JOYSTICK2_OFFSET_ADC_VALUE_12BITS_REG
                                          : (JOYSTICK2_OFFSET_ADC_VALUE_12BITS_REG + 2);
    if (!this->read_le_i16_(reg, &mapped)) {
      return false;
    }
    *value = static_cast<float>(mapped);
    return true;
  }

  return false;
}

bool I2CJoystick2Component::read_button_pressed(bool *pressed) {
  if (pressed == nullptr) {
    return false;
  }

  uint8_t button = 1;
  if (!this->read_u8_(JOYSTICK2_BUTTON_REG, &button)) {
    return false;
  }

  // M5Unit Joystick2 returns 0 when pressed and 1 when released.
  *pressed = (button == 0);
  return true;
}

void I2CJoystick2Component::write_rgb_channel(RGBChannel channel, uint8_t value) {
  uint8_t reg = 0;
  switch (channel) {
    case CHANNEL_R:
      reg = JOYSTICK2_LIGHT_R;
      break;
    case CHANNEL_G:
      reg = JOYSTICK2_LIGHT_G;
      break;
    case CHANNEL_B:
      reg = JOYSTICK2_LIGHT_B;
      break;
    default:
      ESP_LOGW(TAG, "No RGB Channel specified");
      break;
  }

  bool ret = this->write_u8_(reg, value);

  if ( !ret ) {
    ESP_LOGW(TAG, "Failed to write data to RGB channel");
    return;
  } 

}

bool I2CJoystick2Component::read_u8_(uint8_t reg, uint8_t *value) {
  return this->read_byte(reg, value);
}

bool I2CJoystick2Component::write_u8_(uint8_t reg, uint8_t value) {
  return this->write_byte(reg, value);
}

bool I2CJoystick2Component::read_le_u16_(uint8_t reg, uint16_t *value) {
  uint8_t data[2] = {0};
  if (this->read_register(reg, data, 2) != i2c::ERROR_OK) {
    return false;
  }
  *value = static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
  return true;
}

bool I2CJoystick2Component::read_le_i16_(uint8_t reg, int16_t *value) {
  uint16_t raw = 0;
  if (!this->read_le_u16_(reg, &raw)) {
    return false;
  }
  *value = static_cast<int16_t>(raw);
  return true;
}

bool I2CJoystick2Component::read_i8_(uint8_t reg, int8_t *value) {
  uint8_t raw = 0;
  if (!this->read_u8_(reg, &raw)) {
    return false;
  }
  *value = static_cast<int8_t>(raw);
  return true;
}

bool I2CJoystick2Component::read_current_i2c_address(uint8_t *addr) {
  return this->read_u8_(JOYSTICK2_I2C_ADDRESS_REG, addr);
}

void I2CJoystick2Component::set_pending_address(const std::string &addr_str) {
  this->pending_address_ = addr_str;
}

bool I2CJoystick2Component::apply_pending_address() {
  if (this->pending_address_.empty()) {
    ESP_LOGW(TAG, "No address set in text field");
    return false;
  }

  const char *str = this->pending_address_.c_str();
  const char *parse_start = (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) ? str + 2 : str;
  char *end = nullptr;
  long val = strtol(parse_start, &end, 16);

  if (end == parse_start || *end != '\0' || val < 0x08 || val > 0x77) {
    ESP_LOGW(TAG, "Invalid I2C address '%s': must be hex 08-77", str);
    return false;
  }

  uint8_t new_addr = static_cast<uint8_t>(val);
  if (!this->write_u8_(JOYSTICK2_I2C_ADDRESS_REG, new_addr)) {
    ESP_LOGW(TAG, "Failed to write I2C address 0x%02X", new_addr);
    return false;
  }

  ESP_LOGI(TAG, "I2C address set to 0x%02X (takes effect on next power cycle)", new_addr);
  return true;
}

#ifdef USE_TEXT
void I2CJoystick2AddressText::setup() {
  uint8_t addr = 0;
  if (this->parent_->read_current_i2c_address(&addr)) {
    char buf[5];
    snprintf(buf, sizeof(buf), "0x%02X", addr);
    this->publish_state(buf);
  }
}

void I2CJoystick2AddressText::control(const std::string &value) {
  this->parent_->set_pending_address(value);
  this->publish_state(value);
}
#endif  // USE_TEXT

#ifdef USE_BUTTON
void I2CJoystick2AddressButton::press_action() {
  this->parent_->apply_pending_address();
}
#endif  // USE_BUTTON

void I2CJoystick2Component::dump_config() {
  ESP_LOGCONFIG(TAG, "I2C Joystick2:");
  LOG_I2C_DEVICE(this);

  ESP_LOGCONFIG(TAG, "I2C Joystick2: \n"
                     "  Firmware version: %u", 
                     this->firmware_version_);
}

void I2CJoystick2Sensor::update() {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Joystick2 parent component unavailable");
    return;
  }

  float value = 0.0f;
  if (!this->parent_->read_axis_value(this->axis_, this->mode_, &value)) {
    ESP_LOGW(TAG, "Failed reading joystick axis value");
    return;
  }

  this->publish_state(value);
}

void I2CJoystick2Sensor::dump_config() {
  LOG_SENSOR("", "Joystick2 Axis", this);

  ESP_LOGCONFIG(TAG, "  Axis: %s\n"
                     "  Mode: %u", 
                     this->axis_ == AXIS_X ? "X" : "Y",
                     this->mode_);
}

void I2CJoystick2BinarySensor::update() {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Joystick2 parent component unavailable");
    return;
  }

  bool pressed = false;
  if (!this->parent_->read_button_pressed(&pressed)) {
    ESP_LOGW(TAG, "Failed reading joystick button value");
    return;
  }

  this->publish_state(pressed);
}

void I2CJoystick2BinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Joystick2 Button", this);
}

}  // namespace i2c_joystick_2
}  // namespace esphome
