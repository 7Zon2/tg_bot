#pragma once 
#include "LFS/LF_hazardous.hpp"
#include "LFS/share_resource.hpp"
#include "LF_List.hpp"
#include "TestUtils/thread_handler.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <math.h>
#include <cstring>
#include <optional>
#include <type_traits>

constexpr size_t UPPER_BIT = 0x8000000000000000;  

#define ON_TABLE_PRINT 1

#if(ON_TABLE_PRINT == 1)
#define TABLE_PRINT  
#else 
#define TABLE_PRINT return
#endif


[[nodiscard]]
inline size_t 
log2_64 (size_t value) noexcept //for zero returns zero
{
    static const size_t tab64[64] = 
    {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5
    };

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((value - (value >> 1))*0x07EDD5E59A4E28C2) >> 58];
}


[[nodiscard]]
inline size_t 
reverse_bit_order
(size_t v) noexcept 
{
    auto reverse = [](uint v)
    { 
        // swap odd and even bits
        v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
        // swap consecutive pairs
        v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
        // swap nibbles ... 
        v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
        // swap bytes
        v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
        // swap 2-byte long pairs
        v = ( v >> 16             ) | ( v               << 16);
        
        return v;
    };

    uint  arr[2];
    std::memcpy(&arr, &v, sizeof(size_t) / sizeof(unsigned char));
    uint h  = reverse(arr[1]);
    uint l  = reverse(arr[0]);
    arr[0] = h;
    arr[1] = l;
    std::memcpy(&v, &arr, sizeof(size_t) / sizeof(unsigned char));
    return v;
}


using bit_comporator = decltype([](const size_t lhs, const size_t rhs) noexcept
{
  size_t lhs_ = reverse_bit_order(lhs);
  size_t rhs_ = reverse_bit_order(rhs);
  bool res = lhs_ < rhs_; 
  return res;
});


template<typename T>
requires (std::is_integral_v<T>)
[[nodiscard]]
constexpr size_t msb
(T value) noexcept 
{
  return sizeof(T) * CHAR_BIT - __builtin_clzll(value | 1) - 1;
}


[[nodiscard]]
constexpr size_t
set_upper_bit(size_t v) noexcept
{
  return v | UPPER_BIT; 
}


template<typename T>
struct HashNode
{
  bool isDummy = true; //only read access after initialization
  T data_{};
  size_t hash_{};
  size_t rhash_{};

  std::atomic<bool> is_removed = false; //after inflating some parent buckets may point to the same node

  HashNode(){}

  HashNode(bool isDummy, T&& data, size_t hash):
    isDummy(isDummy), data_(std::forward<T>(data)), hash_(hash)
  {
    if(!isDummy)
    {
      hash = set_upper_bit(hash_);
    }
    rhash_ = reverse_bit_order(hash);
  }

  HashNode(HashNode&& rhs) noexcept (std::is_nothrow_move_constructible_v<T>):
    isDummy(rhs.isDummy),
    data_(std::move(rhs.data_)),
    hash_(rhs.hash_), 
    rhash_(rhs.rhash_){}

  HashNode& operator = (HashNode&& rhs) noexcept (std::is_nothrow_move_assignable_v<T>)
  {
    if(this!=&rhs)
    {
      isDummy = rhs.isDummy;
      hash_ = rhs.hash_;
      rhash_ = rhs.rhash_;
      data_ = std::move(rhs.data_);
    }
    return *this;
  }

  HashNode(const HashNode& rhs) noexcept (std::is_nothrow_copy_constructible_v<T>):
    isDummy(rhs.isDummy),
    data_(rhs.data_),
    hash_(rhs.hash_),
    rhash_(rhs.rhash_){}

  HashNode& operator = (const HashNode& rhs) noexcept (std::is_nothrow_copy_assignable_v<T>)
  {
    if(this!=&rhs)
    {
      HashNode temp{rhs};
      *this  = std::move(temp);
    }
    return *this;
  }
};

template<typename T>
HashNode(bool isDummy, T&& data, size_t hash) -> HashNode<T>;


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
    return std::hash<T>{node};
  }
};


template<typename Key, typename Obj>
class LF_HashTable
{
  public:

