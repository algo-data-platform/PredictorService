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

cimport module.types as _module_types


cdef extern from "src/gen-cpp2/SimpleService.h" namespace "py3::simple":
  cdef cppclass cSimpleServiceAsyncClient "py3::simple::SimpleServiceAsyncClient":
      pass

cdef extern from "<utility>" namespace "std":
  cdef unique_ptr[cSimpleServiceClientWrapper] move(unique_ptr[cSimpleServiceClientWrapper])

cdef extern from "src/gen-cpp2/DerivedService.h" namespace "py3::simple":
  cdef cppclass cDerivedServiceAsyncClient "py3::simple::DerivedServiceAsyncClient":
      pass

cdef extern from "<utility>" namespace "std":
  cdef unique_ptr[cDerivedServiceClientWrapper] move(unique_ptr[cDerivedServiceClientWrapper])

cdef extern from "src/gen-cpp2/RederivedService.h" namespace "py3::simple":
  cdef cppclass cRederivedServiceAsyncClient "py3::simple::RederivedServiceAsyncClient":
      pass

cdef extern from "<utility>" namespace "std":
  cdef unique_ptr[cRederivedServiceClientWrapper] move(unique_ptr[cRederivedServiceClientWrapper])

cdef extern from "src/gen-py3/module/clients_wrapper.h" namespace "py3::simple":
  cdef cppclass cSimpleServiceClientWrapper "py3::simple::SimpleServiceClientWrapper":
    cFollyFuture[cFollyUnit] disconnect()
    void setPersistentHeader(const string& key, const string& value)

    cFollyFuture[int32_t] get_five(cRpcOptions, )
    cFollyFuture[int32_t] add_five(cRpcOptions, 
      int32_t arg_num,)
    cFollyFuture[cFollyUnit] do_nothing(cRpcOptions, )
    cFollyFuture[string] concat(cRpcOptions, 
      string arg_first,
      string arg_second,)
    cFollyFuture[int32_t] get_value(cRpcOptions, 
      _module_types.cSimpleStruct arg_simple_struct,)
    cFollyFuture[cbool] negate(cRpcOptions, 
      cbool arg_input,)
    cFollyFuture[int8_t] tiny(cRpcOptions, 
      int8_t arg_input,)
    cFollyFuture[int16_t] small(cRpcOptions, 
      int16_t arg_input,)
    cFollyFuture[int64_t] big(cRpcOptions, 
      int64_t arg_input,)
    cFollyFuture[double] two(cRpcOptions, 
      double arg_input,)
    cFollyFuture[cFollyUnit] expected_exception(cRpcOptions, )
    cFollyFuture[int32_t] unexpected_exception(cRpcOptions, )
    cFollyFuture[int32_t] sum_i16_list(cRpcOptions, 
      vector[int16_t] arg_numbers,)
    cFollyFuture[int32_t] sum_i32_list(cRpcOptions, 
      vector[int32_t] arg_numbers,)
    cFollyFuture[int32_t] sum_i64_list(cRpcOptions, 
      vector[int64_t] arg_numbers,)
    cFollyFuture[string] concat_many(cRpcOptions, 
      vector[string] arg_words,)
    cFollyFuture[int32_t] count_structs(cRpcOptions, 
      vector[_module_types.cSimpleStruct] arg_items,)
    cFollyFuture[int32_t] sum_set(cRpcOptions, 
      cset[int32_t] arg_numbers,)
    cFollyFuture[cbool] contains_word(cRpcOptions, 
      cset[string] arg_words,
      string arg_word,)
    cFollyFuture[string] get_map_value(cRpcOptions, 
      cmap[string,string] arg_words,
      string arg_key,)
    cFollyFuture[int16_t] map_length(cRpcOptions, 
      cmap[string,_module_types.cSimpleStruct] arg_items,)
    cFollyFuture[int16_t] sum_map_values(cRpcOptions, 
      cmap[string,int16_t] arg_items,)
    cFollyFuture[int32_t] complex_sum_i32(cRpcOptions, 
      _module_types.cComplexStruct arg_counter,)
    cFollyFuture[string] repeat_name(cRpcOptions, 
      _module_types.cComplexStruct arg_counter,)
    cFollyFuture[_module_types.cSimpleStruct] get_struct(cRpcOptions, )
    cFollyFuture[vector[int32_t]] fib(cRpcOptions, 
      int16_t arg_n,)
    cFollyFuture[cset[string]] unique_words(cRpcOptions, 
      vector[string] arg_words,)
    cFollyFuture[cmap[string,int16_t]] words_count(cRpcOptions, 
      vector[string] arg_words,)
    cFollyFuture[_module_types.cAnEnum] set_enum(cRpcOptions, 
      _module_types.cAnEnum arg_in_enum,)
    cFollyFuture[vector[vector[int32_t]]] list_of_lists(cRpcOptions, 
      int16_t arg_num_lists,
      int16_t arg_num_items,)
    cFollyFuture[cmap[string,cmap[string,int32_t]]] word_character_frequency(cRpcOptions, 
      string arg_sentence,)
    cFollyFuture[vector[cset[string]]] list_of_sets(cRpcOptions, 
      string arg_some_words,)
    cFollyFuture[int32_t] nested_map_argument(cRpcOptions, 
      cmap[string,vector[_module_types.cSimpleStruct]] arg_struct_map,)
    cFollyFuture[string] make_sentence(cRpcOptions, 
      vector[vector[string]] arg_word_chars,)
    cFollyFuture[cset[int32_t]] get_union(cRpcOptions, 
      vector[cset[int32_t]] arg_sets,)
    cFollyFuture[cset[string]] get_keys(cRpcOptions, 
      vector[cmap[string,string]] arg_string_map,)
    cFollyFuture[double] lookup_double(cRpcOptions, 
      int32_t arg_key,)
    cFollyFuture[string] retrieve_binary(cRpcOptions, 
      string arg_something,)
    cFollyFuture[cset[string]] contain_binary(cRpcOptions, 
      vector[string] arg_binaries,)
    cFollyFuture[vector[_module_types.cAnEnum]] contain_enum(cRpcOptions, 
      vector[_module_types.cAnEnum] arg_the_enum,)


  cdef cppclass cDerivedServiceClientWrapper "py3::simple::DerivedServiceClientWrapper"(cSimpleServiceClientWrapper):

    cFollyFuture[int32_t] get_six(cRpcOptions, )


  cdef cppclass cRederivedServiceClientWrapper "py3::simple::RederivedServiceClientWrapper"(cDerivedServiceClientWrapper):

    cFollyFuture[int32_t] get_seven(cRpcOptions, )
