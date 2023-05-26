
#pragma once

#include <stdio.h>

typedef struct list_node_t {
    struct list_node_t *next;
    void *data;
} ListNode;

typedef struct list_t {
    ListNode *head;
    ListNode *tail;
} List;

typedef void (*list_cb)(ListNode *);

List *new_list();

ListNode *append_node(List *l, void *d);


ListNode *append_data(List *l, void *d, size_t len);

ListNode *insert_data_after(List *l, ListNode *n, void *d, size_t len);

ListNode *insert_node_after(List *l, ListNode *n, void *d);

void remove_node_from_list(List *l, ListNode *n);

ListNode *head(List *);

ListNode *tail(List *);

void for_each_node(List *l, list_cb cb);

void free_list(List *l);
