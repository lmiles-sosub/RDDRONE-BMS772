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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/uavcan/node/435.ExecuteCommand.1.0.uavcan
 *
 * Template:
 * ServiceType.j2
 *
 * Generated at:  2020-11-06 07:34:52.050015 UTC
 * Is deprecated: no
 * Fixed port ID: 435
 * Full name:     uavcan.node.ExecuteCommand
 * Version:       1.0
 *
 */

#ifndef UAVCAN_NODE_EXECUTECOMMAND
#define UAVCAN_NODE_EXECUTECOMMAND
#include <canard_dsdl.h>

#include <uavcan/node/ExecuteCommand/Request_1_0.h>
#include <uavcan/node/ExecuteCommand/Response_1_0.h>

#define UAVCAN_NODE_EXECUTE_COMMAND_PORT_ID 435

#define UAVCAN_NODE_EXECUTE_COMMAND_MSG_SIZE 122



typedef struct uavcan_node_execute_commandType
{
	uavcan_node_execute_command_request request;
	uavcan_node_execute_command_response response;
} uavcan_node_execute_command;

void uavcan_node_execute_command_serializeToBuffer(uavcan_node_execute_command* msg, uint8_t* const buffer, const size_t starting_bit)
{
    uavcan_node_execute_command_request_serializeToBuffer(&msg->request, buffer, starting_bit + 0);
    uavcan_node_execute_command_response_serializeToBuffer(&msg->response, buffer, starting_bit + 920);
}

void uavcan_node_execute_command_deserializeFromBuffer(uavcan_node_execute_command* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        uavcan_node_execute_command_request_deserializeFromBuffer(&msg->request, buffer, buf_size, starting_bit + 0);
        
        uavcan_node_execute_command_response_deserializeFromBuffer(&msg->response, buffer, buf_size, starting_bit + 920);

    return msg;
}

#endif // UAVCAN_NODE_EXECUTECOMMAND