  using pair_t = std::pair<Key, Obj>;
  using HashNode_t = HashNode<pair_t>;
  using List_t = LF_OrderList<HashNode_t, bit_comporator>;
  using Node = typename List_t::Node;


  struct find_comporator
  {
    size_t rhash_;
    bool is_dummy_ = false; 

    find_comporator(const size_t hash) noexcept
    {
      rhash_ = reverse_bit_order(hash);
    }

    bool operator()(HashNode_t& node) noexcept 
    {

      is_dummy_ = node.isDummy;
      if(rhash_ < node.rhash_) // a node must be between a parent bucket and next bucket 
      {
        is_dummy_ = true;
        return true; //return true to stop searching in the list
      }

      return rhash_ == node.rhash_;
    }
  };

  struct insert_comporator : find_comporator
  {
    insert_comporator (size_t hash) noexcept :
    find_comporator(hash){}

    bool operator()(HashNode_t& node) noexcept
    {
      bool res = find_comporator::rhash_ < node.rhash_;
      find_comporator::is_dummy_ = node.isDummy;
      return res;
    }
  };


  protected:

  class HT_iterator : private List_t::iterator
  {
    protected:

    using it_t = typename List_t::iterator;
    using it_t::node_;

    public:

    HT_iterator() noexcept {}


    HT_iterator(it_t it) noexcept:
      it_t(it){}


    friend bool operator == (const HT_iterator&, const HT_iterator&) noexcept = default;


    operator bool() noexcept 
    {
      return it_t::operator bool();
    }


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

  struct Bucket
  {
    std::atomic<Node*> bucket_head_;

    public:
      
      using iterator = List_t::iterator;

      Bucket(Node* node) noexcept:
        bucket_head_(node){}


      [[nodiscard]]
      Node* 
      get() const noexcept
      {
        return bucket_head_.load(std::memory_order_relaxed);
      }

      bool insert(List_t& list_, HashNode_t hashNode)
      {
        Node* bucket_head = bucket_head_.load(std::memory_order_relaxed);
        bool before = true;
        Node* node = list_.insert
        (
            std::move(hashNode), 
            bucket_head, 
            before,
            insert_comporator{hashNode.hash_}
        );
        return node;
      }

      bool remove(List_t& list_, const Key& key)
      {
        size_t hash = std::hash<Key>{}(key);
        hash = set_upper_bit(hash);
        auto* parent = bucket_head_.load(std::memory_order_relaxed);
        assert(parent);

        find_comporator comporator{hash};
        auto[prev, hzp] = list_.find_node(key, parent, comporator);
        if(comporator.is_dummy_ || !hzp)
        {
          return false;
        }

        Hazardous::hazard_pointer& p = hzp;
        Node * node = static_cast<Node*>(p->data_.load(std::memory_order_relaxed));
        HashNode_t& hnode = node->data_.second;
        bool remove = false;
        if(!hnode.is_removed.compare_exchange_strong(remove,true,std::memory_order_relaxed))
        {
          return false;
        }

        bool res = list_.remove(prev);
        return res;
      }
  };

  protected:

  List_t list_;
  size_t load_factor_;

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
  size_t get_hash(const Key& key) const noexcept 
  {
    return std::hash<Key>{}(key);
  }


  [[nodiscard]]
  size_t get_index
  (size_t table_index) const noexcept
  {
    size_t segment_index = get_table_segment(table_index);
    if(segment_index == 0) // only 0
    {
      return 0;
    }
    if(segment_index == 1) //1 or 2
    {
      return table_index - 1;
    }
    size_t table_size = get_table_size_by_table_index(table_index);
    long long index = table_index - table_size - 1;
    assert(index >= 0);
    return index;
  }


  [[nodiscard]]
  size_t get_global_size
  (size_t segment_index) const noexcept
  {
    return 1<<segment_index;
  }


  //returns global table index if the table would be one array 
  [[nodiscard]]
  size_t get_global_index   
  (size_t hash, size_t segment_index) const noexcept 
  {
    size_t size = get_global_size(segment_index);
    size_t mod = hash % size;
    return mod;
  }


