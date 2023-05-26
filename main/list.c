#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void init_list(List *lp) {
    assert(lp != NULL);
    lp->head = NULL;
    lp->tail = NULL;
}

void init_node(ListNode *n, void *d) {
    assert(n != NULL);
    n->next = NULL;
    n->data = d;
}

List *new_list() {
    List *new_list = (List *) malloc(sizeof(List));
    init_list(new_list);
    return new_list;
}

ListNode *append_node(List *l, void *d) {

    ListNode *new_node = (ListNode *) malloc(sizeof(ListNode));
    init_node(new_node, d);

    if (l->head == NULL) {
        // If this is the 1st insertion, allocate the node and set head & tail to
        // it...
        l->head = new_node;
        l->tail = new_node;
    } else {
        // Otherwise append the node to the end of the list
        l->tail->next = new_node;
        l->tail = new_node;
    }

    return new_node;
}

ListNode *insert_node_after(List *l, ListNode *n, void *d) {
    assert(l != NULL);
    assert(n != NULL);

    ListNode *new_node = (ListNode *) malloc(sizeof(ListNode));
    init_node(new_node, d);

    if (n == l->tail) {
        l->tail->next = new_node;
        l->tail = new_node;
    } else {
        ListNode *old_node = n->next;
        n->next = new_node;
        new_node->next = old_node;
    }
    
    return new_node;
}

void for_each_node(List *l, list_cb cb) {
    ListNode *cur_node = l->head;

    while (cur_node != NULL) {
        cb(cur_node);
        cur_node = (ListNode *) cur_node->next;
    }
}

ListNode *append_data(List *l, void *d, size_t len) {
    void *data_storage = malloc(len);
    memcpy(data_storage, d, len);
    ListNode *new_node = append_node(l, data_storage);
    return new_node;
}


ListNode *insert_data_after(List *l, ListNode *n, void *d, size_t len) {
    void *data_storage = malloc(len);
    memcpy(data_storage, d, len);
    ListNode *new_node = insert_node_after(l, n, data_storage);
    return new_node;
}

void remove_node_from_list(List *l, ListNode *n) {
    assert(l != NULL);
    assert(n != NULL);
    if ((n == l->head) && (n == l->tail)) {
        l->head = NULL;
        l->tail = NULL;
        free(n->data);
        free((void *) n);
    } else if (n == l->head) {
        l->head = n->next;
        free(n->data);
        free((void *) n);
    } else if (n == l->tail) {
        ListNode *parent = l->head;
        while (parent->next != n) {
            parent = parent->next;
        }
        parent->next = n->next;
        free(n->data);
        free((void *) n);
    }
}

void free_list(List *l) {
    assert(l != NULL);
    if ((l->head != NULL) && (l->tail != NULL)) {
        ListNode *current_node = l->head;
        while (current_node != NULL) {
            ListNode *next_node = current_node->next;
            free(current_node->data);
            free((void *) current_node);
            current_node = next_node;
        }

        free((void *) l);
    }
}
