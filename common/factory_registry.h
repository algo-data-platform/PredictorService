#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "folly/SpinLock.h"
#include "folly/Singleton.h"

namespace common {

#if defined(__GNUC__)
#define COMMON_ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define COMMON_ATTRIBUTE_UNUSED
#endif

struct ParamFieldInfo {
  /*! \brief name of the field */
  std::string name;
  /*! \brief type of the field in string format */
  std::string type;
  /*!
   * \brief detailed type information string
   *  This include the default value, enum constran and typename.
   */
  std::string type_info_str;
  /*! \brief detailed description of the type */
  std::string description;
};

/*!
 * \brief FactoryRegistry class.
 *  FactoryRegistry can be used to register global singletons.
 *  The most commonly use case are factory functions.
 *
 * \tparam EntryType Type of FactoryRegistry entries,
 *     EntryType need to name a name field.
 */
template <typename EntryType>
class FactoryRegistry {
 public:
  /*! \return list all names registered in the registry, including alias */
  inline static std::vector<std::string> ListAllNames() {
    const std::unordered_map<std::string, std::shared_ptr<EntryType>> &fmap = Get()->fmap_;
    std::vector<std::string> names;

    folly::SpinLockGuard g(Get()->spinlock_);
    for (auto &v : fmap) {
      names.emplace_back(v.first);
    }
    return names;
  }
  /*!
   * \brief Find the entry with corresponding name.
   * \param name name of the function
   * \return the corresponding function, can be NULL
   */
  inline static const std::shared_ptr<EntryType> Find(const std::string &name) {
    const std::unordered_map<std::string, std::shared_ptr<EntryType>> &fmap = Get()->fmap_;

    folly::SpinLockGuard g(Get()->spinlock_);
    auto p = fmap.find(name);
    if (p != fmap.end()) {
      return p->second;
    } else {
      return nullptr;
    }
  }

  /*!
   * \brief Internal function to register a name function under name.
   * \param name name of the function
   * \return ref to the registered entry, used to set properties
   */
  inline EntryType &__REGISTER__(const std::string &name) {
    std::shared_ptr<EntryType> e = std::make_shared<EntryType>();
    e->name = name;
    {
      // when mutil-threads to registe,must be thread safe
      folly::SpinLockGuard g(spinlock_);
      fmap_[name] = e;
    }
    return *e;
  }
  /*!
   * \brief get a singleton of the FactoryRegistry.
   */
  static FactoryRegistry<EntryType> *Get();

 private:
  std::unordered_map<std::string, std::shared_ptr<EntryType>> fmap_;
  folly::SpinLock spinlock_;

  FactoryRegistry() = default;
  ~FactoryRegistry() = default;
  FactoryRegistry(const FactoryRegistry &) = delete;
  FactoryRegistry(FactoryRegistry &&) = delete;
  void operator=(const FactoryRegistry &) = delete;
  void operator=(FactoryRegistry &&) = delete;
};

/*!
 * \brief Common base class for function registry.
 *
 * \code
 *  // This example demonstrates how to use FactoryRegistry to create a factory of trees.
 *  struct TreeFactory :
 *      public FunctionRegEntryBase<TreeFactory, std::function<Tree*()> > {
 *  };
 *
 *  // in a independent cc file
 *  namespace dmlc {
 *  COMMON_REGISTRY_ENABLE(TreeFactory);
 *  }
 *  // register binary tree constructor into the registry.
 *  COMMON_REGISTRY_REGISTER(TreeFactory, TreeFactory, BinaryTree)
 *      .describe("Constructor of BinaryTree")
 *      .set_body([]() { return new BinaryTree(); });
 * \endcode
 *
 * \tparam EntryType The type of subclass that inheritate the base.
 * \tparam FunctionType The function type this registry is registerd.
 */
template <typename EntryType, typename FunctionType>
class FunctionRegEntryBase {
 public:
  /*! \brief name of the entry */
  std::string name;
  /*! \brief description of the entry */
  std::string description;
  /*! \brief additional arguments to the factory function */
  std::vector<ParamFieldInfo> arguments;
  /*! \brief Function body to create ProductType */
  FunctionType body;
  /*! \brief Return type of the function */
  std::string return_type;

  /*!
   * \brief Set the function body.
   * \param body Function body to set.
   * \return reference to self.
   */
  inline EntryType &set_body(FunctionType body) {
    this->body = body;
    return this->self();
  }
  /*!
   * \brief Describe the function.
   * \param description The description of the factory function.
   * \return reference to self.
   */
  inline EntryType &describe(const std::string &description) {
    this->description = description;
    return this->self();
  }
  /*!
   * \brief Add argument information to the function.
   * \param name Name of the argument.
   * \param type Type of the argument.
   * \param description Description of the argument.
   * \return reference to self.
   */
  inline EntryType &add_argument(const std::string &name, const std::string &type, const std::string &description) {
    ParamFieldInfo info;
    info.name = name;
    info.type = type;
    info.type_info_str = info.type;
    info.description = description;
    arguments.push_back(info);
    return this->self();
  }
  /*!
   * \brief Append list if arguments to the end.
   * \param args Additional list of arguments.
   * \return reference to self.
   */
  inline EntryType &add_arguments(const std::vector<ParamFieldInfo> &args) {
    arguments.insert(arguments.end(), args.begin(), args.end());
    return this->self();
  }
  /*!
  * \brief Set the return type.
  * \param type Return type of the function, could be Symbol or Symbol[]
  * \return reference to self.
  */
  inline EntryType &set_return_type(const std::string &type) {
    return_type = type;
    return this->self();
  }

 protected:
  /*!
   * \return reference of self as derived type
   */
  inline EntryType &self() { return *(static_cast<EntryType *>(this)); }
};

/*!
 * \def COMMON_REGISTRY_ENABLE
 * \brief Macro to enable the registry of EntryType.
 * This macro must be used under namespace dmlc, and only used once in cc file.
 * \param EntryType Type of registry entry
 */
#define COMMON_REGISTRY_ENABLE(EntryType)                                              \
  template <>                                                                          \
  ::common::FactoryRegistry<EntryType> * ::common::FactoryRegistry<EntryType>::Get() { \
    static common::FactoryRegistry<EntryType> inst;                                    \
    return &inst;                                                                      \
  }

/*!
 * \brief Generic macro to register an EntryType
 *  There is a complete example in FactoryRegistryEntryBase.
 *
 * \param EntryType The type of registry entry.
 * \param EntryTypeName The typename of EntryType, must do not contain namespace :: .
 * \param Name The name to be registered.
 * \sa FactoryRegistryEntryBase
 */
#define COMMON_REGISTRY_REGISTER(EntryType, EntryTypeName, Name)                  \
  static COMMON_ATTRIBUTE_UNUSED EntryType &__make_##EntryTypeName##_##Name##__ = \
      ::common::FactoryRegistry<EntryType>::Get()->__REGISTER__(#Name)

}  // namespace common

