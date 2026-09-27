/*
 * ring_buffer.c
 *
 *  Created on: May 16, 2026
 *      Author: HLC
 */

#include "ring_buffer.h"

bool RingBuffer_Push(RingBuffer_t *rb, uint8_t data) {
	uint32_t next = (rb->head + 1) & (RING_BUFFER_SIZE - 1);
	if (next == rb->tail) {
		return false; // Full
	}
	rb->buffer[rb->head] = data;
	rb->head = next;
	return true;
}

bool RingBuffer_Pop(RingBuffer_t *rb, uint8_t *data) {
	if (rb->head == rb->tail) {
		return false; // Empty
	}

	*data = rb->buffer[rb->tail];

	rb->tail = (rb->tail + 1) & (RING_BUFFER_SIZE - 1);

	return true;
}

bool RingBuffer_IsEmpty(RingBuffer_t *rb) {
	return 	(rb->head == rb->tail);
}

bool RingBuffer_IsFull(RingBuffer_t *rb) {
	return 	((rb->head + 1) & (RING_BUFFER_SIZE - 1)) == rb->tail;
}
