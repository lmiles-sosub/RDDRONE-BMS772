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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/uavcan/register/Name.1.0.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 10:23:21.289289 UTC
 * Is deprecated: no
 * Fixed port ID: None
 * Full name:     uavcan.register.Name
 * Version:       1.0
 *
 */

#ifndef UAVCAN_REGISTER_NAME
#define UAVCAN_REGISTER_NAME
#include <canard_dsdl.h>


#define UAVCAN_REGISTER_NAME_MSG_SIZE 51



typedef struct uavcan_register_nameType
{
	saturated_uint8_ name;
} uavcan_register_name;

void uavcan_register_name_serializeToBuffer(uavcan_register_name* msg, uint8_t* const buffer, const size_t starting_bit)
{
    saturated_uint8__serializeToBuffer(&msg->name, buffer, starting_bit + 0);
}

void uavcan_register_name_deserializeFromBuffer(uavcan_register_name* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        saturated_uint8__deserializeFromBuffer(&msg->name, buffer, buf_size, starting_bit + 0);

    return msg;
}

#endif // UAVCAN_REGISTER_NAME
