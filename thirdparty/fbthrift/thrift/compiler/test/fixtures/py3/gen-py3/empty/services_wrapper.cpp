/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#include <src/gen-py3/empty/services_wrapper.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace cpp2 {

NullServiceWrapper::NullServiceWrapper(PyObject *obj, folly::Executor* exc)
  : if_object(obj), executor(exc)
  {
    Py_XINCREF(this->if_object);
  }

NullServiceWrapper::~NullServiceWrapper() {
    Py_XDECREF(this->if_object);
}

std::shared_ptr<apache::thrift::ServerInterface> NullServiceInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<NullServiceWrapper>(if_object, exc);
}
} // namespace cpp2
