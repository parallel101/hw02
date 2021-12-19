/* 基于智能指针实现双向链表 */
#include <cstdio>
#include "List.hpp"
#include <iterator>

void print(const List<int>& lst) {  // 有什么值得改进的？
	printf("[");
	for (auto&& value : lst) {
		printf(" %d", value);
	}
	printf(" ]\n");
}

int main() {


	List<int> a;

	a.push_front(1, 4, 9, 2, 8, 5, 7);
	/*a.push_front(7);
	a.push_front(5);
	a.push_front(8);
	a.push_front(2);
	a.push_front(9);
	a.push_front(4);
	a.push_front(1);*/

	print(a);   // [ 1 4 9 2 8 5 7 ]

	a.at(2)->erase();

	print(a);   // [ 1 4 2 8 5 7 ]

	List<int> b = a;

	a.at(3)->erase();

	print(a);   // [ 1 4 2 5 7 ]
	print(b);   // [ 1 4 2 8 5 7 ]

	b = {};
	a = {};

	return 0;
}