  [[nodiscard]]
  segment& get_table
  (size_t table_index) const noexcept
  {
    size_t segment_index = get_table_segment(table_index);
    return *get_segment(segment_index);
  }

 
  [[nodiscard]]
  constexpr size_t 
  get_parent_index(size_t nBucket) noexcept
  {
      if(nBucket == 1 || nBucket == 0)
      {
        return 0;
      }
      return nBucket & ~( 1 << msb( nBucket ) );
  } 


  [[nodiscard]]
  Bucket* 
  get_parent_bucket
  (const size_t hash, size_t segment_index) noexcept
  {
    if(segment_index == 0)
    {
      auto & table = *get_segment(0);
      return table[0];
    }

    struct LocalIndexes
    {
      size_t parent_index = 0;
      size_t segment_index = 0;
    };

    auto take_parent_bucket = [&](size_t parent_index)
    {
      segment& table = get_table(parent_index); // local table of that parent
      size_t index = get_index(parent_index); // index of that parent in that local table
      Bucket* bucket = table[index].load(std::memory_order_relaxed); //its bucket
      return bucket;
    };

    std::vector<LocalIndexes> vec;
    size_t parent_index = get_global_index(hash, segment_index);
    Bucket* bucket  = take_parent_bucket(parent_index);
    vec.push_back(LocalIndexes(parent_index, segment_index));
    
    while(!bucket) //search for the first initialized bucket
    {
      parent_index = get_parent_index(parent_index);
      bucket = take_parent_bucket(parent_index);
      vec.push_back(LocalIndexes(parent_index, segment_index));
    }


    for(int i = vec.size(); i!=0; i--) // initializes all new parent buckets with their previous parent. 
                                                   // if previous parent equal the next parent then nothing happens
    {
      size_t id = i-1;
      parent_index = vec[id].parent_index;
      segment_index = vec[id].segment_index;
      auto& table = get_table(parent_index);
      size_t index = get_index(parent_index);
      bucket = insert_parent_bucket(table, segment_index, index, parent_index, bucket);
    }

    assert(bucket);
    return bucket;
  }


