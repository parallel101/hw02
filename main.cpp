/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>

struct Node {
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next;
    Node* prev = nullptr;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) :value(val) {
    }

    void insert(int val) {
        auto newNode = std::make_unique<Node>(val);
        auto oldNode = std::make_unique<Node>(value);

        newNode->prev = prev;
        if (next)
            next->prev = oldNode.get();

        oldNode->next = std::move(next);
        newNode->next = std::move(oldNode);
		if (prev)
            prev->next = std::move(newNode);

    }

    void erase() {
        if (next)
        {
			next->prev = prev;
        }
        if (prev)
        {
			prev->next = std::move(next);
        }
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List copied\n");
  //      auto newHead = std::make_unique<Node>(other.head->value);
		//for (auto curr = other.front()->next.get(); curr; curr = curr->next.get()) {
		//	std::cout << curr->value << std::endl;
		//	auto newNode = std::make_unique<Node>(curr->value);
		//	newHead->prev = newNode.get();
		//	newNode->next = std::move(newHead);
  //          //newNode->prev = newHead.get();
  //          //newHead->next = std::move(newNode);

  //          //newNode = std::move(newHead);
		//	newHead = std::move(newNode);

		//}

  //      Node* headPtr = newHead.get();
  //      if (headPtr != nullptr)
  //      {
		//	while (headPtr->next.get())
		//	{
		//		headPtr = headPtr->next.get();
		//	}

  //      }

  //      std::cout << "---------------" << std::endl;
  //      std::cout << headPtr << std::endl;
  //      std::cout << headPtr->value << std::endl;
  //      std::cout << "---------------" << std::endl;

  //      head.reset(headPtr);
		auto tempPtr = other.head.get();
		if (tempPtr != nullptr)
		{
			while (tempPtr->next.get())
			{
				tempPtr = tempPtr->next.get();

			}

			for (auto curr = tempPtr; curr != other.head.get(); curr = curr->prev)
			{
				int _value = curr->value;
				push_front(_value);
			}

			push_front(other.head->value);
		}
		else
		{
			head = nullptr;
		}
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        if (head != nullptr)
        {
			int ret = head->value;
			head = std::move(head->next);
			return ret;

        }
    }

    void push_front(int value) {
        auto node = std::unique_ptr<Node>(new Node(value));
        if (head)
        {
            head->prev = node.get();
        }

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

void print(const List& lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
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
    a.at(3)->insert(90);

    print(a);   // [ 1 4 2 90 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]
    //b.pop_front();
    //print(b);   // [ 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
