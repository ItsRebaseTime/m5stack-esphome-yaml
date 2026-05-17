import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID

from .. import CONF_I2C_JOYSTICK_2_ID, i2c_joystick_2_ns, I2CJoystick2Component

DEPENDENCIES = ["i2c_joystick_2", "button"]

I2CJoystick2AddressButton = i2c_joystick_2_ns.class_(
    "I2CJoystick2AddressButton", button.Button
)

CONFIG_SCHEMA = (
    button.button_schema(I2CJoystick2AddressButton)
    .extend(
        {
            cv.GenerateID(CONF_I2C_JOYSTICK_2_ID): cv.use_id(I2CJoystick2Component),
        }
    )
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await button.register_button(var, config)
    parent = await cg.get_variable(config[CONF_I2C_JOYSTICK_2_ID])
    cg.add(var.set_parent(parent))
