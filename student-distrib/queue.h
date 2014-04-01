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

#define DECLARE_CIRC_BUF(type, name, size)  \
	struct {                                \
		type *head;                         \
		type *tail;                         \
		type buf[size];                     \
	} (name)

#define CIRC_BUF_FULL(queue) (BUF_PTR_DIFF(queue, tail, head) == (SIZEOF_BUF(queue) - 1))
#define CIRC_BUF_EMPTY(queue) (BUF_PTR_DIFF(queue, tail, head) == 0)

#define CIRC_BUF_INIT(queue)                \
	do {                                    \
		(queue).head = (queue).tail =       \
		    (queue).buf;                    \
	} while (0)

#define CIRC_BUF_PUSH(buf, val, ok)           \
	do {                                      \
		if (CIRC_BUF_FULL(buf)) {             \
			(ok) = 0;                         \
			break;                            \
		}                                     \
		*(buf).tail = val;                    \
		(buf).tail = BUF_PTR_INC(buf, tail);  \
		(ok) = 1;                             \
	} while (0)

#define CIRC_BUF_POP(buf, val, ok)            \
	do {                                      \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                         \
			break;                            \
		}                                     \
		val = *(buf).head;                    \
		(buf).head = BUF_PTR_INC(buf, head);  \
		(ok) = 1;                             \
	} while (0)

#define CIRC_BUF_POP_TAIL(buf, val, ok)       \
	do {                                      \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                         \
			break;                            \
		}                                     \
		(buf).tail = BUF_PTR_DEC(buf, tail);  \
		val = *(buf).tail;                    \
		(ok) = 1;                             \
	} while (0)

#define CIRC_BUF_PEEK(buf, val, ok)           \
	do {                                      \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                         \
			break;                            \
		}                                     \
		val = *(buf).head;                    \
		(ok) = 1;                             \
	} while (0)

#define CIRC_BUF_POKE(buf, val, ok)           \
	do {                                      \
		if (CIRC_BUF_EMPTY(buf)) {            \
			(ok) = 0;                         \
			break;                            \
		}                                     \
		*(buf).head = val;                    \
		(ok) = 1;                             \
	} while (0)

#endif
#endif
