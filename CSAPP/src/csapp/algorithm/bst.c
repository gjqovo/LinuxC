#include <assert.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/algorithm.h"
#include "../headers/common.h"

// We should know that "i_node" is only used for performance polymorphism

void rbt_validate_interface(rbtree_node_interface *i_node, uint64_t flags) {
  assert(i_node != NULL);
  if ((flags & IRBT_CONSTRUCT) != 0) {
    assert(i_node->construct_node != NULL);
  }
  if ((flags & IRBT_DESTRUCT) != 0) {
    assert(i_node->destruct_node != NULL);
  }
  if ((flags & IRBT_CHECKNULL) != 0) {
    assert(i_node->is_null_node != NULL);
  }
  if ((flags & IRBT_COMPARE) != 0) {
    assert(i_node->compare_nodes != NULL);
  }
  if ((flags & IRBT_PARENT) != 0) {
    assert(i_node->get_parent != NULL);
    assert(i_node->set_parent != NULL);
  }
  if ((flags & IRBT_LEFT) != 0) {
    assert(i_node->get_leftchild != NULL);
    assert(i_node->set_leftchild != NULL);
  }
  if ((flags & IRBT_RIGHT) != 0) {
    assert(i_node->get_rightchild != NULL);
    assert(i_node->set_rightchild != NULL);
  }
  if ((flags & IRBT_COLOR) != 0) {
    assert(i_node->get_color != NULL);
    assert(i_node->set_color != NULL);
  }
  if ((flags & IRBT_KEY) != 0) {
    assert(i_node->get_key != NULL);
    assert(i_node->set_key != NULL);
  }
}

void bst_internal_setchild(uint64_t parent, uint64_t child, child_t direction,
                           rbtree_node_interface *i_node) {
  rbt_validate_interface(i_node, IRBT_COMPARE | IRBT_CHECKNULL | IRBT_LEFT | IRBT_RIGHT);
  switch(direction) {
    case LEFT_CHILD:
      i_node->set_leftchild(parent, child);
      if (i_node->is_null_node(child) == 0) {
        i_node->set_parent(child, parent);
      }
      break;
    case RIGHT_CHILD:
      i_node->set_rightchild(parent, child);
      if (i_node->is_null_node(child) == 0) {
        i_node->set_parent(child, parent);
      }
      break;
    default:
      assert(0);
  }
}

void bst_internal_replace(uint64_t victim, uint64_t node,
                          rbtree_internal_t *tree,
                          rbtree_node_interface *i_node) {
  rbt_validate_interface(i_node, IRBT_COMPARE | IRBT_CHECKNULL | IRBT_PARENT |
                                     IRBT_LEFT | IRBT_RIGHT);
  assert(i_node->is_null_node(victim) == 0);
  assert(i_node->is_null_node(tree->root));

  uint64_t v_parent = i_node->get_parent(victim);
  if (i_node->compare_nodes(tree->root, victim) == 0) { // the same
    // victim is root
    assert(i_node->is_null_node(v_parent) == 1);
    tree->update_root(tree, node);
    i_node->set_parent(node, NULL_ID);
    return;
  } else {
    // victim has parent
    uint64_t v_parent_left = i_node->get_leftchild(v_parent);
    uint64_t v_aprent_right = i_node->get_rightchild(v_parent);

    if (i_node->compare_nodes(v_parent_left, victim) == 0) {
      bst_internal_setchild(v_parent, node, LEFT_CHILD, i_node);
      return;
    } else if (i_node->compare_nodes(victim, v_aprent_right) == 0) {
      bst_internal_setchild(v_parent, node, RIGHT_CHILD, i_node);
      return;
    } else {
      assert(0);
    }
  }
}

