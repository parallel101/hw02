/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

template<class T>
struct Node {
    // 这两个指针会造成什么问题？请修复
    // 会造成循环引用
    std::unique_ptr<Node<T>> next;
    Node<T> *prev;
    // 如果能改成 unique_ptr 就更好了!

    T value;

    // 这个构造函数有什么可以改进的？
    // 改为 member list initilization 并初始化所有成员
    Node(T _val): next{nullptr}, prev{nullptr}, value{_val} {}

    // 头插法
    void insert(T val) {
        std::unique_ptr<Node<T>> node = std::make_unique<Node<T>>(val);
        if (next)
            next->prev = node.get();
        node->next = std::move(next);
        node->prev = this;
        next = std::move(node);
    }

    // 将前后两个 Node 链接，本身 Node 没有 unique_ptr 管理所有权，析构
    void erase() {
        if (next) 
            next->prev = prev;
        if (prev != nullptr) 
            prev->next = std::move(next);
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？:之前有循环引用，导致内存泄露
    }
};

template<typename T>
struct List {
    std::unique_ptr<Node<T>> head;

    List() = default;

    List(List<T> const &other) {
        printf("List 被拷贝！\n");
        Node<T> *tmp;
        // 请实现拷贝构造函数为 **深拷贝**
        for (Node<T>* curr = other.front(); curr != nullptr; curr = curr->next.get()) {
            if(head) {
                tmp->next = std::make_unique<Node<T>>(curr->value);
                tmp = tmp->next.get();
            } else {
                head = std::make_unique<Node<T>>(curr->value);
                tmp = head.get();
            }
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 因为在拷贝赋值的 call site 可以变成  拷贝构造(构造出一个右值) + 移动赋值 的调用

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node<T> *front() const {
        return head.get();
    }

    T pop_front() {
        if (!head) {
            throw std::out_of_range("pop_front()");
        }
        T ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(T value) {
        if(!head) {
            head = std::make_unique<Node<T>>(value);
        }
        else {
            auto node = std::make_unique<Node<T>>(value);
            head->prev = node.get();
            node->next = std::move(head);
            node->prev = nullptr;
            head = std::move(node);
        }
    }

    Node<T> *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; curr != nullptr && i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template<typename T>
void print(const List<T> &lst) {  // 参数类型改为 const ref 
    printf("[");
    for (auto curr = lst.front(); curr != nullptr; curr = curr->next.get()) {
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
