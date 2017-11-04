//
// Created by cicerolp on 11/2/17.
//
#include "stdafx.h"
#include "AVLTree.h"

void AVLTree::resize(size_t capacity) {
  _parent.resize(capacity);
  _left.resize(capacity);
  _right.resize(capacity);
  _depth.resize(capacity);
}
int32_t AVLTree::first(int32_t node) const {
  if (node == NIL) {
    return NIL;
  }

  while (true) {
    const int32_t l = left(node);
    if (l == NIL) {
      break;
    }
    node = l;
  }
  return node;
}

int32_t AVLTree::last(int32_t node) const {
  while (true) {
    const int32_t r = right(node);

    if (r == NIL) {
      break;
    }
    node = r;
  }
  return node;
}

int32_t AVLTree::next(int32_t node) const {
  const int32_t r = right(node);

  if (r != NIL) {
    return first(r);

  } else {
    int32_t p = parent(node);
    while (p != NIL && node == right(p)) {
      node = p;
      p = parent(p);
    }
    return p;
  }
}

int32_t AVLTree::prev(int32_t node) const {
  const int32_t l = left(node);

  if (l != NIL) {
    return last(l);

  } else {
    int32_t p = parent(node);

    while (p != NIL && node == left(p)) {
      node = p;
      p = parent(p);
    }
    return p;
  }
}
bool AVLTree::add() {
  if (_root == NIL) {
    // TODO root = nodeAllocator.newNode();
    copy(_root);
    fixAggregates(_root);
    return true;

  } else {
    int32_t node = _root;
    int32_t p, cmp;

    do {
      cmp = compare(node);

      if (cmp < 0) {
        p = node;
        node = left(node);

      } else if (cmp > 0) {
        p = node;
        node = right(node);

      } else {
        merge(node);
        return false;
      }
    } while (node != NIL);

    // TODO node = nodeAllocator.newNode()
    if (node >= capacity()) {
      resize(oversize(node + 1));
    }

    copy(node);
    parent(node, p);

    if (cmp < 0) {
      left(p, node);
    } else {
      right(p, node);
    }

    rebalance(node);

    return true;
  }
}

void AVLTree::rebalance(int32_t node) {
  for (auto n = node; n != NIL;) {
    const int32_t p = parent(n);

    fixAggregates(n);

    switch (balanceFactor(n)) {
      case -2: {
        const int32_t r = right(n);
        if (balanceFactor(r) == 1) {
          rotateRight(r);
        }
        rotateLeft(n);

        break;
      }
      case 2: {
        const int32_t l = left(n);
        if (balanceFactor(l) == -1) {
          rotateLeft(l);
        }
        rotateRight(n);
        break;
      }

      case -1: break;
      case 0: break;
      case 1: break;
    }

    n = p;
  }
}
void AVLTree::rotateLeft(int32_t n) {
  const int32_t r = right(n);

  const int32_t lr = left(r);
  right(n, lr);

  if (lr != NIL) {
    parent(lr, n);
  }

  const int32_t p = parent(n);
  parent(r, p);

  if (p == NIL) {
    _root = r;

  } else if (left(p) == n) {
    left(p, r);

  } else {
    right(p, r);
  }

  left(r, n);
  parent(n, r);
  fixAggregates(n);
  fixAggregates(parent(n));
}

void AVLTree::rotateRight(int32_t n) {
  const int32_t l = left(n);

  const int32_t rl = right(l);
  left(n, rl);

  if (rl != NIL) {
    parent(rl, n);
  }

  const int32_t p = parent(n);
  parent(l, p);

  if (p == NIL) {
    _root = l;

  } else if (right(p) == n) {
    right(p, l);

  } else {
    left(p, l);
  }

  left(l, n);
  parent(n, l);
  fixAggregates(n);
  fixAggregates(parent(n));
}

int32_t AVLTree::find() const {
  for (int32_t node = _root; node != NIL;) {
    const int32_t cmp = compare(node);

    if (cmp < 0) {
      node = left(node);

    } else if (cmp > 0) {
      node = right(node);

    } else {
      return node;
    }
  }
  return NIL;
}

void AVLTree::update(int32_t node) {
  const int32_t pr = prev(node);
  const int32_t ne = next(node);

  if ((pr == NIL || compare(pr) > 0) && (ne == NIL || compare(ne) < 0)) {
    // update can be done in-place
    copy(node);

    for (int n = node; n != NIL; n = parent(n)) {
      fixAggregates(n);
    }

  } else {
    // TODO: it should be possible to find the new node position without
    // starting from scratch
    remove(node);
    add();
  }
}

void AVLTree::remove(int32_t node) {
  if (left(node) != NIL && right(node) != NIL) {
    // inner node
    const int32_t ne = next(node);
    swap(node, ne);
  }

  const int32_t p = parent(node);
  int child = left(node);

  if (child == NIL) {
    child = right(node);
  }

  if (child == NIL) {
    // no children
    if (node == _root) {
      _root = NIL;

    } else {
      if (node == left(p)) {
        left(p, NIL);

      } else {
        right(p, NIL);
      }
    }

  } else {
    // one single child
    if (node == _root) {
      _root = child;

    } else if (node == left(p)) {
      left(p, child);

    } else {
      right(p, child);
    }
    parent(child, p);
  }

  release(node);
  rebalance(p);
}
