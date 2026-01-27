import logging

import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = ["blauberg_s14"]

CONF_RESPONSE_SENSOR = "response"

blauberg_ns = cg.esphome_ns.namespace("blauberg_s14")

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Required("blauberg_s14_id"): cv.use_id(blauberg_ns.BlaubergS14Controller),
        cv.Optional(CONF_RESPONSE_SENSOR): text_sensor.text_sensor_schema(),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    controller_ = await cg.get_variable(config["blauberg_s14_id"])

    if response_sensor := config.get(CONF_RESPONSE_SENSOR):
        sensor_var = await text_sensor.new_text_sensor(response_sensor)
        cg.add(controller_.setResponseSensor(sensor_var))
