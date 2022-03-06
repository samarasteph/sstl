#include "bin_tree.hpp"

#define LOG_DEBUG 	0

int TEST () {

	AVLTree<double> tree;

	double key;
	//std::cout << "starting read:" << std::endl;
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
	return 0;

}