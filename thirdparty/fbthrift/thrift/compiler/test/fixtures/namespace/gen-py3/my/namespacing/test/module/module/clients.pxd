#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#  @generated
#
from libcpp.memory cimport shared_ptr
cimport thrift.py3.client


from my.namespacing.test.module.module.clients_wrapper cimport cTestServiceClientWrapper

cdef class TestService(thrift.py3.client.Client):
    cdef shared_ptr[cTestServiceClientWrapper] _module_TestService_client

    @staticmethod
    cdef _module_TestService_set_client(TestService inst, shared_ptr[cTestServiceClientWrapper] c_obj)

    cdef _module_TestService_reset_client(TestService self)

