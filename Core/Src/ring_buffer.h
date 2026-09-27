/*
 * ring_buffer.h
 *
 *  Created on: May 16, 2026
 *      Author: HLC
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_
#define RING_BUFFER_SIZE 1024
#include <stdbool.h>
#include <stdint.h>
typedef struct {
	uint8_t buffer[RING_BUFFER_SIZE];
	volatile uint32_t head;
	volatile uint32_t tail;
} RingBuffer_t;

bool RingBuffer_Push(RingBuffer_t *rb, uint8_t data);

bool RingBuffer_Pop(RingBuffer_t *rb, uint8_t *data);

bool RingBuffer_IsEmpty(RingBuffer_t *rb);

bool RingBuffer_IsFull(RingBuffer_t *rb);

#endif /* RING_BUFFER_H_ */
