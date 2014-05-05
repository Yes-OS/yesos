/* queue.h, defines general queue structures
 * vim:ts=4 sw=4 noexpandtab
 */
#ifndef _QUEUE_H_
#define _QUEUE_H_

#ifndef ASM

#define SIZEOF_BUF(queue) (sizeof((queue).buf)/sizeof((queue).buf[0]))
#define BUF_PTR_INC(queue, ptr) ((queue).buf + (((queue).ptr - (queue).buf + 1) % SIZEOF_BUF(queue)))
#define BUF_PTR_DEC(queue, ptr) ((queue).buf + (((queue).ptr - (queue).buf - 1 + SIZEOF_BUF(queue)) % SIZEOF_BUF(queue)))
#define BUF_PTR_DIFF(queue, ptr1, ptr2) (((queue).ptr1 - (queue).ptr2 + SIZEOF_BUF(queue)) % SIZEOF_BUF(queue))

/* Creates a buffer of a given type and size
 */
#define CIRC_BUF_TYPE(type, size)           \
	struct {                            \
		type *head;                 \
		type *tail;                 \
		type buf[size];             \
	}
/* Creates a named buffer of a given type, name, and size
 */
#define DECLARE_CIRC_BUF(type, name, size)  \
	CIRC_BUF_TYPE(type, size) (name)

/* Check if a given queue is full
 */
#define CIRC_BUF_FULL(queue) (BUF_PTR_DIFF(queue, tail, head) == (SIZEOF_BUF(queue) - 2))

/* Check if a given queue is empty
 */
#define CIRC_BUF_EMPTY(queue) (BUF_PTR_DIFF(queue, tail, head) == 0)

/* Initialize and set pointers for a given buffer
 */
#define CIRC_BUF_INIT(queue)                    \
	do {                                    \
		(queue).head = (queue).tail =   \
		    (queue).buf;                \
	} while (0)

/* Push to the tail of the buffer
 */
#define CIRC_BUF_PUSH(buf, val, ok)                   \
	do {                                          \
		if (CIRC_BUF_FULL(buf)) {             \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		*(buf).tail = val;                    \
		(buf).tail = BUF_PTR_INC(buf, tail);  \
		(ok) = 1;                             \
	} while (0)

/* Pop from the head of the buffer
 */
#define CIRC_BUF_POP(buf, val, ok)                    \
	do {                                          \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		val = *(buf).head;                    \
		(buf).head = BUF_PTR_INC(buf, head);  \
		(ok) = 1;                             \
	} while (0)

/* Pop from the tail of the buffer 
 */
#define CIRC_BUF_POP_TAIL(buf, val, ok)               \
	do {                                          \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		(buf).tail = BUF_PTR_DEC(buf, tail);  \
		val = *(buf).tail;                    \
		(ok) = 1;                             \
	} while (0)

/* Give the value at the head of the buffer
 */
#define CIRC_BUF_PEEK(buf, val, ok)                   \
	do {                                          \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		val = *(buf).head;                    \
		(ok) = 1;                             \
	} while (0)

/* Give the value of the tail of the buffer
 */
#define CIRC_BUF_PEEK_TAIL(buf, val, ok)              \
	do {                                          \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		val = *((buf).tail - 1);              \
		(ok) = 1;                             \
	} while (0)

/* Give the buffer head a new value
 */
#define CIRC_BUF_POKE(buf, val, ok)                   \
	do {                                          \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		*(buf).head = val;                    \
		(ok) = 1;                             \
	} while (0)

/* Give the buffer tail a new value
 */
#define CIRC_BUF_POKE_TAIL(buf, val, ok)              \
	do {                                          \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                     \
			break;                        \
		}                                     \
		*((buf).tail - 1) = val;              \
		(ok) = 1;                             \
	} while (0)

/* Return the value at a passed index
 */
#define CIRC_BUF_IDX(_buf, idx) ((((idx >= 0) ? (_buf).head : (_buf).tail) - (_buf).buf + (idx)) % SIZEOF_BUF(_buf) + (_buf).buf)


#endif /* ASM */
#endif /* _QUEUE_H */
