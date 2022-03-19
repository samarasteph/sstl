#include <stack>
#include <list>
#include <cmath>
#include <iostream>

typedef int bf_t;

template<class tKey>
class AVLNode {
	public:
	AVLNode(tKey k): key(k), pl(nullptr),pr(nullptr), height(0){}
	tKey key;
	AVLNode<tKey> *pl, *pr;
	int height;
};

#define _LEFT(n) (n)->pl
#define _RIGHT(n) (n)->pr
#define _KEY(n) (n)->key
#define _BF(n) _bf(n)
#define _HEIGHT(n) (n)->height

template <class tKey>
std::ostream& operator << (std::ostream& os, const AVLNode<tKey>* pn)  {
	os << _KEY(pn);
	return os;
}

enum RBType { RED, BLACK };
template<class tKey>
class RedBlackTreeNode {
	public:
	RedBlackTreeNode(tKey k): key(k), pl(nullptr),pr(nullptr), type(RED){}
	tKey key;
	RedBlackTreeNode<tKey> *pl, *pr;
	RBType type;
};

template <class tKey>
std::ostream& operator << (std::ostream& os, const RedBlackTreeNode<tKey>* pn)  {
	os << _KEY(pn);
	return os;
}

template<class tKey, class tNode, class tCmp>
class BinaryTree {
protected:
	typedef tNode MyNode;

	BinaryTree(): root(nullptr){}
	~BinaryTree() { 
		std::list<MyNode*> nodes;
		if (root) nodes.push_back(root);
		while(!nodes.empty()){
			MyNode* pn = nodes.front();
			if (_LEFT(pn))	nodes.push_back(_LEFT(pn));
			if (_RIGHT(pn)) nodes.push_back(_RIGHT(pn));
			delete pn;
			nodes.pop_front();
		}
	}
	MyNode** _find(tKey k, std::stack<MyNode**>& path){
		MyNode** ppn = &root;
		tCmp cmp;
		while(*ppn){
			path.push(ppn);
			if(not cmp(_KEY(*ppn),k) and not cmp(k,_KEY(*ppn))) return ppn;
			if(cmp(_KEY(*ppn),k)) ppn = &_LEFT(*ppn);
			else ppn = &_RIGHT(*ppn);
		}
		return nullptr;
	}
	void _right_rot(MyNode** ppn){
		MyNode* pn = *ppn ;

		*ppn = _LEFT(pn);
		_LEFT(pn) = _RIGHT(*ppn);
		_RIGHT(*ppn) = pn;
	}
	void _left_rot(MyNode** ppn){
		MyNode* pn = *ppn;

		*ppn = _RIGHT(pn);
		_RIGHT(pn) = _LEFT(*ppn);
		_LEFT(*ppn) = pn;
	}
	void _print(std::ostream& os, std::function<void(const MyNode*,std::ostream&)>& disp_label){
		using namespace std;
		os << endl << "strict graph {" << endl;
					
		std::list<MyNode*> nodes;
		if (root) {
			MyNode* rightmost = root;
			while(true){
				if (_RIGHT(rightmost)){
					rightmost = _RIGHT(rightmost);
				}
				else {
					break;
				}
			}
			if (root) 
				nodes.push_back(root);

			typename std::list<MyNode* >::iterator it = nodes.begin();
			while(nodes.end() != it) {
				if (_LEFT(*it)) nodes.push_back(_LEFT(*it));
				if (_RIGHT(*it)) nodes.push_back(_RIGHT(*it));
				++it;
			}
			for (const MyNode* pn:nodes){
				disp_label(pn,os);
			}
			os << std::endl;
		}

		while(not nodes.empty()) {
			MyNode* pn = nodes.front();
			if (_LEFT(pn)) {
				os << "\t" << pn << " -- " << _LEFT(pn) << endl;
			}
			if (_RIGHT(pn)) {
				os << "\t" << pn << " -- " << _RIGHT(pn) << endl;
			}
			nodes.pop_front();
		}
		os << "}" << endl;
	}
public:
	bool empty() { return root == nullptr; }
public:
	class iterator {
	public:
		iterator(){}
		iterator(const iterator& it): nodes(it.nodes) {}
		iterator(iterator&& it): nodes(std::move(it.nodes)) {}
		const iterator& operator = (const iterator& it) { if (&it!=this) nodes=it.nodes; return *this; }
		iterator& operator = (iterator&& it) { nodes=std::move(it.nodes); return *this; }
	private:
		iterator(std::unique_ptr<std::stack<MyNode*>>&& nodes): nodes(std::move(nodes)){}
		std::shared_ptr<std::stack<MyNode*>> nodes;
	};
protected:
	MyNode* root;
};

