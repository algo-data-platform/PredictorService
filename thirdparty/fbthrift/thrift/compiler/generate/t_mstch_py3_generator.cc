/*
 * Copyright 2016-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <memory>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace {

// Reserved Cython / Python keywords that are not blocked by thrift grammer
static const std::unordered_set<string> KEYWORDS = {
    "async",
    "await",
    "cdef",
    "cimport",
    "cpdef",
    "cppclass",
    "ctypedef",
    "from",
    "nonlocal",
    "DEF",
    "ELIF",
    "ELSE",
    "False",
    "IF",
    "None",
    "True",
};

class t_mstch_py3_generator : public t_mstch_generator {
 public:
  enum class ModuleType {
    TYPES,
    CLIENTS,
    SERVICES,
  };

  t_mstch_py3_generator(
      t_program* program,
      const std::map<std::string, std::string>& parsed_options,
      const std::string& /* option_string unused */)
      : t_mstch_generator(program, "py3", parsed_options) {
    out_dir_base_ = "gen-py3";
    auto include_prefix = get_option("include_prefix");
    if (include_prefix && !include_prefix->empty()) {
      program->set_include_prefix(*include_prefix);
    }
  }

  void generate_program() override;
  mstch::map extend_program(const t_program&) override;
  mstch::map extend_field(const t_field&) override;
  mstch::map extend_type(const t_type&) override;
  mstch::map extend_service(const t_service&) override;
  mstch::map extend_enum(const t_enum&) override;
  mstch::map extend_annotation(const annotation&) override;
  mstch::map extend_enum_value(const t_enum_value&) override;

 protected:
  bool should_resolve_typedefs() const override {
    return true;
  }

  void generate_init_files(const t_program&);
  boost::filesystem::path package_to_path(std::string package);
  mstch::array get_return_types(const t_program&);
  void add_per_type_data(const t_program&, mstch::map&);
  void add_cpp_includes(const t_program&, mstch::map&);
  mstch::array get_cpp2_namespace(const t_program&);
  mstch::array get_py3_namespace(
      const t_program&,
      std::initializer_list<string> tails = {});
  std::string flatten_type_name(const t_type&);
  std::string get_module_name(ModuleType module);
  template <class T>
  std::string get_rename(const T&);
  void generate_module(const t_program&, ModuleType moduleType);

 private:
  const std::vector<std::string> extensions{".pyx", ".pxd", ".pyi"};
  struct type_data {
    vector<const t_type*> containers;
    std::set<string> container_names;
    vector<const t_type*> custom_templates;
    std::set<string> custom_template_names;
    vector<const t_type*> custom_types;
    std::set<string> custom_type_names;
    mstch::array extra_namespaces;
    std::set<string> extra_namespace_paths;
  };
  void visit_type(t_type* type, type_data& data);
  void visit_single_type(
      const t_type& type,
      const t_type& orig_type,
      type_data& data);
  string ref_type(const t_field& field) const;
  string get_cpp_template(const t_type& type) const;
  string to_cython_template(const string& cpp_template) const;
  bool is_default_template(const string& cpp_template, const t_type& type)
      const;
  string get_cpp_type(const t_type& type) const;
  string to_cython_type(const string& cpp_type) const;
  bool is_external_program(const t_program& program) const;
  inline const t_program& get_type_program(const t_type& type);
};

bool t_mstch_py3_generator::is_external_program(
    const t_program& program) const {
  return program.get_path() != get_program()->get_path();
}

inline const t_program& t_mstch_py3_generator::get_type_program(
    const t_type& type) {
  auto type_program = type.get_program();
  return type_program ? *type_program : *get_program();
}

