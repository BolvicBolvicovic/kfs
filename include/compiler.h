#ifndef COMPILER_H
#define COMPILER_H

#define __always_inline	inline __attribute__((__always_inline__))
#define	__packed	__attribute__((__packed__))	
#define __aligned(x)	__attribute__((__aligned(x)__))
#define __counted_by(x)	__attribute__((__counted_by(x)))
#define __may_alias	__attribute__((__may_alias__))

#endif
