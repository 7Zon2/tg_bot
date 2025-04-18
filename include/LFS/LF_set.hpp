#include "LF_HashTable.hpp"
#include <type_traits>


template<typename T>
class LF_set : protected LF_HashTable<T,T>
{
  protected:

  using Table = LF_HashTable<T,T>;
  using Table::insert;

  class Set_iterator : protected Table::iterator
  {
    protected:

      using it_t = Table::iterator;

    public:

    Set_iterator() noexcept {}


    Set_iterator(it_t it) noexcept :
      it_t(it){}


    friend bool operator == (const Set_iterator& lhs, const Set_iterator& rhs) noexcept = default;


    operator bool() noexcept 
    {
      return it_t::operator bool();
    }

    public:

    using iterator_category  = typename it_t::iterator_category;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using difference_type = typename it_t::difference_type;

    public:


    Set_iterator& operator ++() noexcept
    {
      it_t::operator++();
      return *this;
    }


    Set_iterator operator++(int) noexcept 
    {
      Set_iterator temp{*this};
      it_t::operator++();
      return temp;
    }


    reference operator*() const noexcept
    {
      auto& pair = it_t::operator*();
      return pair.first;
    }


    pointer operator->() const noexcept
    {
      auto& pair = it_t::operator*();
      return &pair.first;
    }
  };

  static_assert(std::forward_iterator<Set_iterator>);

  public:

  using iterator = Set_iterator;
  using Table::clear;
  using Table::remove;
  using Table::size;
  using Table::get_hash;

  LF_set(const size_t load_factor = 2):
    Table(load_factor){}

  public:

  template<typename U>
  bool insert(U&& key)
  {
    using type = std::remove_reference_t<U>;
    return Table::insert(std::forward<U>(key), type{});
  }

  auto begin() noexcept 
  {
    return Set_iterator{Table::begin()};
  }

  
  auto end() noexcept 
  {
    return Set_iterator{Table::end()};
  }
};
