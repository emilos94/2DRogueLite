#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#define LinkedListAddBack(item, to_add)\
do {\
    item->Next = to_add;\
    to_add->Prev = item;\
} while(0)

#define LinkedListAddFront(item, to_add) LinkedListAddBack(to_add, next)


#endif