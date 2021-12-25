/* 基于智能指针实现双向链表 */
#include <cstddef>
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 存在循环引用，会导致无法自动释放，典型场景：双节点成环
    // std::shared_ptr<Node> next;
    // std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node *prev;

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) {
        value = val;
        next = nullptr;
        prev = nullptr;
    }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);

        node->next = std::move(next);
        node->prev = prev;

        if (next)
            next->prev = node.get();
        if (prev)
            prev->next = std::move(node);
    }

    void erase() {
        if (prev)
            prev->next = std::move(next);
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other)  {
#define DEEP_COPY
#ifndef DEEP_COPY
        printf("List 被浅拷贝！\n");
        head = other.head;  // 这是浅拷贝！
#else
        // 请实现拷贝构造函数为 **深拷贝**;
        printf("List 被深拷贝！\n");
        if (other.head)
        {
            head = std::make_unique<Node>(other.head->value);
        }
        Node *p = other.head.get();
        Node *q = head.get();
        while (p->next)
        {
            q->next = std::make_unique<Node>(p->next->value);
            q->next->prev = q;
            q = q->next.get();
            p = p->next.get();
        }
#endif
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 因为当拷贝赋值不行时，会尝试通过构造函数，隐式类型转换，构造赋值, 构造函数加上explicit就会编译出错。

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

void print(const List &lst) {  // 有什么值得改进的？ 应该改用常量引用，避免触发类型转换构造或者是拷贝赋值
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
