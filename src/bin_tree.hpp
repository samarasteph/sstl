#include <stack>
#include <list>
#include <cmath>
#include <iostream>

#if 0
#define LOG(msg) std::cout << msg << std::endl 
#else
#define LOG(msg)
#endif

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
	RedBlackTreeNode(tKey k): key(k), pl(nullptr),pr(nullptr), type(RED), parent(nullptr){}
	tKey key;
	RedBlackTreeNode<tKey> *pl, *pr;
	RBType type;
	RedBlackTreeNode* parent;
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
		return ppn;
	}
	MyNode* _insert(tKey k, MyNode** new_node){
		MyNode* parent = root;
		tCmp cmp;

		do {
			if (not parent) {
				*new_node = root = new MyNode(k);
				return nullptr;
			}
			if(cmp(k,_KEY(parent))){
				if (not _LEFT(parent)){
					*new_node = _LEFT(parent) = new MyNode(k);
					return parent;
				}
				else {
					parent = _LEFT(parent);
				}
			}
			else if(not cmp(_KEY(parent),k) and not cmp(k,_KEY(parent))) {
				//strict equality: no insertion
				*new_node = nullptr;
				return nullptr;
			}
			else {
				if (not _RIGHT(parent)){
					*new_node = _RIGHT(parent) = new MyNode(k);
					return parent;
				}
				else {
					parent = _RIGHT(parent);
				}
			}
		} while(parent);
		return parent;
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
#define _IS_RED(n) ((n)->type==RED)
#define _BLACK(n) (n)->type=BLACK
#define _RED(n) (n)->type=RED
#define _PARENT(n) (n)->parent
#define _SWAP_COL(n1,n2)	auto c = (n1)->type; (n1)->type = (n2)->type; (n2)->type = c;

template<class tKey, class tCmp=std::less<tKey>>
class RedBlackTree: private BinaryTree<tKey,RedBlackTreeNode<tKey>,tCmp> {
	typedef BinaryTree<tKey,RedBlackTreeNode<tKey>,tCmp>	MyBaseTree;
	typedef typename MyBaseTree::MyNode MyNode;
public:
	void add(tKey k){
		MyNode* new_node = nullptr;
		auto parent = MyBaseTree::_insert(k,&new_node);
		if (new_node) { // if key already exist, no new node
			if (parent) LOG("Add new node key=" << k << (_LEFT(parent)==new_node?" LEFT":" RIGHT"));
			else LOG("Add new node key=" << k );
			_PARENT(new_node) = parent;
			if(parent and _IS_RED(parent) and not _recolour(new_node)){
				LOG("rotate key=" << k);
				_rotate(new_node);
			}
		}
	}
	bool remove(tKey k) { return false; }
	void print(std::ostream& os){
		std::function<void(const MyNode*,std::ostream&)> f = [this](const MyNode* pn, std::ostream& os) {
			if (_IS_BLACK(pn))
				os << "\t \"" << _KEY(pn) << "\" [fontcolor=white,fillcolor=black,style=filled]" << std::endl;
			if(_IS_RED(pn))
				os << "\t \"" << _KEY(pn) << "\" [fontcolor=white,fillcolor=red,style=filled]" << std::endl;
		};
		this->_print(os,f);
	}
	private:
	MyNode* _uncle(MyNode* node){
		if(_PARENT(node) and _PARENT(_PARENT(node))){
			MyNode* granpa = _PARENT(_PARENT(node));
			if (_LEFT(granpa)==_PARENT(node)) return _RIGHT(granpa);
			else return _LEFT(granpa);
		}
		return nullptr;
	}
	bool _recolour(MyNode* node){
		if (node==this->root) {
			LOG("Node k="<<_KEY(node)<< " is ROOT");
			_BLACK(node);
			return true;
		}
		if(_IS_RED(node) and _PARENT(node) and _IS_RED(_PARENT(node))){
			MyNode* uncle = _uncle(node); 
			if (uncle and _IS_RED(uncle)){
				LOG(__func__<<" Change parent color parent k=" << _KEY(_PARENT(node)) << " uncle k="<<_KEY(uncle));
				_RED(_PARENT(uncle));
				_BLACK(_PARENT(node));
				_BLACK(uncle);
				// same process with granpa
				_recolour(_PARENT(uncle));
				return true;
			}
		}
		return false;
	}
	void _rotate(MyNode* node){
		if (_PARENT(node) and _PARENT(_PARENT(node))){
			auto parent = _PARENT(node), granpa = _PARENT(parent);
			if(node == _LEFT(parent) and parent == _LEFT(granpa)){ //All left => Right rotate
				LOG("Rotate LL");
				MyBaseTree::_right_rot( get_pp(granpa) );
				_PARENT(parent) = _PARENT(granpa);
				_PARENT(granpa) = parent;
				_SWAP_COL(parent,granpa);
			}
			else if(node == _RIGHT(parent) and parent == _RIGHT(granpa)){ //All right => Left rotate
				LOG("Rotate RR");
				MyBaseTree::_left_rot( get_pp(granpa) );
				_PARENT(parent) = _PARENT(granpa);
				_PARENT(granpa) = parent;
				_SWAP_COL(parent,granpa);
			}
			else if(node == _RIGHT(parent) and parent == _LEFT(granpa)){
				LOG("Rotate LR");
				// left rotate node over parent
				MyBaseTree::_left_rot(&_LEFT(granpa));
				_PARENT(node) = granpa;
				_PARENT(parent) = node;
				_rotate(parent); // finalize with RIGHT rotation (case 1 LL)
			}
			else { // node == LEFT(parent) and parent == RIGHT(granpa)
				LOG("Rotate RL");
				// right rotate node over parent
				MyBaseTree::_right_rot(&_RIGHT(granpa));
				_PARENT(node) = granpa;
				_PARENT(parent) = node;
				_rotate(parent); // finalize with LEFT rotation (case 2 RR) 
			}
		}
		else LOG(__func__<<" Node k="<<_KEY(node)<< " has no granpa or parent");
	}
	MyNode** get_pp(MyNode* node){
		if (_PARENT(node)==nullptr) { 
			LOG("node parent pointer is ROOT");
			return &this->root; 
		}
		if(_LEFT(_PARENT(node))==node) {
			LOG("node parent pointer is LEFT");
			return &_LEFT(_PARENT(node));
		}  
		LOG("node parent pointer is RIGHT");
		return &_RIGHT(_PARENT(node));
	}
};