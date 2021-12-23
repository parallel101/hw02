/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>
#include <assert.h>
using namespace std;

struct Node {
    // 这两个指针会造成什么问题？请修复
    unique_ptr<Node> next{nullptr};
    Node *prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val): value(val) {}

    void insert(int val) {
        auto node = make_unique<Node>(val);
        // cout << 1 << endl;
    
        if (next) {
        // cout << 1 << endl;
            next->prev = node.get();
        // cout << 1 << endl;
            node->next = move(next);
        }
        // cout << 1 << endl;
        node->prev = this;
        next = move(node);
    }

    void erase() {
        if (!prev) {
            assert(false);
        }
        if (next) {
            next->prev = prev;
            prev->next = move(next);
        }
        else {
            prev->next = nullptr;
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
        printf("List 被拷贝！\n");
        head = std::make_unique<Node>(other.head->value);
        
        auto other_head = other.head->next.get();
        auto this_head = head.get();


        for (; other_head; other_head = other_head->next.get()) {
            this_head->insert(other_head->value);
            this_head = this_head->next.get();
        }
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
        head = move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if (head) {
            head->prev = node.get();
            node->next = move(head);
        }
        head = move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List const &lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

void test_node_insert() {
    auto ptr = make_unique<Node>(19);
    ptr->insert(1);
    
    cout << ptr->value << endl;
    cout << ptr->next->value << endl;
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
    // cout << "erase()" << endl;

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a;
    // b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};
    // cout << "b a destruct" << endl;

    return 0;

    // test_node_insert();
}