void bst_internal_insert(rbtree_internal_t *tree, rbtree_node_interface *i_node,
                         uint64_t node_id) {
  if (tree == NULL) {
    return;
  }
  assert(tree->update_root != NULL);
  rbt_validate_interface(i_node, IRBT_CHECKNULL | IRBT_PARENT | IRBT_LEFT |
                                     IRBT_RIGHT | IRBT_COLOR | IRBT_KEY);
  assert(i_node->is_null_node(node_id) != 0);

  uint64_t x = node_id;
  if (i_node->is_null_node(tree->root) == 1) {
    i_node->set_parent(x, NULL_ID);
    i_node->set_leftchild(x, NULL_ID);
    i_node->set_rightchild(x, NULL_ID);
    // For RBT
    i_node->set_color(x, COLOR_BLACK);

    tree->update_root(tree, x);
    return;
  }

  uint64_t p = tree->root;
  uint64_t x_key = i_node->get_key(x);

  while(i_node->is_null_node(p) == 0) {
    uint64_t p_key = i_node->get_key(x);

    if (x_key < p_key) {
      uint64_t p_left = i_node->get_leftchild(p);

      if (i_node->is_null_node(p_left) == 1) {
        bst_internal_setchild(p, x, LEFT_CHILD, i_node);
        return;
      } else {
        p = p_left;
      }
    } else { // if (x_key <= p_key)
      uint64_t p_right = i_node->get_rightchild(p);

      if (i_node->is_null_node(p_right) == 1) {
        bst_internal_setchild(p, x, RIGHT_CHILD, i_node);
        return;
      } else {
        p = p_right;
      }
    }
  }
}

void bst_internal_delete(rbtree_internal_t *tree, rbtree_node_interface *i_node,
                         uint64_t node_id, int is_rbt, uint64_t *db_parent) {

}

uint64_t bst_internal_find(rbtree_internal_t *tree,
                           rbtree_node_interface *i_node, uint64_t key) {
  if (tree == NULL) {
    return NULL_ID;
  }
  rbt_validate_interface(i_node, IRBT_CHECKNULL | IRBT_LEFT | IRBT_RIGHT | IRBT_KEY);
  if (i_node->is_null_node(tree->root) == 1) {
    return NULL_ID;
  }

  uint64_t p = tree->root;

  while (i_node->is_null_node(p) == 0) {
    uint64_t p_key = i_node->get_key(p);
    if (key == p_key) {
      return p;
    } else if (key < p_key){
      p = i_node->get_leftchild(p);
    } else {
      p = i_node->get_rightchild(p);
    }
  }
  return NULL_ID;
}

uint64_t bst_internal_find_succ(rbtree_internal_t *tree,
                                rbtree_node_interface *i_node, uint64_t key) {
  if (tree == NULL) {
    return NULL_ID;
  }
  rbt_validate_interface(i_node, IRBT_CHECKNULL | IRBT_LEFT | IRBT_RIGHT | IRBT_KEY);
  if (i_node->is_null_node(tree->root) == 1) {
    return NULL_ID;
  }

  uint64_t p = tree->root;
  uint64_t successor = NULL_ID;
  uint64_t successor_key = 0x7FFFFFFFFFFFFFFF;

  while (i_node->is_null_node(p) == 0) {
    uint64_t p_key = i_node->get_key(p);
    if (key == p_key) {
      return p;
    } else if (key < p_key){
      if (p_key <= successor_key) {
        successor = p;
        successor_key = p_key;
      }
      p = i_node->get_leftchild(p);
    } else {
      p = i_node->get_rightchild(p);
    }
  }
  // if no node key <= target key, return NULL_ID
  return successor; /// reutnr supremum(上确界)
}

static void bst_internal_dfs_print(uint64_t node, rbtree_node_interface *i_node,
                                   int depth) {
  rbt_validate_interface(i_node, IRBT_CHECKNULL | IRBT_LEFT | IRBT_RIGHT |
                                     IRBT_KEY | IRBT_COLOR);

  if (depth >= 10) {
    // meanless to print depth >= 10
    printf("*");
    return;
  }

  if (i_node->is_null_node(node) == 1) {
    printf("#");
    return;
  }

  if (i_node->get_color(node) == COLOR_RED) {
    printf("(0x%lx/%lu:\033[31m%lu\033[0m,", node, node, i_node->get_key(node));
  } else {
    printf("(0x%lx/%lu:%lu,", node, node, i_node->get_key(node));
  }

  bst_internal_dfs_print(i_node->get_leftchild(node), i_node, depth + 1);
  printf(",");
  bst_internal_dfs_print(i_node->get_rightchild(node), i_node, depth + 1);
  printf(")");
}

