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
#ifndef cring_h
#define cring_h

/** @defgroup CRING Double-Linked Rings.
 * @{
 The @ref cring_t structure it is the easiest way to organize rounded
 double-linked lists of any data items. Insertion and removing of items
 are very lightweight operations. Rings are usefull for any types of
 queues, stacks and other types of data storage does not need the index-based
 access to the items.

 Every ring item have two pointers to the next and previous ones. This
 structure can be embedded to the data item structure at the begining or
 in any position (see @ref cring_cast).

 Here we have 3 data items inserted to the ring, headed by @c items member of
 Container structure.

 @code
+==============+  +----->+===========+<--------+
I  Container   I  |      I Data item I         |
I  structure   I  |      I cring_t   I         |
I              I  |      I   {next}--I-----+   |
I    . . .     I  |  +---I---{prev}  I     |   |
I              I  |  |   I  . . .    I     |   |
Icring_t items I<-|--+   +===========+     +----->+===========+
I       {next}-I--+  |                     |   |  I Data item I
I       {prev}-I---+ |                     |   |  I cring_t   I
I              I   | |                     |   +--I---{prev}  I
I    . . .     I   | |   +===========+<-----------I---{next}  I
I              I   +---->I Data item I     |      I  . . .    I
I              I     |   I cring_t   I     |      +===========+
I              I     |   I   {prev}--I-----+
I              I     +---I---{next}  I
I              I         I  . . .    I
+==============+         +===========+
@endcode
 The next and prev pointer of an empty @ref cring_t item points to itself.
 */
#ifdef _MSC_VER
#define __typeof__ __typeof
#endif

typedef struct cring_t cring_t;

/** Base ring structure. */
struct cring_t
{
   cring_t * next; /**< pointer to the next ring element. */
   cring_t * prev; /**< pointer to the next ring element. */
};

#ifndef offsetof
/** @def offsetof(TYPE, MEMBER)
    Return the offset of given member from the begining of the given
    structure type.
    @param TYPE   Structure type.
    @param MEMBER Structure member name.

    Example:
    @code
    struct Base {
        int n1;
        int n2;
        .  .  .
    };
    int ofs = 
    @endcode
    In this example offsetof(Base,n2) is equal to 4 (sizeof(int))
 */
   #define offsetof(TYPE, MEMBER) (unsigned int) &((TYPE *) 0)->MEMBER
#endif

/** Cast the pointer to the member of structure to the pointer
    to the base structure.
 * @param TYPE   Base structure type.
 * @param MEMBER Member of the base structure.
 * @param PTR    Pointer to this member.
 * @return       Pointer to the base structure.
 *
 * Example:
 * The base structure looks like:
 * @code
 * struct Base {
 *     cring_t   ring1;
 *     cring_t   ring2;
 *     .  .  .
 * };
 * @endcode
 * We need to organize this type objects to two independent rings.
 * @code
 * static cring_t r1;
 * static cring_t r2;
 * @endcode
 * We will use @a ring1 member for work with the first ring and @a ring2
 * for the second one.
 * To take the first item in @c r1 we need just cast @c r1.next to the
 * <c>(Base *)</c> type or use the special macro.
 * @code
 * struct Base * ptr = cring_next_cast(&r1, struct Base);
 * @endcode
 * But we will make the same operations for the second ring, we will got
 * a pointer to the @a ring2 member of Base of the first ring item.
 * To cast it to the @a Base type we need for @a cring_cast.
 * @code
 * struct cring_t * r = cring_next(&r2);
 * struct Base * ptr = cring_cast(struct Base, ring2, r);
 * @endcode
 */
#define cring_cast(TYPE,MEMBER,PTR) ( (TYPE*) ((char*)(PTR) - offsetof(TYPE,MEMBER)))

/** Return next ring item.
 * @param x Pointer could been converted to the @ref cring_t structure.
 * @return Next ring item automaticaly converted to the type of x. */
#define cring_next(x) ((__typeof__(x))(((cring_t*)(x))->next))

/** Return previous ring item.
 * @param x Pointer could been converted to the @ref cring_t structure.
 * @return Previous ring item automaticaly converted to the type of x. */
#define cring_prev(x) ((__typeof__(x))(((cring_t*)(x))->prev))

