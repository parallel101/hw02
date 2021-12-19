/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复 -> 两个shared_ptr前后节点循环引用
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了! -> 每个Node的生命周期由上一个节点管理

    int value;

    explicit Node(int value) : value(value), prev(nullptr) {}  // 有什么可以改进的？-> 声明为explicit 避免implicit casting，初始化prev指针（POD）

    void insert(int value) {
        auto node = std::make_unique<Node>(value);
        node->value = value;
        node->next = std::move(next);
        node->prev = prev;
        if (next)
            next->prev = node.get();
        if (prev)
            prev->next = std::move(node);
    }

    void erase() {
        // prev->next manages the lifecycle of current Node;
        // so it is important to set next->prev before setting prev->next.
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
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**

        auto curr=other.front();
        // tail insertion step 1. go to the tail
        while (curr->next.get()) {
            curr = curr->next.get();
        }
        // tail insertion step 2. traverse from tail to head
        while (curr) {
            push_front(curr->value);
            curr = curr->prev;
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？ 
    // -> l1=l2 => l1=List(l2), 先调用拷贝构造函数，然后调用移动复制函数

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        // TODO: 边界检查
        return head.get();
    }

    int pop_front() {
        // TODO: 边界检查
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if (head.get())
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            // TODO: 边界检查
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List& lst) {  // 有什么值得改进的？-> 传递引用常量，避免拷贝
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
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
