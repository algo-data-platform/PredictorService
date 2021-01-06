include "module.thrift"
include "includes.thrift"
namespace java.swift test.fixtures.includes

/** This is a service-level docblock */
service MyService {
  /** This is a function-level docblock */
  void query(1: module.MyStruct s, 2: includes.Included i)
  void has_arg_docs(1: /** arg doc*/ module.MyStruct s, /** arg doc */ 2: includes.Included i)
}

// Ensure that we can alias included types into the main namespace.
typedef includes.Included IncludesIncluded
// Typedef-of-typedef, cross-module.
typedef includes.TransitiveFoo IncludesTransitiveFoo
