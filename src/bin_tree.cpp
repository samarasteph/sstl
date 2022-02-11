#include <stack>

template<class tKey>
class Node {
	Node(tKey k): key(k), pl(nullptr),pr(nullptr), bf(0){}
	tKey key;
	Node* pl,pr;
	int bf; //balance factor = h(Right(node)) - h(Left(node))
};

#define _LEFT(n) n->pl
#define _RIGHT(n) n->pl
#define _BF(n) n ? n->bf : 0

template<class tKey>
class Tree {
	typedef Node<tKey> MyNode;
	public:
		void add(tKey k){
			MyNode** ppn = &root;
			std::stack<MyNode**> path;
			while(ppn!=nullptr){
				path.push(ppn);
				if ((*ppn)->key < k){
					ppn = &(*ppn)->right;
				}else {
					ppn = &(*ppn)->left;
				}
			}
			*ppn = new MyNode(k);

			do{
				ppn = path.top();
				rebalance(ppn);
				path.pop();
			}while(not path.empty());
			
		}
		void remove(tKey k) {
			auto ppn = _find(k);
			if (ppn) {
				
			}
		}
	private:
	void rebalance(MyNode** ppn){
		if (_BF(*ppn) > 1 or _BF(*ppn) < -1) {
			int nbl = _BF(_LEFT(*ppn));
			int nbr = _BF(_RIGHT(*ppn));
			if (_BF(*ppn) > 1){
				_left_rot(ppn);
			}
			else if (nbr - nbl > 1){
				_right_rot(ppn);
			}
		}
	}
	void _right_rot(MyNode** ppn){
		MyNode* pn = *ppn , left = _LEFT(pn);

		if(_RIGHT(left)){
			_LEFT(pn) = _RIGHT(left);
			_RIGHT(left) = nullptr;
			_LEFT(_LEFT(pn)) = left;
		}
		*ppn = _LEFT(pn);
		_RIGHT(*ppn) = pn;
	}
	void _left_rot(MyNode** ppn){
		MyNode* pn = *ppn, right = _RIGHT(pn);
		
		if(_LEFT(right)){
			_RIGHT(pn) = _LEFT(right);
			_LEFT(right) = nullptr;
			_RIGHT(_RIGHT(pn)) = right;
		}
		*ppn = _RIGHT(pn);
		_LEFT(*ppn) = pn;
	}
	MyNode** _find(tKey k){
		MyNode** ppn = &root;
		while(*ppn){
			if ((*ppn)->key == k) return ppn;
			if((*ppn)->key > k) ppn = &_LEFT(ppn);
			else ppn = &_RIGHT(ppn);
		}
		return nullptr;
	}
	MyNode* root;
};

int main () {

	Tree<int> tree;

	return 0;

}