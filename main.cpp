// ConsoleApplication3.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>
struct Node {
	// 这两个指针会造成什么问题？请修复      done
	std::unique_ptr<Node> next;
	Node* prev;
	int value;

	// 这个构造函数有什么可以改进的？      done
	Node() = default;

	Node(int val, Node* prev1 = nullptr) {
		value = val;
		std::unique_ptr<Node> next = std::make_unique<Node>();
		prev = prev1;
	}

	void insert(int val) {
		auto node = std::make_unique<Node>(val);
		node->next = std::move(next);
		node->prev = prev;
		if (prev)
			prev->next = std::move(node);
		prev = nullptr;
		if (next)
			next->prev = node.get();
		next->prev = nullptr;
	}

	void erase() {
		if (next)
			next->prev = prev;

		if (prev)
			prev->next = std::move(next);
		prev = nullptr;
	}

	~Node() {
		// printf("~Node()\n");   // 应输出多少次？为什么少了？
	}
};

template<class T>
struct List {
	std::unique_ptr<Node> head;

	List() = default;

	List(List const &other) {
		printf("List 被拷贝！\n");

		// head = other.head;  // 这是浅拷贝！
		// 请实现拷贝构造函数为 **深拷贝**
		head = std::make_unique<Node>(other.head->value);

		if ((!other.head->next.get()) && (!other.head->prev)) {
			std::cout << "ok" << std::endl;
			return;
		}
		else {
			auto now_ptr = other.head->next.get();
			auto prev_ptr = head.get();
			while (now_ptr) {
				auto list_ptr = std::make_unique<Node>(now_ptr->value, prev_ptr);
				prev_ptr->next = std::move(list_ptr);
				std::cout << prev_ptr->value << std::endl;

				now_ptr = now_ptr->next.get();
				prev_ptr = prev_ptr->next.get();
			}
		}
	}

	List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？ done

	List(List &&) = default;
	List &operator=(List &&) = default;

	Node *front() const {
		return head.get();
	}

	T pop_front() {
		T ret = head->value;
		head = head->next;
		return ret;
	}

	void push_front(T value) {
		auto node = std::make_unique<Node>(value);
		if (head)
			head->prev = node.get();
		node->next = std::move(head);
		head = std::move(node);
	}

	Node *at(size_t index) const {
		auto curr = front();
		for (size_t i = 0; i < index; i++) {
			curr = curr->next.get();
		}
		return curr;
	}
};
template<class T>
void print(const List<T> &lst) {  // 有什么值得改进的？    done
	printf("[");
	for (auto curr = lst.front(); curr; curr = curr->next.get()) {
		printf(" %d", curr->value);
	}
	printf(" ]\n");
}

int main() {
	List<int> a;

	a.push_front(7);
	a.push_front(5);
	a.push_front(8);
	a.push_front(2);
	a.push_front(9);
	a.push_front(4);
	a.push_front(1);

	print(a);   // [ 1 4 9 2 8 5 7 ]
	// std::cout << a.at(0) << std::endl;
	// std::cout << a.at(1)->prev << std::endl;

	// std::cout << a.at(1) << std::endl;
	// std::cout << a.at(0)->next.get()  << std::endl;


	a.at(2)->erase();

	print(a);   // [ 1 4 2 8 5 7 ]

	List<int> b = a;

	a.at(3)->erase();

	print(a);   // [ 1 4 2 5 7 ]
	print(b);   // [ 1 4 2 8 5 7 ]

	// b = {};
	// a = {};

	return 0;
}
