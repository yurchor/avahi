#include "prioq.h"

flxPrioQueue* flx_prio_queue_new(gint (*compare) (gconstpointer a, gconstpointer b)) {
    flxPrioQueue *q;
    g_assert(compare);

    q = g_new(flxPrioQueue, 1);
    q->root = q->last = NULL;
    q->n_nodes = 0;
    q->compare = compare;
    return q;
}

void flx_prio_queue_free(flxPrioQueue *q) {
    g_assert(q);

    while (q->last)
        flx_prio_queue_remove(q, q->last);

    g_assert(!q->n_nodes);
    g_free(q);
}

static flxPrioQueueNode* get_node_at_xy(flxPrioQueue *q, guint x, guint y) {
    guint r;
    flxPrioQueueNode *n;
    g_assert(q);

    n = q->root;
    g_assert(n);

    for (r = 0; r < y; r++) {
        g_assert(n);
        
        if ((x >> (y-r-1)) & 1)
            n = n->right;
        else
            n = n->left;
    }

    g_assert(n->x == x);
    g_assert(n->y == y);

    return n;
}

static void exchange_nodes(flxPrioQueue *q, flxPrioQueueNode *a, flxPrioQueueNode *b) {
    flxPrioQueueNode *l, *r, *p, *ap, *an, *bp, *bn;
    gint t;
    g_assert(q);
    g_assert(a);
    g_assert(b);
    g_assert(a != b);

    /* Swap positions */
    t = a->x; a->x = b->x; b->x = t;
    t = a->y; a->y = b->y; b->y = t;

    if (a->parent == b) {
        /* B is parent of A */
        
        p = b->parent;
        b->parent = a;

        if ((a->parent = p)) {
            if (a->parent->left == b)
                a->parent->left = a;
            else
                a->parent->right = a;
        } else
            q->root = a;

        if (b->left == a) {
            if ((b->left = a->left))
                b->left->parent = b;
            a->left = b;

            r = a->right;
            if ((a->right = b->right))
                a->right->parent = a;
            if ((b->right = r))
                b->right->parent = b;
            
        } else {
            if ((b->right = a->right))
                b->right->parent = b;
            a->right = b;

            l = a->left;
            if ((a->left = b->left))
                a->left->parent = a;
            if ((b->left = l))
                b->left->parent = b;
        }
    } else if (b->parent == a) {
        /* A ist parent of B */
        
        p = a->parent;
        a->parent = b;

        if ((b->parent = p)) {
            if (b->parent->left == a)
                b->parent->left = b;
            else
                b->parent->right = b;
        } else
            q->root = b;

        if (a->left == b) {
            if ((a->left = b->left))
                a->left->parent = a;
            b->left = a;

            r = a->right;
            if ((a->right = b->right))
                a->right->parent = a;
            if ((b->right = r))
                b->right->parent = b;
        } else {
            if ((a->right = b->right))
                a->right->parent = a;
            b->right = a;

            l = a->left;
            if ((a->left = b->left))
                a->left->parent = a;
            if ((b->left = l))
                b->left->parent = b;
        }
    } else {
        /* Swap parents */
        p = a->parent;
        
        if ((a->parent = b->parent)) {
            if (a->parent->left == b)
                a->parent->left = a;
            else
                a->parent->right = a;
        } else
            q->root = a;
                
        if ((b->parent = p)) {
            if (b->parent->left == a)
                b->parent->left = b;
            else
                b->parent->right = b;
        } else
            q->root = b;

        /* Swap children */
        l = a->left; 
        r = a->right; 

        if ((a->left = b->left))
            a->left->parent = a;

        if ((b->left = l))
            b->left->parent = b;

        if ((a->right = b->right))
            a->right->parent = a;

        if ((b->right = r))
            b->right->parent = b;
    }
    
    /* Swap siblings */
    ap = a->prev; an = a->next;
    bp = b->prev; bn = b->next;

    if (a->next == b) {
        /* A is predecessor of B */
        a->prev = b;
        b->next = a;

        if ((a->next = bn))
            a->next->prev = a;
        else
            q->last = a;

        if ((b->prev = ap))
            b->prev->next = b;
        
    } else if (b->next == a) {
        /* B is predecessor of A */
        a->next = b;
        b->prev = a;

        if ((a->prev = bp))
            a->prev->next = a;

        if ((b->next = an))
            b->next->prev = b;
        else
            q->last = b;

    } else {
        /* A is no neighbour of B */

        if ((a->prev = bp))
            a->prev->next = a;
        
        if ((a->next = bn))
            a->next->prev = a;
        else
            q->last = a;
        
        if ((b->prev = ap))
            b->prev->next = b;
        
        if ((b->next = an))
            b->next->prev = b;
        else
            q->last = b;
    }
}

