#!/usr/bin/env php
<?php 
//
// 对于 java 来说可以使用 lombok 注解来轻易实现 getter / setter ，并且 gson 也支持通过注解来实现 json 序列化
// 由于 cpp 没有注解功能，所以我们需要借助一个工具来自动生成抽象的代码
// 所以实现了一个 entity 生成器，来减少编码工作量
// 该脚本主要是通过定义一组 .h .cc 的模板文件以及 .config 的配置文件，最终生成定义的 entity 类
// entity 类主要实现的接口：
// class ClassName {
//  public:
//   getter()
//   setter()
//   void describe(std::ostream& os) const;
//   const folly::dynamic serialize() const;
//   bool deserialize(const folly::dynamic& data);
// }
class GeneratorEntity {
  private $className;
  private $output;
  private $template;
  private $isDebug;
  private $members = [];
  private $data = [];

  private $options = [];
  private $enums = [];

  private $exists_class = [];
  private $exists_enum = [];

  private $annotation_option = [];
  private $podTypes = [
    'bool',
    'int',
    'uint8_t',
    'uint16_t',
    'uint32_t',
    'uint64_t',
    'int8_t',
    'int16_t',
    'int32_t',
    'int64_t',
    'double',
    'std::string',
  ];

// {{{ templates

  private $setter = <<<EOT
  void set__NAME__(__TYPE__ __INAME__) { __INAME___ = __INAME__; } 
EOT;
  private $setterConst = <<<EOT
  void set__NAME__(const __TYPE__& __INAME__) { __INAME___ = __INAME__; } 
EOT;
  private $getter = <<<EOT
  __TYPE__ get__NAME__() const { return __INAME___; } 
EOT;

  private $getterConst = <<<EOT
  const __TYPE__& get__NAME__() const { return __INAME___; } 
EOT;

  private $memInit = <<<EOT
  __TYPE__ __NAME__{__INIT__};
EOT;
  private $mem = <<<EOT
  __TYPE__ __NAME__;
EOT;

  private $class = <<<EOT

class __CLASS_NAME__ {
 public:
  __CLASS_NAME__() = default;
  ~__CLASS_NAME__() = default;

__FUNCS__

  void describe(std::ostream& os) const;

  const folly::dynamic serialize() const;

  bool deserialize(const folly::dynamic& data);

 private:
__MEMBERS__
};

std::ostream& operator<<(std::ostream& os, const __CLASS_NAME__& value);

EOT;

  private $enumclass = <<<EOT

enum class __CLASS_NAME__ {
  __ENUMS__
};

folly::dynamic serialize__CLASS_NAME__(const __CLASS_NAME__& value);

bool deserialize__CLASS_NAME__(const folly::dynamic& data, __CLASS_NAME__* value);

std::ostream& operator<<(std::ostream& os, const __CLASS_NAME__& value);

__STRING_DECLARE__
EOT;

  private $enumclass_string_declare = <<<EOT
folly::Optional<__CLASS_NAME__> stringTo__CLASS_NAME__(const std::string& name);

const std::string toString__CLASS_NAME__(const __CLASS_NAME__& value);

EOT;
  private $describeSource = <<<EOT

void __CLASS_NAME__::describe(std::ostream& os) const {
  os << "__CLASS_NAME__{"
     __ITEMS__
     << "}";
}

std::ostream& operator<<(std::ostream& os, const __CLASS_NAME__& value) {
  value.describe(os);
  return os;
}

const folly::dynamic __CLASS_NAME__::serialize() const {
  folly::dynamic result = folly::dynamic::object;

__SER_OBJECT_ITEMS__

  return result;
}

bool __CLASS_NAME__::deserialize(const folly::dynamic& data) {
  if (!data.isObject()) {
    return false;
  }

__UNSER_OBJECT_ITEMS__

  return true;
}

EOT;

  private $describeEnumSourceInt = <<<EOT

std::ostream& operator<<(std::ostream& os, const __CLASS_NAME__& value) {
  os << static_cast<int>(value);
  return os;
}

folly::dynamic serialize__CLASS_NAME__(const __CLASS_NAME__& value) {
  folly::dynamic result = static_cast<int>(value);
  return result;
}

bool deserialize__CLASS_NAME__(const folly::dynamic& data, __CLASS_NAME__* result) {
  if (!data.isInt()) {
    return false;
  }

  int value = data.asInt();
  switch(value) {
__INT_ENUM_UNSER_CASE__
    default:
      return false;
  }

  return true;
}

EOT;

  private $describeEnumSourceIntCaseDeserialize = <<<EOT
    case __ITEM_ID__:
      *result = __CLASS_NAME__::__ITEM__;
      break;
EOT;

