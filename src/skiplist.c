#include "Python.h"

#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P 0.25

typedef struct SkipListNode {
  PyObject *obj;
  double score;
  struct SkipListNode *backward;
  struct SkipListLevel {
    struct SkipListNode *forward;
    unsigned int span;
  } level[];
} SkipListNode;

typedef struct SkipList {
  struct SkipListNode *header, *tail;
  unsigned long length;
  int level;
} SkipList;

SkipListNode *SkipList_CreateNode(int level, double score, PyObject *obj) {
  SkipListNode *sln = (SkipListNode *)PyMem_Malloc(
      sizeof(*sln) + level * (sizeof(struct SkipListLevel)));
  sln->score = score;
  sln->obj = obj;

  return sln;
}

SkipList *SkipList_New(void) {
  SkipList *sl;

  sl = (SkipList *)PyMem_Malloc(sizeof(*sl));
  sl->level = 1;
  sl->length = 0;

  sl->header = SkipList_CreateNode(ZSKIPLIST_MAXLEVEL, 0, NULL);
  for (int i = 0; i < ZSKIPLIST_MAXLEVEL; i++) {
    sl->header->level[i].forward = NULL;
    sl->header->level[i].span = 0;
  }
  sl->header->backward = NULL;

  sl->tail = NULL;

  return sl;
}

void SkipList_FreeNode(SkipListNode *sln) { PyMem_Free(sln); }

void SkipList_Free(SkipList *sl) {
  SkipListNode *node = sl->header->level[0].forward, *next;

  SkipList_FreeNode(sl->header);
  while (node) {
    next = node->level[0].forward;
    SkipList_FreeNode(node);
    node = next;
  }

  return PyMem_Free(sl);
}

int SkipList_RandomNodeLevel(void) {
  int level = 1;
  while ((random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
    level += 1;
  return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

SkipListNode *SkipList_Insert(SkipList *sl, double score, PyObject *obj) {

  SkipListNode *update[ZSKIPLIST_MAXLEVEL];
  unsigned int rank[ZSKIPLIST_MAXLEVEL];

  SkipListNode *x = sl->header;
  for (int i = sl->level - 1; i >= 0; i--) {
    rank[i] = i == (sl->level - 1) ? 0 : rank[i + 1];
    while (x->level[i].forward && (x->level[i].forward->score < score)) {
      rank[i] += x->level[i].span;
      x = x->level[i].forward;
    }
    update[i] = x;
  }

  int level = SkipList_RandomNodeLevel();
  if (level > sl->level) {
    for (int i = sl->level; i < level; i++) {
      rank[i] = 0;
      update[i] = sl->header;
      update[i]->level[i].span = sl->length;
    }
    sl->level = level;
  }

  x = SkipList_CreateNode(level, score, obj);
  for (int i = 0; i < level; i++) {
    x->level[i].forward = update[i]->level[i].forward;
    update[i]->level[i].forward = x;

    x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
    update[i]->level[i].span = (rank[0] - rank[i]) + 1;
  }

  for (int i = level; i < sl->level; i++) {
    update[i]->level[i].span++;
  }

  x->backward = (update[0] == sl->header) ? NULL : update[0];
  if (x->level[0].forward)
    x->level[0].forward->backward = x;
  else
    sl->tail = x;
  sl->length++;

  return x;
}

void SkipList_DeleteNode(SkipList *sl, SkipListNode *x, SkipListNode **update) {
  for (int i = 0; i < sl->level; i++) {
    if (update[i]->level[i].forward == x) {
      update[i]->level[i].span += x->level[i].span - 1;
      update[i]->level[i].forward = x->level[i].forward;
    } else {
      update[i]->level[i].span -= 1;
    }
  }
  if (x->level[0].forward) {
    x->level[0].forward->backward = x->backward;
  } else {
    sl->tail = x->backward;
  }
  while (sl->level > 1 && sl->header->level[sl->level - 1].forward == NULL)
    sl->level--;
  sl->length--;
}

int SkipList_Delete(SkipList *sl, double score, PyObject *obj) {
  SkipListNode *update[ZSKIPLIST_MAXLEVEL];

  SkipListNode *x = sl->header;
  for (int i = sl->level - 1; i >= 0; i--) {
    while (x->level[i].forward && (x->level[i].forward->score < score)) {
      x = x->level[i].forward;
    }
    update[i] = x;
  }

  x = x->level[0].forward;
  if (x && score == x->score &&
      (PyObject_RichCompareBool(x->obj, obj, Py_EQ) > 0)) {
    SkipList_DeleteNode(sl, x, update);
    SkipList_FreeNode(x);
    return 1;
  }
  return 0;
}

unsigned long SkipList_GetRank(SkipList *sl, double score, PyObject *obj) {
  unsigned long rank = 0;
  SkipListNode *x = sl->header;
  for (int i = sl->level - 1; i >= 0; i--) {
    while (x->level[i].forward && (x->level[i].forward->score < score)) {
      rank += x->level[i].span;
      x = x->level[i].forward;
    }
  }

  if (x->obj && (x == sl->header || x == sl->tail))
    return 0;

  return rank;
}