/** Return next ring item with cast conversion.
 * @param x Pointer could been converted to the @ref cring_t structure.
 * @param t Type to cast to.
 * @return Next ring item converted to the given type. */
#define cring_next_cast(x,t)  ((t*)((cring_t*)(x))->next)

/** Return next ring item with cast conversion.
 * @param x Pointer could been converted to the @ref cring_t structure.
 * @param t Type to cast to.
 * @return Previous ring item converted to the given type. */
#define cring_prev_cast(x,t)  ((t*)((cring_t*)(x))->prev)

/** Return first ring item with cast conversion
 * @param x Root ring item.
 * @param t Type to cast to.
 * @return The first item of the ring converted to the given type. */
#define cring_first_cast(x,t) cring_next_cast(&x,t)

/** Return last ring item with cast conversion
 * @param x Root ring item.
 * @param t Type to cast to.
 * @return The last item of the ring converted to the given type. */
#define cring_last_cast(x,t)  cring_prev_cast(&x,t)

void      _cring_init  ( cring_t * const r );
#define    cring_init(R) _cring_init((cring_t*)(R))
cring_t * _cring_erase ( cring_t * const r );
#define    cring_erase(R) _cring_erase((cring_t*)(R))
cring_t * _cring_insert_after(cring_t * const r, cring_t * const i);
#define    cring_insert_after(R,I) _cring_insert_after((cring_t*)(R), (cring_t*)(I))
cring_t * _cring_insert_before(cring_t * const r, cring_t * const i);
#define    cring_insert_before(R,I) _cring_insert_before((cring_t*)(R), (cring_t*)(I))
#define    cring_enqueue(R,I) cring_insert_before(R,I)
cring_t * _cring_insert_ring_after(cring_t * const r, cring_t * const i);
#define    cring_insert_ring_after(R,I) _cring_insert_ring_after((cring_t*)(R), (cring_t*)(I))
cring_t * _cring_insert_ring_before(cring_t * const r, cring_t * const i);
#define    cring_insert_ring_before(R,I) _cring_insert_ring_before((cring_t*)(R), (cring_t*)(I))
cring_t * _cring_erase_ring(cring_t * const from, cring_t * const to);
#define    cring_erase_ring(F,T) _cring_erase_ring((cring_t*)(F), (cring_t*)(T))
int       cring_is_empty(cring_t * const r);

void      cring_cleanup(cring_t * const r, void * const fn_destructor);

typedef int cring_compare_fn(void * const p1, void * const p2);
cring_t * _cring_insert_sorted(cring_t * const r, cring_t * const i, cring_compare_fn * const fn_compare);
cring_t * _cring_find_sorted(cring_t * const r, cring_t * const n, cring_compare_fn * const fn_compare);
#define    cring_insert_sorted(R,I,C) _cring_insert_sorted((cring_t*)(R), (cring_t*)(I), (cring_compare_fn *)(C))
#define    cring_find_sorted(R,I,C) _cring_find_sorted((cring_t*)(R), (cring_t*)(I), (cring_compare_fn *)(C))

cring_t * cring_zerase( cring_t * * const root, cring_t * const r );
cring_t * cring_zerase_ring( cring_t * * const root, cring_t * const from, cring_t * const to);
cring_t * cring_zinsert_after (cring_t * * const root, cring_t * const i);
cring_t * cring_zinsert_before(cring_t * * const root, cring_t * const i);
void      cring_zcleunup( cring_t * * const root, void * const fn_destructor);



#define XRING_PREALLOC 16

typedef struct xcring_t
{
   struct xcring_t * next;
   struct xcring_t * prev;
   void            * data;
} xcring_t;

xcring_t * xcring_new (void * const data);
void       xcring_free(xcring_t * const r, void * const fn_destructor);
void       xcring_cleanup(cring_t * const root, void * const fn_destructor);
#define    xcring_data(TYPE,R) ( (TYPE*) ((xcring_t*)R)->data )

xcring_t * xcring_enqueue(cring_t * const root, void * const data);
void *     xcring_dequeue(cring_t * const root);
void *     xcring_find   (cring_t * const root, void * const pattern,
                          int(*comparator)(const void * const pattern,
                                           const void * const data));

/** @} */

#endif