template<class tKey, class tCmp=std::less<tKey>>
class AVLTree: private BinaryTree<tKey,AVLNode<tKey>,tCmp> {
	typedef BinaryTree<tKey,AVLNode<tKey>,tCmp>	MyBaseTree;
	typedef typename MyBaseTree::MyNode MyNode;
	public:
		void add(tKey k){
			std::stack<MyNode**> path;
			MyNode** ppn = MyBaseTree::_find(k,path);
			if (not *ppn) {
				*ppn = new MyNode(k);
				while(not path.empty()){
					ppn = path.top();
					_HEIGHT(*ppn) = _max_height(*ppn);
					_rebalance(ppn);
					path.pop();
				}
			}
		}
		bool remove(tKey k) {
			std::stack<MyNode**> path;
			MyNode** ppn = MyBaseTree::_find(k,path);
			if (*ppn){
				MyNode* pn = *ppn;
				if (not _LEFT(*ppn) and not _RIGHT(*ppn)){
					*ppn = nullptr;
				}
				else if (_LEFT(*ppn) and _RIGHT(*ppn)){
					MyNode** replace,*child;
					
					replace = &_RIGHT(*ppn); // get 
					path.push(replace);
					while(_LEFT(*replace)){
						replace = &_LEFT(*replace);
						path.push(replace);
					}
					_KEY(*ppn) = std::move(_KEY(*replace));
					pn = *replace; // delete pn
					*replace = _RIGHT(*replace);
				}
				else {
					if (_LEFT(*ppn)){
						*ppn = _LEFT(*ppn);
					}
					else {
						*ppn = _RIGHT(*ppn);
					}
				}
				delete pn;
				while(not path.empty()){
					ppn = path.top();
					_HEIGHT(*ppn) = _max_height(*ppn);
					_rebalance(ppn);
					path.pop();
				}
				return true; //found
			}
			return false;
		}
		void print(std::ostream& os){
			std::function<void(const MyNode*,std::ostream&)> f = [this](const MyNode* pn, std::ostream& os) {
				os << "\t" << _KEY(pn) << " [label=\"" << _KEY(pn) << " bf=" << _BF(pn) /*<< " h=" << _HEIGHT(pn)*/ << "\"]" << std::endl;
			};
			this->_print(os,f);
		}
	private:
	void _rebalance(MyNode** ppn){
		if (_BF(*ppn) > 1 || _BF(*ppn) < -1) {
			if (_BF(*ppn) > 1){
				if (_BF(_RIGHT(*ppn)) < 0){
					_right_rot(&_RIGHT(*ppn));
				}
				_left_rot(ppn);
			}
			else{
				if(_BF(_LEFT(*ppn))>0){
					_left_rot(&_LEFT(*ppn));
				}
				_right_rot(ppn);
			}
		}
	}
	void _right_rot(MyNode** ppn){
		MyNode* pn = *ppn ;

		MyBaseTree::_right_rot(ppn);
		_HEIGHT(pn) = _max_height(pn);
		_HEIGHT(*ppn) = _max_height(*ppn);
	}
	void _left_rot(MyNode** ppn){
		MyNode* pn = *ppn;
		
		MyBaseTree::_left_rot(ppn);
		_HEIGHT(pn) = _max_height(pn);
		_HEIGHT(*ppn) = _max_height(*ppn);
	}
	bf_t _bf(const MyNode* pn) const {
		return _right_height(pn) - _left_height(pn);
	}
	int _left_height(const MyNode* pn) const {
		if (pn and _LEFT(pn)) 
			return 1 + _HEIGHT(_LEFT(pn));
		return 0;
	}
	int _right_height(const MyNode* pn) const {
		if (pn and _RIGHT(pn)) 
			return 1 + _HEIGHT(_RIGHT(pn));
		return 0;
	}
	int _max_height(MyNode* pn){
		return std::max(_left_height(pn),_right_height(pn));
	}
};


#define _IS_BLACK(n) ((n)->type==BLACK)
#define _IS_RED(n) ((n)->type)==RED)
#define _BLACK(n) (n)->type=BLACK
#define _RED(n) (n)->type=RED

template<class tKey, class tCmp=std::less<tKey>>
class RedBlackTree: private BinaryTree<tKey,RedBlackTreeNode<tKey>,tCmp> {
	typedef BinaryTree<tKey,RedBlackTreeNode<tKey>,tCmp>	MyBaseTree;
	typedef typename MyBaseTree::MyNode MyNode;
public:
	void add(tKey k){
		std::stack<MyNode**> path;
		MyNode** ppn = this->_find(k,path);
		if (not *ppn) {
			*ppn = new MyNode(k);
			if (ppn==&this->root){
				_BLACK(*ppn);
			}
			else if (not path.empty()){
				
			}
		}
	}
	bool remove(tKey k) { return false; }
	void print(std::ostream& os){ }
};