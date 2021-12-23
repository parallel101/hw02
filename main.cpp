/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) {
        value = val;
    }

    Node(Node const& other) {
        value = other.value;
    }

    Node &operator=(Node const& other) {
        this->~Node();
        new (this) Node(other);
        return *this;
    }

    void insert(int val) {
        auto node = std::make_shared<Node>(val);
        node->next = next;
        std::shared_ptr<Node> temp = prev.lock();
        node->prev = temp;
        if (temp)
            temp->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        std::shared_ptr<Node> temp = prev.lock();
        if (temp)
            temp->next = next;
        if (next)
            next->prev = temp;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List is copied!\n");
        //head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        auto node = std::make_shared<Node>(*(other.head));
        head = node;
        std::shared_ptr<Node> temp = other.head;
        while (temp->next) {
            auto nodeNew = std::make_shared<Node>(*(temp->next));
            node->next = nodeNew;
            nodeNew->prev = node;
            node = node->next;
            temp = temp->next;
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = head->next;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_shared<Node>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List &lst) {  // 有什么值得改进的？
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
    a.at(3)->insert(10);
    print(a);   // [ 1 4 2 10 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
