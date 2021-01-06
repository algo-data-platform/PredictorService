#include <iostream>
#include <functional>
#include "factory_registry.h"

/**
 * test case one: shared_ptr, call ->body once, return object had created
 */
namespace person {
struct Person {
  virtual void Print() = 0;
  virtual ~Person() {}
};

struct Teacher : public Person {
  virtual void Print() { std::cout << "I am a teacher" << std::endl; }
};

struct Engineer : public Person {
  virtual void Print() { std::cout << "I am an engineer" << std::endl; }
};
// registry to get the person
struct PersonFactory : public common::FunctionRegEntryBase<PersonFactory, std::shared_ptr<Person> > {};

#define REGISTER_PERSON(ClassName, Name) \
  COMMON_REGISTRY_REGISTER(::person::PersonFactory, PersonFactory, Name).set_body(std::make_shared<ClassName>())

}  // namespace person

// usually this sits on a seperate file
namespace common {
COMMON_REGISTRY_ENABLE(person::PersonFactory);
}

namespace person {
// Register the trees, can be in seperate files
REGISTER_PERSON(Teacher, teacher).describe("This is a teacher.");

REGISTER_PERSON(Engineer, engineer);
}

/**
 * test case two: shared_ptr, call ->body() once, new shared_ptr<object> once
 */
namespace car {
struct Car {
  virtual void Print() = 0;
  virtual ~Car() {}
};

struct Benz : public Car {
  virtual void Print() { std::cout << "I am a benz car" << std::endl; }
};

struct Bmw : public Car {
  virtual void Print() { std::cout << "I am a bmw car" << std::endl; }
};
// registry to get the car
struct CarFactory : public common::FunctionRegEntryBase<CarFactory, std::function<std::shared_ptr<Car>()> > {};

#define REGISTER_CAR(ClassName, Name)                           \
  COMMON_REGISTRY_REGISTER(::car::CarFactory, CarFactory, Name) \
      .set_body([]() { return std::shared_ptr<Car>(new ClassName()); })

}  // namespace car

// usually this sits on a seperate file
namespace common {
COMMON_REGISTRY_ENABLE(car::CarFactory);
}

namespace car {
// Register the cars, can be in seperate files
REGISTER_CAR(Benz, benz).describe("This is a benz car.");
REGISTER_CAR(Bmw, bmw);
}

int main(int argc, char *argv[]) {
  // example one
  std::shared_ptr<person::Person> person_one = common::FactoryRegistry<person::PersonFactory>::Find("teacher")->body;
  person_one->Print();
  std::shared_ptr<person::Person> person_two = common::FactoryRegistry<person::PersonFactory>::Find("engineer")->body;
  person_two->Print();

  // example two
  std::shared_ptr<car::Car> car_one = common::FactoryRegistry<car::CarFactory>::Find("benz")->body();
  car_one->Print();
  std::shared_ptr<car::Car> car_two = common::FactoryRegistry<car::CarFactory>::Find("bmw")->body();
  car_two->Print();

  return 0;
}

