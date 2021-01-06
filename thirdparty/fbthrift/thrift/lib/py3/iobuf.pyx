from folly.executor cimport get_executor
from cpython cimport Py_buffer
from weakref import WeakValueDictionary
from cython.view cimport memoryview

__cache = WeakValueDictionary()
__all__ = ['IOBuf']


cdef unique_ptr[cIOBuf] from_python_buffer(memoryview view):
    """Take a python object that supports buffer protocol"""
    return move(
        iobuf_from_python(
            get_executor(),
            <PyObject*>view,
            view.view.buf,
            view.shape[0],
        )
    )

cdef IOBuf from_unique_ptr(unique_ptr[cIOBuf] ciobuf):
    inst = <IOBuf>IOBuf.__new__(IOBuf)
    inst._ours = move(ciobuf)
    inst._parent = inst
    inst._this = inst._ours.get()
    __cache[(<unsigned long>inst._this, id(inst))] = inst
    return inst


cdef class IOBuf:
    def __init__(self, buffer not None):
        cdef memoryview view = memoryview(buffer, PyBUF_C_CONTIGUOUS)
        self._ours = move(from_python_buffer(view))
        self._this = self._ours.get()
        self._parent = self
        self._hash = None
        __cache[(<unsigned long>self._this, id(self))] = self

    @staticmethod
    cdef IOBuf create(cIOBuf* this, object parent):
        key = (<unsigned long>this, id(parent))
        cdef IOBuf inst = __cache.get(key)
        if inst is None:
            inst = <IOBuf>IOBuf.__new__(IOBuf)
            inst._this = this
            inst._parent = parent
            __cache[key] = inst
        return inst

    cdef unique_ptr[cIOBuf] c_clone(self):
        return move(self._this.clone())

    def clone(self):
        """ Clone the iobuf chain """
        return from_unique_ptr(self._this.clone())

    @property
    def next(self):
        _next = self._this.next()
        if _next == self._this:
            return None

        return IOBuf.create(_next, self._parent)

    @property
    def prev(self):
        _prev = self._this.prev()
        if _prev == self._this:
            return None

        return IOBuf.create(_prev, self._parent)

    @property
    def is_chained(self):
        return self._this.isChained()

    def chain_size(self):
        return self._this.computeChainDataLength()

    def chain_count(self):
        return self._this.countChainElements()

    def __bytes__(self):
        return <bytes>self._this.data()[:self._this.length()]

    def __bool__(self):
        return not self._this.empty()

    def __getbuffer__(self, Py_buffer *buffer, int flags):
        self.shape[0] = self._this.length()
        self.strides[0] = 1

        buffer.buf = <void *>self._this.data()
        buffer.format = NULL
        buffer.internal = NULL
        buffer.itemsize = 1
        buffer.len = self.shape[0]
        buffer.ndim = 1
        buffer.obj = self
        buffer.readonly = 1
        buffer.shape = self.shape
        buffer.strides = self.strides
        buffer.suboffsets = NULL

    def __releasebuffer__(self, Py_buffer *buffer):
        # Read-only means we need no logic here
        pass

    def __len__(self):
        return self._this.length()

    def __iter__(self):
        "Iterates through the chain of buffers returning a memory view for each"
        yield memoryview(self, PyBUF_C_CONTIGUOUS)
        next = self.next
        while next is not None and next != self:
            yield memoryview(next, PyBUF_C_CONTIGUOUS)
            next = next.next

    def __hash__(self):
        if not self._hash:
            self._hash = hash(b''.join(self))
        return self._hash
