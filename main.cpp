/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 两个指针都使用shared_ptr的话，在相邻的节点之间会有循环引用，导致内存泄漏
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;

    int value;

    // 这个构造函数有什么可以改进的？
    // Node(int val) {
    //     value = val;
    // }
    Node(int val)
        : value(val), next(nullptr), prev(nullptr) {}

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->next = std::move(next);

        if (node->next) {
            node->prev = node->next->prev;
            node->next->prev = node.get();
        }
        next = std::move(node);
    }

    void erase() {
        // printf("erase(): %d\n", value);
        // if (prev)
        //     prev->next = std::move(next);
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        printf("~Node()\n");  // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const& other) {
        printf("List 被拷贝！\n");

        // 这是浅拷贝！
        // 浅拷贝只是增加了other.head的引用计数
        // head = other.head;

        // 请实现拷贝构造函数为 **深拷贝**
        head.reset();
        if (!other.front())
            return;

        std::unique_ptr<Node> new_head = std::make_unique<Node>(other.front()->value);
        Node* raw = other.front();
        Node* rear = new_head.get();
        while (raw->next) {
            raw = raw->next.get();
            auto new_node = std::make_unique<Node>(raw->value);
            new_node->prev = rear;
            rear->next = std::move(new_node);

            // cannot use "rear = new_node.get()" here, since new_node is unique and it is moved
            rear = rear->next.get();
        }
        head = std::move(new_head);
    }

    List& operator=(List const&) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List&&) = default;
    List& operator=(List&&) = default;

    Node* front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        node->next = std::move(head);
        if (node->next) {
            node->next->prev = node.get();
        }
        head = std::move(node);
    }

    Node* at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

// 改进传入参数形式 ，避免隐式拷贝
void print(List const& lst) {  // 有什么值得改进的？
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

    print(a);  // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();

    print(a);  // [ 1 4 2 8 5 7 ]

    List b = a;

    a.at(3)->erase();

    print(a);  // [ 1 4 2 5 7 ]
    print(b);  // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
