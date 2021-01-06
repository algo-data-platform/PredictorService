/*
 * Copyright 2017-present Facebook, Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#ifndef T_STREAM_H
#define T_STREAM_H

/**
 * A stream is a lightweight object type that just wraps another data type.
 *
 */
class t_stream : public t_type {
 public:
  explicit t_stream(t_type* elem_type) : elem_type_(elem_type) {}

  t_type* get_elem_type() const {
    return elem_type_;
  }

  bool is_stream() const override {
    return true;
  }

  std::string get_full_name() const override {
    return "stream<" + elem_type_->get_full_name() + ">";
  }

  std::string get_impl_full_name() const override {
    return "stream<" + elem_type_->get_impl_full_name() + ">";
  }

  TypeValue get_type_value() const override {
    return TypeValue::TYPE_STREAM;
  }

 private:
  t_type* elem_type_;
};

class t_pubsub_stream : public t_type {
 public:
  explicit t_pubsub_stream(t_type* elem_type) : elem_type_(elem_type) {}

  t_type* get_elem_type() const {
    return elem_type_;
  }

  bool is_pubsub_stream() const override {
    return true;
  }

  std::string get_full_name() const override {
    return "stream " + elem_type_->get_full_name();
  }

  std::string get_impl_full_name() const override {
    return "stream " + elem_type_->get_impl_full_name();
  }

  TypeValue get_type_value() const override {
    return TypeValue::TYPE_I32;
  }

 protected:
  t_type* elem_type_;
};

class t_stream_response : public t_pubsub_stream {
 public:
  explicit t_stream_response(t_type* elem_type, t_type* extra_type = nullptr)
      : t_pubsub_stream(elem_type), extra_type_(extra_type) {}

  t_type* get_extra_type() const {
    return extra_type_;
  }

  bool is_streamresponse() const override {
    return true;
  }

  bool has_extratype() const override {
    return (bool)(extra_type_);
  }

  std::string get_full_name() const override {
    return "streamresponse " + elem_type_->get_full_name() +
        (has_extratype() ? (", " + extra_type_->get_full_name()) : "");
  }

  std::string get_impl_full_name() const override {
    return "streamresponse " + elem_type_->get_impl_full_name() +
        (has_extratype() ? (", " + extra_type_->get_impl_full_name()) : "");
  }

 private:
  t_type* extra_type_;
};

#endif