  private $describeEnumSourceString = <<<EOT

std::ostream& operator<<(std::ostream& os, const __CLASS_NAME__& value) {
  switch(value) {
__CASE_ITMES__
    default:
      os << "type unknow";
  }
  return os;
}

folly::dynamic serialize__CLASS_NAME__(const __CLASS_NAME__& value) {
  folly::dynamic result = toString__CLASS_NAME__(value);
  return result;
}

bool deserialize__CLASS_NAME__(const folly::dynamic& data, __CLASS_NAME__* result) {
  if (!data.isString()) {
    return false;
  }

  std::string value = data.asString();
  auto enum_obj = stringTo__CLASS_NAME__(value);
  if (enum_obj) {
    *result = *enum_obj;    
    return true;
  }

  return false;
}

folly::Optional<__CLASS_NAME__> stringTo__CLASS_NAME__(const std::string& name) {
__STRING_ENUM_UNSER_CASE__
     
  return folly::none;
}

const std::string toString__CLASS_NAME__(const __CLASS_NAME__& value) {
  std::string result;
  switch(value) {
__STRING_ENUM_SER_CASE__
    default:
      result = "unknow";
  }

  return result;
}


EOT;

  private $describeEnumSourceStringCaseSerialize = <<<EOT
    case __CLASS_NAME__::__ITEM__:
      result = "__IITEM__";
      break;
EOT;

  private $describeEnumSourceStringCaseDeserialize = <<<EOT
  if (name == "__IITEM__") {
    return __CLASS_NAME__::__ITEM__;
  }
EOT;

  private $describeEnumSourceCase = <<<EOT
    case __CLASS_NAME__::__ITEM__:
      os << "__IITEM__";
      break;
EOT;

  private $itemQuote = <<<EOT
<< "__NAME__='" << __INAME___ << "'"
EOT;
  
  private $item = <<<EOT
<< "__NAME__=" << __INAME___
EOT;

  private $itemVector = <<<EOT
<< "__NAME__=[";
  for (auto& t : __INAME___) {
    os << t << ",";
  }
  os << "]"
EOT;

  private $itemMap = <<<EOT
<< "__NAME__={";
  for (auto& t : __INAME___) {
    os << t.first << "=" << t.second << ",";
  }
  os << "}"
EOT;

  private $serialize_item_pod = <<<EOT
  result.insert("__SR_NAME__", __INAME___);
EOT;

  private $serialize_item_vector = <<<EOT
  folly::dynamic __INAME__ = folly::dynamic::array;
  for (auto& t : __INAME___) {
__CONTAINER__ITEM__
  }
  result.insert("__SR_NAME__", __INAME__);
EOT;

  private $serialize_item_map = <<<EOT
  folly::dynamic __INAME__ = folly::dynamic::object;
  for (auto& t : __INAME___) {
__CONTAINER__ITEM__
  }
  result.insert("__SR_NAME__", __INAME__);
EOT;

  private $serialize_container_item_vector_pod = <<<EOT
    __INAME__.push_back(t);
EOT;
  private $serialize_container_item_vector_object = <<<EOT
    folly::dynamic vec___INAME__ = folly::dynamic::object;
    vec___INAME__ = t.serialize();
    __INAME__.push_back(vec___INAME__);
EOT;
  private $serialize_container_item_vector_enum = <<<EOT
    folly::dynamic enum___INAME__ = serialize__ENUM__CLASS__(t);
    __INAME__.push_back(enum___INAME__);
EOT;
  private $serialize_container_item_map_pod = <<<EOT
    __INAME__.insert(t.first, t.second);
EOT;
  private $serialize_container_item_map_object = <<<EOT
    folly::dynamic map___INAME__ = folly::dynamic::object;
    map___INAME__ = t.second.serialize();
    __INAME__.insert(t.first, map___INAME__);
EOT;
  private $serialize_container_item_map_enum = <<<EOT
    folly::dynamic enum___INAME__ = serialize__ENUM__CLASS__(t.second);
    __INAME__.insert(t.first, enum___INAME__);
EOT;

  private $serialize_item_other = <<<EOT
  folly::dynamic __INAME__ = folly::dynamic::object;
  __INAME__ = __INAME___.serialize();
  result.insert("__SR_NAME__", __INAME__);
EOT;

  private $serialize_item_enum = <<<EOT
  folly::dynamic __INAME__ = serialize__ENUM__CLASS__(__INAME___);
  result.insert("__SR_NAME__", __INAME__);
EOT;

