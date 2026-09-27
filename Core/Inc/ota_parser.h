/*
 * ota_parser.h
 *
 *  Created on: May 30, 2026
 *      Author: HLC
 */

#ifndef INC_OTA_PARSER_H_
#define INC_OTA_PARSER_H_
#include "ota.h"
typedef struct {
    OtaParseState_t state;
    uint8_t  frame_buffer[sizeof(OtaFrame_t)];
    uint16_t frame_len;
    uint8_t  state_byte_count;
    uint16_t parsed_sequence;
    uint16_t parsed_payload_len;
    uint16_t parsed_crc16;
} OtaParser_t;

void OtaParser_Init(OtaParser_t *parser);
bool OtaParser_FeedByte(OtaParser_t *parser, uint8_t rx_byte, OtaFrame_t *out_frame);

#endif /* INC_OTA_PARSER_H_ */
