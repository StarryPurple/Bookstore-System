/** blinktree.h
 * Author: StarryPurple
 * Date: Since 2024.12.17
 *
 * Includes some useful data structures;
 *
 * BLink Tree:
 *   A concurrency-friendly data structure widely used in file io.
 *   supports basic functions of a key-value map, similar to std::multimap.
 *
 */

#pragma once
#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include "filestream.h"

#include <vector>
#include <utility>

namespace StarryPurple {

// degree for the maximum size of a node
// node_size should be in [degree / 2 - 1, degree - 1]
// no ValueType is directly used. we only reads and passes fpointer of ValueType.
template<class KeyType, class ValueType, size_t degree = 1 << 7, size_t elementCount = cElementCount>
class BLinkTree {

  static_assert(degree >= 6); // used in erase(...)

  struct MapNode;
  struct VlistNode;

public:

  using KeyPtr = typename Fstream<KeyType, size_t, elementCount>::fpointer; // ptr of KeyType node
  // using ValuePtr = typename Fstream<ValueType, size_t, elementCount>::fpointer; // ptr of ValurType node
  using VlistPtr = typename Fstream<VlistNode, size_t, elementCount>::fpointer; // ptr of VlistNode
  using MnodePtr = typename Fstream<MapNode, size_t, elementCount>::fpointer; // ptr of MapNode

  BLinkTree() = default;
  ~BLinkTree(); // remember to close files.
  // the route to upper_bound result.
  // if tree is empty, returns an empty vector.
  // if finding fails (key too big), the result end with {leaf_ptr, degree + 1}
  std::vector<std::pair<MnodePtr, size_t>> upper_bound_route(const KeyType &key);
  // the route to lower_bound result.
  // if tree is empty, returns an empty vector.
  // if finding fails (key too small), the result is {leaf_ptr, degree + 1}
  std::vector<std::pair<MnodePtr, size_t>> lower_bound_route(const KeyType &key);


  // open the three BLinkTree files.
  // naming rule: with parameter @filename, the names of files would be
  //     filename_map_index.dat, filename_map_key.dat, filename_map_val.dat
  void open(const filenameType &map_filename, const filenameType &key_filename, const filenameType &vlist_filename);
  // close BLinkTree files
  void close();

  void insert(const KeyType &key, const ValueType &value);
  // returns the list of value with the provided key.
  std::vector<ValueType> find(const KeyType &key);
  void erase(const KeyType &key, const ValueType &value);

  // todo: iterator
private:

  void merge(const MnodePtr &parent_ptr, size_t left_pos); // [1, 2], [5] --> [1, 2, 5]
  void move_from_left(const MnodePtr &parent_ptr, size_t left_pos); // [1, 2, 3, 4], ->[5] --> [1, 2, 3], ->[4, 5]
  void move_from_right(const MnodePtr &parent_ptr, size_t left_pos); // ->[1], [3, 4, 5, 6] --> ->[1, 3], [4, 5, 6]
  void split(const MnodePtr &parent_ptr, size_t pos); // ->[1, 2, 3, 4, 5] --> ->[1, 2], [3, 4, 5]

  struct MapNode {

    MapNode() = default;

    bool is_leaf_ = false;
    size_t node_size_ = 0;
    KeyType high_key_{};
    MnodePtr parent_ptr_{}, link_ptr_{};
    KeyPtr key_ptr_[degree + 1]{};
    MnodePtr mnode_ptr_[degree + 1]{}; // nullptr for is_leaf_ = true
    VlistPtr vlist_ptr_[degree + 1]{}; // the begin of the list. nullptr for is_leaf_ = false
    // todo: reduce space cost by using union or std::variant<MnodePtr, VlistPtr>
  };
  // a link-list of values.
  // arranged in order (operator<)
  struct VlistNode {

    VlistNode() = default;
    VlistNode(const ValueType &value, const VlistPtr &next_ptr);
    ~VlistNode() = default;

    ValueType value_{};
    VlistPtr next_ptr_{};
  };

  Fstream<MapNode, MnodePtr, elementCount> map_fstream_;
  Fstream<KeyType, size_t, elementCount> key_fstream_;
  Fstream<VlistNode, size_t, elementCount> vlist_fstream_;
  MnodePtr root_ptr_{};
  bool is_open = false;
};

} // namespace StarryPurple

#endif // DATA_STRUCTURE_H