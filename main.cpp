/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;
    // 类似于课上所讲的情况，两个共享指针互相指向，会造成循环引用，引用计数无法置零，无法被回收
    // 前一个指针“拥有”后一个指针

    int value;

    // 这个构造函数有什么可以改进的？
    // 使用引用增加效率
    Node(const int& val):value(val) {
    }

    void insert(int val) {
        // 插入当前结点之后，但是如果当前节点是独立的节点，似乎没有办法实现
        // 但是双向链表有头结点，而且insert函数没有被使用过
        auto node = std::make_unique<Node>(val);
        if(next)
        {
            next->prev = node.get();
            node->next = std::move(next);
        }
        node->prev = prev->next.get();
        next = std::move(node);
    }

    void erase() {
        if(next.get())
            next->prev = prev;
        if(prev)
            prev->next = std::move(next);
        // 因为调用std::move的原因，调整顺序
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
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        auto cur = other.head.get();
        if(cur) {
            while(cur->next.get()) {
                cur = cur->next.get();
            }
            while(cur->prev) {
                this->push_front(cur->value);
                cur = cur->prev;
            }
            this->push_front(cur->value);
        }
        else {
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
        // 未作非合法判断，不知道返回什么值合适，exception也还没有学过
        int ret = head->value;
        if(head->next.get())
            head = std::move(head->next);
        else
            head = nullptr;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        node->next = std::move(head);
        if (node->next.get())
            node->next.get()->prev = node.get();
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            if(curr == nullptr) // 越界处理
                return nullptr;
            curr = curr->next.get();
        }
        return curr;
    }
};

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
