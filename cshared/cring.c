/*********************************************************************
######################################################################
##
##  Created by: Denis Filatov
##  Date      : 10.11.2005
##
##  Copyleft (c) 2003 - 2015
##  This code is provided under the CeCill-C license agreement.
######################################################################
*********************************************************************/
#include "cring.h"
#include "cmem.h"
#include <stdlib.h>
void      _cring_init( cring_t * const r )
{
    r->next = r;
    r->prev = r;
}

cring_t * _cring_erase( cring_t * const x )
{
    cring_t * n = x->next;
    cring_t * p = x->prev;
    n->prev = p;
    p->next = n;
    x->next = x;
    x->prev = x;
    return n;
}

cring_t * _cring_insert_after( cring_t * const r, cring_t * const i)
{
    cring_t * n = r->next;
    i->prev = r;
    r->next = i;
    i->next = n;
    n->prev = i;
    return n;
}

cring_t * _cring_insert_before( cring_t * const r, cring_t * const i)
{
    cring_t * p = r->prev;
    i->next = r;
    r->prev = i;
    i->prev = p;
    p->next = i;
    return p;
}

cring_t * _cring_insert_ring_after( cring_t * const p, cring_t * const b)
{
    cring_t *n, *e;

    if(b->next == b){
        return _cring_insert_after(p, b);
    }

    n = p->next;
    e = b->prev;

    p->next = b;
    b->prev = p;
    n->prev = e;
    e->next = n;
    return n;
}

cring_t * _cring_insert_ring_before( cring_t * const n, cring_t * const b)
{
    cring_t *p, *e;

    if(b->next == b){
        return _cring_insert_before(n, b);
    }
    p = n->prev;
    e = b->prev;

    p->next = b;
    b->prev = p;
    n->prev = e;
    e->next = n;
    return p;
}

cring_t * _cring_erase_ring( cring_t * const f, cring_t * const l)
{
    cring_t *p, *n;

    if(f == l){
        return _cring_erase(f);
    }

    p = f->prev;
    n = l->next;

    f->prev = l;
    l->next = f;

    p->next = n;
    n->prev = p;
    return n;
}

int       cring_is_empty( cring_t * const r )
{
    return r->next == r;
}

void      cring_cleanup(cring_t * const r, void * const fn_destructor)
{
    while(r->next != r){
        cring_t * x = r->next;
        _cring_erase(x);
        if(fn_destructor){
            ((void(*)(void*))fn_destructor)(x);
        }
    }
}

cring_t * _cring_insert_sorted(cring_t * const r, cring_t * const n, cring_compare_fn * const fn_compare)
{
	cring_t * i = r->next;
	for (; i != r; i = i->next){
		int x = fn_compare(i, n);
		if (x == 0)
			return i;
		if (x > 0) /* i > n */
			break;
	}
	_cring_insert_before(i, n);
	return n;
}

cring_t * _cring_find_sorted(cring_t * const r, cring_t * const n, cring_compare_fn * const fn_compare)
{
	cring_t * i = r->next;
	for (; i != r; i = i->next){
		int x = fn_compare(i, n);
		if (x == 0)
			return i;
		if (x > 0) /* i > n */
			break;
	}
	return NULL;
}

cring_t * cring_zerase( cring_t * * const root, cring_t * const r )
{
    if(r->next == r){
        *root = NULL;
        return r;
    }

    if(*root == r){
        *root = r->next;
    }
    return _cring_erase(r);
}

cring_t * cring_zinsert_after (cring_t * * const root, cring_t * const i)
{
    if(*root == NULL) {
        *root = i;
        return i;
    }
    return _cring_insert_ring_after(*root, i);
}

cring_t * cring_zinsert_before(cring_t * * const root, cring_t * const i)
{
    if(*root == NULL) {
        *root = i;
        return i;
    }
    return _cring_insert_ring_before(*root, i);
}

cring_t * cring_zerase_ring( cring_t * * const root, cring_t * const f, cring_t * const l)
{
    cring_t *i, *ret;
    if(f->prev == l){
        /* remove full ring */
        *root = NULL;
        return NULL;
    }

    ret = _cring_erase_ring( f, l);

    /* need to check if *root occurs in erased space */
    i = f;
    do{
        if(i == *root){
            *root = ret->prev;
            break;
        }
        if(i == l){
            break;
        }
        i=i->next;
    }while(1);
    return ret;
}

void cring_zcleunup( cring_t * * const root, void * const fn_destructor)
{
    cring_t * r = *root;
    if(r){
        *root = NULL;
        do{
            cring_t * n = r->next;
            _cring_erase(r);
            if(fn_destructor){
                ((void(*)(void*))fn_destructor)(r);
            }
            if(r == n){
                break;
            }
            r=n;
        }while(1);
    }
}


static cring_t __xcring_pool = {&__xcring_pool, &__xcring_pool};

xcring_t * xcring_new (void * const data)
{
    xcring_t * r;
    if(__xcring_pool.next == &__xcring_pool){
        int i;
        /* preallocate PREALLOC_D items */
        r = callocate(sizeof(xcring_t)*XRING_PREALLOC);
        if(NULL == r) return NULL;
        for(i = 0; i<XRING_PREALLOC-1; i++, r++){
            r->next = r;
            r->prev = r;
            r->data = NULL;
            _cring_insert_after( &__xcring_pool, (cring_t *) r);
        }
        /* use last allocated item */
        r->next = r;
        r->prev = r;
    }else{
        r = (xcring_t *)__xcring_pool.next;
        _cring_erase( __xcring_pool.next );
    }
    r->data = data;
    return r;
}

void       xcring_free(xcring_t * const r, void * const fn_destructor)
{
    if(r->data){
        if(fn_destructor){
            ((void(*)(void*))fn_destructor)(r->data);
        }
        r->data = NULL;
    }
    _cring_insert_after( &__xcring_pool, (cring_t *) r);
}

void       xcring_cleanup(cring_t * const root, void * const fn_destructor)
{
    while(!cring_is_empty(root)){
        xcring_t * r = (xcring_t *)root->next;
        _cring_erase(root->next);
        xcring_free(r, fn_destructor);
    }
}

xcring_t * xcring_enqueue(cring_t * const root, void * const data)
{
    xcring_t * r = xcring_new (data);
    _cring_insert_before(root, (cring_t*)r);
    return r;
}

void * xcring_dequeue(cring_t * const root)
{
    void * data;
    xcring_t * r;

    if(root->next == root){
        return NULL;
    }

    r = (xcring_t*)root->next;
    _cring_erase((cring_t*)r);
    data=r->data;
    r->data = NULL;
    _cring_insert_after( &__xcring_pool, (cring_t *) r);
    return data;
}

void *     xcring_find   (cring_t * const root, void * const pattern,
	                      int(*comparator)(const void * const pattern,
                                           const void * const data))
{
    xcring_t * r = (xcring_t *)root->next;
    for(;r!=(xcring_t *)root; r=r->next){
        if(0==comparator(pattern, r->data)){
            return r->data;
        }
    }
    return NULL;
}

