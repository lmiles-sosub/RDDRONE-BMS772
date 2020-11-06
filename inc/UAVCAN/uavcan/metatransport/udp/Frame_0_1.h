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
 * /home/cis/drones/s32k-bms/software/rddrone-bms772/src/nxp_bms/BMS_v1/public_regulated_data_types/uavcan/metatransport/udp/Frame.0.1.uavcan
 *
 * Template:
 * StructureType.j2
 *
 * Generated at:  2020-11-06 07:34:50.920498 UTC
 * Is deprecated: no
 * Fixed port ID: None
 * Full name:     uavcan.metatransport.udp.Frame
 * Version:       0.1
 *
 */

#ifndef UAVCAN_METATRANSPORT_UDP_FRAME
#define UAVCAN_METATRANSPORT_UDP_FRAME
#include <canard_dsdl.h>

#include <uavcan/time/SynchronizedTimestamp_1_0.h>
#include <uavcan/metatransport/udp/Endpoint_0_1.h>
#include <uavcan/metatransport/udp/Endpoint_0_1.h>

#define UAVCAN_METATRANSPORT_UDP_FRAME_MSG_SIZE 9262

#define MTU 0x9188


typedef struct uavcan_metatransport_udp_frameType
{
	uavcan_time_synchronized_timestamp timestamp;
	uavcan_metatransport_udp_endpoint source;
	uavcan_metatransport_udp_endpoint destination;
	saturated_uint8_9 data;
} uavcan_metatransport_udp_frame;

void uavcan_metatransport_udp_frame_serializeToBuffer(uavcan_metatransport_udp_frame* msg, uint8_t* const buffer, const size_t starting_bit)
{
    uavcan_time_synchronized_timestamp_serializeToBuffer(&msg->timestamp, buffer, starting_bit + 0);
    uavcan_metatransport_udp_endpoint_serializeToBuffer(&msg->source, buffer, starting_bit + 64);
    uavcan_metatransport_udp_endpoint_serializeToBuffer(&msg->destination, buffer, starting_bit + 320);
    saturated_uint8_9_serializeToBuffer(&msg->data, buffer, starting_bit + 576);
}

void uavcan_metatransport_udp_frame_deserializeFromBuffer(uavcan_metatransport_udp_frame* msg, const uint8_t* const buffer, const size_t buf_size, const size_t starting_bit)
{

        
        uavcan_time_synchronized_timestamp_deserializeFromBuffer(&msg->timestamp, buffer, buf_size, starting_bit + 0);
        
        uavcan_metatransport_udp_endpoint_deserializeFromBuffer(&msg->source, buffer, buf_size, starting_bit + 64);
        
        uavcan_metatransport_udp_endpoint_deserializeFromBuffer(&msg->destination, buffer, buf_size, starting_bit + 320);
        
        saturated_uint8_9_deserializeFromBuffer(&msg->data, buffer, buf_size, starting_bit + 576);

    return msg;
}

#endif // UAVCAN_METATRANSPORT_UDP_FRAME
