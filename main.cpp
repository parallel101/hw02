/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    
    // 如果能改成 unique_ptr 就更好了!
    
    // 改进1: 使用unique_ptr
    std::unique_ptr<Node> next;
    Node *prev;

    int value;

    // 这个构造函数有什么可以改进的？
    // Node(int val) {
    //     value = val;
    // }

    // 改进2：使用初始化列表
    Node(int val) : value(val), next(nullptr), prev(nullptr) {}

    // void insert(int val) {
    //     auto node = std::make_shared<Node>(val);
    //     node->next = next;
    //     node->prev = prev;
    //     if (prev)
    //         prev->next = node;
    //     if (next)
    //         next->prev = node;
    // }

    // 改进3: 修改插入逻辑，在当前节点后插入新值
    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->prev = this;
        if (next) {
            next->prev = node.get();
            node->next = std::move(next);
        }
        next = std::move(node);
    }

    // void erase() {
    //     if (prev)
    //         prev->next = next;
    //     if (next)
    //         next->prev = prev;
    // }

    // 改进4: 修改删除逻辑，删除当前节点的后继节点
    void erase() {
        if (next) {
            if (next->next) {
                next->next->prev = this;
            }
            next = std::move(next->next);
        }
    }

    // 改进5: 修改析构函数，输出当前节点的值
    ~Node() {
        printf("~Node(%d)\n", value);   // 应输出多少次？为什么少了？
    }
};

struct List {
    // std::shared_ptr<Node> head;
    // 修复1: 这里必须同步改为 unique_ptr，否则无法与 Node::next 兼容
    std::unique_ptr<Node> head;

    List() = default;

    // List(List const &other) {
    //     printf("List 被拷贝！\n");
    //     head = other.head;  // 这是浅拷贝！
    //     // 请实现拷贝构造函数为 **深拷贝**
    // }

    // 改进6：实现真正的深拷贝
    List(List const &other) {
        printf("List 被拷贝！进行深拷贝...\n");
        if (!other.head) return;

        // 拷贝第一个节点
        head = std::make_unique<Node>(other.head->value);
        
        Node *curr = head.get();
        Node *other_curr = other.head->next.get();

        // 顺序拷贝后续节点
        while (other_curr) {
            curr->next = std::make_unique<Node>(other_curr->value);
            curr->next->prev = curr;
            curr = curr->next.get();
            other_curr = other_curr->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    // 问：为什么可以删除拷贝赋值函数？
    // 答：因为在 main 中的 "List b = a;" 是初始化行为，调用的是拷贝构造函数而非赋值运算符。
    // 如果没有类似 "b = a;" (在初始化之后赋值) 的代码，编译器就不需要拷贝赋值函数。

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        // head = head->next;
        
        // 改进7: 使用 std::move
        head = std::move(head->next);
        if (head) head->prev = nullptr;
        
        return ret;
    }

    void push_front(int value) {
        // 修复2: 这里也应该统一使用 make_unique
        auto node = std::make_unique<Node>(value);
        // node->next = head;
        // if (head)
        //     head->prev = node;
        // head = node;

        // 改进8: 使用 std::move
        if (head) {
            head->prev = node.get();
            node->next = std::move(head);
        }
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        
        // 改进9: 修改for循环终止条件
        for (size_t i = 0; i < index && curr; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

// void print(List lst) {  // 有什么值得改进的？
//     printf("[");
//     for (auto curr = lst.front(); curr; curr = curr->next.get()) {
//         printf(" %d", curr->value);
//     }
//     printf(" ]\n");
// }

// 改进10：使用 const & 避免按值传递导致的 List 拷贝
void print(List const &lst) {
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

    // 改进11：显式清空，触发析构输出
    b = {};
    a = {};

    return 0;
}
