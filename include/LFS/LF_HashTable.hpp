#pragma once 
#include "LFS/share_resource.hpp"
#include "LF_List.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <math.h>
#include <byteswap.h>
#include <optional>

[[nodiscard]]
size_t 
constexpr inline reverse_bit_order
(size_t x) noexcept 
{
  return bswap_64(x);
}


[[nodiscard]]
constexpr size_t
inline msb
(size_t value) noexcept 
{
  return 64 - __builtin_clzll(value);
}


template<typename T>
struct HashNode
{
  bool isDummy = true;
  T data_;
};


template<typename T>
requires requires(T&& data)
{
  typename T::first_type;
  typename T::second_type;
}
struct std::hash<HashNode<T>>
{
  constexpr 
  size_t operator()(const HashNode<T>& node) const noexcept
  {
    return std::hash<typename T::first_type>{}(node.data_.first);
  }
};


template<typename T>
requires requires(T&& data)
{
  std::hash<T>{}(data);
}
struct std::hash<HashNode<T>>
{
  constexpr 
  size_t operator()(const HashNode<T>& node) const noexcept 
  {
    return std::hash<T>{data};
  }
};


template<typename Key, typename Obj>
class LF_HashTable
{
  public:

  using comporator_type = decltype([](const size_t lhs, const size_t rhs) noexcept
  {
    return reverse_bit_order(lhs) <= reverse_bit_order(rhs);
  });

  using pair_t = std::pair<Key, Obj>;
  using HashNode_t = HashNode<pair_t>;
  using List_t = LF_OrderList<HashNode_t, comporator_type>;
  using Node = typename List_t::Node;

  protected:

  class HT_iterator : private List_t::iterator
  {
    protected:

    using it_t = typename List_t::iterator;
    using it_t::node_;

    public:

    HT_iterator(){}

    HT_iterator(it_t it):
      it_t(it){}

    friend bool operator == (const HT_iterator& it, const Node* node) noexcept
    {
      return it.node_;
    }

    friend bool operator == (const HT_iterator&, const HT_iterator&) noexcept = default;

    public:

    using iterator_category = typename it_t::iterator_category;
    using value_type = pair_t;
    using pointer = pair_t*;
    using reference = pair_t&;
    using difference_type = typename it_t::difference_type;
    
    public:

    HT_iterator& operator ++() noexcept
    {
      it_t::operator++();
      return *this;
    }


    HT_iterator operator ++(int) noexcept
    {
      HT_iterator temp{*this};
      it_t::operator++();
      return temp;
    }


    reference operator*() const noexcept
    {
      HashNode_t& node = it_t::operator*();
      return node.data_;
    }


    pointer operator->() const noexcept
    {
      HashNode_t& node =  it_t::operator*();
      return &node.data_;
    }
  };

  static_assert(std::forward_iterator<HT_iterator>);

  protected:

  class Bucket
  {
    std::atomic<Node*> bucket_head_;
    std::atomic<size_t> bucket_count_;
    size_t bucket_hash_;

    public:
      
      using iterator = List_t::iterator;

      Bucket(Node* node, size_t hash) noexcept:
        bucket_head_(node),
        bucket_count_(0),
        bucket_hash_(hash)
        {}


      [[nodiscard]]
      Node* 
      get() const noexcept
      {
        return bucket_head_.load(std::memory_order_relaxed);
      }

      [[nodiscard]]
      size_t 
      count() const noexcept
      {
        return bucket_count_.load(std::memory_order_relaxed);
      }

      void 
      reset_count() noexcept
      {
        bucket_count_.store(0, std::memory_order_relaxed);
      }

      [[nodiscard]]
      size_t 
      hash() const noexcept
      {
        return bucket_hash_;
      }

      bool insert(List_t& list_, HashNode_t hashNode)
      {
        bool before = true;
        Node* node = list_.insert(std::move(hashNode), bucket_head_.load(std::memory_order_relaxed), before);
        bucket_count_.fetch_add(static_cast<bool>(node), std::memory_order_release); 
        return node;
      }