/* Move a node to the correct position */
void flx_prio_queue_shuffle(flxPrioQueue *q, flxPrioQueueNode *n) {
    g_assert(q);
    g_assert(n);

    /* Move up until the position is OK */
    while (n->parent && q->compare(n->parent->data, n->data) > 0)
        exchange_nodes(q, n, n->parent);

    /* Move down until the position is OK */
    for (;;) {
        flxPrioQueueNode *min;

        if (!(min = n->left)) {
            /* No children */
            g_assert(!n->right);
            break;
        }

        if (n->right && q->compare(n->right->data, min->data) < 0)
            min = n->right;

        /* min now contains the smaller one of our two children */

        if (q->compare(n->data, min->data) <= 0)
            /* Order OK */
            break;

        exchange_nodes(q, n, min);
    }
}

flxPrioQueueNode* flx_prio_queue_put(flxPrioQueue *q, gpointer data) {
    flxPrioQueueNode *n;
    g_assert(q);

    n = g_new(flxPrioQueueNode, 1);
    n->queue = q;
    n->data = data;

    if (q->last) {
        g_assert(q->root);
        g_assert(q->n_nodes);
        
        n->y = q->last->y;
        n->x = q->last->x+1;

        if (n->x >= ((guint) 1 << n->y)) {
            n->x = 0;
            n->y++;
        }

        q->last->next = n;
        n->prev = q->last;
        
        g_assert(n->y > 0);
        n->parent = get_node_at_xy(q, n->x/2, n->y-1);

        if (n->x & 1)
            n->parent->right = n;
        else
            n->parent->left = n;
    } else {
        g_assert(!q->root);
        g_assert(!q->n_nodes);
        
        n->y = n->x = 0;
        q->root = n;
        n->prev = n->parent = NULL;
    }

    n->next = n->left = n->right = NULL;
    q->last = n;
    q->n_nodes++;

    flx_prio_queue_shuffle(q, n);

    return n;
}

void flx_prio_queue_remove(flxPrioQueue *q, flxPrioQueueNode *n) {
    g_assert(q);
    g_assert(n);

    if (n != q->last) {
        flxPrioQueueNode *replacement = q->last;
        exchange_nodes(q, replacement, n);
        flx_prio_queue_remove(q, q->last);
        flx_prio_queue_shuffle(q, replacement);
        return;
    }

    g_assert(n == q->last);
    g_assert(!n->next);
    g_assert(!n->left);
    g_assert(!n->right);

    q->last = n->prev;
    
    if (n->prev)
        n->prev->next = NULL;
    else
        g_assert(!n->parent);

    if (n->parent) {
        g_assert(n->prev);
        if (n->parent->left == n) {
            g_assert(n->parent->right == NULL);
            n->parent->left = NULL;
        } else {
            g_assert(n->parent->right == n);
            g_assert(n->parent->left != NULL);
            n->parent->right = NULL;
        }
    } else {
        g_assert(q->root == n);
        g_assert(!n->prev);
        q->root = NULL;
    }

    q->n_nodes--;
    
    g_free(n);
}