  private $deserialize_item_pod = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ == nullptr || !__INAME__->__IS_TYPE__()) {
    return false;
  }
  set__NAME__(__INAME__->__AS__TYPE__());
EOT;
  private $deserialize_item_pod_opt = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ && __INAME__->__IS_TYPE__()) {
    set__NAME__(__INAME__->__AS__TYPE__());
  }
EOT;

  private $deserialize_item_vector = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ == nullptr || !__INAME__->isArray()) {
    return false;
  }

  __TYPE__ vec___INAME__;
  for (size_t i = 0; i < __INAME__->size(); i++) {
__CONTAINER__ITEM__
  }
  set__NAME__(vec___INAME__);
EOT;
  private $deserialize_item_vector_opt = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ && __INAME__->isArray()) {
    __TYPE__ vec___INAME__;
    for (size_t i = 0; i < __INAME__->size(); i++) {
__CONTAINER__ITEM__
    }
    set__NAME__(vec___INAME__);
  }
EOT;

  private $deserialize_container_item_vector_pod = <<<EOT
    if (!__INAME__->at(i).__IS_TYPE__()) {
      return false;
    }
    vec___INAME__.push_back(__INAME__->at(i).__AS__TYPE__());
EOT;
  private $deserialize_container_item_vector_object = <<<EOT
    if (!__INAME__->at(i).isObject()) {
      return false;
    }

    __CONTAINER__CLASS_NAME__ obj___INAME__;
    if (!obj___INAME__.deserialize(__INAME__->at(i))) {
      return false;
    }
    vec___INAME__.push_back(obj___INAME__);
EOT;
  private $deserialize_container_item_vector_enum = <<<EOT
    if (!__INAME__->at(i).isObject()) {
      return false;
    }

    __CONTAINER__CLASS_NAME__ enum___INAME__;
    if (!deserialize__CONTAINER__CLASS_NAME__(__INAME__->at(i), &enum___INAME__)) {
      return false;
    }
    vec___INAME__.push_back(enum___INAME__);
EOT;

  private $deserialize_item_map = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ == nullptr || !__INAME__->isObject()) {
    return false;
  }

  __TYPE__ map___INAME__;
  for (auto iter = __INAME__->keys().begin(); iter != __INAME__->keys().end(); iter++) {
__CONTAINER__ITEM__
  }
  set__NAME__(map___INAME__);
EOT;
  private $deserialize_item_map_opt = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ && __INAME__->isObject()) {
    __TYPE__ map___INAME__;
    for (auto iter = __INAME__->keys().begin(); iter != __INAME__->keys().end(); iter++) {
__CONTAINER__ITEM__
    }
    set__NAME__(map___INAME__);
  }
EOT;

  private $deserialize_container_item_map_pod = <<<EOT
    if (!__INAME__->at(*iter).__IS_TYPE__()) {
      return false;
    }
  
    if (!iter->__KEY_IS_TYPE__()) {
      return false;
    }
    map___INAME__.insert({iter->__KEY__AS_TYPE__(), __INAME__->at(*iter).__AS__TYPE__()});
EOT;
  private $deserialize_container_item_map_object = <<<EOT
    if (!__INAME__->at(*iter).isObject()) {
      return false;
    }

    __CONTAINER__CLASS_NAME__ obj___INAME__;
    if (!obj___INAME__.deserialize(__INAME__->at(*iter))) {
      return false;
    }
    if (!iter->__KEY_IS_TYPE__()) {
      return false;
    }
    map___INAME__.insert({iter->__KEY__AS_TYPE__(), obj___INAME__});
EOT;
  private $deserialize_container_item_map_enum = <<<EOT
    if (!__INAME__->at(i).isObject()) {
      return false;
    }

    __CONTAINER__CLASS_NAME__ enum___INAME__;
    if (!deserialize__CONTAINER__CLASS_NAME__(__INAME__->at(*iter), &enum___INAME__)) {
      return false;
    }
    if (!iter->__KEY_IS_TYPE__()) {
      return false;
    }
    vec___INAME__.insert({iter->__KEY__AS_TYPE__(), enum___INAME__});
EOT;

  private $deserialize_item_other = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ == nullptr) {
    return false;
  }

  __CONTAINER__CLASS_NAME__ item__INAME__;
  if (!item__INAME__.deserialize(*__INAME__)) {
    return false;
  }
  set__NAME__(item__INAME__);
EOT;

  private $deserialize_item_enum = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__ == nullptr) {
    return false;
  }

  __ENUM__CLASS__ item__INAME__;
  if (!deserialize__ENUM__CLASS__(*__INAME__, &item__INAME__)) {
    return false;
  }
  set__NAME__(item__INAME__);
