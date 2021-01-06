#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from libcpp.memory cimport shared_ptr, make_shared, unique_ptr, make_unique
from libcpp.string cimport string
from libcpp cimport bool as cbool
from libcpp.iterator cimport inserter as cinserter
from cpython cimport bool as pbool
from libc.stdint cimport int8_t, int16_t, int32_t, int64_t, uint32_t
from cython.operator cimport dereference as deref, preincrement as inc, address as ptr_address
import thrift.py3.types
cimport thrift.py3.types
cimport thrift.py3.exceptions
from thrift.py3.types import NOTSET as __NOTSET
from thrift.py3.types cimport translate_cpp_enum_to_python
cimport thrift.py3.std_libcpp as std_libcpp
from thrift.py3.serializer import Protocol
cimport thrift.py3.serializer as serializer
from thrift.py3.serializer import deserialize, serialize
import folly.iobuf as __iobuf
from folly.optional cimport cOptional

import sys
import itertools
from collections import Sequence, Set, Mapping, Iterable
import enum as __enum
import warnings
import builtins as _builtins


class AnEnum(__enum.Enum):
    FIELDA = 2
    FIELDB = 4

    __hash__ = __enum.Enum.__hash__

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            warnings.warn(f"comparison not supported between instances of {type(self)} and {type(other)}", RuntimeWarning, stacklevel=2)
            return False
        return self.value == other.value

    def __int__(self):
        return self.value

cdef inline cAnEnum AnEnum_to_cpp(value):
    cdef int cvalue = value.value
    if cvalue == 2:
        return AnEnum__FIELDA
    elif cvalue == 4:
        return AnEnum__FIELDB


cdef cAStruct _AStruct_defaults = cAStruct()

