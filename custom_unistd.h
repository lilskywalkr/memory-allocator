//
// Created by ronald on 27.04.2025.
//

#ifndef CUSTOM_UNISTD_H
#define CUSTOM_UNISTD_H

#include <unistd.h>
#include <assert.h>

void* custom_sbrk(intptr_t delta);

#if defined(sbrk)
#undef sbrk
#endif

#if defined(brk)
#undef brk
#endif


#define sbrk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), (void*)-1)
#define brk(__arg__) (assert("Proszę nie używać standardowej funkcji sbrk()" && 0), -1)

#endif //CUSTOM_UNISTD_H
