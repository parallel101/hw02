/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复。
    std::unique_ptr<Node> next;
    Node *prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val): value(val) {}

    void insert(int val) {
        value = val;
    }

    void erase() {
        if (next) next->prev = prev;
        if (prev) prev->next = std::move(next);
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
        // 请实现拷贝构造函数为 **深拷贝**
        if (other.head == nullptr) return;  // 空链表
        Node *it = other.front(), *tail;
        head = std::make_unique<Node>(it->value);
        for (it = it->next.get(), tail = head.get(); it; it = it->next.get()) {
          tail->next = std::make_unique<Node>(it->value);
          tail->next->prev = tail;
          tail = tail->next.get();
        } 
    }

    List &operator=(List const &) = delete; // 为什么删除拷贝赋值函数也不出错？
    // 因为编译器会用“解构函数+拷贝构造”代替拷贝赋值。

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        if (head == nullptr) return 0x114154;
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = new Node(value);
        if (head)
            head->prev = node;
        node->next = std::move(head);
        head = std::unique_ptr<Node>(node);
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
    for (auto curr = lst.front(); curr; curr = curr->next.get()) 
        printf("this: %p prev: %p next: %p value: %d\n",
            curr, curr->prev, curr->next.get(), curr->value);
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