  [[nodiscard]]
  Bucket* 
  insert_parent_bucket
  (segment& table, const size_t segment_index, const size_t index, const size_t parent_hash, Bucket* parent_bucket)
  {
    Bucket* old_bucket = table[index].load(std::memory_order_acquire);
    if(old_bucket) //if the parent bucket is already in the table - it is already in the list 
    {
      return old_bucket;
    }


    Node* parent = parent_bucket->get();
    Bucket* new_bucket = new Bucket{parent}; // on time we will lie)

    //if two or more threads inserted the same node we need to remove redundant
    if(!table[index].compare_exchange_strong
        (old_bucket, new_bucket, std::memory_order_release, std::memory_order_release))
    {
      delete new_bucket;
      return parent_bucket;
    }


    bool before = true; // we need to insert a new node before another one 
                        // that we will find (due to the order list insertion) 

    Node* insert_node{};
    while(!insert_node)
    {
      HashNode_t hnode{true, pair_t {parent_hash, Obj{}}, parent_hash};
      insert_node = list_.insert(std::move(hnode), parent, before, insert_comporator(parent_hash));
    }

    new_bucket->bucket_head_.store(insert_node, std::memory_order_relaxed);
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
  get_table_size_by_table_index(size_t table_index) const noexcept 
  {
    size_t segment_index = get_table_segment(table_index);
    size_t table_size =  get_table_size_by_segment(segment_index);
    return table_size;
  }
  

  [[nodiscard]]
  size_t 
  get_table_size_by_segment
  (size_t segment_index) const noexcept 
  {
    if(segment_index == 0)
    {
      return 1;
    }
    if(segment_index == 1)
    {
      return 2;
    }

    return 1 << (segment_index - 1);
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

    bool ap = __builtin_popcount(table_index) - 1;
    size_t segment_index =  log2_64(table_index) + ap;
    return segment_index;
  }


  [[nodiscard]]
  bool
  check_load_factor() const noexcept
  {
    size_t seg = current_segment_.load(std::memory_order_relaxed);
    size_t sz = get_global_size(seg);
    return check_load_factor(sz);
  }


  [[nodiscard]]
  bool
  check_load_factor(const size_t sz) const noexcept
  {
    size_t counter = list_.size();
    return (counter / sz) > load_factor_;
  }


  bool 
  inflate_table()
  {
    size_t old_index = current_segment_.load(std::memory_order_acquire);
    if(!check_load_factor())
    {
      return false;
    }

    PRINT_2("\nINFLATE_TABLE\n");
    PRINT_2("counter:", list_.size(),"\n");

    size_t new_index = old_index + 1;
    size_t table_size = get_table_size_by_segment(new_index);
    segment * new_table = new segment(table_size, &ShareResource::res_);
    segment * table{};
    if(!seg_arr_[new_index].compare_exchange_strong(table, new_table, std::memory_order_acq_rel))
    {
      delete new_table;
      return false;
    }

    current_segment_.store(new_index, std::memory_order_release);
    return true;
  }


  void allocate()
  {
    segment * table = new segment(1, &ShareResource::res_);

    pair_t pa{Key{},Obj{}};
    HashNode_t hash_node{true, std::move(pa),0};

    Node* node = list_.insert(std::move(hash_node));
    node->data_.first = 0;

    Bucket* new_bucket = new Bucket(node);

    segment& vec = *table;
    vec[0].store(new_bucket, std::memory_order_relaxed);
    seg_arr_[0].store(table, std::memory_order_release);
  }

  public:

  using iterator = HT_iterator;

  LF_HashTable(const size_t load_factor = 2):
    load_factor_(load_factor), current_segment_(0)
  {
    allocate();
  }


  virtual 
  ~LF_HashTable()
  {
    clear();
    segment* seg = seg_arr_[0].exchange(nullptr,std::memory_order_relaxed);
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
  }

  public:

  [[nodiscard]]
  size_t size() const noexcept
  {
    return list_.size();
  }


  [[nodiscard]]
  size_t get_hash
  (const HashNode_t& node) const noexcept
  {
    return std::hash<HashNode_t>{node};
  }


  template<typename T, typename U>
  bool insert(T&& key, U&& value)
  {
    inflate_table();

    if(find(key))
    {
      return false;
    }

    size_t hash  = get_hash(key);
    Bucket* parent_bucket = get_parent_bucket(hash, current_segment_.load(std::memory_order_relaxed));
    
    pair_t pa{std::forward<T>(key), std::forward<U>(value)};
    HashNode_t hashNode{false, std::move(pa), hash};

    bool res = parent_bucket->insert(list_, std::move(hashNode));
    return res;
  }


  template<typename T>
  bool remove(const T& key)
  {
    size_t hash = get_hash(key);
    size_t segment_index = current_segment_.load(std::memory_order_relaxed);

    Bucket * bucket = get_parent_bucket(hash, segment_index);
    return bucket->remove(list_, key);
  }


  [[nodiscard]]
  bool 
  find(const Key& key)
  {
    size_t hash = get_hash(key);
    size_t segment_index = current_segment_.load(std::memory_order_relaxed);

    Bucket* bucket = get_parent_bucket(hash, segment_index);
    Node * parent = bucket->get();

    hash = set_upper_bit(hash);
    find_comporator comporator{hash};
    bool res = list_.find(key, parent, comporator);
    if(comporator.is_dummy_)
    {
      return false;
    }
    return res;
  }


  [[nodiscard]]
  std::optional<Obj> 
  contain(const Key& key)
  {
    size_t hash = get_hash(key);
    size_t segment_index = current_segment_.load(std::memory_order_relaxed);

    Bucket* bucket = get_parent_bucket(hash, segment_index);
    Node* parent = bucket->get();

    hash = set_upper_bit(hash);
    find_comporator comporator{hash};
    std::optional<HashNode<pair_t>> opt = list_.contain(key, parent, comporator);
    if(opt.has_value() && (comporator.is_dummy_ == false))
    {
      HashNode_t& hn = opt.value();
      return hn.data_.second;
    }
    return {};
  }

 
  void clear()
  {
    list_.clear();
    for(size_t i = 0; i < seg_arr_.size(); i++)
    {
      auto * seg = seg_arr_[i].exchange(nullptr, std::memory_order_relaxed);
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
    current_segment_.store(0, std::memory_order_relaxed);
    allocate();
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
