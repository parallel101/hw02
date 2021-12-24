/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复，会构成循环引用，可以把prev变为weak_ptr.
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？ ans: 使用explicit，防止隐式转换; 改为初始化表达式，减少一次赋值操作。
    explicit Node(int val): value(val), prev(nullptr) {
    }

    void erase() {
        if (next){
            next->prev = prev;
        }
            
        if (prev){
            prev->next = std::move(next);
        }
        
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？ans: 应该7次，因为有循环引用，无法释放剩余的5个Node。
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other): head(std::make_unique<Node>(other.head->value)) {
        printf("List 被拷贝！\n");
        // 请实现拷贝构造函数为 **深拷贝**
        Node* ori = head.get();
        for (auto curr = other.front()->next.get(); curr; curr = curr->next.get()) {
            std::unique_ptr<Node> next = std::make_unique<Node>(curr->value);
            next->prev = ori;
            ori->next = std::move(next);
            ori = ori->next.get();
        }
    }

    

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？ans：不清楚，难道是 a = {} 这里调用的移动赋值？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = std::move(head->next);
        head->prev = nullptr;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if(head)
            head->prev = node.get();
        node->next = std::move(head);
        head = std::move(node);
    }

    Node *at(size_t index) const {
        Node* curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(const List& lst) {  // 有什么值得改进的？ans: 传入const引用
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