mstch::map t_mstch_py3_generator::extend_program(const t_program& program) {
  const auto& cppNamespaces = get_cpp2_namespace(program);
  const auto& py3Namespaces = get_py3_namespace(program);
  const auto& svcs = program.get_services();
  const auto hasServiceFunctions =
      std::any_of(svcs.begin(), svcs.end(), [](auto svc) {
        return !svc->get_functions().empty();
      });

  mstch::array includeNamespaces;
  for (const auto included_program : program.get_includes()) {
    if (included_program->get_path() == program.get_path()) {
      continue;
    }
    const auto ns =
        get_py3_namespace(*included_program, {included_program->get_name()});
    auto const hasServices = included_program->get_services().size() > 0;
    auto const hasStructs = included_program->get_objects().size() > 0;
    auto const hasEnums = included_program->get_enums().size() > 0;
    auto const hasTypeDefs = included_program->get_typedefs().size() > 0;
    auto const hasConstants = included_program->get_consts().size() > 0;
    auto const hasTypes = hasStructs || hasEnums || hasTypeDefs || hasConstants;

    const mstch::map include_ns{
        {"includeNamespace", ns},
        {"hasServices?", hasServices},
        {"hasTypes?", hasTypes},
    };
    includeNamespaces.push_back(include_ns);
  }

  bool optionals_setting = cache_->parsed_options_.count("optionals") != 0;
  mstch::map result{
      {"returnTypes", get_return_types(program)},
      {"cppNamespaces", cppNamespaces},
      {"py3Namespaces", py3Namespaces},
      {"hasServiceFunctions?", hasServiceFunctions},
      {"includeNamespaces", includeNamespaces},
      {"optionals?", optionals_setting},
  };
  add_cpp_includes(program, result);
  add_per_type_data(program, result);
  return result;
}

template <class T>
std::string t_mstch_py3_generator::get_rename(const T& elem) {
  auto& annotation = elem.annotations_;
  auto it = annotation.find("py3.rename");
  if (KEYWORDS.find(elem.get_name()) != KEYWORDS.end()) {
    if (it != annotation.end()) {
      return it->second;
    }
    return elem.get_name() + "_";
  }
  return elem.get_name();
}

mstch::map t_mstch_py3_generator::extend_field(const t_field& field) {
  const auto ref_type = this->ref_type(field);
  bool reference = ref_type != "";
  bool is_iobuf = false;
  bool is_iobuf_ref = false;

  if (field.get_type() != nullptr) {
    auto& resolved_type = resolve_typedef(*field.get_type());
    string type_override = this->get_cpp_type(resolved_type);
    if (type_override == "std::unique_ptr<folly::IOBuf>") {
      reference = true;
      is_iobuf_ref = true;
    }
    if (type_override == "folly::IOBuf") {
      is_iobuf = true;
    }
  }

  auto req = field.get_req();
  const auto required = req == t_field::e_req::T_REQUIRED;
  const auto optional = req == t_field::e_req::T_OPTIONAL;
  const auto unqualified = !required && !optional;
  const auto hasValue = field.get_value() != nullptr;
  const auto flag_optionals = cache_->parsed_options_.count("optionals") != 0;
  const auto follyOptional = optional && flag_optionals;
  const auto hasDefaultValue =
      !follyOptional && !reference && (hasValue || unqualified);
  const auto requireValue = required && !hasDefaultValue;
  const auto isset = !flag_optionals && !reference && !required;
  // For typing, can a property getter return None, if so it needs to Optional[]
  const auto isPEP484Optional =
      ((!hasDefaultValue && !required) || follyOptional);
  const auto nameToUse = get_rename(field);

  mstch::map result{
      {"reference?", reference},
      {"unique_ref?", (ref_type == "unique")},
      {"shared_ref?", (ref_type == "shared")},
      {"shared_const_ref?", (ref_type == "shared_const")},
      {"iobuf?", is_iobuf},
      {"iobuf_ref?", is_iobuf_ref},
      {"hasDefaultValue?", hasDefaultValue},
      {"requireValue?", requireValue},
      {"follyOptional?", follyOptional},
      {"PEP484Optional?", isPEP484Optional},
      {"isset?", isset},
      // We replace the previously-set name on the field with the modified
      // name, and put the raw value in origName
      {"name", nameToUse},
      {"origName", field.get_name()},
      {"hasModifiedName?", (field.get_name() != nameToUse)},
  };
  return result;
}

