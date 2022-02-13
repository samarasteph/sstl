#include <stack>
#include <list>
#include <cmath>
#include <iostream>

typedef int bf_t;

template<class tKey>
class Node {
	public:
	Node(tKey k): key(k), pl(nullptr),pr(nullptr), bf(0){}
	tKey key;
	Node<tKey> *pl, *pr;
	bf_t bf; //balance factor = h(Right(node)) - h(Left(node))
};

#define _LEFT(n) (n)->pl
#define _RIGHT(n) (n)->pr
#define _KEY(n) (n)->key
#define _BF(n) (n)->bf

template <class tKey>
std::ostream& operator << (std::ostream& os, const Node<tKey>* pn)  {
	os << _KEY(pn);
	return os;
}

template<class tKey>
class Tree {
	typedef Node<tKey> MyNode;
	public:
		Tree(): root(nullptr){}
		~Tree() { 
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
			while(*ppn){
				path.push(ppn);
				if (_KEY(*ppn) < k) ppn = &_RIGHT(*ppn);
				else ppn = &_LEFT(*ppn);
			}
			*ppn = new MyNode(k);
			
			while(not path.empty()){
				ppn = path.top();
				_bf(*ppn);
				_rebalance(ppn);
				path.pop();
			}	
		}
		void remove(tKey k) {
			auto ppn = _find(k);
			if (ppn) {
				
			}
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
					os << "\t" << _KEY(pn) << " [label=\"" << _KEY(pn) << " bf=" << _BF(pn) << "\"]" << std::endl;
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

		int hr = _right_height(pn), hl = _left_height(pn);
		_BF(pn) = hr - hl;
		_BF(*ppn) = std::max<int>(hr,hl)+1 - _left_height(*ppn);
	}
	void _left_rot(MyNode** ppn){
		MyNode* pn = *ppn;

		*ppn = _RIGHT(pn);
		_RIGHT(pn) = _LEFT(*ppn);
		_LEFT(*ppn) = pn;
		
		int hr = _right_height(pn), hl = _left_height(pn);
		_BF(pn) = hr - hl;
		_BF(*ppn) = _right_height(*ppn) - std::max<int>(hr,hl) - 1;
	}
	MyNode** _find(tKey k){
		MyNode** ppn = &root;
		while(*ppn){
			if(_KEY(*ppn) == k) return ppn;
			if(_KEY(*ppn) > k) ppn = &_LEFT(ppn);
			else ppn = &_RIGHT(ppn);
		}
		return nullptr;
	}
	void _bf(MyNode* pn){
		bf_t bf = 0;
		//if (_LEFT(pn))  bf +=  - std::abs(_BF(_LEFT(pn)))  - 1;
		//if (_RIGHT(pn)) bf +=    std::abs(_BF(_RIGHT(pn))) + 1;
		pn->bf = _right_height(pn) - _left_height(pn);
	}
	int _left_height(MyNode* pn){
		if (pn and _LEFT(pn)) 
			return (1 + _max_height(_LEFT(pn)));
		return 0;
	}
	int _right_height(MyNode* pn){
		if (pn and _RIGHT(pn)) 
			return (1 + _max_height(_RIGHT(pn)));
		return 0;
	}
	int _max_height(MyNode* pn){
		int hl = 0, hr = 0;
		hl = _left_height(pn);
		hr = _right_height(pn);
		return std::max(hl,hr);	
	}
	MyNode* root;
};

int main () {

	Tree<int> tree;

	int key;
	while(not std::cin.eof()){
		std::cin >> key;
		if (std::cin) tree.add(key);
	}
	
	tree.print(std::cout);
	return 0;

}