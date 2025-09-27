#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head)
        INIT_LIST_HEAD(head);
    else
        fprintf(stderr, "Error: malloc failed in q_new()\n");
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *cur, *next;
    list_for_each_safe(cur, next, head) {
        element_t *entry = list_entry(cur, element_t, list);
        list_del(cur);
        free(entry->value);
        free(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        return false;

    char *tmp = strdup(s);
    if (!tmp) {
        free(new_elem);
        return false;  // to check if allocation was successful
    }

    new_elem->value = tmp;
    list_add(&new_elem->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_elem = malloc(sizeof(element_t));
    if (!new_elem)
        return false;

    new_elem->value = strdup(s);
    if (!new_elem->value) {
        free(new_elem);
        return false;
    }

    list_add_tail(&new_elem->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *first = head->next;
    element_t *entry = list_entry(first, element_t, list);
    list_del(first);

    if (sp && bufsize > 0) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';  // Ensure null-termination
    }

    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    struct list_head *last = head->prev;
    element_t *entry = list_entry(last, element_t, list);
    list_del(last);

    if (sp && bufsize > 0) {
        strncpy(sp, entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';  // Ensure null-termination
    }

    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *cur;
    list_for_each(cur, head)
        len++;

    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    struct list_head *slow = head->next;
    struct list_head *fast = head->next;
    if (list_empty(head))
        return false;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    list_del(slow);
    element_t *entry = list_entry(slow, element_t, list);
    free(entry->value);
    free(entry);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    struct list_head *cur, *safe;
    bool deleted = false;

    list_for_each_safe(cur, safe, head) {
        element_t *entry = list_entry(cur, element_t, list);
        if (cur->next != head) {
            const element_t *next_entry =
                list_entry(cur->next, element_t, list);
            if (strcmp(entry->value, next_entry->value) == 0) {
                // Found duplicate
                deleted = true;
                list_del(cur);
                free(entry->value);
                free(entry);
            } else if (deleted) {
                // End of duplicates
                deleted = false;
                list_del(cur);
                free(entry->value);
                free(entry);
            }
        } else if (deleted) {
            // Last element is a duplicate
            list_del(cur);
            free(entry->value);
            free(entry);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || head->next->next == head)
        return;
    struct list_head *cur = head->next;
    while (cur != head && cur->next != head) {
        struct list_head *first = cur;
        struct list_head *second = cur->next;
        // Swap first and second
        first->next = second->next;
        second->prev = first->prev;
        first->prev->next = second;
        second->next->prev = first;
        second->next = first;
        first->prev = second;
        // Move cur to the next pair
        cur = first->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || head->next->next == head)
        return;
    struct list_head *cur = head;
    do {
        struct list_head *temp = cur->next;
        cur->next = cur->prev;
        cur->prev = temp;
        cur = temp;
    } while (cur != head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || head->next->next == head || k < 2)
        return;

    int len = q_size(head);
    if (len < k)
        return;

    struct list_head *cur, *safe, *tmp_head = head, rse;
    INIT_LIST_HEAD(&rse);
    int count = 0;
    list_for_each_safe(cur, safe, head) {
        count++;
        if (count == k) {
            count = 0;
            list_cut_position(&rse, tmp_head, cur);
            q_reverse(&rse);
            list_splice(&rse, tmp_head);
            tmp_head = safe->prev;
        }
    }
}

static struct list_head *merge(struct list_head *left,
                               struct list_head *right,
                               bool descend)
{
    struct list_head dummy;
    struct list_head *tail = &dummy;
    dummy.next = dummy.prev = NULL;

    while (left && right) {
        const element_t *left_entry = list_entry(left, element_t, list);
        const element_t *right_entry = list_entry(right, element_t, list);
        int cmp_result = strcmp(left_entry->value, right_entry->value);
        bool cmp = descend ? (cmp_result > 0) : (cmp_result < 0);

        if (cmp || cmp_result == 0) {  // Pick left if equal for stability
            tail->next = left;
            left->prev = tail;
            left = left->next;
        } else {
            tail->next = right;
            right->prev = tail;
            right = right->next;
        }
        tail = tail->next;
    }
    tail->next = left ? left : right;
    if (tail->next)
        tail->next->prev = tail;
    return dummy.next;
}

static struct list_head *merge_sort(struct list_head *head,
                                    int len,
                                    bool descend)
{
    if (len < 2)
        return head;
    int mid = len / 2;
    struct list_head *left = head, *right = head;
    for (int i = 0; i < mid; i++)
        right = right->next;

    struct list_head *left_tail = right->prev;
    if (left_tail)
        left_tail->next = NULL;  // Break the link
    right->prev = NULL;

    left = merge_sort(left, mid, descend);
    right = merge_sort(right, len - mid, descend);
    return merge(left, right, descend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || head->next->next == head)
        return;

    int len = 0;
    struct list_head *cur;
    list_for_each(cur, head)
        len++;

    struct list_head *list = head->next;
    head->prev->next = NULL;  // Break the circular link
    list->prev = NULL;

    list = merge_sort(list, len, descend);

    head->next = list;
    struct list_head *prev = head;

    while (list) {
        list->prev = prev;
        prev = list;
        list = list->next;
    }
    prev->next = head;
    head->prev = prev;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;

    struct list_head *max = head->prev, *node = head->prev->prev;
    for (; node != head;) {
        const char *s1 = list_entry(max, element_t, list)->value;
        const char *s2 = list_entry(node, element_t, list)->value;
        if (strcmp(s1, s2) >= 0) {
            max = node;
            node = node->prev;
        } else {
            list_del(max->prev);
            q_release_element(list_entry(node, element_t, list));
            node = max->prev;
        }
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;

    if (list_is_singular(head))
        return 1;

    struct list_head *min = head->prev, *node = head->prev->prev;
    for (; node != head;) {
        const char *s1 = list_entry(min, element_t, list)->value;
        const char *s2 = list_entry(node, element_t, list)->value;
        if (strcmp(s1, s2) <= 0) {
            min = node;
            node = node->prev;
        } else {
            list_del(min->prev);
            q_release_element(list_entry(node, element_t, list));
            node = min->prev;
        }
    }
    return q_size(head);
}


/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return list_entry(head, queue_contex_t, chain)->size;

    queue_contex_t *cur, *next;
    int total_size = 0;
    LIST_HEAD(tmp);
    list_for_each_entry_safe(cur, next, head, chain) {
        total_size += cur->size;
        list_splice_init(cur->q, &tmp);
        cur->size = 0;
    }
    if (list_empty(&tmp))
        return 0;

    q_sort(&tmp, descend);

    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    list_splice_init(&tmp, first->q);
    first->size = total_size;
    return total_size;
}
