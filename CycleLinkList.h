#pragma once

//    node.prev -> node -> node.next

typedef struct LinkNode
{
	LinkNode* prev;
	LinkNode* next;
}LINKNODE, * PLINKNODE;


class CycleLinkList
{
public:
	static inline void Linklist_init(LinkNode* head);
	static inline void Linklist_add_front(LinkNode* head, LinkNode* node);
	static inline void Linklist_add_back(LinkNode* head, LinkNode* node);
	static inline int Linklist_is_empty(LinkNode* head);
	static inline void Linklist_remote(LinkNode* node);
	static inline void Linklist_splice(LinkNode* head1, LinkNode* node2);
};

inline void CycleLinkList::Linklist_init(LinkNode* head)
{
	head->prev = head;
	head->next = head;
}

inline void CycleLinkList::Linklist_add_front(LinkNode* head, LinkNode* node)
{
	node->prev = head->prev;
	head->prev->next = node;
	node->next = head;
	head->prev = node;
}

inline void CycleLinkList::Linklist_add_back(LinkNode* head, LinkNode* node)
{
	node->prev = head->prev;
	node->next = head;
	node->prev->next = node;
	head->prev = node;
}

inline int CycleLinkList::Linklist_is_empty(LinkNode* head)
{
	return head == head->next;
}

inline void CycleLinkList::Linklist_remote(LinkNode* node)
{
	node->next->prev = node->prev;
	node->prev->next = node->next;
	Linklist_init(node);
}

inline void CycleLinkList::Linklist_splice(LinkNode* head1, LinkNode* head2)
{
	//将head1上的链表数据转移到head2上
	if (!Linklist_is_empty(head1))
	{
		LinkNode* first = head1->next;
		LinkNode* last = head1->prev;
		LinkNode* at = head2->next;
		first->prev = head2;
		head2->next = first;
		last->next = at;
		at->prev = last;
		Linklist_init(head1);
	}
}