#include "bin_tree.hpp"

#define LOG_DEBUG 	0

template <class tTree>
void test() {
	tTree tree;
	double key;
	#if LOG_DEBUG
	std::cout << "starting read:" << std::endl;
	#endif
	while(not std::cin.eof()){
		char action;
		std::cin >> action;
		#if LOG_DEBUG
		std::cout << "action read=" << action << std::endl;
		#endif
		if (action == '+') {
			std::cin >> key;
			if (std::cin) { 
				#if LOG_DEBUG
				std::cout << "add key=" << key << std::endl;
				#endif
				tree.add(key);
			}
		}
		else if (action == '-') {
			std::cin >> key;
			if (std::cin) { 
				#if LOG_DEBUG
				std::cout << "remove key=" << key << std::endl;
				if(not tree.remove(key)) std::cout << key << " not found" << std::endl;
				#else 
				tree.remove(key);
				#endif
				
			}
		}
		#if LOG_DEBUG
		else {
			std::cout << "bad action " << action << std::endl;
		}
		#endif
	}
	
	tree.print(std::cout);
}

int TEST () {

	test<AVLTree<double>>();
	test<RedBlackTree<double>>();
	return 0;

}