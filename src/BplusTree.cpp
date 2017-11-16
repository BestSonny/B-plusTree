#if !defined BPLUSTREE_HPP
#define BPLUSTREE_HPP

#include <stdlib.h>
#include <map>
#include <stack>
#include <assert.h>
#include <type_traits>
#include <iostream>
using std::cout;
using std::endl;

#define MAX_CHILDREN 300

template <typename KEY, typename VALUE>
class BPlusTree
{
public:


	// Builds a new empty tree.
	BPlusTree(unsigned N, unsigned M)
	: depth(0), N(N), M(M),
	  root(new_leaf_node(M))
	{
		// N must be greater than two to make the split of
		// two inner nodes sensible.
		assert(N>2);
		// Leaf nodes must be able to hold at least one element
		assert(M>0);
	}

	~BPlusTree() {
		;
	}

	// Inserts a pair (key, value). If there is a previous pair with
	// the same key, we will still insert the new one.
	void insert(KEY key, VALUE value) {
		InsertionResult result;
		bool was_split;
		if( depth == 0 ) {
			// The root is a leaf node
			was_split= leaf_insert(reinterpret_cast<LeafNode*>
				(root), key, value, &result);
		} else {
			// The root is an inner node
			was_split= inner_insert(reinterpret_cast<InnerNode*>
				(root), depth, key, value, &result);
		}
		if( was_split ) {
			// The old root was splitted in two parts.
			// We have to create a new root pointing to them
			depth++;
			root= new_inner_node(N);
			InnerNode* rootProxy=
				reinterpret_cast<InnerNode*>(root);
			rootProxy->num_keys= 1;
			rootProxy->keys[0]= result.key;
			rootProxy->children[0]= result.left;
			rootProxy->children[1]= result.right;
		}
	}

	// Search for all values in a range of (key1, key2), and store them in res
  void getRange(const KEY& key1, const KEY& key2, std::multimap<KEY, VALUE>& res) {
    InnerNode* inner;
    register void* node= root;
    register unsigned d= depth;
    inner= reinterpret_cast<InnerNode*>(node);
		// if depth is 0. directly search in its leaf nodes
		if(depth==0){
			LeafNode* leaf = reinterpret_cast<LeafNode*>(root);
			int index;
			while(index < leaf->num_keys) {
				if(leaf->keys[index] >= key1 && leaf->keys[index] <= key2) {
					res.insert(std::pair<KEY, VALUE>(leaf->keys[index], leaf->values[index]));
				}
				++index;
			}
		}
		//Recursively traverse leaf nodes within the range of (key1, key2)
		else{
			windowQuery(inner, d, key1, key2, res);
		}

  }

	//Recursively traverse leaf nodes within the range of (key1, key2)
  void windowQuery(void* node, int depth, const KEY& key1, const KEY& key2, std::multimap<KEY, VALUE>& res){
    InnerNode* inner;
		// the node's children is leaf nodes
    if( depth-1 == 0 ){
      inner= reinterpret_cast<InnerNode*>(node);
      // The children are leaf nodes
      for(unsigned kk=0; kk < inner->num_keys+1; ++kk) {
        LeafNode* leaf= reinterpret_cast<LeafNode*>(inner->children[kk]);
        int index = 0;
        while(index < leaf->num_keys) {
          if(leaf->keys[index] >= key1 && leaf->keys[index] <= key2) {
            res.insert(std::pair<KEY, VALUE>(leaf->keys[index], leaf->values[index]));
          }
          ++index;
        }
      }
    }
		// Otherwise, Recursive query on the subtrees.
		else{
      inner= reinterpret_cast<InnerNode*>(node);
      int index_lower = inner_position_for(key1, inner->keys, inner->num_keys);
      int index_higher = inner_position_for(key2, inner->keys, inner->num_keys);
      for(int index= index_lower; index <= index_higher; index++){
        node= inner->children[index];
        windowQuery(node, depth-1, key1, key2, res);
      }

    }
  }

private:

	// Leaf nodes store pairs of keys and values.
	struct LeafNode {
		LeafNode(unsigned M) : num_keys(0) {
			this->M = M;
			this->keys = new KEY[M];
			this->values = new VALUE[M];
		}

		unsigned num_keys;
		KEY      *keys;
		VALUE    *values;
		unsigned M;
	};

	// Inner nodes store pointers to other nodes interleaved with keys.
	struct InnerNode {
		InnerNode(unsigned N) : num_keys(0) {
			this->N = N;
			this->keys = new KEY[N];
		}
		unsigned num_keys;
		unsigned N;
		KEY      *keys;
		void     *children[MAX_CHILDREN];
	};

	// Returns a pointer to a fresh leaf node.
	LeafNode* new_leaf_node(unsigned M) {
		LeafNode* result;
		//result= new LeafNode();
		result= new LeafNode(M);
		//cout << "New LeafNode at " << result << endl;
		return result;
	}

	// Returns a pointer to a fresh inner node.
	InnerNode* new_inner_node(unsigned N) {
		InnerNode* result;
		// Alternatively: result= new InnerNode();
		result= new InnerNode(N);
		//cout << "New InnerNode at " << result << endl;
		return result;
	}

	// Data type returned by the private insertion methods.
	struct InsertionResult {
		KEY key;
		void* left;
		void* right;
	};

	// Returns the position where 'key' should be inserted in a leaf node
	// that has the given keys.
	static unsigned leaf_position_for(const KEY& key, const KEY* keys,
			unsigned num_keys) {
		// Simple linear search. Faster for small values of N or M
		unsigned k= 0;
		while((k < num_keys) && (keys[k]<key)) {
			++k;
		}
		return k;
	}

