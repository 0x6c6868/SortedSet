#include "Python.h"
#include "skiplist.c"

typedef struct {
  PyObject_HEAD PyDictObject *_dict;
  SkipList *_list;
} SortedSetObject;

static void sort_set_dealloc(SortedSetObject *self) {
  Py_DECREF(self->_dict);
  SkipList_Free(self->_list);
}

static int sorted_set_init(PyObject *self, PyObject *args, PyObject *kwds) {
  int ret = 0;

  ((SortedSetObject *)self)->_dict = (PyDictObject *)PyDict_New();
  ((SortedSetObject *)self)->_list = SkipList_New();

  return ret;
}

static PyObject *sorted_set_add(SortedSetObject *self, PyObject *args) {

  PyObject *key, *score;

  key = PyTuple_GET_ITEM(args, 0);
  score = PyTuple_GET_ITEM(args, 1);

  int status = PyDict_SetItem((PyObject *)self->_dict, key, score);
  if (status < 0) {
    goto Fail;
  }

  SkipList_Insert(self->_list, PyFloat_AsDouble(score), key);
  Py_RETURN_NONE;

Fail:
  return NULL;
}

static PyObject *sorted_set_rem(SortedSetObject *self, PyObject *key) {

  PyObject *score = PyDict_GetItem((PyObject *)self->_dict, key);
  SkipList_Delete(self->_list, PyFloat_AsDouble(score), key);

  int status = PyDict_DelItem((PyObject *)self->_dict, key);
  if (status < 0) {
    goto Fail;
  }

  Py_RETURN_NONE;

Fail:
  return NULL;
}

static PyObject *sorted_set_get_score(SortedSetObject *self, PyObject *key) {

  return PyDict_GetItem((PyObject *)self->_dict, key);
}

static PyObject *sorted_set_get_rank(SortedSetObject *self, PyObject *key) {

  PyObject *score = PyDict_GetItem((PyObject *)self->_dict, key);

  unsigned long rank =
      SkipList_GetRank(self->_list, PyFloat_AsDouble(score), key);

  return PyLong_FromUnsignedLong(rank);
}

static PyMethodDef sorted_set_methods[] = {
    {"add", (PyCFunction)sorted_set_add, METH_VARARGS, ""},
    {"rem", (PyCFunction)sorted_set_rem, METH_O, ""},
    {"get_score", (PyCFunction)sorted_set_get_score, METH_O, ""},
    {"get_rank", (PyCFunction)sorted_set_get_rank, METH_O, ""},
    {NULL, NULL} /* sentinel */
};

static PyTypeObject SortedSet_TYPE = {
    PyVarObject_HEAD_INIT(NULL, 0) "sorted_set.SortedSet",
    sizeof(SortedSetObject),
    0,
    (destructor)sort_set_dealloc, /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_reserved */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    0,                            /* tp_doc */
    0,                            /* tp_traverse */
    0,                            /* tp_clear */
    0,                            /* tp_richcompare */
    0,                            /* tp_weaklistoffset */
    0,                            /* tp_iter */
    0,                            /* tp_iternext */
    sorted_set_methods,           /* tp_methods */
    0,                            /* tp_members */
    0,                            /* tp_getset */
    0,                            /* tp_base */
    0,                            /* tp_dict */
    0,                            /* tp_descr_get */
    0,                            /* tp_descr_set */
    0,                            /* tp_dictoffset */
    sorted_set_init,              /* tp_init */
    PyType_GenericAlloc,          /* tp_alloc */
    PyType_GenericNew,            /* tp_new */
    PyObject_Del,                 /* tp_free */
};

static PyModuleDef sorted_set_module = {
    PyModuleDef_HEAD_INIT, "sorted_set", "", -1, NULL, NULL, NULL, NULL, NULL};

PyMODINIT_FUNC PyInit_sorted_set(void) {

  PyObject *m;

  if (PyType_Ready(&SortedSet_TYPE) < 0)
    return NULL;

  m = PyModule_Create(&sorted_set_module);
  if (m == NULL)
    return NULL;

  Py_INCREF(&SortedSet_TYPE);
  PyModule_AddObject(m, "SortedSet", (PyObject *)&SortedSet_TYPE);

  return m;
}
