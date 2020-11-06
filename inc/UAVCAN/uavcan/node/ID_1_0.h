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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/uavcan/node/ID.1.0.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 10:23:21.855256 UTC
 * Is deprecated: no
 * Fixed port ID: None
 * Full name:     uavcan.node.ID
 * Version:       1.0
 *
 */

#ifndef UAVCAN_NODE_ID
#define UAVCAN_NODE_ID
#include <canard_dsdl.h>


#define UAVCAN_NODE_ID_MSG_SIZE 2



typedef struct uavcan_node_idType
{
	uint16_t value;
} uavcan_node_id;

void uavcan_node_id_serializeToBuffer(uavcan_node_id* msg, uint8_t* const buffer, const size_t starting_bit)
{
	canardDSDLSetUxx(buffer, starting_bit + 0, msg->value, 16);
}

void uavcan_node_id_deserializeFromBuffer(uavcan_node_id* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

	msg->value = canardDSDLGetU16(buffer, buf_size, starting_bit + 0, 16);

    return msg;
}

#endif // UAVCAN_NODE_ID