EOT;

  private $deserialize_item_other_opt = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__) {
    __CONTAINER__CLASS_NAME__ item__INAME__;
    if (!item__INAME__.deserialize(*__INAME__)) {
      return false;
    }
    set__NAME__(item__INAME__);
  }
EOT;

  private $deserialize_item_enum_opt = <<<EOT
  auto* __INAME__ = data.get_ptr("__SR_NAME__");
  if (__INAME__) {
    __ENUM__CLASS__ item__INAME__;
    if (!deserialize__ENUM__CLASS__(*__INAME__, &item__INAME__)) {
      return false;
    }
    set__NAME__(item__INAME__);
  }
EOT;


// }}}
  // {{{ public function __construct
  
  public function __construct($config, $output, $template, $isDebug) {
    if (!file_exists($config)) {
      throw new Exception("配置文件不存在");
    }
    if (!$isDebug && $output == "") {
      throw new Exception("输出文件不合法");
    }
    if (!$isDebug && (file_exists($output . '.h') || file_exists($output . '.cc'))) {
      throw new Exception("输出文件已存在");
    }

    if (!$isDebug && (!file_exists($template . '.h.template') || !file_exists($template . '.cc.template'))) {
      throw new Exception("模板文件不存在");
    }

    $this->output = $output;
    $this->template = $template;
    $this->isDebug = $isDebug;
    $data = file($config);
    foreach ($data as $val) {
      $v = trim($val);

      if ($v == "") {
        continue;
      }
      if (false !== strpos($v, '#')) {
        continue;
      }
      $this->data[] = $v;
    }
  }

  // }}}
  // {{{ public function run()

  public function run() {
    $this->parse();   
    $this->dynamic();
    $this->output();
  }

  // }}}
  // {{{ private function parse()

  private function parse() {
    $this->parseClassName();
    foreach ($this->data as $item) {
      if (false !== strpos($item, 'class')) {
        $this->parseClass($item);
      } else {
        if ($this->options['type'] == 'enum') {
          $this->parseEnum($item);
        } else {
          $this->parseMember($item); 
        }
      }
    }
  }

  // }}}
  // {{{ private function parseClassName()

  private function parseClassName() {
    foreach ($this->data as $item) {
      if (false === strpos($item, 'class')) {
        continue;
      }
      $items = explode(' ', $item);

      if (count($items) == 3 && 'enumclass' == $items[0]) {
        $this->exists_enum[trim($items[2])] = 1;
      }

      if (count($items) == 2 && 'class' == $items[0]) {
        $this->exists_class[trim($items[1])] = 1;
      }
    }
  }

  // }}}
  // {{{ private function parseClass()

  private function parseClass($item) {
    // 写入上一个 class
    if ($this->className != '' && (!empty($this->members) || !empty($this->enums))) {
      $this->dynamic();
    } 

    $items = explode(' ', $item);

    if (isset($items[0]) && 'enumclass' == $items[0]) {
      if (count($items) != 3) {
        return;
      }
      $this->className = trim($items[2]);
      $this->options['type'] = 'enum'; 
      $this->options['enum_type'] = trim($items[1]);
    } else {
      if (count($items) != 2) {
        return;
      }
      $this->className = trim($items[1]);
    }

  }

  // }}}
  // {{{ private function parseMember()

  private function parseMember($item) {
    $regx = '/^\@SR\((.*)\)$/';
    if (preg_match($regx, $item, $matches) && isset($matches[1])) {
      $options = explode('|', $matches[1]);
      $annotation_option = [
        'isoptional' => false,
        'serialize_name' => $options[0]
      ];
      if (isset($options[1]) && $options[1] == 'optional') {
        $annotation_option['isoptional'] = true; 
      }
      $this->annotation_option = $annotation_option;
      return; 
    }

    $memberInfo = [
      'init' => '',
      'isconst' => false,
      'isvector' => false,
      'isstring' => false,
      'ismap' => false,
      'isoptional' => false,
      'serialize_name' => '',
      'container_type' => '',
    ];

    $memberInfo = array_merge($memberInfo, $this->annotation_option);
    $this->annotation_option = [];
    // init
    if (false !== strpos($item, '=')) {
      $inits = explode('=', $item);
      $memberInfo['init'] = trim($inits[1]);
      $item = trim($inits[0]);
    }

    // const
    if (false !== strpos($item, 'const')) {
      $isconst = true; 
      $memberInfo['isconst'] = true;
      $item = trim(str_replace("const", '', $item));
    }

    if (false !== strpos($item, '"')) {
      $infos = explode('"', ltrim($item, '"'));
    } else {
      $infos = explode(' ', $item);
    }
    $memberName = trim($infos[1]);
    $memberInfo['type'] = trim($infos[0]);

    if (false !== strpos($memberInfo['type'], 'std::string')) {
      $memberInfo['isstring'] = true;
    }
    if (false !== strpos($memberInfo['type'], 'std::vector')) {
      $memberInfo['isvector'] = true;
      $memberInfo['isstring'] = false;
      $regx = '/^std::vector<(.*)>$/';
      if (preg_match($regx, $memberInfo['type'], $matches) && isset($matches[1])) {
        $memberInfo['container_type'] = trim($matches[1]);
      }
    }
    if (false !== strpos($memberInfo['type'], 'std::unordered_map') ||
          false !== strpos($memberInfo['type'], 'std::map')) {
      $memberInfo['ismap'] = true;
      $memberInfo['isstring'] = false;
      $regx = '/^(std::unordered_map|std::map)<(.*),(.*)>$/';
      if (preg_match($regx, $memberInfo['type'], $matches) && isset($matches[3])) {
        $memberInfo['container_type'] = [
          trim($matches[2]),
          trim($matches[3])
        ];
      }
    }

    $this->members[$memberName] = $memberInfo;
  }
  // }}}
  // {{{ private function parseEnum()

  private function parseEnum($item) {
    $this->enums[] = trim($item);
  }
  // }}}
  // {{{ private function dynamic()

  private function dynamic() {
    if ($this->options['type'] == 'enum') {
      $this->dynamicEnum();
    } else {
      $this->dynamicClass(); 
    }
    $this->clear();
  }

  // }}}
  // {{{ private function output()

  private function output() {
    if ($this->isDebug) {
      return;
    }

    $header = file_get_contents($this->template . '.h.template');
    $source = file_get_contents($this->template . '.cc.template');

    $header_data = file_get_contents($this->output . '.h');
    $source_data = file_get_contents($this->output . '.cc');

    file_put_contents($this->output . '.h', str_replace('__HEADER__DATA__', $header_data, $header));
    file_put_contents($this->output . '.cc', str_replace('__SOURCE__DATA__', $source_data, $source));
  }

  // }}}
  // {{{ private function dynamicClass()

  private function dynamicClass() {
    $memberDeclares = [];
    $functionDefines = [];
    $items = [];
    foreach ($this->members as $name => $option) {
      if ($option['init'] == '') {
        $memberDeclares[] = str_replace(['__TYPE__', '__NAME__'], 
            [$option['type'], $name . '_'], $this->mem);
      } else {
        $memberDeclares[] = str_replace(['__TYPE__', '__NAME__', '__INIT__'], 
            [$option['type'], $name . '_', $option['init']], $this->memInit);
      }
      $funcName = $this->convertName($name);
      if ($option['isconst']) {
        $functionDefines[] = str_replace(['__TYPE__', '__NAME__', '__INAME__'], 
            [$option['type'], $funcName, $name], $this->getterConst);
        $functionDefines[] = str_replace(['__TYPE__', '__NAME__', '__INAME__'], 
            [$option['type'], $funcName, $name], $this->setterConst);
      } else {
        $functionDefines[] = str_replace(['__TYPE__', '__NAME__', '__INAME__'], 
            [$option['type'], $funcName, $name], $this->getter);
        $functionDefines[] = str_replace(['__TYPE__', '__NAME__', '__INAME__'], 
            [$option['type'], $funcName, $name], $this->setter);
      }

      // cc
      if ($option['isstring']) {
        $items[] = str_replace(['__NAME__', '__INAME__'], [$funcName, $name], $this->itemQuote);
      } else if ($option['isvector']) {
        $items[] = str_replace(['__NAME__', '__INAME__'], [$funcName, $name], $this->itemVector);
      } else if ($option['ismap']) {
        $items[] = str_replace(['__NAME__', '__INAME__'], [$funcName, $name], $this->itemMap);
      } else {
        $items[] = str_replace(['__NAME__', '__INAME__'], [$funcName, $name], $this->item);
      }
    }

    $func = implode("\n\n", $functionDefines);
    $memberstr  = implode("\n", $memberDeclares);
    $result = str_replace(['__CLASS_NAME__', '__FUNCS__', '__MEMBERS__'],
        [$this->className, $func, $memberstr], $this->class);

    $itemstr = implode("\n     << \", \" ", $items);
    $serializes = $this->serialize();
    $deserializes = $this->deserialize();
    $cc = str_replace(['__CLASS_NAME__', '__ITEMS__', '__SER_OBJECT_ITEMS__', '__UNSER_OBJECT_ITEMS__'], 
      [$this->className, $itemstr, implode("\n", $serializes), implode("\n", $deserializes)],
      $this->describeSource);

    if ($this->isDebug) {
        echo $result;
    } else {
      file_put_contents($this->output . '.h', $result, FILE_APPEND);
    }
    if ($this->isDebug) {
        echo $cc;
    } else {
      file_put_contents($this->output . '.cc', $cc, FILE_APPEND);
    }
  }

  // }}}
  // {{{ private function serialize()

  private function serialize() {
    $serializes = [];
    foreach ($this->members as $name => $option) {
      $funcName = $this->convertName($name);
      // serialize
      $serializeName = $option['serialize_name'];
      if ($serializeName == '') {
        $serializeName = $funcName;
      }
      if (in_array($option['type'], $this->podTypes) || $option['isstring']) {
        $serializes[] = str_replace(['__SR_NAME__', '__INAME__'], 
            [$serializeName, $name], $this->serialize_item_pod); 
      } else if ($option['isvector']) {
        if (in_array($option['container_type'], $this->podTypes)) {
          $container_value =  str_replace('__INAME__', $name, $this->serialize_container_item_vector_pod);
        } else if (isset($this->exists_enum[$option['container_type']])) {
          $container_value =  str_replace('__INAME__', $name, $this->serialize_container_item_vector_enum);
        } else if (isset($this->exists_class[$option['container_type']])) {
          $container_value =  str_replace('__INAME__', $name, $this->serialize_container_item_vector_object);
        } else {
          throw new Exception('serialize type is invalid, current type:' . $option['container_type'] . ' name:' . $name);
        }
        $serializes[] = str_replace(['__SR_NAME__', '__INAME__', '__CONTAINER__ITEM__'], 
            [$serializeName, $name, $container_value], $this->serialize_item_vector); 
      } else if ($option['ismap']) {
        if (in_array($option['container_type'][1], $this->podTypes)) {
          $container_value =  str_replace('__INAME__', $name, $this->serialize_container_item_map_pod);
        } else if (isset($this->exists_enum[$option['container_type'][1]])) {
          $container_value =  str_replace('__INAME__', $name, $this->serialize_container_item_map_enum);
        } else if (isset($this->exists_class[$option['container_type'][1]])) {
          $container_value =  str_replace('__INAME__', $name, $this->serialize_container_item_map_object);
        } else {
          throw new Exception('serialize type is invalid, current type:' . $option['container_type'][1] . ' name:' . $name);
        }
        $serializes[] = str_replace(['__SR_NAME__', '__INAME__', '__CONTAINER__ITEM__'], 
            [$serializeName, $name, $container_value], $this->serialize_item_map); 
      } else if (isset($this->exists_enum[$option['type']])) {
        $serializes[] = str_replace(['__SR_NAME__', '__INAME__', '__ENUM__CLASS__'], 
            [$serializeName, $name, $option['type']], $this->serialize_item_enum); 
      } else if (isset($this->exists_class[$option['type']])) {
        $serializes[] = str_replace(['__SR_NAME__', '__INAME__'], 
            [$serializeName, $name], $this->serialize_item_other); 
      } else {
        throw new Exception('serialize type is invalid, current type:' . $option['type'] . ' name:' . $name);
      }
    }

    return $serializes;
  }

  // }}}
  // {{{ private function deserialize()

  private function deserialize() {
    $deserializes = [];
    foreach ($this->members as $name => $option) {
      $funcName = $this->convertName($name);
      // deserialize
      $serializeName = $option['serialize_name'];
      if ($serializeName == '') {
        $serializeName = $funcName;
      }

      if (in_array($option['type'], $this->podTypes) || $option['isstring']) {
        $diagnosticTypes = $this->diagnostic($option['type']);
        if (!$option['isoptional']) {
          $deserializes[] = str_replace(['__SR_NAME__', '__NAME__', '__INAME__', '__IS_TYPE__', '__AS__TYPE__'], 
            [$serializeName, $funcName, $name, $diagnosticTypes['is'], $diagnosticTypes['as']], 
            $this->deserialize_item_pod); 
        } else {
          $deserializes[] = str_replace(['__SR_NAME__', '__NAME__', '__INAME__', '__IS_TYPE__', '__AS__TYPE__'], 
            [$serializeName, $funcName, $name, $diagnosticTypes['is'], $diagnosticTypes['as']], 
            $this->deserialize_item_pod_opt); 
        }
      } else if ($option['isvector']) {
        if (in_array($option['container_type'], $this->podTypes)) {
          $diagnosticTypes = $this->diagnostic($option['container_type']);
          $container_value =  str_replace(['__INAME__', '__NAME__', '__IS_TYPE__', '__AS__TYPE__'],
            [$name, $funcName, $diagnosticTypes['is'], $diagnosticTypes['as']], 
            $this->deserialize_container_item_vector_pod);
        } else if (isset($this->exists_enum[$option['container_type']])) {
          $container_value =  str_replace(['__INAME__', '__NAME__', '__CONTAINER__CLASS_NAME__'],
            [$name, $funcName, $option['container_type']], $this->deserialize_container_item_vector_enum);
        } else if (isset($this->exists_class[$option['container_type']])) {
          $container_value =  str_replace(['__INAME__', '__NAME__', '__CONTAINER__CLASS_NAME__'],
            [$name, $funcName, $option['container_type']], $this->deserialize_container_item_vector_object);
        } else {
          throw new Exception('deserialize type is invalid, current type:' . $option['container_type'] . ' name:' . $name);
        }
        if (!$option['isoptional']) {
          $deserializes[] = str_replace(['__SR_NAME__', '__TYPE__', '__NAME__', '__INAME__', '__CONTAINER__ITEM__'], 
            [$serializeName, $option['type'], $funcName, $name, $container_value], $this->deserialize_item_vector); 
        } else {
          $deserializes[] = str_replace(['__SR_NAME__', '__TYPE__', '__NAME__', '__INAME__', '__CONTAINER__ITEM__'], 
            [$serializeName, $option['type'], $funcName, $name, $container_value], $this->deserialize_item_vector_opt); 
        }
      } else if ($option['ismap']) {
        $keyDiagnosticTypes = $this->diagnostic($option['container_type'][0]);
        if (in_array($option['container_type'][1], $this->podTypes)) {
          $diagnosticTypes = $this->diagnostic($option['container_type'][1]);
          $container_value =  str_replace(['__INAME__', '__NAME__', '__IS_TYPE__', '__AS__TYPE__',
            '__KEY_IS_TYPE__', '__KEY__AS_TYPE__'],
            [$name, $funcName, $diagnosticTypes['is'], $diagnosticTypes['as'], 
            $keyDiagnosticTypes['is'], $keyDiagnosticTypes['as']], 
            $this->deserialize_container_item_map_pod);
        } else if (isset($this->exists_enum[$option['container_type'][1]])) {
          $container_value =  str_replace(['__INAME__', '__NAME__', '__CONTAINER__CLASS_NAME__',
            '__KEY_IS_TYPE__', '__KEY__AS_TYPE__'],
            [$name, $funcName, $option['container_type'][1],
            $keyDiagnosticTypes['is'], $keyDiagnosticTypes['as']], $this->deserialize_container_item_map_enum);
        } else if (isset($this->exists_class[$option['container_type'][1]])) {
          $container_value =  str_replace(['__INAME__', '__NAME__', '__CONTAINER__CLASS_NAME__',
            '__KEY_IS_TYPE__', '__KEY__AS_TYPE__'],
            [$name, $funcName, $option['container_type'][1],
            $keyDiagnosticTypes['is'], $keyDiagnosticTypes['as']], $this->deserialize_container_item_map_object);
        } else {
          throw new Exception('deserialize type is invalid, current type:' . $option['container_type'][1] . ' name:' . $name);
        }
        if (!$option['isoptional']) {
          $deserializes[] = str_replace(['__SR_NAME__', '__TYPE__', '__NAME__', '__INAME__', '__CONTAINER__ITEM__'], 
            [$serializeName, $option['type'], $funcName, $name, $container_value], $this->deserialize_item_map); 
        } else {
          $deserializes[] = str_replace(['__SR_NAME__', '__TYPE__', '__NAME__', '__INAME__', '__CONTAINER__ITEM__'], 
            [$serializeName, $option['type'], $funcName, $name, $container_value], $this->deserialize_item_map_opt); 
        }
      } else if (isset($this->exists_enum[$option['type']])) {
        if (!$option['isoptional']) {
          $deserializes[] = str_replace(['__SR_NAME__', '__NAME__', '__INAME__', '__ENUM__CLASS__'], 
              [$serializeName, $funcName, $name, $option['type']], $this->deserialize_item_enum); 
        } else {
          $deserializes[] = str_replace(['__SR_NAME__', '__NAME__', '__INAME__', '__ENUM__CLASS__'], 
              [$serializeName, $funcName, $name, $option['type']], $this->deserialize_item_enum_opt); 
        }
      } else if (isset($this->exists_class[$option['type']])) {
        if (!$option['isoptional']) {
          $deserializes[] = str_replace(['__SR_NAME__', '__NAME__', '__INAME__', '__CONTAINER__CLASS_NAME__'], 
              [$serializeName, $funcName, $name, $option['type']], $this->deserialize_item_other); 
        } else {
          $deserializes[] = str_replace(['__SR_NAME__', '__NAME__', '__INAME__', '__CONTAINER__CLASS_NAME__'], 
              [$serializeName, $funcName, $name, $option['type']], $this->deserialize_item_other_opt); 
        }
      } else {
        throw new Exception('deserialize type is invalid, current type:' . $option['type'] . ' name:' . $name);
      }
    }

    return $deserializes;
  }

  // }}}
  // {{{ private function diagnostic()

  private function diagnostic($type) {
    if ($type == 'std::string') {
      $types = [
        'is' => 'isString',
        'as' => 'asString',
      ];
    } else if ($type == 'bool') {
      $types = [
        'is' => 'isBool',
        'as' => 'asBool',
      ];
    } else if ($type == 'double') {
      $types = [
        'is' => 'isDouble',
        'as' => 'asDouble',
      ];
    } else {
      $types = [
        'is' => 'isInt',
        'as' => 'asInt',
      ];
    }

    return $types;
  }

  // }}}
  // {{{ private function dynamicEnum()

  private function dynamicEnum() {
    $declares = [];

    if ($this->options['enum_type'] == 'string') {
      $stringDeclare = str_replace('__CLASS_NAME__', $this->className, $this->enumclass_string_declare);
    } else {
      $stringDeclare = '';
    }
    $declaresStr = implode(",\n  ", $this->enums);
    $result = str_replace(['__CLASS_NAME__', '__ENUMS__', '__STRING_DECLARE__'],
        [$this->className, $declaresStr, $stringDeclare], $this->enumclass);

    $cases = [];

    if ($this->options['enum_type'] == 'string') {
      $deserializeStringCases = [];
      $serializeStringCases = [];
      foreach ($this->enums as $val) {
        $serializeStringCases[] = str_replace(['__CLASS_NAME__', '__ITEM__', '__IITEM__'],
          [$this->className, $val, strtolower($val)], $this->describeEnumSourceStringCaseSerialize); 
        $cases[] = str_replace(['__CLASS_NAME__', '__ITEM__', '__IITEM__'],
          [$this->className, $val, strtolower($val)], $this->describeEnumSourceCase); 
        $deserializeStringCases[] = str_replace(['__CLASS_NAME__', '__ITEM__', '__IITEM__'],
          [$this->className, $val, strtolower($val)], $this->describeEnumSourceStringCaseDeserialize); 
      }

      $cc = str_replace(['__CLASS_NAME__', '__CASE_ITMES__', '__STRING_ENUM_SER_CASE__',
        '__STRING_ENUM_UNSER_CASE__'], [$this->className, implode("\n", $cases),
        implode("\n", $serializeStringCases), implode("\n", $deserializeStringCases)],
        $this->describeEnumSourceString);
    } else {
      $deserializeIntCases = [];
      foreach ($this->enums as $key => $val) {
        $deserializeIntCases[] = str_replace(['__ITEM_ID__', '__CLASS_NAME__', '__ITEM__'],
            [$key, $this->className, $val], $this->describeEnumSourceIntCaseDeserialize);
      }
      $cc = str_replace(['__CLASS_NAME__', '__INT_ENUM_UNSER_CASE__'], 
          [$this->className, implode("\n", $deserializeIntCases)], $this->describeEnumSourceInt); 
EOT;
    }

    if ($this->isDebug) {
        echo $result;
    } else {
      file_put_contents($this->output . '.h', $result, FILE_APPEND);
    }
    if ($this->isDebug) {
        echo $cc;
    } else {
      file_put_contents($this->output . '.cc', $cc, FILE_APPEND);
    }
  }

  // }}}
  // {{{ private function convertName()

  private function convertName($name) {
    $arr = explode('_', $name);
    $str = '';
    foreach ($arr as $val) {
      $str .= ucfirst($val);
    }

    return $str;
  }

  // }}}
  // {{{ private function clear()

  private function clear() {
    $this->className = '';
    $this->members = [];
    $this->options = [];
    $this->enums = [];
  }

  // }}}
}

$param_arr = getopt('c:o:t:dh');
if (!isset($param_arr['c']) || !file_exists($param_arr['c'])) {
  echo "./entity_generator -c entity.config -o output", PHP_EOL;
  exit(1);
}

$isDebug = isset($param_arr['d']) ? true : false;
$generatorObj = new GeneratorEntity($param_arr['c'], $param_arr['o'], $param_arr['t'], $isDebug);
$generatorObj->run();
