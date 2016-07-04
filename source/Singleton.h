#pragma once

template<typename T> 
class Singleton {
protected:
   Singleton() = default;
   Singleton(const Singleton&) = delete;
   Singleton& operator=(const Singleton&) = delete;
   virtual ~Singleton() = default;

public:
   template<typename... Args>
   static T& getInstance(Args... args) {
      static auto onceFunction = std::bind(createInstanceInternal<Args...>, args...);
      return apply(onceFunction);
   }

private:

   static T& apply(const std::function<T&()>& function) {
      static T& instanceRef = function();
      return instanceRef;
   }

   template<typename... Args>
   static T& createInstanceInternal(Args... args) {
      static T instance{ std::forward<Args>(args)... };
      return instance;
   }
};