cdef class AStruct(thrift.py3.types.Struct):

    def __init__(
        AStruct self, *,
        FieldA=None
    ):
        if FieldA is not None:
            if not isinstance(FieldA, int):
                raise TypeError(f'FieldA is not a { int !r}.')
            FieldA = <int32_t> FieldA

        self._cpp_obj = move(AStruct._make_instance(
          NULL,
          FieldA,
        ))

    def __call__(
        AStruct self,
        FieldA=__NOTSET
    ):
        changes = any((
            FieldA is not __NOTSET,
        ))

        if not changes:
            return self

        if None is not FieldA is not __NOTSET:
            if not isinstance(FieldA, int):
                raise TypeError(f'FieldA is not a { int !r}.')
            FieldA = <int32_t> FieldA

        inst = <AStruct>AStruct.__new__(AStruct)
        inst._cpp_obj = move(AStruct._make_instance(
          self._cpp_obj.get(),
          FieldA,
        ))
        return inst

    @staticmethod
    cdef unique_ptr[cAStruct] _make_instance(
        cAStruct* base_instance,
        object FieldA
    ) except *:
        cdef unique_ptr[cAStruct] c_inst
        if base_instance:
            c_inst = make_unique[cAStruct](deref(base_instance))
        else:
            c_inst = make_unique[cAStruct]()

        if base_instance:
            # Convert None's to default value. (or unset)
            if FieldA is None:
                deref(c_inst).FieldA = _AStruct_defaults.FieldA
                deref(c_inst).__isset.FieldA = False
                pass
            elif FieldA is __NOTSET:
                FieldA = None

        if FieldA is not None:
            deref(c_inst).FieldA = FieldA
            deref(c_inst).__isset.FieldA = True
        # in C++ you don't have to call move(), but this doesn't translate
        # into a C++ return statement, so you do here
        return move_unique(c_inst)

    def __iter__(self):
        yield 'FieldA', self.FieldA

    def __bool__(self):
        return True

    @staticmethod
    cdef create(shared_ptr[cAStruct] cpp_obj):
        inst = <AStruct>AStruct.__new__(AStruct)
        inst._cpp_obj = cpp_obj
        return inst

    @property
    def FieldA(self):

        return self._cpp_obj.get().FieldA


    def __hash__(AStruct self):
        if not self.__hash:
            self.__hash = hash((
            self.FieldA,
            ))
        return self.__hash

    def __repr__(AStruct self):
        return f'AStruct(FieldA={repr(self.FieldA)})'
    def __richcmp__(self, other, op):
        cdef int cop = op
        if cop not in (2, 3):
            raise TypeError("unorderable types: {}, {}".format(self, other))
        if not (
                isinstance(self, AStruct) and
                isinstance(other, AStruct)):
            if cop == 2:  # different types are never equal
                return False
            else:         # different types are always notequal
                return True

        cdef cAStruct cself = deref((<AStruct>self)._cpp_obj)
        cdef cAStruct cother = deref((<AStruct>other)._cpp_obj)
        cdef cbool cmp = cself == cother
        if cop == 2:
            return cmp
        return not cmp

    cdef __iobuf.IOBuf _serialize(AStruct self, proto):
        cdef __iobuf.cIOBufQueue queue = __iobuf.cIOBufQueue(__iobuf.cacheChainLength())
        cdef cAStruct* cpp_obj = self._cpp_obj.get()
        if proto is Protocol.COMPACT:
            with nogil:
                serializer.CompactSerialize[cAStruct](deref(cpp_obj), &queue, serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.BINARY:
            with nogil:
                serializer.BinarySerialize[cAStruct](deref(cpp_obj), &queue, serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.JSON:
            with nogil:
                serializer.JSONSerialize[cAStruct](deref(cpp_obj), &queue, serializer.SHARE_EXTERNAL_BUFFER)
        return __iobuf.from_unique_ptr(queue.move())

    cdef uint32_t _deserialize(AStruct self, const __iobuf.cIOBuf* buf, proto) except? 0:
        cdef uint32_t needed
        self._cpp_obj = make_shared[cAStruct]()
        cdef cAStruct* cpp_obj = self._cpp_obj.get()
        if proto is Protocol.COMPACT:
            with nogil:
                needed = serializer.CompactDeserialize[cAStruct](buf, deref(cpp_obj), serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.BINARY:
            with nogil:
                needed = serializer.BinaryDeserialize[cAStruct](buf, deref(cpp_obj), serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.JSON:
            with nogil:
                needed = serializer.JSONDeserialize[cAStruct](buf, deref(cpp_obj), serializer.SHARE_EXTERNAL_BUFFER)
        return needed

    def __reduce__(self):
        return (deserialize, (AStruct, serialize(self)))


cdef cAStructB _AStructB_defaults = cAStructB()

cdef class AStructB(thrift.py3.types.Struct):

    def __init__(
        AStructB self, *,
        AStruct FieldA=None
    ):
        self._cpp_obj = move(AStructB._make_instance(
          NULL,
          FieldA,
        ))

    def __call__(
        AStructB self,
        FieldA=__NOTSET
    ):
        changes = any((
            FieldA is not __NOTSET,
        ))

        if not changes:
            return self

        if None is not FieldA is not __NOTSET:
            if not isinstance(FieldA, AStruct):
                raise TypeError(f'FieldA is not a { AStruct !r}.')

        inst = <AStructB>AStructB.__new__(AStructB)
        inst._cpp_obj = move(AStructB._make_instance(
          self._cpp_obj.get(),
          FieldA,
        ))
        return inst

    @staticmethod
    cdef unique_ptr[cAStructB] _make_instance(
        cAStructB* base_instance,
        object FieldA
    ) except *:
        cdef unique_ptr[cAStructB] c_inst
        if base_instance:
            c_inst = make_unique[cAStructB](deref(base_instance))
        else:
            c_inst = make_unique[cAStructB]()

        if base_instance:
            # Convert None's to default value. (or unset)
            if FieldA is None:
                deref(c_inst).FieldA.reset()
                pass
            elif FieldA is __NOTSET:
                FieldA = None

        if FieldA is not None:
            deref(c_inst).FieldA = const_pointer_cast((<AStruct?>FieldA)._cpp_obj)
        # in C++ you don't have to call move(), but this doesn't translate
        # into a C++ return statement, so you do here
        return move_unique(c_inst)

    def __iter__(self):
        yield 'FieldA', self.FieldA

    def __bool__(self):
        return <bint>(deref(self._cpp_obj).FieldA)

    @staticmethod
    cdef create(shared_ptr[cAStructB] cpp_obj):
        inst = <AStructB>AStructB.__new__(AStructB)
        inst._cpp_obj = cpp_obj
        return inst

    @property
    def FieldA(self):

        if self.__FieldA is None:
            if not deref(self._cpp_obj).FieldA:
                return None
            self.__FieldA = AStruct.create(aliasing_constructor_FieldA(self._cpp_obj, <cAStruct*>(deref(self._cpp_obj).FieldA.get())))
        return self.__FieldA


    def __hash__(AStructB self):
        if not self.__hash:
            self.__hash = hash((
            self.FieldA,
            ))
        return self.__hash

    def __repr__(AStructB self):
        return f'AStructB(FieldA={repr(self.FieldA)})'
    def __richcmp__(self, other, op):
        cdef int cop = op
        if cop not in (2, 3):
            raise TypeError("unorderable types: {}, {}".format(self, other))
        if not (
                isinstance(self, AStructB) and
                isinstance(other, AStructB)):
            if cop == 2:  # different types are never equal
                return False
            else:         # different types are always notequal
                return True

        cdef cAStructB cself = deref((<AStructB>self)._cpp_obj)
        cdef cAStructB cother = deref((<AStructB>other)._cpp_obj)
        cdef cbool cmp = cself == cother
        if cop == 2:
            return cmp
        return not cmp

    cdef __iobuf.IOBuf _serialize(AStructB self, proto):
        cdef __iobuf.cIOBufQueue queue = __iobuf.cIOBufQueue(__iobuf.cacheChainLength())
        cdef cAStructB* cpp_obj = self._cpp_obj.get()
        if proto is Protocol.COMPACT:
            with nogil:
                serializer.CompactSerialize[cAStructB](deref(cpp_obj), &queue, serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.BINARY:
            with nogil:
                serializer.BinarySerialize[cAStructB](deref(cpp_obj), &queue, serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.JSON:
            with nogil:
                serializer.JSONSerialize[cAStructB](deref(cpp_obj), &queue, serializer.SHARE_EXTERNAL_BUFFER)
        return __iobuf.from_unique_ptr(queue.move())

    cdef uint32_t _deserialize(AStructB self, const __iobuf.cIOBuf* buf, proto) except? 0:
        cdef uint32_t needed
        self._cpp_obj = make_shared[cAStructB]()
        cdef cAStructB* cpp_obj = self._cpp_obj.get()
        if proto is Protocol.COMPACT:
            with nogil:
                needed = serializer.CompactDeserialize[cAStructB](buf, deref(cpp_obj), serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.BINARY:
            with nogil:
                needed = serializer.BinaryDeserialize[cAStructB](buf, deref(cpp_obj), serializer.SHARE_EXTERNAL_BUFFER)
        elif proto is Protocol.JSON:
            with nogil:
                needed = serializer.JSONDeserialize[cAStructB](buf, deref(cpp_obj), serializer.SHARE_EXTERNAL_BUFFER)
        return needed

    def __reduce__(self):
        return (deserialize, (AStructB, serialize(self)))


IncludedConstant = 42
IncludedInt64 = int
