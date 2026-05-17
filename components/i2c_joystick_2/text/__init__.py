import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text
from esphome.const import CONF_ID

from .. import CONF_I2C_JOYSTICK_2_ID, i2c_joystick_2_ns, I2CJoystick2Component

DEPENDENCIES = ["i2c_joystick_2", "text"]

I2CJoystick2AddressText = i2c_joystick_2_ns.class_(
    "I2CJoystick2AddressText", text.Text, cg.Component
)

CONFIG_SCHEMA = (
    text.text_schema(I2CJoystick2AddressText, mode="TEXT")
    .extend(
        {
            cv.GenerateID(CONF_I2C_JOYSTICK_2_ID): cv.use_id(I2CJoystick2Component),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text.register_text(var, config)
    parent = await cg.get_variable(config[CONF_I2C_JOYSTICK_2_ID])
    cg.add(var.set_parent(parent))
