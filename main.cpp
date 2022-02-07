/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <cstring>

#include <iostream>

template <class T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    std::shared_ptr<Node<T>> next;
    std::weak_ptr<Node<T>> prev;
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    Node(const T &val) : value(val) {}

    void insert(const T &val) {
        auto node = std::make_shared<Node<T>>(val);
        node->next = next;
        node->prev = prev;
        if (prev.expired())
            prev.lock()->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        if (prev.expired())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        // std::cout << prev.use_count() << std::endl;
        // std::cout << next.use_count() << std::endl;
        printf("~Node()\n");   // 应输出多少次？为什么少了？
        /*
            用use_count 测试了一下显示被循环引用了。
        */
    }
};

template <class T>
struct List {
    std::shared_ptr<Node<T>> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        memcpy(head.get(), other.head.get(), sizeof(std::shared_ptr<Node<T>>));
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    /*
    好像这里直接隐式调用了拷贝构造。当加入explicit就全错了
    */

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node<T> *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = head->next;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_shared<Node<T>>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
    }

    Node<T> *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template <class T>
void print(List<T> lst) {  // 有什么值得改进的？
    /*
         还不太懂数据结构不会改。
    */
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

    print<int>(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print<int>(a);   // [ 1 4 2 8 5 7 ]

    List<int> b = a;

    a.at(3)->erase();

    print<int>(a);   // [ 1 4 2 5 7 ]
    print<int>(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
