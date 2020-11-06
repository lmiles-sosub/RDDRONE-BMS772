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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/uavcan/pnp/cluster/391.RequestVote.1.0.uavcan
 *
 * Template:
 * ServiceType.j2
 *
 * Generated at:  2020-11-06 07:34:39.034862 UTC
 * Is deprecated: no
 * Fixed port ID: 391
 * Full name:     uavcan.pnp.cluster.RequestVote
 * Version:       1.0
 *
 */

#ifndef UAVCAN_PNP_CLUSTER_REQUESTVOTE
#define UAVCAN_PNP_CLUSTER_REQUESTVOTE
#include <canard_dsdl.h>


#define UAVCAN_PNP_CLUSTER_REQUEST_VOTE_PORT_ID 391

#define UAVCAN_PNP_CLUSTER_REQUEST_VOTE_MSG_SIZE 14



typedef struct uavcan_pnp_cluster_request_voteType
{
	uavcan_pnp_cluster_request_vote_request request;
	uavcan_pnp_cluster_request_vote_response response;
} uavcan_pnp_cluster_request_vote;

void uavcan_pnp_cluster_request_vote_serializeToBuffer(uavcan_pnp_cluster_request_vote* msg, uint8_t* const buffer, const size_t starting_bit)
{
    uavcan_pnp_cluster_request_vote_request_serializeToBuffer(&msg->request, buffer, starting_bit + 0);
    uavcan_pnp_cluster_request_vote_response_serializeToBuffer(&msg->response, buffer, starting_bit + 80);
}

void uavcan_pnp_cluster_request_vote_deserializeFromBuffer(uavcan_pnp_cluster_request_vote* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        uavcan_pnp_cluster_request_vote_request_deserializeFromBuffer(&msg->request, buffer, buf_size, starting_bit + 0);
        
        uavcan_pnp_cluster_request_vote_response_deserializeFromBuffer(&msg->response, buffer, buf_size, starting_bit + 80);

    return msg;
}

#endif // UAVCAN_PNP_CLUSTER_REQUESTVOTE
