from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import (
    CONF_ID,
    CONF_INPUT,
    CONF_INVERTED,
    CONF_MODE,
    CONF_NUMBER,
    CONF_OUTPUT,
)


DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@m5stack"]
AUTO_LOAD = ["gpio_expander"]

CONF_AXP192_ID = "axp192_id"
CONF_AXP192 = "axp192"

axp192_ns = cg.esphome_ns.namespace("axp192")
AXP192 = axp192_ns.class_("AXP192", cg.Component, i2c.I2CDevice)
AXP192GPIOPin = axp192_ns.class_("AXP192GPIOPin", cg.GPIOPin)


BASE_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_AXP192_ID): cv.use_id(AXP192),
})

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AXP192)
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x34))
)


def _validate_gpio_mode(value):
    if not value[CONF_OUTPUT]:
        raise cv.Invalid("AXP192 GPIO pins only support output mode")
    return value


# Pin schema: allows `axp192: axp192_pmu` + `number: 0-4` anywhere a pin is expected.
AXP192_GPIO_PIN_SCHEMA = pins.gpio_base_schema(
    AXP192GPIOPin,
    cv.int_range(min=0, max=4),
    modes=[CONF_OUTPUT],
    mode_validator=_validate_gpio_mode,
    invertible=True,
).extend(
    {
        cv.Required(CONF_AXP192): cv.use_id(AXP192),
    }
)


@pins.PIN_SCHEMA_REGISTRY.register(CONF_AXP192, AXP192_GPIO_PIN_SCHEMA)
async def axp192_pin_to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    parent = await cg.get_variable(config[CONF_AXP192])
    cg.add(var.set_parent(parent))
    cg.add(var.set_pin(config[CONF_NUMBER]))
    cg.add(var.set_inverted(config[CONF_INVERTED]))
    cg.add(var.set_flags(pins.gpio_flags_expr(config[CONF_MODE])))
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
