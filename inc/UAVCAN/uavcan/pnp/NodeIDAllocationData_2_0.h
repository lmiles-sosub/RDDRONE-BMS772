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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/uavcan/pnp/32741.NodeIDAllocationData.2.0.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 07:34:38.447781 UTC
 * Is deprecated: no
 * Fixed port ID: 32741
 * Full name:     uavcan.pnp.NodeIDAllocationData
 * Version:       2.0
 *
 */

#ifndef UAVCAN_PNP_NODEIDALLOCATIONDATA
#define UAVCAN_PNP_NODEIDALLOCATIONDATA
#include <canard_dsdl.h>

#include <uavcan/node/ID_1_0.h>

#define UAVCAN_PNP_NODE_IDALLOCATION_DATA_PORT_ID 32741

#define UAVCAN_PNP_NODE_IDALLOCATION_DATA_MSG_SIZE 18



typedef struct uavcan_pnp_node_idallocation_dataType
{
	uavcan_node_id node_id;
	saturated_uint8 unique_id;
} uavcan_pnp_node_idallocation_data;

void uavcan_pnp_node_idallocation_data_serializeToBuffer(uavcan_pnp_node_idallocation_data* msg, uint8_t* const buffer, const size_t starting_bit)
{
    uavcan_node_id_serializeToBuffer(&msg->node_id, buffer, starting_bit + 0);
    saturated_uint8_serializeToBuffer(&msg->unique_id, buffer, starting_bit + 16);
}

void uavcan_pnp_node_idallocation_data_deserializeFromBuffer(uavcan_pnp_node_idallocation_data* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        uavcan_node_id_deserializeFromBuffer(&msg->node_id, buffer, buf_size, starting_bit + 0);
        
        saturated_uint8_deserializeFromBuffer(&msg->unique_id, buffer, buf_size, starting_bit + 16);

    return msg;
}

#endif // UAVCAN_PNP_NODEIDALLOCATIONDATA
