/*
 *   File name: list.h
 *
 *  Created on: 2017年3月14日, 下午8:57:49
 *      Author: victor 
 *   Toolchain: 
 *    Language: C/C++
 * description: Important Notes: all the following functions do not check the validity of the function parameters,
 *              so the caller must make sure that the parameters of the following functions are valid,
 *              otherwise it will interrupt
 */

#ifndef __INCLUDE_LIST_H__
#define __INCLUDE_LIST_H__

typedef struct stSingleList
{
	struct stSingleList * next;
}SingleList;

InlineStatic void initRingSingleList(SingleList *list)
{
	list->next = list;
}

InlineStatic void addNodeToRingSingleListHeader(SingleList * const header, SingleList* const newNode)
{
	newNode->next = header->next;
	header->next = newNode;
}

InlineStatic SingleList* delNodeFromRingSingleListHeader(SingleList * const header)
{
	SingleList* ret = header->next;
	header->next = ret->next;
	return ret;
}

typedef struct stDoubleList
{
	struct stDoubleList * next;
	struct stDoubleList * prev;
} DoubleList ;

InlineStatic void initRingDoubleList(DoubleList * const header)
{
	header->next = header;
	header->prev = header;
}

InlineStatic void addNodeToRingDoubleListHeader(DoubleList * const header, DoubleList* const newNode)
{
	newNode->next = header->next;
	newNode->prev = header;
	header->next = newNode;
	(newNode->next)->prev = newNode;
}

InlineStatic void addNodeToRingDoubleListTail(DoubleList * const header, DoubleList* const newNode)
{
	newNode->prev = header->prev;
	newNode->next = header;
	header->prev = newNode;
	(newNode->prev)->next = newNode;
}

InlineStatic void delNodeFromRingDoubleList(DoubleList* const deletingNode)
{
	(deletingNode->prev)->next = deletingNode->next;
	(deletingNode->next)->prev = deletingNode->prev;
	deletingNode->next = deletingNode->prev = 0;
}

template<typename T>
InlineStatic bool ringListIsEmpty(const T * const header)
{
	return (header->next == header);
}

#define List_Entry(ptr, type, member) \
		(type*) ((char*) (ptr) - (char*) &((type*)(0))->member)

#define List_FirstNode_Entry(header, type, member) \
		List_Entry((header)->next, type, member)

#define RingList_Foreach(p, header) \
	for(p = (header)->next; p != (header); p = p->next)

#define RingList_Foreach_Safe(p, n, header) \
	for(p = (header)->next, n = p->next; p != (header); p = n, n = n->next)

#define RingList_Foreach_Entry(p, header, member) \
	for(p = List_Entry((header)->next, typeof(*p), member); \
		&p->member != (header); p = List_Entry(p->member.next, typeof(*p), member))

#endif