// TODO: This needs to mirror the behavior of t_cpp_generator::cpp_ref_type
// but it's not obvious how to get there
string t_mstch_py3_generator::ref_type(const t_field& field) const {
  auto& annotations = field.annotations_;

  // backward compatibility with 'ref' annotation
  if (annotations.count("cpp.ref") != 0 || annotations.count("cpp2.ref") != 0) {
    return "unique";
  }

  auto it = annotations.find("cpp.ref_type");
  if (it == annotations.end()) {
    it = annotations.find("cpp2.ref_type");
  }

  if (it == annotations.end()) {
    return "";
  }

  auto& reftype = it->second;

  if (reftype == "unique" || reftype == "std::unique_ptr") {
    return "unique";
  } else if (reftype == "shared" || reftype == "std::shared_ptr") {
    return "shared";
  } else if (reftype == "shared_const") {
    return "shared_const";
  } else {
    // It is legal to get here but hopefully nobody will in practice, since
    // we're not set up to handle other kinds of refs:
    std::ostringstream err;
    err << "Unhandled ref_type " << reftype;
    throw std::runtime_error{err.str()};
  }
}

mstch::map t_mstch_py3_generator::extend_type(const t_type& type) {
  const auto& program = get_type_program(type);
  const auto modulePath =
      get_py3_namespace(program, {program.get_name(), "types"});
  const auto& cppNamespaces = get_cpp2_namespace(program);
  const auto externalProgram = is_external_program(program);

  string cpp_template = this->get_cpp_template(type);
  string cython_template = this->to_cython_template(cpp_template);
  bool is_default_template = this->is_default_template(cpp_template, type);

  string cpp_type = this->get_cpp_type(type);
  bool has_custom_type = (cpp_type != "");
  string cython_type = this->to_cython_type(cpp_type);
  const auto is_integer =
      type.is_byte() || type.is_i16() || type.is_i32() || type.is_i64();
  const auto is_number = is_integer || type.is_floating_point();
  // We don't use the Cython Type for Containers, and enums are Python Only
  const auto hasCythonType = !type.is_container() && !type.is_enum();
  const auto cythonTypeNoneable = !is_number && hasCythonType;

  bool is_iobuf =
      (cpp_type == "folly::IOBuf" ||
       cpp_type == "std::unique_ptr<folly::IOBuf>");

  mstch::map result{
      {"modulePath", modulePath},
      {"externalProgram?", externalProgram},
      {"flat_name", flatten_type_name(type)},
      {"cppNamespaces", cppNamespaces},
      {"cppTemplate", cpp_template},
      {"cythonTemplate", cython_template},
      {"defaultTemplate?", is_default_template},
      {"cppCustomType", cpp_type},
      {"cythonCustomType", cython_type},
      {"hasCustomType?", has_custom_type},
      {"number?", is_number},
      {"integer?", is_integer},
      {"cythonTypeNoneable?", cythonTypeNoneable},
      {"hasCythonType?", hasCythonType},
      {"iobuf_wrapper?", is_iobuf},
  };
  return result;
}

mstch::map t_mstch_py3_generator::extend_enum(const t_enum& enm) {
  const auto is_flags = enm.annotations_.count("py3.flags") != 0;
  mstch::map result{
      {"flags?", is_flags},
  };
  return result;
}

mstch::map t_mstch_py3_generator::extend_enum_value(const t_enum_value& val) {
  mstch::map result{
      // We replace the previously-set name on the enum value with the modified
      // name, and put the raw value in origName
      {"name", get_rename(val)},
      {"origName", val.get_name()},
  };
  return result;
}

// This handles is_unordered as a special case
string t_mstch_py3_generator::get_cpp_template(const t_type& type) const {
  auto& annotations = type.annotations_;

  auto it = annotations.find("cpp.template");
  if (it == annotations.end()) {
    it = annotations.find("cpp2.template");
  }

  if (it != annotations.end()) {
    return it->second;
  } else if (type.is_list()) {
    return "std::vector";
  } else if (type.is_set()) {
    bool unordered = dynamic_cast<const t_set&>(type).is_unordered();
    return unordered ? "std::unordered_set" : "std::set";
  } else if (type.is_map()) {
    bool unordered = dynamic_cast<const t_map&>(type).is_unordered();
    return unordered ? "std::unordered_map" : "std::map";
  } else {
    return "";
  }
}

string t_mstch_py3_generator::to_cython_template(
    const string& cpp_template) const {
  // handle special built-ins first:
  if (cpp_template == "std::vector") {
    return "vector";
  } else if (cpp_template == "std::set") {
    return "cset";
  } else if (cpp_template == "std::map") {
    return "cmap";
  }

  // then default handling:
  string cython_template = cpp_template;
  boost::algorithm::replace_all(cython_template, "::", "_");
  return cython_template;
}

