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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/regulated/zubax/actuator/esc/Status.0.1.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 10:23:23.751133 UTC
 * Is deprecated: no
 * Fixed port ID: None
 * Full name:     regulated.zubax.actuator.esc.Status
 * Version:       0.1
 *
 */

#ifndef REGULATED_ZUBAX_ACTUATOR_ESC_STATUS
#define REGULATED_ZUBAX_ACTUATOR_ESC_STATUS
#include <canard_dsdl.h>


#define REGULATED_ZUBAX_ACTUATOR_ESC_STATUS_MSG_SIZE 32

#define MAX_PUBLICATION_PERIOD 0x1


typedef struct regulated_zubax_actuator_esc_statusType
{
	uint8_t index;
	uint8_t demand_factor;
	bool dc_voltage_warning;
	bool overload_warning;
	bool motor_temperature_warning;
	bool inverter_temperature_warning;
	uint32_t error_count;
	uavcan_si_unit_angular_velocity_scalar motor_mechanical_angular_velocity;
	uavcan_si_unit_torque_scalar motor_torque;
	uavcan_si_unit_frequency_scalar motor_electrical_frequency;
	regulated_zubax_sensor_wattmeter_dcsample dc_link_power;
	float motor_temperature;
	float inverter_temperature;
} regulated_zubax_actuator_esc_status;

void regulated_zubax_actuator_esc_status_serializeToBuffer(regulated_zubax_actuator_esc_status* msg, uint8_t* const buffer, const size_t starting_bit)
{
	canardDSDLSetUxx(buffer, starting_bit + 0, msg->index, 6);
	canardDSDLSetUxx(buffer, starting_bit + 8, msg->demand_factor, 8);
	canardDSDLSetBit(buffer, starting_bit + 16, msg->dc_voltage_warning);
	canardDSDLSetBit(buffer, starting_bit + 17, msg->overload_warning);
	canardDSDLSetBit(buffer, starting_bit + 18, msg->motor_temperature_warning);
	canardDSDLSetBit(buffer, starting_bit + 19, msg->inverter_temperature_warning);
	canardDSDLSetUxx(buffer, starting_bit + 32, msg->error_count, 32);
    uavcan_si_unit_angular_velocity_scalar_serializeToBuffer(&msg->motor_mechanical_angular_velocity, buffer, starting_bit + 64);
    uavcan_si_unit_torque_scalar_serializeToBuffer(&msg->motor_torque, buffer, starting_bit + 96);
    uavcan_si_unit_frequency_scalar_serializeToBuffer(&msg->motor_electrical_frequency, buffer, starting_bit + 128);
    regulated_zubax_sensor_wattmeter_dcsample_serializeToBuffer(&msg->dc_link_power, buffer, starting_bit + 160);
	canardDSDLSetF16(buffer, starting_bit + 224, msg->motor_temperature);
	canardDSDLSetF16(buffer, starting_bit + 240, msg->inverter_temperature);
}

void regulated_zubax_actuator_esc_status_deserializeFromBuffer(regulated_zubax_actuator_esc_status* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

	msg->index = canardDSDLGetU8(buffer, buf_size, starting_bit + 0, 6);
	msg->demand_factor = canardDSDLGetU8(buffer, buf_size, starting_bit + 8, 8);
	msg->dc_voltage_warning = canardDSDLGetBit(buffer, buf_size, starting_bit + 16);
	msg->overload_warning = canardDSDLGetBit(buffer, buf_size, starting_bit + 17);
	msg->motor_temperature_warning = canardDSDLGetBit(buffer, buf_size, starting_bit + 18);
	msg->inverter_temperature_warning = canardDSDLGetBit(buffer, buf_size, starting_bit + 19);
	msg->error_count = canardDSDLGetU32(buffer, buf_size, starting_bit + 32, 32);
        
        uavcan_si_unit_angular_velocity_scalar_deserializeFromBuffer(&msg->motor_mechanical_angular_velocity, buffer, buf_size, starting_bit + 64);
        
        uavcan_si_unit_torque_scalar_deserializeFromBuffer(&msg->motor_torque, buffer, buf_size, starting_bit + 96);
        
        uavcan_si_unit_frequency_scalar_deserializeFromBuffer(&msg->motor_electrical_frequency, buffer, buf_size, starting_bit + 128);
        
        regulated_zubax_sensor_wattmeter_dcsample_deserializeFromBuffer(&msg->dc_link_power, buffer, buf_size, starting_bit + 160);
	msg->motor_temperature = canardDSDLGetF16(buffer, buf_size, starting_bit + 224);
	msg->inverter_temperature = canardDSDLGetF16(buffer, buf_size, starting_bit + 240);

    return msg;
}

#endif // REGULATED_ZUBAX_ACTUATOR_ESC_STATUS
