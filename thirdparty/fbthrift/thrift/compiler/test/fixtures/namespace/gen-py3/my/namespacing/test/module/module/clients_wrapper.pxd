#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#

from cpython.ref cimport PyObject
from libc.stdint cimport int8_t, int16_t, int32_t, int64_t
from libcpp cimport bool as cbool
from libcpp.map cimport map as cmap, pair as cpair
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.set cimport set as cset
from libcpp.string cimport string
from libcpp.vector cimport vector

from folly cimport cFollyFuture, cFollyTry, cFollyUnit
from thrift.py3.common cimport cRpcOptions

cimport my.namespacing.test.module.module.types as _my_namespacing_test_module_module_types


cdef extern from "src/gen-cpp2/TestService.h" namespace "cpp2":
  cdef cppclass cTestServiceAsyncClient "cpp2::TestServiceAsyncClient":
      pass

cdef extern from "<utility>" namespace "std":
  cdef unique_ptr[cTestServiceClientWrapper] move(unique_ptr[cTestServiceClientWrapper])

cdef extern from "src/gen-py3/module/clients_wrapper.h" namespace "cpp2":
  cdef cppclass cTestServiceClientWrapper "cpp2::TestServiceClientWrapper":
    cFollyFuture[cFollyUnit] disconnect()
    void setPersistentHeader(const string& key, const string& value)

    cFollyFuture[int64_t] init(cRpcOptions, 
      int64_t arg_int1,)