bool t_mstch_py3_generator::is_default_template(
    const string& cpp_template,
    const t_type& type) const {
  return (!type.is_container() && cpp_template == "") ||
      (type.is_list() && cpp_template == "std::vector") ||
      (type.is_set() && cpp_template == "std::set") ||
      (type.is_map() && cpp_template == "std::map");
}

string t_mstch_py3_generator::get_cpp_type(const t_type& type) const {
  auto& annotations = type.annotations_;

  auto it = annotations.find("cpp.type");
  if (it == annotations.end()) {
    it = annotations.find("cpp2.type");
  }

  if (it != annotations.end()) {
    return it->second;
  } else {
    return "";
  }
}

string strip_comments(const string& str) {
  string s = str;
  while (true) {
    size_t comment_idx = s.find("/*");
    if (comment_idx == string::npos) {
      return s;
    }

    size_t end_comment_idx = s.find("*/", comment_idx);
    if (end_comment_idx != string::npos) {
      end_comment_idx += 2;
    }

    s = s.substr(0, comment_idx) + s.substr(end_comment_idx);
  }
}

string t_mstch_py3_generator::to_cython_type(const string& cpp_type) const {
  if (cpp_type == "") {
    return "";
  }

  string cython_type = cpp_type;
  cython_type = strip_comments(cython_type);
  boost::algorithm::replace_all(cython_type, "::", "_");
  boost::algorithm::replace_all(cython_type, "<", "_");
  boost::algorithm::replace_all(cython_type, ">", "");
  boost::algorithm::replace_all(cython_type, " ", "");
  boost::algorithm::replace_all(cython_type, ", ", "_");
  boost::algorithm::replace_all(cython_type, ",", "_");
  return cython_type;
}

mstch::map t_mstch_py3_generator::extend_service(const t_service& service) {
  const auto& program = *service.get_program();
  const auto& cppNamespaces = get_cpp2_namespace(program);
  const auto& py3Namespaces = get_py3_namespace(program);
  string include_prefix = program.get_include_prefix();
  const auto externalProgram = is_external_program(program);
  mstch::map result{
      {"externalProgram?", externalProgram},
      {"cppNamespaces", cppNamespaces},
      {"py3Namespaces", py3Namespaces},
      {"programName", program.get_name()},
      {"includePrefix", include_prefix},
  };
  return result;
}

mstch::map t_mstch_py3_generator::extend_annotation(const annotation& pair) {
  mstch::map result{
      {"value?", !pair.second.empty()},
  };
  return result;
}

void t_mstch_py3_generator::generate_init_files(const t_program& program) {
  auto path = package_to_path(program.get_namespace("py3"));
  auto directory = boost::filesystem::path{};
  for (auto path_part : path) {
    directory /= path_part;
    render_to_file(
        program, "common/AutoGeneratedPy", directory / "__init__.py");
  }
}

std::string t_mstch_py3_generator::get_module_name(
    t_mstch_py3_generator::ModuleType module) {
  using ModuleType = ModuleType;
  switch (module) {
    case ModuleType::TYPES:
      return "types";
    case ModuleType::CLIENTS:
      return "clients";
    case ModuleType::SERVICES:
      return "services";
  }
  return nullptr; // This should never happen but it silences compiler warning
}

void t_mstch_py3_generator::generate_module(
    const t_program& program,
    t_mstch_py3_generator::ModuleType moduleType) {
  using ModuleType = ModuleType;
  if (moduleType != ModuleType::TYPES && program.get_services().empty()) {
    // There is no need to generate empty / broken code for non existent
    // services.
    return;
  }
  mstch::map extra_context{
      {"program:typeContext?", moduleType == ModuleType::TYPES},
  };

  auto path = package_to_path(program.get_namespace("py3"));
  auto name = program.get_name();
  auto module = get_module_name(moduleType);

  for (auto ext : extensions) {
    render_to_file(
        program, extra_context, module + ext, path / name / (module + ext));
  }
  if (moduleType != ModuleType::TYPES) {
    auto basename = module + "_wrapper";
    auto cpp_path = boost::filesystem::path{name};
    for (auto ext : {".h", ".cpp"}) {
      render_to_file(
          program, extra_context, basename + ext, cpp_path / (basename + ext));
    }

    render_to_file(
        program,
        extra_context,
        basename + ".pxd",
        path / name / (basename + ".pxd"));
  }
}