      bool remove(List_t& list_, const Key& key)
      {
        int count = list_.erase(key, bucket_head_.load(std::memory_order_relaxed));
        bucket_count_.fetch_sub(count, std::memory_order_release);
        return count;
      }
  };

  protected:

  List_t list_;
  size_t load_factor_;
  std::atomic<int> counter_; 

  using segment = std::pmr::vector<std::atomic<Bucket*>>;
  std::array<std::atomic<segment*>, 64> seg_arr_;
  std::atomic<size_t> current_segment_;

  protected:

  [[nodiscard]]
  segment* 
  get_segment(const size_t segment_index) const noexcept
  {
    segment * seg;
    do
    {
      seg = seg_arr_[segment_index].load(std::memory_order_relaxed);
    }while(!seg);// in the inflate method this segment may be not allocated yet
    
    return seg;
  }


  [[nodiscard]]
  size_t
  get_hash(const Key& key) const noexcept 
  {
    return std::hash<Key>{}(key);
  }


  [[nodiscard]]
  size_t 
  get_index
  (const size_t hash, size_t segment_index) const noexcept
  {
    size_t table_index = get_table_index(hash, segment_index);
    segment_index = get_table_segment(table_index);
    size_t table_size = get_table_size(segment_index);
    size_t offset = get_table_offset(table_index);
    size_t index = table_size - offset -1;
    return index;
  }


  [[nodiscard]]
  size_t 
  get_index
  (const size_t table_index)
  {
    return get_index(table_index, current_segment_.load(std::memory_order_relaxed));
  }


  [[nodiscard]]
  segment& get_table
  (const size_t hash,size_t segment_index) const noexcept
  {
    size_t table_index = get_table_index(hash, segment_index);
    segment_index = get_table_segment(table_index);
    return *get_segment(segment_index);
  }

  
  [[nodiscard]]
  segment& get_table
  (size_t table_index) const noexcept 
  {
    return get_table(table_index, current_segment_.load(std::memory_order_relaxed));
  }


  [[nodiscard]]
  Bucket* 
  get_parent_bucket
  (const size_t modulo, size_t segment_index) noexcept
  {
    struct LocalIndexes
    {
      size_t parent_index = 0;
      size_t segment_index = 0;
    };

    auto find_parent_bucket = [&](size_t parent_index, size_t segment_index)
    {
      auto& table = get_table(parent_index, segment_index);
      size_t index = get_index(parent_index, segment_index);
      Bucket* bucket = table[index].load(std::memory_order_relaxed);
      return bucket;
    };

    std::vector<LocalIndexes> vec;

    auto& table = get_table(modulo, segment_index);
    size_t index = get_index(modulo, segment_index);
    Bucket* bucket = table[index].load(std::memory_order_relaxed);

    vec.push_back(LocalIndexes(modulo, segment_index));
    
    size_t parent_index = modulo;
    while(!bucket) //search for the first initialized bucket
    {
      parent_index = get_table_index(parent_index, segment_index);
      bucket = find_parent_bucket(parent_index, --segment_index);
      vec.push_back(LocalIndexes(parent_index, segment_index));
    }


    for(auto indexes : vec | std::views::reverse) // initializes all new parent buckets with their previous parent. 
                                                   // if previous parent equal the next parent then nothing happens
    {
      parent_index = indexes.parent_index;
      segment_index = indexes.segment_index;
      auto& table = get_table(parent_index);
      index = get_index(parent_index);
      bucket = insert_parent_bucket(table, segment_index, index, parent_index, bucket);
    }

    assert(bucket && parent_index == modulo);
    return bucket;
  }


