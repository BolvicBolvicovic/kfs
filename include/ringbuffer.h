#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#define RINGBUFFER_R(rb, i, s)		(rb)[(i) % (s)]
#define RINGBUFFER_W(rb, i, v, s)	(rb)[(i) % (s)] = (v)

#endif
