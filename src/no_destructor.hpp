#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

// 其成员对象，在析构时不进行对其成员变量的析构
// Wraps an instance whose destructor is never called.
//
// This is intended for use with function-level static variables.
template <typename InstanceType>
class NoDestructor {
 public:
  template <typename... ConstructorArgTypes>
  explicit NoDestructor(ConstructorArgTypes&&... constructor_args) {
    static_assert(sizeof(instance_storage_) >= sizeof(InstanceType),
                  "instance_storage_ is not large enough to hold the instance");
    static_assert(std::is_standard_layout_v<NoDestructor<InstanceType>>);
    static_assert(
        offsetof(NoDestructor, instance_storage_) % alignof(InstanceType) == 0,
        "instance_storage_ does not meet the instance's alignment requirement");
    static_assert(
        alignof(NoDestructor<InstanceType>) % alignof(InstanceType) == 0,
        "instance_storage_ does not meet the instance's alignment requirement");
    new (instance_storage_)
        InstanceType(std::forward<ConstructorArgTypes>(constructor_args)...);
  }

  ~NoDestructor() = default;

  NoDestructor(const NoDestructor&) = delete;
  NoDestructor& operator=(const NoDestructor&) = delete;

  InstanceType* get() {
    return reinterpret_cast<InstanceType*>(&instance_storage_);
  }

 private:
  alignas(InstanceType) char instance_storage_[sizeof(InstanceType)];
// alignas 保证内存的对齐方式， 声明一个对象的内存空间
};
