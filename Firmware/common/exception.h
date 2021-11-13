#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <setjmp.h>

extern jmp_buf exception_state;

void exception_call(char *);
void exception_draw(char *);

#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X
#define PANIC(msg) exception_call(msg "\n" __FILE__ ":" STRINGIZE(__LINE__))

#endif /* EXCEPTION_H_ */