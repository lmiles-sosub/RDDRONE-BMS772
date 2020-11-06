/*
 *
 * BSD 3-Clause License
 * 
 * Copyright 2020 NXP 
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * UAVCAN data structure definition.
 *
 * AUTOGENERATED, DO NOT EDIT.
 *
 * Source File:
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/regulated/zubax/sensor/bms/BatteryPackParams.0.1.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 10:23:23.295517 UTC
 * Is deprecated: no
 * Fixed port ID: None
 * Full name:     regulated.zubax.sensor.bms.BatteryPackParams
 * Version:       0.1
 *
 */

#ifndef REGULATED_ZUBAX_SENSOR_BMS_BATTERYPACKPARAMS
#define REGULATED_ZUBAX_SENSOR_BMS_BATTERYPACKPARAMS
#include <canard_dsdl.h>


#define REGULATED_ZUBAX_SENSOR_BMS_BATTERY_PACK_PARAMS_MSG_SIZE 134

#define CYCLE_COUNT_UNKNOWN 0x4294967295


typedef struct regulated_zubax_sensor_bms_battery_pack_paramsType
{
	uavcan_si_unit_mass_scalar mass;
	uavcan_si_unit_electric_charge_scalar design_capacity;
	uavcan_si_unit_electric_current_scalar design_load_current;
	uavcan_si_unit_voltage_scalar_1_ design_cell_voltage_min_typ_max;
	uavcan_si_unit_voltage_scalar design_charging_voltage;
	uavcan_si_unit_electric_current_scalar design_charging_current;
	uint32_t cycle_count;
	regulated_zubax_sensor_bms_technology technology;
	uint8_t state_of_health;
	uint64_t vendor_specific_parameter;
	uint64_t unique_id;
	saturated_uint8_ name;
} regulated_zubax_sensor_bms_battery_pack_params;

void regulated_zubax_sensor_bms_battery_pack_params_serializeToBuffer(regulated_zubax_sensor_bms_battery_pack_params* msg, uint8_t* const buffer, const size_t starting_bit)
{
    uavcan_si_unit_mass_scalar_serializeToBuffer(&msg->mass, buffer, starting_bit + 0);
    uavcan_si_unit_electric_charge_scalar_serializeToBuffer(&msg->design_capacity, buffer, starting_bit + 32);
    uavcan_si_unit_electric_current_scalar_serializeToBuffer(&msg->design_load_current, buffer, starting_bit + 64);
    uavcan_si_unit_voltage_scalar_1__serializeToBuffer(&msg->design_cell_voltage_min_typ_max, buffer, starting_bit + 96);
    uavcan_si_unit_voltage_scalar_serializeToBuffer(&msg->design_charging_voltage, buffer, starting_bit + 192);
    uavcan_si_unit_electric_current_scalar_serializeToBuffer(&msg->design_charging_current, buffer, starting_bit + 224);
	canardDSDLSetUxx(buffer, starting_bit + 256, msg->cycle_count, 32);
    regulated_zubax_sensor_bms_technology_serializeToBuffer(&msg->technology, buffer, starting_bit + 288);
	canardDSDLSetUxx(buffer, starting_bit + 296, msg->state_of_health, 7);
	canardDSDLSetUxx(buffer, starting_bit + 432, msg->vendor_specific_parameter, 64);
	canardDSDLSetUxx(buffer, starting_bit + 496, msg->unique_id, 64);
    saturated_uint8__serializeToBuffer(&msg->name, buffer, starting_bit + 560);
}

void regulated_zubax_sensor_bms_battery_pack_params_deserializeFromBuffer(regulated_zubax_sensor_bms_battery_pack_params* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        uavcan_si_unit_mass_scalar_deserializeFromBuffer(&msg->mass, buffer, buf_size, starting_bit + 0);
        
        uavcan_si_unit_electric_charge_scalar_deserializeFromBuffer(&msg->design_capacity, buffer, buf_size, starting_bit + 32);
        
        uavcan_si_unit_electric_current_scalar_deserializeFromBuffer(&msg->design_load_current, buffer, buf_size, starting_bit + 64);
        
        uavcan_si_unit_voltage_scalar_1__deserializeFromBuffer(&msg->design_cell_voltage_min_typ_max, buffer, buf_size, starting_bit + 96);
        
        uavcan_si_unit_voltage_scalar_deserializeFromBuffer(&msg->design_charging_voltage, buffer, buf_size, starting_bit + 192);
        
        uavcan_si_unit_electric_current_scalar_deserializeFromBuffer(&msg->design_charging_current, buffer, buf_size, starting_bit + 224);
	msg->cycle_count = canardDSDLGetU32(buffer, buf_size, starting_bit + 256, 32);
        
        regulated_zubax_sensor_bms_technology_deserializeFromBuffer(&msg->technology, buffer, buf_size, starting_bit + 288);
	msg->state_of_health = canardDSDLGetU8(buffer, buf_size, starting_bit + 296, 7);
	msg->vendor_specific_parameter = canardDSDLGetU64(buffer, buf_size, starting_bit + 432, 64);
	msg->unique_id = canardDSDLGetU64(buffer, buf_size, starting_bit + 496, 64);
        
        saturated_uint8__deserializeFromBuffer(&msg->name, buffer, buf_size, starting_bit + 560);

    return msg;
}

#endif // REGULATED_ZUBAX_SENSOR_BMS_BATTERYPACKPARAMS