boost::filesystem::path t_mstch_py3_generator::package_to_path(
    std::string package) {
  boost::algorithm::replace_all(package, ".", "/");
  return boost::filesystem::path{package};
}

mstch::array t_mstch_py3_generator::get_return_types(const t_program& program) {
  mstch::array distinct_return_types;
  std::set<string> visited_names;

  for (const auto service : program.get_services()) {
    for (const auto function : service->get_functions()) {
      const auto returntype = function->get_returntype();
      string flat_name = flatten_type_name(*returntype);
      if (!visited_names.count(flat_name)) {
        distinct_return_types.push_back(dump(*returntype));
        visited_names.insert(flat_name);
      }
    }
  }
  return distinct_return_types;
}

/*
 * Add two items to the results map, one "containerTypes" that lists all
 * container types, and one "moveContainerTypes" that treats binary and string
 * as one type. Required because in pxd's we can't have duplicate move(string)
 * definitions */
void t_mstch_py3_generator::add_per_type_data(
    const t_program& program,
    mstch::map& results) {
  type_data data;

  // Put in all the directly-referenced paths, since we don't need to repeat
  // them in extras
  data.extra_namespace_paths.insert(program.get_path());
  for (const auto included_program : program.get_includes()) {
    data.extra_namespace_paths.insert(included_program->get_path());
  }

  for (const auto service : program.get_services()) {
    for (const auto function : service->get_functions()) {
      for (const auto field : function->get_arglist()->get_members()) {
        auto arg_type = field->get_type();
        visit_type(arg_type, data);
      }
      auto return_type = function->get_returntype();
      visit_type(return_type, data);
    }
  }
  for (const auto object : program.get_objects()) {
    for (const auto field : object->get_members()) {
      auto ref_type = field->get_type();
      visit_type(ref_type, data);
    }
  }
  for (const auto constant : program.get_consts()) {
    const auto const_type = constant->get_type();
    visit_type(const_type, data);
  }
  for (const auto typedef_def : program.get_typedefs()) {
    const auto typedef_type = typedef_def->get_type();
    visit_type(typedef_type, data);
  }

  results.emplace("containerTypes", dump_elems(data.containers));
  results.emplace("customTemplates", dump_elems(data.custom_templates));
  results.emplace("customTypes", dump_elems(data.custom_types));
  // extra_namespaces is already a mstch::array, so we don't need to dump it:
  results.emplace("extraNamespaces", data.extra_namespaces);

  // create second set of container types that treats strings and binaries
  // the same
  vector<const t_type*> move_containers;
  std::set<string> visited_names;

  for (const auto type : data.containers) {
    auto flat_name = flatten_type_name(*type);
    boost::algorithm::replace_all(flat_name, "binary", "string");

    if (visited_names.count(flat_name)) {
      continue;
    }
    visited_names.insert(flat_name);
    move_containers.push_back(type);
  }
  results.emplace("moveContainerTypes", dump_elems(move_containers));
}

void t_mstch_py3_generator::add_cpp_includes(
    const t_program& program,
    mstch::map& results) {
  mstch::array a{};
  for (auto const& include : program.get_cpp_includes()) {
    mstch::map cpp_include;
    cpp_include.emplace(
        "system?", include.at(0) == '<' ? std::to_string(0) : "");
    cpp_include.emplace("path", std::string(include));
    a.push_back(cpp_include);
  }
  results.emplace("cppIncludes", a);
}

void t_mstch_py3_generator::visit_type(t_type* orig_type, type_data& data) {
  auto type = &resolve_typedef(*orig_type);

  if (type->is_list()) {
    const auto elem_type = dynamic_cast<const t_list*>(type)->get_elem_type();
    visit_type(elem_type, data);
  } else if (type->is_set()) {
    const auto elem_type = dynamic_cast<const t_set*>(type)->get_elem_type();
    visit_type(elem_type, data);
  } else if (type->is_map()) {
    const auto map_type = dynamic_cast<const t_map*>(type);
    const auto key_type = map_type->get_key_type();
    const auto value_type = map_type->get_val_type();
    visit_type(key_type, data);
    visit_type(value_type, data);
  }

  visit_single_type(*type, *orig_type, data);
}

