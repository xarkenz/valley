#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <functional>
#include <stack>
#include <utility>
#include <vector>


namespace valley {

  using CharGetter = std::function<int()>;

  class PushBackStream {
  private:
    const CharGetter& _input;
    std::stack<int> _stack;
    size_t _line_number;
    size_t _char_index;
    
  public:
    PushBackStream(const CharGetter& input);

    int operator()();

    void pushBack(int c);

    size_t lineNumber() const;
    size_t charIndex() const;
  };

  template <typename K, typename V>
  class Lookup {
  public:
    using ElementType = std::pair<K, V>;
    using ContainerType = std::vector<ElementType>;

  private:
    ContainerType _container;

  public:
    using Iterator = typename ContainerType::const_iterator;
    
    Lookup(std::initializer_list<ElementType> init): _container(init) {
      std::sort(_container.begin(), _container.end());
    }

    Lookup(ContainerType container): _container(std::move(container)) {
      std::sort(_container.begin(), _container.end());
    }

    Iterator begin() const {
      return _container.begin();
    }

    Iterator end() const {
      return _container.end();
    }

    Iterator find(const K& key) const {
      Iterator it = std::lower_bound(begin(), end(), key,
        [](const ElementType& p, const K& key) {
          return p.first < key;
        }
      );
      return it != end() && it->first == key ? it : end();
    }

    size_t size() const {
      return _container.size();
    }
  };

  // From https://en.cppreference.com/w/cpp/utility/variant/visit
  template <class... Ts>
  struct Overloaded: Ts... {
    using Ts::operator()...;
  };

  template <class... Ts>
  Overloaded(Ts...) -> Overloaded<Ts...>;

}


#endif