void bst_internal_prinf(uint64_t node, rbtree_node_interface *i_node) {
  bst_internal_dfs_print(node, i_node, 0);
  printf("\n");
}


// -------------------------------------
//       For Unit Test
// -------------------------------------
void internal_tree_construct_keystr(rbtree_internal_t *tree,
                                    rbtree_node_interface *i_node, char *str) {}

uint internal_tree_compare(uint64_t a, uint64_t b, rbtree_node_interface *i_node, int is_rbt) {

}

static void rbt_verify_dfs(uint64_t p, uint64_t *black_height,
                           uint64_t *key_min, uint64_t *key_max,
                           rbtree_node_interface *i_node, int is_rbt) {}

void rbt_internal_verify(rbtree_internal_t *tree, rbtree_node_interface *i_node,
                         int is_rbt) {}

// =====================================
//         Default Implementation
// =====================================
//Implementation of BST node access
static int is_null_node(uint64_t node_id) {}

static uint64_t construct_node() {}

static int destruct_node(uint64_t node_id) {}

static int compare_nodes(uint64_t first, uint64_t second) {}

static uint64_t get_parent(uint64_t node_id) {}

static int set_parent(uint64_t node_id, uint64_t parent_id) {}

static uint64_t get_leftchild(uint64_t node_id) {}

static int set_leftchild(uint64_t node_id, uint64_t left_id) {}

static uint64_t get_rightchild(uint64_t node_id) {}

static int set_rightchild(uint64_t node_id, uint64_t right_id) {}

static rb_color_t get_color(uint64_t node_id) {}

static int set_color(uint64_t node_id, rb_color_t color) {}

static uint64_t get_key(uint64_t node_id) {}

static int set_key(uint64_t node_id, uint64_t key) {}

static uint64_t get_value(uint64_t node_id) {}

static int set_value(uint64_t node_id, uint64_t value) {}

// shared with red-black tree
rbtree_node_interface default_i_rbt_node = {
    .construct_node = &construct_node,
    .destruct_node = &destruct_node,
    .is_null_node = &is_null_node,
    .compare_nodes = &compare_nodes,
    .get_parent = &get_parent,
    .set_parent = &set_parent,
    .get_leftchild = &get_leftchild,
    .set_leftchild = &set_leftchild,
    .get_rightchild = &get_rightchild,
    .set_rightchild = &set_rightchild,
    .get_color = &get_color,
    .set_color = &set_color,
    .get_key = &get_key,
    .set_key = &set_key,
    .get_value = &get_value,
    .set_value = &set_value,
};

// child class of base class
static int update_root(rbtree_internal_t *this, uint64_t new_root) {}

// ----------------------------
//  The exposed interfaces
// ----------------------------

// constructor and destructor
rb_tree_t *bst_construct() {}

// for test use
rb_tree_t *bst_construct_keystr(char *str) {}

void bst_print(rb_node_t *node) {}

static void bst_destruct_subtree(uint64_t root) {}

void bst_free(rb_tree_t *tree) {}

void bst_add(rb_tree_t *tree, uint64_t key) {}

void bst_insert(rb_tree_t *tree, rb_node_t *node) {}

void bst_remove(rb_tree_t *tree, uint64_t key) {}

void bst_delete(rb_tree_t *tree, rb_node_t *node) {}

rb_node_t *bst_find(rb_tree_t *tree, uint64_t key) {}

rb_node_t *bst_find_succ(rb_tree_t *tree, uint64_t key) {}

int bst_compare(rb_tree_t *a, rb_tree_t *b) {}

void bst_validate(rb_tree_t *tree) {}