#pragma once
#include "LFS/LF_FreeList.hpp"
#include "LFS/LF_hazardous.hpp"
#include "LF_allocator.hpp"
#include <atomic>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <thread>


template<typename T, typename Compare = std::less_equal<T>>
class LF_OrderList : protected FreeList<std::pair<size_t,T>, true>
{
  public:

  using type = std::pair<size_t, T>;
  using FreeList_t = FreeList<type, true>;
  
  using typename FreeList_t::Node;
  using typename FreeList_t::size;
  
  protected:

  using FreeList_t::head_;
  using FreeList_t::counter_;
  using FreeList_t::tail_;
  using FreeList_t::allocate;
  using FreeList_t::begin;
  using FreeList_t::end;
  
  protected:

  class order_iterator : private FreeList_t::iterator
  {
    protected:

    using it_t = FreeList_t::iterator; 
    using it_t::node_;

    public:

    order_iterator(){}
    
    order_iterator(Node* node):
      FreeList_t::iterator(node){}

    
    friend bool operator == (const order_iterator& it, const Node* node) noexcept
    {
      return it.node_ == node;
    }

    friend bool operator ==(const order_iterator&, const order_iterator&) noexcept = default; 

    public:

    using iterator_category = typename it_t::iterator_category;
    using value_type = std::remove_cvref_t<T>;
    using pointer =  T*;
    using reference = T&;
    using difference_type = typename it_t::difference_type;

    public:

    order_iterator& operator++() noexcept
    {
      Node* next = node_->next();
      next = clear_tag(next).second;
      node_ = next;
      return *this;
    }


    order_iterator operator++(int) noexcept
    {
      order_iterator temp{*this};
      operator++();
      return temp;
    }


    reference operator*() const noexcept
    {
      type& pa = it_t::operator*();
      return pa.second;
    }


    pointer operator->() const noexcept
    {
      type& pa = it_t::operator*();
      return &pa.second;
    }

  };

  static_assert(std::forward_iterator<order_iterator>);

  protected:

  LF_allocator alloc_;
  Compare comporator_;

  public:

  using iterator = order_iterator;

  LF_OrderList(Compare comporator = Compare{}):
    alloc_([this](void* data, size_t data_size)
    {    
      std::destroy_at(static_cast<type*>(data));
    }), 
    comporator_(comporator)
  {
    FreeList<type,true>::set_allocator(&alloc_);
    void* p = allocate();
    Node* next = ::new(p) Node(); //the ghost node for the find method
    Node* head = head_.load(std::memory_order_relaxed);
    head->next_.store(next, std::memory_order_relaxed);
    tail_.store(next, std::memory_order_relaxed);
  }


  LF_OrderList(LF_OrderList&& rhs) noexcept
  {
    LF_OrderList temp;
    head_ =  rhs.head_.exchange(temp.head_, std::memory_order_acq_rel);
    tail_  = rhs.tail_.exchange(temp.tail_, std::memory_order_acq_rel);
    comporator_ = std::move(rhs.comporator_);
    alloc_ = std::move(rhs.alloc_);
  }


  LF_OrderList& operator = (LF_OrderList&& rhs) noexcept
  {
    if(this!=&rhs)
    {
      LF_OrderList temp;
      head_ =  rhs.head_.exchange(temp.head_, std::memory_order_acq_rel);
      tail_  = rhs.tail_.exchange(temp.tail_, std::memory_order_acq_rel);
      comporator_ = std::move(rhs.comporator_);
      alloc_ = std::move(temp.alloc_);
      temp.reset();
    }
    return *this;
  }


  ~LF_OrderList()
  {
    clear();
    Node* head = head_.exchange(nullptr, std::memory_order_relaxed);
    while(head)
    {
      Node* next = head->next();
      alloc_.deallocate(head, sizeof(Node));
      head = next;
    }
  }

  protected:

  [[nodiscard]]
  std::pair<Node*, Hazardous::hazard_pointer>
  search_node(size_t key, Node* it = {})
  {
    if(!it)
    {
      it = head_.load(std::memory_order_relaxed);
    }

    Node * prev = it;
    Node * curr = nullptr;
    Node * next = nullptr;
    auto hzp_c = alloc_.get_hazard();
    for(;;)
    {
      curr = prev->next();
      if(!curr)
      {
        return std::make_pair(prev, nullptr);
      }

      auto pc = clear_tag(curr);
      Node * cleared = pc.second;
      bool marked = pc.first;

      hzp_c->protect(cleared);
      if(curr != prev->next())
      {
        prev = head_.load(std::memory_order_relaxed);
        continue;
      }


      while(marked)
      {
        if(cleared)
        {
          next = cleared->next();
        }
        else
        {
          next = nullptr;
        }

        bool success = prev->next_.compare_exchange_strong(curr, next, std::memory_order_relaxed);
        if(success)
        {
          hzp_c.reclaim();
          hzp_c = alloc_.get_hazard();
        }
        else
        {
          prev = head_.load(std::memory_order_relaxed);
          curr = nullptr;
          break;
        }

        curr = next; //next may be corrupted or nullptr
        pc = clear_tag(curr);
        marked = pc.first;
        cleared = pc.second;

        hzp_c->protect(cleared);
        if(prev->next()!=next)
        {
          prev = head_.load(std::memory_order_relaxed);
          curr = nullptr;
          break;
        }
      }

      if(!curr)
      {
        continue;
      }

      assert(curr==cleared);

      type& pair = curr->data_;
      size_t key_ = pair.first;
      if(comporator_(key, key_))
      {
        return std::make_pair(prev, std::move(hzp_c));
      }

      prev = curr;
    }
  }