  [[nodiscard]]
  Bucket* 
  insert_parent_bucket
  (segment& table, const size_t segment_index, const size_t index, const size_t parent_hash, Bucket* parent)
  {
    Bucket* old_bucket = table[index].load(std::memory_order_acquire);
    if(old_bucket) //if the parent bucket is already in the table - it is already in the list 
    {
      return old_bucket;
    }

    Node* insert_node{};
    
    bool before = true; // we need to insert a new node before another one 
                        // that we will find (due to order list insertion) 

    Bucket* new_bucket{};
    insert_node = list_.insert(HashNode_t{true, pair_t{parent_hash, Obj{}}}, parent->get(), before);
    if(!insert_node)
    {
      while(!new_bucket)
      {
        new_bucket = table[index].load(std::memory_order_acquire);
      }
      return new_bucket;
    }

    new_bucket = new Bucket(insert_node, parent_hash);

    //if two or more threads inserted the same node we need to remove redundant
    if(!table[index].compare_exchange_strong
        (old_bucket, new_bucket, std::memory_order_release, std::memory_order_relaxed))
    {
      delete new_bucket;
    }

    return table[index].load(std::memory_order_relaxed);
  }


  [[nodiscard]]
  size_t 
  get_table_size() const noexcept
  {    
    return get_table_size(current_segment_.load(std::memory_order_relaxed));
  }


  [[nodiscard]]
  size_t 
  get_table_size
  (const size_t segment_index) const noexcept 
  {
    return 1 << segment_index;
  }


  [[nodiscard]]
  size_t
  get_table_index
  (const size_t hash, const size_t segment_index) const noexcept 
  {
    size_t table_size = get_table_size(segment_index);
    size_t res = hash % table_size;
    return res;
  }


  [[nodiscard]]
  size_t
  get_table_segment
  (const size_t table_index) const noexcept
  {
    if(table_index == 0)
    {
      return 0;
    }
    if(table_index == 1)
    {
      return 1;
    }
    return std::ceil(std::log2(table_index));
  }


  [[nodiscard]]
  size_t 
  get_table_offset
  (const size_t table_index) const noexcept
  {
    if(table_index == 0)
    {
      return 0;
    }
    size_t segment_index = get_table_segment(table_index);
    size_t table_size = get_table_size(segment_index);
    size_t dif = table_size - table_index;
    return dif;
  }


  [[nodiscard]]
  bool
  check_load_factor() const noexcept
  {
    size_t sz = get_table_size();
    return (list_.size() + 1) / sz > load_factor_;
  }


  [[nodiscard]]
  bool
  check_load_factor(const size_t sz) const noexcept
  {
    return list_.size() /sz > load_factor_;
  }


  bool 
  inflate_table()
  {
    size_t old_index = current_segment_.load(std::memory_order_relaxed);
    if(!check_load_factor())
    {
      return false;
    }
    
    if(!current_segment_.compare_exchange_strong(old_index, old_index+1, std::memory_order_relaxed))
    {
      return false;
    }
    
    size_t new_index = old_index + 1;
    size_t table_size = get_table_size(new_index);
    segment* table = seg_arr_[new_index].load(std::memory_order_acquire);
    assert(!table);
    table = new segment(table_size, &ShareResource::res_);
    seg_arr_[new_index].store(table, std::memory_order_release);
    return true;
  }

  public:

  using iterator = HT_iterator;

  LF_HashTable(const size_t load_factor = 2):
    load_factor_(load_factor), current_segment_(0)
  {
    segment  * table = seg_arr_[0].load(std::memory_order_acquire);
    table = new segment(1, &ShareResource::res_);

    pair_t pa{Key{},Obj{}};
    HashNode_t hash_node{true, std::move(pa)};

    Node* node = list_.insert(hash_node);
    node->data_.first = 0;

    Bucket* new_bucket = new Bucket(node, 0);

    segment& vec = *table;
    vec[0].store(new_bucket, std::memory_order_relaxed);

    seg_arr_[0].store(table, std::memory_order_release);
  }


