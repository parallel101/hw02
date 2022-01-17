/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next = nullptr;
    Node* prev = nullptr;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    explicit Node(int val):value(val) {
    }

    void insert(int val) {
	printf("insert val %d\n", val);
	auto node = std::make_unique<Node>(val);
	printf("inital unique\n");
	if (next) {
	    next->prev = node.get();
	    node->next = std::move(next);
	}
	printf("after unique\n");
	node->prev = this;
	next = std::move(node);
    }

    void erase() {
	if (next)
	    next->prev = prev;
	if (prev)
	    prev->next = std::move(next);

    }

    ~Node() {
	printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head = nullptr;

    List() = default;

    List(List const &other) {
	printf("List 被拷贝！\n");
	Node* copy = other.front();
	Node* curr = head.get();
	printf("obtain copy curr\n");
	while (copy) {
	    printf("start ieration\n");
	    printf("copyvalue = %d\n", copy->value);
	    if (!curr) {
		head = std::make_unique<Node>(copy->value);
		curr = head.get();
		printf("head\n");
	    } else {
		curr->insert(copy->value);
		curr = curr->next.get();
		printf("next round\n");
	    }
	    copy = copy->next.get();
	}
	printf("copy done");
	// 这是浅拷贝！
	// 请实现拷贝构造函数为 **深拷贝**
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
	return head.get();
    }

    int pop_front() {
	int ret = head->value;
	head = std::move(head->next);
	return ret;
    }

    void push_front(int value) {
	auto node = std::make_unique<Node>(value);
	if (head) {
	    head->prev = node.get();
	    node->next = std::move(head);
	}
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

void print(const List& lst) {  // 有什么值得改进的？
    printf("[");
    auto one = lst.front();
    while (one) {
	printf(" %d", one->value);
	one = one->next.get();
    }
    printf(" ]\n");
}

int main() {
    List a;
    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
