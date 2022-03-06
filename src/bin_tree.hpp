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

template<class tKey, class tCmp=std::less<tKey>>
class AVLTree {
	typedef AVLNode<tKey> MyNode;
	public:
		AVLTree(): root(nullptr){}
		~AVLTree() { 
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
		void add(tKey k){
			MyNode** ppn = &root;
			std::stack<MyNode**> path;
			tCmp cmp;
			while(*ppn){
				path.push(ppn);
				if (cmp(_KEY(*ppn), k)) ppn = &_RIGHT(*ppn);
				else ppn = &_LEFT(*ppn);
			}
			*ppn = new MyNode(k);
			while(not path.empty()){
				ppn = path.top();
				_HEIGHT(*ppn) = _max_height(*ppn);
				_rebalance(ppn);
				path.pop();
			}
		}
		bool remove(tKey k) {
			std::stack<MyNode**> path;
			MyNode** ppn = &root;
			tCmp cmp;
			while (*ppn){
				path.push(ppn);
				if (not cmp(_KEY(*ppn),k) and not cmp(k,_KEY(*ppn))) break;
				else if (not cmp(_KEY(*ppn),k)) ppn = &_LEFT(*ppn);
				else ppn = &_RIGHT(*ppn);
			}
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
				path.pop();
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
					os << "\t" << _KEY(pn) << " [label=\"" << _KEY(pn) << " bf=" << _BF(pn) /*<< " h=" << _HEIGHT(pn)*/ << "\"]" << std::endl;
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

		*ppn = _LEFT(pn);
		_LEFT(pn) = _RIGHT(*ppn);
		_RIGHT(*ppn) = pn;

		_HEIGHT(pn) = _max_height(pn);
		_HEIGHT(*ppn) = _max_height(*ppn);
	}
	void _left_rot(MyNode** ppn){
		MyNode* pn = *ppn;

		*ppn = _RIGHT(pn);
		_RIGHT(pn) = _LEFT(*ppn);
		_LEFT(*ppn) = pn;
		
		_HEIGHT(pn) = _max_height(pn);
		_HEIGHT(*ppn) = _max_height(*ppn);
	}
	MyNode** _find(tKey k){
		MyNode** ppn = &root;
		tCmp cmp;
		while(*ppn){
			if(not cmp(_KEY(*ppn),k) and not cmp(k,_KEY(*ppn))) return ppn;
			if(cmp(_KEY(*ppn),k)) ppn = &_LEFT(ppn);
			else ppn = &_RIGHT(ppn);
		}
		return nullptr;
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
	MyNode* root;
};