  virtual 
  ~LF_HashTable()
  {
    segment* seg = seg_arr_[0].exchange(nullptr,std::memory_order_relaxed);
    if(!seg)
    {
      return;
    }
   
    segment& table = *seg;
    for(size_t i = 0; i < table.size(); i++)
    {
      Bucket* bucket = table[i].load(std::memory_order_relaxed);
      if(bucket)
      {
        delete bucket;
      }
    }

    delete seg;
    size_t sz = seg_arr_.size();

    for(size_t i = 1; i <sz; i++)
    {
      seg = seg_arr_[i].load(std::memory_order_relaxed);
      if(seg)
      {
        segment& table = *seg;
        for(size_t j = 0; j < table.size(); j++)
        {
          Bucket* bucket = table[j].load(std::memory_order_relaxed);
          if(bucket)
          {
            delete bucket;
          }
        }
        delete seg;
      }
    }
  }

  public:

  [[nodiscard]]
  size_t size() const noexcept
  {
    return counter_.load(std::memory_order_acquire);
  }


  template<typename T, typename U>
  bool insert(T&& key, U&& value)
  {
    if(find(key))
    {
      return false;
    }

    size_t hash  = get_hash(key);
    inflate_table();
    Bucket* parent_bucket = get_parent_bucket(hash, current_segment_.load(std::memory_order_relaxed));
    
    pair_t pa{std::forward<T>(key), std::forward<U>(value)};
    HashNode_t hashNode{false, std::move(pa)};

    bool res = parent_bucket->insert(list_, std::move(hashNode));
    counter_.fetch_add(res, std::memory_order_release);
    return res;
  }


  template<typename T>
  bool remove(const T& key)
  {
    size_t hash = get_hash(key);
    size_t segment_index = current_segment_.load(std::memory_order_relaxed);

    Bucket * bucket = find_bucket(hash, segment_index);

    bool res = list_.erase(key, bucket->get());
    counter_.fetch_sub(res, std::memory_order_release);
    return res;
  }


  [[nodiscard]]
  Bucket* 
  get_bucket(size_t hash, size_t segment_index) noexcept
  {
    auto& table = get_table(hash, segment_index);
    size_t index = get_index(hash, segment_index);
    return table[index].load(std::memory_order_relaxed);
  }
  

  [[nodiscard]]
  Bucket* 
  find_bucket(size_t hash, int segment_index) noexcept 
  {
    Bucket* bucket{};
    while(!bucket && (segment_index>=0))
    {
      bucket = get_bucket(hash, segment_index);
      --segment_index;
    }

    return bucket;
  }


  [[nodiscard]]
  bool 
  find(const Key& key)
  {
    size_t hash = get_hash(key);
    size_t segment_index = current_segment_.load(std::memory_order_relaxed);
    Bucket* bucket = find_bucket(hash, segment_index);

    std::optional<HashNode_t> opt = list_.contain(key, bucket->get());
    if(opt.has_value() && (opt.value().isDummy == false))
    {
      return true;
    }
    return false;
  }


  [[nodiscard]]
  std::optional<Obj> 
  contain(const Key& key)
  {
    size_t hash = get_hash(key);
    size_t segment_index = current_segment_.load(std::memory_order_relaxed);
    
    Bucket* bucket = find_bucket(hash, segment_index);

    std::optional<HashNode<pair_t>> opt = list_.contain(hash, bucket->get());
    if(opt.has_value() && (opt.value().isDummy==false))
    {
      HashNode_t& hn = opt.value();
      return hn.data_.second;
    }
    return {};
  }

 
  void clear()
  {
    for(auto& node: list_)
    {
      const Key& key = node.data_.first;
      remove(key);
    }
    counter_.store(0, std::memory_order_release);
  }


  Bucket* get_bucket(const Key& key) 
  {
    size_t hash = get_hash(key);
    return get_parent_bucket(hash, current_segment_.load(std::memory_order_relaxed));
  }


  auto begin() noexcept
  {
    return HT_iterator(list_.begin());
  }

  
  auto end() noexcept
  {
    return HT_iterator(list_.end());
  }
};