  template<typename U>
  [[nodiscard]]
  std::pair<Node*,Hazardous::hazard_pointer> 
  find_node(const U& key, Node* it = {})
  {
    if(!it)
    {
      it = head_.load(std::memory_order_relaxed);
    }

    size_t key_ = std::hash<U>{}(key);
    auto [prev, hzp] = search_node(key_, it);
    if(!hzp)
    {
      return {};
    }

    Node* ptr = static_cast<Node*>(hzp->data_.load(std::memory_order_acquire));
    if(ptr && (ptr->data_.first == key_))
    {
      return std::make_pair(prev, std::move(hzp));
    }
    return {};
  }
  
  public:

  [[nodiscard]]
  Node* data() noexcept
  {
    return head_.load(std::memory_order_relaxed);
  }


  [[nodiscard]]
  Node* back() noexcept
  {
    return tail_.load(std::memory_order_relaxed);
  }


  [[nodiscard]]
  auto begin() noexcept
  {
    return order_iterator(head_.load(std::memory_order_relaxed));
  }


  [[nodiscard]]
  auto end() noexcept
  {
    return order_iterator(nullptr);
  }


  void 
  clear() override
  {
    alloc_.clear();
    void * p = allocate();
    Node * new_head = ::new(p) Node();

    p = allocate();
    Node * new_next = ::new(p) Node();
    new_head->next_.store(new_next, std::memory_order_relaxed);

    Node* head = head_.exchange(new_head, std::memory_order_relaxed);
    tail_.exchange(new_next, std::memory_order_relaxed);
    while(head)
    {
      Node* next = head->next();
      alloc_.deallocate(head, sizeof(Node));
      head = clear_tag(next).second;
    }
    counter_.store(0, std::memory_order_release);
  }


  template<typename U>
  [[nodiscard]]
  std::optional<T>
  contain(const U& key, Node * it = {})
  {
    auto hzp = std::move(find_node(key, it).second);
    if(!hzp)
    {
      return {};
    }

    Node* ptr = static_cast<Node*>(hzp->data_.load(std::memory_order_acquire));
    std::optional<T> opt = ptr->data_.second;
    return opt;
  }


  template<typename U>
  [[nodiscard]]
  bool 
  find(const U& key, Node* it = {})
  {
    auto[prev,hzp] = find_node(key, it);
    bool res = hzp;
    hzp->reset();
    return res;
  }


  template<typename U>
  Node* 
  insert(U&& data, Node* it = {}, bool before = false)
  {
    if(!it)
    {
      it = head_.load(std::memory_order_relaxed);
    }

    auto u = std::hash<std::remove_cvref_t<U>>{};
    size_t key = u(std::forward<U>(data));  
    auto [prev, hzp] = search_node(key, it);
    if(!prev)
    {
      return {};
    }

    Node* node{};
    if(hzp == nullptr || before)
    {
      node = prev;
    }
    else
    {
      node = static_cast<Node*>(hzp->data_.load(std::memory_order_acquire));
    }
    
    assert(node);

    Node* tail = tail_.load(std::memory_order_relaxed);
    Node* next = node->next_.load(std::memory_order_relaxed);

    void* ptr =  alloc_.allocate(sizeof(Node));
    Node* new_node = ::new(ptr) Node(type{key, std::forward<U>(data)});
    new_node->next_.store(next, std::memory_order_relaxed);

    if(!node->next_.compare_exchange_strong(next, new_node, std::memory_order_acq_rel))
    {
      alloc_.deallocate(new_node, sizeof(Node));
      return {};
    }
    if(node == tail)
    {
      while(!tail_.compare_exchange_weak(tail, new_node, std::memory_order_relaxed));
    }
    counter_.fetch_add(1, std::memory_order_release);
  
    return new_node;
  }


  template<typename U>
  bool 
  remove(const U& key, Node* it = {})
  {
    if(!it)
    {
      it = head_.load(std::memory_order_relaxed);
    }

    auto[prev,hzp] = find_node(key, it);
    if(!hzp)
    {
      return false;
    }
   
    Node * curr = prev;
    if(!curr)
    {
      return false;
    }

    Node * next = curr->next_.load(std::memory_order_relaxed);
    Node * corrupted = corrupt_node(next);
    if(!curr->next_.compare_exchange_strong(next, corrupted, std::memory_order_relaxed))
    {
      return false;
    }
    
    tail_.compare_exchange_strong(next, prev, std::memory_order_relaxed);
    counter_.fetch_sub(1, std::memory_order_release);
    return true;
  }


  template<typename U>
  size_t erase(const U& key, Node* it ={})
  {
    bool res = true;
    int count = 0;
    while(res)
    {
      res = remove(key, it);
      count+=res;
    }
    return count;
  }


  [[nodiscard]]
  LF_OrderList 
  cut(Node* it) noexcept
  {
    FreeList_t old_list = FreeList_t::cut(it);
    LF_OrderList temp{};
    temp.merge(std::move(old_list));
    return std::move(temp);
  }

};
