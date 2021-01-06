/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#include <src/gen-py3/module/services_wrapper.h>
#include <src/gen-py3/module/services_api.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>

namespace cpp2 {

MyServiceWrapper::MyServiceWrapper(PyObject *obj, folly::Executor* exc)
  : if_object(obj), executor(exc)
  {
    import_module__services();
    Py_XINCREF(this->if_object);
  }

MyServiceWrapper::~MyServiceWrapper() {
    Py_XDECREF(this->if_object);
}

folly::Future<folly::Unit> MyServiceWrapper::future_ping() {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyService_ping(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

folly::Future<std::unique_ptr<std::string>> MyServiceWrapper::future_getRandomData() {
  folly::Promise<std::unique_ptr<std::string>> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyService_getRandomData(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

folly::Future<bool> MyServiceWrapper::future_hasDataById(
  int64_t id
) {
  folly::Promise<bool> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id    ]() mutable {
        call_cy_MyService_hasDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id        );
    });

  return future;
}

folly::Future<std::unique_ptr<std::string>> MyServiceWrapper::future_getDataById(
  int64_t id
) {
  folly::Promise<std::unique_ptr<std::string>> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id    ]() mutable {
        call_cy_MyService_getDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id        );
    });

  return future;
}

folly::Future<folly::Unit> MyServiceWrapper::future_putDataById(
  int64_t id,
  std::unique_ptr<std::string> data
) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id,
data = std::move(data)    ]() mutable {
        call_cy_MyService_putDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id,
            std::move(data)        );
    });

  return future;
}

folly::Future<folly::Unit> MyServiceWrapper::future_lobDataById(
  int64_t id,
  std::unique_ptr<std::string> data
) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id,
data = std::move(data)    ]() mutable {
        call_cy_MyService_lobDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id,
            std::move(data)        );
    });

  return future;
}

std::shared_ptr<apache::thrift::ServerInterface> MyServiceInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<MyServiceWrapper>(if_object, exc);
}


MyServiceFastWrapper::MyServiceFastWrapper(PyObject *obj, folly::Executor* exc)
  : if_object(obj), executor(exc)
  {
    import_module__services();
    Py_XINCREF(this->if_object);
  }

MyServiceFastWrapper::~MyServiceFastWrapper() {
    Py_XDECREF(this->if_object);
}

folly::Future<folly::Unit> MyServiceFastWrapper::future_ping() {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyServiceFast_ping(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

folly::Future<std::unique_ptr<std::string>> MyServiceFastWrapper::future_getRandomData() {
  folly::Promise<std::unique_ptr<std::string>> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyServiceFast_getRandomData(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

folly::Future<bool> MyServiceFastWrapper::future_hasDataById(
  int64_t id
) {
  folly::Promise<bool> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id    ]() mutable {
        call_cy_MyServiceFast_hasDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id        );
    });

  return future;
}

folly::Future<std::unique_ptr<std::string>> MyServiceFastWrapper::future_getDataById(
  int64_t id
) {
  folly::Promise<std::unique_ptr<std::string>> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id    ]() mutable {
        call_cy_MyServiceFast_getDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id        );
    });

  return future;
}

folly::Future<folly::Unit> MyServiceFastWrapper::future_putDataById(
  int64_t id,
  std::unique_ptr<std::string> data
) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id,
data = std::move(data)    ]() mutable {
        call_cy_MyServiceFast_putDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id,
            std::move(data)        );
    });

  return future;
}

folly::Future<folly::Unit> MyServiceFastWrapper::future_lobDataById(
  int64_t id,
  std::unique_ptr<std::string> data
) {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise),
id,
data = std::move(data)    ]() mutable {
        call_cy_MyServiceFast_lobDataById(
            this->if_object,
            ctx,
            std::move(promise),
            id,
            std::move(data)        );
    });

  return future;
}

std::shared_ptr<apache::thrift::ServerInterface> MyServiceFastInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<MyServiceFastWrapper>(if_object, exc);
}


MyServiceEmptyWrapper::MyServiceEmptyWrapper(PyObject *obj, folly::Executor* exc)
  : if_object(obj), executor(exc)
  {
    import_module__services();
    Py_XINCREF(this->if_object);
  }

MyServiceEmptyWrapper::~MyServiceEmptyWrapper() {
    Py_XDECREF(this->if_object);
}

std::shared_ptr<apache::thrift::ServerInterface> MyServiceEmptyInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<MyServiceEmptyWrapper>(if_object, exc);
}


MyServicePrioParentWrapper::MyServicePrioParentWrapper(PyObject *obj, folly::Executor* exc)
  : if_object(obj), executor(exc)
  {
    import_module__services();
    Py_XINCREF(this->if_object);
  }

MyServicePrioParentWrapper::~MyServicePrioParentWrapper() {
    Py_XDECREF(this->if_object);
}

folly::Future<folly::Unit> MyServicePrioParentWrapper::future_ping() {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyServicePrioParent_ping(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

folly::Future<folly::Unit> MyServicePrioParentWrapper::future_pong() {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyServicePrioParent_pong(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

std::shared_ptr<apache::thrift::ServerInterface> MyServicePrioParentInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<MyServicePrioParentWrapper>(if_object, exc);
}


MyServicePrioChildWrapper::MyServicePrioChildWrapper(PyObject *obj, folly::Executor* exc)
  : cpp2::MyServicePrioParentWrapper(obj, exc)
  {
    import_module__services();
  }

folly::Future<folly::Unit> MyServicePrioChildWrapper::future_pang() {
  folly::Promise<folly::Unit> promise;
  auto future = promise.getFuture();
  auto ctx = getConnectionContext();
  folly::via(
    this->executor,
    [this, ctx,
     promise = std::move(promise)    ]() mutable {
        call_cy_MyServicePrioChild_pang(
            this->if_object,
            ctx,
            std::move(promise)        );
    });

  return future;
}

std::shared_ptr<apache::thrift::ServerInterface> MyServicePrioChildInterface(PyObject *if_object, folly::Executor *exc) {
  return std::make_shared<MyServicePrioChildWrapper>(if_object, exc);
}
} // namespace cpp2