void t_mstch_py3_generator::visit_single_type(
    const t_type& type,
    const t_type& orig_type,
    type_data& data) {
  if (type.is_container()) {
    string flat_name = flatten_type_name(type);
    if (!data.container_names.count(flat_name)) {
      data.container_names.insert(flat_name);
      data.containers.push_back(&type);
    }
  }

  string cpp_template = this->get_cpp_template(type);
  if (!this->is_default_template(cpp_template, type) &&
      !data.custom_template_names.count(cpp_template)) {
    data.custom_template_names.insert(cpp_template);
    data.custom_templates.push_back(&type);
  }

  string cpp_type = this->get_cpp_type(type);
  if (cpp_type != "" && !data.custom_type_names.count(cpp_type)) {
    data.custom_type_names.insert(cpp_type);
    data.custom_types.push_back(&type);
  }

  // If the original type is a typedef, then add the namespace of the
  // *resolved* type:
  if (orig_type.is_typedef()) {
    auto prog = type.get_program();
    if (prog != nullptr) {
      auto path = prog->get_path();
      if (!data.extra_namespace_paths.count(path)) {
        const auto ns = get_py3_namespace(*prog, {prog->get_name()});
        data.extra_namespace_paths.insert(path);
        const mstch::map extra_ns{
            {"extraNamespace", ns},
        };
        data.extra_namespaces.push_back(extra_ns);
      }
    }
  }
}

std::string t_mstch_py3_generator::flatten_type_name(const t_type& orig_type) {
  auto& type = resolve_typedef(orig_type);
  const auto& program = get_type_program(type);
  const auto externalProgram = is_external_program(program);

  string cpp_template = this->get_cpp_template(type);
  string custom_prefix = "";
  if (!this->is_default_template(cpp_template, type)) {
    custom_prefix = this->to_cython_template(cpp_template) + "__";
  } else {
    string cpp_type = this->get_cpp_type(type);
    if (cpp_type != "") {
      custom_prefix = this->to_cython_type(cpp_type) + "__";
    }
  }

  if (type.is_list()) {
    return custom_prefix + "List__" +
        flatten_type_name(*dynamic_cast<const t_list&>(type).get_elem_type());
  } else if (type.is_set()) {
    return custom_prefix + "Set__" +
        flatten_type_name(*dynamic_cast<const t_set&>(type).get_elem_type());
  } else if (type.is_map()) {
    return (
        custom_prefix + "Map__" +
        flatten_type_name(*dynamic_cast<const t_map&>(type).get_key_type()) +
        "_" +
        flatten_type_name(*dynamic_cast<const t_map&>(type).get_val_type()));
  } else if (type.is_binary()) {
    return custom_prefix + "binary";
  } else if (externalProgram) {
    return custom_prefix + program.get_name() + '_' + type.get_name();
  } else {
    return custom_prefix + type.get_name();
  }
}

mstch::array t_mstch_py3_generator::get_cpp2_namespace(
    const t_program& program) {
  auto cpp_namespace = program.get_namespace("cpp2");
  if (cpp_namespace == "") {
    cpp_namespace = program.get_namespace("cpp");
    if (cpp_namespace == "") {
      cpp_namespace = "cpp2";
    } else {
      cpp_namespace = cpp_namespace + ".cpp2";
    }
  }
  vector<string> ns = split_namespace(cpp_namespace);
  return dump_elems(ns);
}

mstch::array t_mstch_py3_generator::get_py3_namespace(
    const t_program& program,
    std::initializer_list<string> tails) {
  const auto& py3_namespace = program.get_namespace("py3");
  vector<string> ns = split_namespace(py3_namespace);
  for (auto tail : tails) {
    ns.push_back(tail);
  }
  return dump_elems(ns);
}

void t_mstch_py3_generator::generate_program() {
  using ModuleType = ModuleType;
  mstch::config::escape = [](const std::string& s) { return s; };
  generate_init_files(*get_program());
  generate_module(*get_program(), ModuleType::TYPES);
  generate_module(*get_program(), ModuleType::SERVICES);
  generate_module(*get_program(), ModuleType::CLIENTS);
}

THRIFT_REGISTER_GENERATOR(
    mstch_py3,
    "Python 3",
    "    include_prefix:  Use full include paths in generated files.\n");
} // namespace
