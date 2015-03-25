//
//  gc-alternative.h
//  InkscapeCore
//
//  Created by 张 光建 on 14/12/19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#ifndef __InkscapeCore__gc_alternative__
#define __InkscapeCore__gc_alternative__

#include "gc-core.h"

using namespace Inkscape::GC;

void GA_do_init();

void *GA_malloc(size_t size);

void GA_free(void *p);

void *GA_base(void *p);

void GA_register_finalizer(void *p, CleanupFunc f, void *q,
                           CleanupFunc *old_func, void **old_data);

int GA_general_register_disappearing_link(void **pptr, void const *base);

int GA_unregister_disappearing_link(void **link);

std::size_t GA_get_heap_size();

std::size_t GA_get_free_bytes();

void GA_gcollect();

void GA_enable();

void GA_disable();

#endif /* defined(__InkscapeCore__gc_alternative__) */
