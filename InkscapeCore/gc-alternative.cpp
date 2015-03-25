//
//  gc-alternative.cpp
//  InkscapeCore
//
//  Created by 张 光建 on 14/12/19.
//  Copyright (c) 2014年 Doonsoft. All rights reserved.
//

#include "gc-alternative.h"
#include "gc-core.h"
#include <map>
#include <functional>
#include <vector>

using namespace std;
using namespace Inkscape::GC;

class GA_Base {
public:
    GA_Base() {
        _cleanupFunc = NULL;
        _cleanupData = NULL;
    }
    ~GA_Base() {
        for (int i = 0; i < _softPtrs.size(); i++) {
            *_softPtrs[i] = 0;
        }
    }
    void addSoftPtr(void **pptr) {
        _softPtrs.push_back(pptr);
    }
    void removeSoftPtr(void **pptr) {
        for (size_t i = _softPtrs.size() - 1; i != 0;  i--) {
            if (_softPtrs[i] == pptr) {
                _softPtrs.erase(_softPtrs.begin() + i);
            }
        }
    }
    vector<void**> _softPtrs;
    CleanupFunc _cleanupFunc;
    void *_cleanupData;
};

static map<void*, GA_Base*> baseMap;

void GA_do_init() {}

void *GA_malloc(size_t size) {
    void *p = new char[size];
    memset(p, 0, size);
    baseMap[p] = new GA_Base();
    return p;
}

void GA_free(void *p) {
    GA_Base *pB = baseMap[p];
    if (pB->_cleanupFunc) {
        pB->_cleanupFunc(p, pB->_cleanupData);
    }
    delete pB;
    baseMap.erase(baseMap.find(p));
    delete[] (char *)p;
}

void *GA_base(void *p)
{
    return baseMap[p];
}

void GA_register_finalizer(void *p, CleanupFunc f, void *q,
                              CleanupFunc *old_func, void **old_data)
{
    if (old_func) {
        *old_func = NULL;
    }
    if (old_data) {
        *old_data = NULL;
    }
    
    GA_Base *pB = baseMap[p];
    if (pB) {
        if (old_func) {
            *old_func = pB->_cleanupFunc;
        }
        if (old_data) {
            *old_data = pB->_cleanupData;
        }
        pB->_cleanupFunc = f;
        pB->_cleanupData = q;
    }
}

int GA_general_register_disappearing_link(void **pptr, void const *base)
{
    GA_Base *pB = (GA_Base *)base;
    pB->addSoftPtr(pptr);
    return true;
}

int GA_unregister_disappearing_link(void **link)
{
    GA_Base *pB = baseMap[*link];
    pB->removeSoftPtr(link);
    return false;
}

std::size_t GA_get_heap_size() { return 0; }

std::size_t GA_get_free_bytes() { return 0; }

void GA_gcollect() {}

void GA_enable() {}

void GA_disable() {}