	// Returns the position where 'key' should be inserted in an inner node
	// that has the given keys.
	static unsigned inner_position_for(const KEY& key, const KEY* keys,
			unsigned num_keys) {
		// Simple linear search. Faster for small values of N or M
		unsigned k= 0;
		while((k < num_keys) && ( keys[k]<key) || (keys[k]==key)) {
			++k;
		}
		return k;
		// Binary search is faster when N or M is > 100,
		// but the cost of the division renders it useless
		// for smaller values of N or M.
	}

	bool leaf_insert(LeafNode* node, KEY& key,
			VALUE& value, InsertionResult* result) {
		assert( node->num_keys <= M );
		bool was_split= false;
		// Simple linear search
		unsigned i= leaf_position_for(key, node->keys, node->num_keys);
		if( node->num_keys == M ) {
			// The node was full. We must split it
			unsigned treshold= (M+1)/2;
			LeafNode* new_sibling= new_leaf_node(M);
			new_sibling->num_keys= node->num_keys -treshold;
			for(unsigned j=0; j < new_sibling->num_keys; ++j) {
				new_sibling->keys[j]= node->keys[treshold+j];
				new_sibling->values[j]=
					node->values[treshold+j];
			}
			node->num_keys= treshold;
			if( i < treshold ) {
				// Inserted element goes to left sibling
				leaf_insert_nonfull(node, key, value, i);
			} else {
				// Inserted element goes to right sibling
				leaf_insert_nonfull(new_sibling, key, value,
					i-treshold);
			}
			// Notify the parent about the split
			was_split= true;
			result->key= new_sibling->keys[0];
			result->left= node;
			result->right= new_sibling;
		} else {
			// The node was not full
			leaf_insert_nonfull(node, key, value, i);
		}
		return was_split;
	}

	void leaf_insert_nonfull(LeafNode* node, KEY& key, VALUE& value,
			unsigned index) {
		assert( node->num_keys < M );
		assert( index <= M );
		assert( index <= node->num_keys );
		// if( (index < M) && (node->num_keys!=0) && (node->keys[index] == key) ) {
		// 	// We are inserting a duplicate value.
		// 	// Simply overwrite the old one
		// 	node->values[index]= value;
		// } else {
			// The key we are inserting is unique
		for(unsigned i=node->num_keys; i > index; --i) {
			node->keys[i]= node->keys[i-1];
			node->values[i]= node->values[i-1];
		}
		node->num_keys++;
		node->keys[index]= key;
		node->values[index]= value;
		// }
	}

	bool inner_insert(InnerNode* node, unsigned depth, KEY& key,
			VALUE& value, InsertionResult* result) {
		assert( depth != 0 );
		// Early split if node is full.
		// This is not the canonical algorithm for B+ trees,
		// but it is simpler and does not break the definition.
		bool was_split= false;
		if( node->num_keys == N ) {
			// Split
			unsigned treshold= (N+1)/2;
			InnerNode* new_sibling= new_inner_node(N);
			new_sibling->num_keys= node->num_keys -treshold;
			for(unsigned i=0; i < new_sibling->num_keys; ++i) {
				new_sibling->keys[i]= node->keys[treshold+i];
				new_sibling->children[i]=
					node->children[treshold+i];
			}
			new_sibling->children[new_sibling->num_keys]=
				node->children[node->num_keys];
			node->num_keys= treshold-1;
			// Set up the return variable
			was_split= true;
			result->key= node->keys[treshold-1];
			result->left= node;
			result->right= new_sibling;
			// Now insert in the appropriate sibling
			if( key < result->key ) {
				inner_insert_nonfull(node, depth, key, value);
			} else {
				inner_insert_nonfull(new_sibling, depth, key,
					value);
			}
		} else {
			// No split
			inner_insert_nonfull(node, depth, key, value);
		}
		return was_split;
	}

	void inner_insert_nonfull(InnerNode* node, unsigned depth, KEY& key,
			VALUE& value) {
		assert( node->num_keys < N );
		assert( depth != 0 );
		// Simple linear search
		unsigned index= inner_position_for(key, node->keys,
			node->num_keys);
		InsertionResult result;
		bool was_split;
		if( depth-1 == 0 ) {
			// The children are leaf nodes
			was_split= leaf_insert(reinterpret_cast<LeafNode*>
				(node->children[index]), key, value, &result);
		} else {
			// The children are inner nodes
			InnerNode* child= reinterpret_cast<InnerNode*>
				(node->children[index]);
			was_split= inner_insert( child, depth-1, key, value,
				&result);
		}
		if( was_split ) {
			if( index == node->num_keys ) {
				// Insertion at the rightmost key
				node->keys[index]= result.key;
				node->children[index]= result.left;
				node->children[index+1]= result.right;
				node->num_keys++;
			} else {
				// Insertion not at the rightmost key
				node->children[node->num_keys+1]=
					node->children[node->num_keys];
				for(unsigned i=node->num_keys; i!=index; --i) {
					node->children[i]= node->children[i-1];
					node->keys[i]= node->keys[i-1];
				}
				node->children[index]= result.left;
				node->children[index+1]= result.right;
				node->keys[index]= result.key;
				node->num_keys++;
			}
		} // else the current node is not affected
	}

	// Depth of the tree. A tree of depth 0 only has a leaf node.
	unsigned depth;

	unsigned N;
	unsigned M;
	// Pointer to the root node. It may be a leaf or an inner node, but
	// it is never null.
	void*    root;
};

#endif
