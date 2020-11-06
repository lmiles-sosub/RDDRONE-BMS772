/*
 *
 * BSD 3-Clause License
 * 
 * Copyright 2020 NXP 
 * All rights reserved.
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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/regulated/drone/sensor/BMSInfo.1.0.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 07:34:55.479357 UTC
 * Is deprecated: no
 * Fixed port ID: None
 * Full name:     regulated.drone.sensor.BMSInfo
 * Version:       1.0
 *
 */

#ifndef REGULATED_DRONE_SENSOR_BMSINFO
#define REGULATED_DRONE_SENSOR_BMSINFO
#include <canard_dsdl.h>

#include <regulated/drone/sensor/BatteryChemistry_1_0.h>

#define REGULATED_DRONE_SENSOR_BMSINFO_MSG_SIZE 58



typedef struct regulated_drone_sensor_bmsinfoType
{
	saturated_uint8_ model_name;
	uint32_t model_instance_id;
	float cell_voltage;
	float cell_full_voltage;
	float discharge_rating_cont;
	float discharge_rating_burst;
	float charge_rating_nom;
	float charge_rating_fast;
	float term_curr;
	float design_capacity;
	float remaining_capacity;
	uint16_t n_charges;
	uint8_t cell_count;
	regulated_drone_sensor_battery_chemistry chemistry;
} regulated_drone_sensor_bmsinfo;

void regulated_drone_sensor_bmsinfo_serializeToBuffer(regulated_drone_sensor_bmsinfo* msg, uint8_t* const buffer, const size_t starting_bit)
{
    saturated_uint8__serializeToBuffer(&msg->model_name, buffer, starting_bit + 0);
	canardDSDLSetUxx(buffer, starting_bit + 256, msg->model_instance_id, 32);
	canardDSDLSetF16(buffer, starting_bit + 288, msg->cell_voltage);
	canardDSDLSetF16(buffer, starting_bit + 304, msg->cell_full_voltage);
	canardDSDLSetF16(buffer, starting_bit + 320, msg->discharge_rating_cont);
	canardDSDLSetF16(buffer, starting_bit + 336, msg->discharge_rating_burst);
	canardDSDLSetF16(buffer, starting_bit + 352, msg->charge_rating_nom);
	canardDSDLSetF16(buffer, starting_bit + 368, msg->charge_rating_fast);
	canardDSDLSetF16(buffer, starting_bit + 384, msg->term_curr);
	canardDSDLSetF16(buffer, starting_bit + 400, msg->design_capacity);
	canardDSDLSetF16(buffer, starting_bit + 416, msg->remaining_capacity);
	canardDSDLSetUxx(buffer, starting_bit + 432, msg->n_charges, 16);
	canardDSDLSetUxx(buffer, starting_bit + 448, msg->cell_count, 8);
    regulated_drone_sensor_battery_chemistry_serializeToBuffer(&msg->chemistry, buffer, starting_bit + 456);
}

void regulated_drone_sensor_bmsinfo_deserializeFromBuffer(regulated_drone_sensor_bmsinfo* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        saturated_uint8__deserializeFromBuffer(&msg->model_name, buffer, buf_size, starting_bit + 0);
	msg->model_instance_id = canardDSDLGetU32(buffer, buf_size, starting_bit + 256, 32);
	msg->cell_voltage = canardDSDLGetF16(buffer, buf_size, starting_bit + 288);
	msg->cell_full_voltage = canardDSDLGetF16(buffer, buf_size, starting_bit + 304);
	msg->discharge_rating_cont = canardDSDLGetF16(buffer, buf_size, starting_bit + 320);
	msg->discharge_rating_burst = canardDSDLGetF16(buffer, buf_size, starting_bit + 336);
	msg->charge_rating_nom = canardDSDLGetF16(buffer, buf_size, starting_bit + 352);
	msg->charge_rating_fast = canardDSDLGetF16(buffer, buf_size, starting_bit + 368);
	msg->term_curr = canardDSDLGetF16(buffer, buf_size, starting_bit + 384);
	msg->design_capacity = canardDSDLGetF16(buffer, buf_size, starting_bit + 400);
	msg->remaining_capacity = canardDSDLGetF16(buffer, buf_size, starting_bit + 416);
	msg->n_charges = canardDSDLGetU16(buffer, buf_size, starting_bit + 432, 16);
	msg->cell_count = canardDSDLGetU8(buffer, buf_size, starting_bit + 448, 8);
        
        regulated_drone_sensor_battery_chemistry_deserializeFromBuffer(&msg->chemistry, buffer, buf_size, starting_bit + 456);

    return msg;
}

#endif // REGULATED_DRONE_SENSOR_BMSINFO
