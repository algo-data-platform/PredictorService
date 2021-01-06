#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from libcpp.string cimport string
from libcpp cimport bool as cbool
from cpython cimport bool as pbool
from libc.stdint cimport int8_t, int16_t, int32_t, int64_t
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.vector cimport vector
from libcpp.set cimport set as cset
from libcpp.map cimport map as cmap, pair as cpair
from thrift.py3.exceptions cimport cTException
cimport folly.iobuf as __iobuf
cimport thrift.py3.exceptions
cimport thrift.py3.types
from folly.optional cimport cOptional
cimport includes.types as _includes_types




cdef extern from "gen-cpp2/module_types_custom_protocol.h" namespace "cpp2":
    # Forward Declaration
    cdef cppclass cMyStruct "cpp2::MyStruct"

cdef extern from "gen-cpp2/module_types.h" namespace "cpp2":
    cdef cppclass cMyStruct__isset "cpp2::MyStruct::__isset":
        bint MyIncludedField
        bint MyOtherIncludedField
        bint MyIncludedInt

    cdef cppclass cMyStruct "cpp2::MyStruct":
        cMyStruct() except +
        cMyStruct(const cMyStruct&) except +
        bint operator==(cMyStruct&)
        _includes_types.cIncluded MyIncludedField
        _includes_types.cIncluded MyOtherIncludedField
        int64_t MyIncludedInt
        cMyStruct__isset __isset


cdef extern from "<utility>" namespace "std" nogil:
    cdef shared_ptr[cMyStruct] move(unique_ptr[cMyStruct])
    cdef shared_ptr[cMyStruct] move_shared "std::move"(shared_ptr[cMyStruct])
    cdef unique_ptr[cMyStruct] move_unique "std::move"(unique_ptr[cMyStruct])

cdef extern from "<memory>" namespace "std" nogil:
    cdef shared_ptr[const cMyStruct] const_pointer_cast "std::const_pointer_cast<const cpp2::MyStruct>"(shared_ptr[cMyStruct])

# Forward Definition of the cython struct
cdef class MyStruct(thrift.py3.types.Struct)

cdef class MyStruct(thrift.py3.types.Struct):
    cdef object __hash
    cdef object __weakref__
    cdef shared_ptr[cMyStruct] _cpp_obj
    cdef _includes_types.Included __MyIncludedField
    cdef _includes_types.Included __MyOtherIncludedField

    @staticmethod
    cdef unique_ptr[cMyStruct] _make_instance(
        cMyStruct* base_instance,
        object MyIncludedField,
        object MyOtherIncludedField,
        object MyIncludedInt
    ) except *

    @staticmethod
    cdef create(shared_ptr[cMyStruct])




