//
// Created by cicerolp on 11/2/17.
//

#pragma once

class AVLTreeInterface {
 public:
  using node_t = int32_t;
  static const uint32_t NIL = 0;

  // grow a size by 1/8
  static int oversize(int size) {
    return size + (size >> 3);
  }

  AVLTreeInterface() : AVLTreeInterface(16) {}

  AVLTreeInterface(size_t capacity) {
    _root = NIL;
    resize(capacity);
  }

  // return the current root of the tree
  inline node_t root() const {
    return _root;
  }

  // return the current capacity, which is the number of nodes that this tree can hold
  inline size_t capacity() const {
    return _parent.size();
  }

  inline size_t size() const {
    // TODO nodeAllocator.size();
    return 0;
  }

  // return the parent of the provided node
  inline node_t parent(node_t node) const {
    return _parent[node];
  }

  // return the left child of the provided node
  inline node_t left(node_t node) const {
    return _left[node];
  }

  // return the right child of the provided node
  inline node_t right(node_t node) const {
    return _right[node];
  }

  // return the depth nodes that are stored below node including itself
  inline node_t depth(node_t node) const {
    return _depth[node];
  }

  // return the least node under node
  node_t first(node_t node) const;

  // return the largest node under node
  node_t last(node_t node) const;

  // return the least node that is strictly greater than node
  node_t next(node_t node) const;

  // return the highest node that is strictly less than node
  node_t prev(node_t node) const;

  // add current data to the tree and return true if a new node was added
  // to the tree or false if the node was merged into an existing node
  bool add();

  // find a node in this tree
  int32_t find() const;

  // update node with the current data
  void update(int32_t node);

  void remove(int32_t node);

 protected:
  // resize internal storage in order to be able to store data for nodes up to capacity (excluded)
  virtual void resize(size_t capacity);

  inline int32_t balanceFactor(node_t node) const {
    return depth(left(node)) - depth(right(node));
  }

  void rebalance(node_t node);

  inline void fixAggregates(node_t node) {
    depth(node, 1 + std::max(depth(left(node)), depth(right(node))));
  }

  // rotate left the subtree under n
  void rotateLeft(int32_t n);

  // rotate right the subtree under n
  void rotateRight(int32_t n);

  // compare data against data which is stored in node
  virtual int32_t compare(node_t node) const = 0;

  // compare data into node
  virtual void copy(node_t node) = 0;

  // merge data into node
  virtual void merge(node_t node) = 0;

 private:
  inline void parent(node_t node, node_t parent) {
    _parent[node] = parent;
  }

  inline void left(node_t node, node_t left) {
    _left[node] = left;
  }

  inline void right(node_t node, node_t right) {
    _right[node] = right;
  }

  inline void depth(node_t node, char depth) {
    _depth[node] = depth;
  }

  inline void release(int32_t node) {
    left(node, NIL);
    right(node, NIL);
    parent(node, NIL);
    // TODO nodeAllocator.release(node);
  }

  inline void swap(int32_t node1, int32_t node2) {
    const int32_t parent1 = parent(node1);
    const int32_t parent2 = parent(node2);

    if (parent1 != NIL) {
      if (node1 == left(parent1)) {
        left(parent1, node2);
      } else {
        right(parent1, node2);
      }
    } else {
      _root = node2;
    }

    if (parent2 != NIL) {
      if (node2 == left(parent2)) {
        left(parent2, node1);
      } else {
        right(parent2, node1);
      }
    } else {
      _root = node1;
    }
    parent(node1, parent2);
    parent(node2, parent1);

    const int32_t left1 = left(node1);
    const int32_t left2 = left(node2);

    left(node1, left2);
    if (left2 != NIL) {
      parent(left2, node1);
    }

    left(node2, left1);
    if (left1 != NIL) {
      parent(left1, node2);
    }

    const int32_t right1 = right(node1);
    const int32_t right2 = right(node2);

    right(node1, right2);
    if (right2 != NIL) {
      parent(right2, node1);
    }

    right(node2, right1);
    if (right1 != NIL) {
      parent(right1, node2);
    }

    const int32_t depth1 = depth(node1);
    const int32_t depth2 = depth(node2);

    depth(node1, depth2);
    depth(node2, depth1);
  }

  uint32_t _root;
  std::vector<node_t> _parent;
  std::vector<node_t> _left;
  std::vector<node_t> _right;
  std::vector<char> _depth;
};
