/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 若都用 shared_ptr 会造成循环引用，导致内存泄漏
    std::unique_ptr<Node> next; // 当前node拥有对其后继的所有权
    Node *prev;
    int value;

    // 添加 explicit 避免隐式转换
    // 改写为初始化列表，避免重复初始化
    explicit Node(int val) : prev(nullptr), next(nullptr), value(val)
    {}

    void insert(int val) { // replace?
        auto node = std::make_unique<Node>(val);
        node->prev = prev;
        if (next) {
            next->prev = node.get();
            node->next = std::move(next);
        }
        if (prev) {
            prev->next = std::move(node);
        }
    }

    void erase() {
        if (next) {
            next->prev = prev;
        }
        if (prev) {
            prev->next = std::move(next);
        }
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
        if (!other.head) {
            head = nullptr;
        }
        else {
            head = std::make_unique<Node>(other.head->value);
            Node *ptr = head.get();
            Node *oldPtr = other.head->next.get();
            while(oldPtr) {
                ptr->next = std::make_unique<Node>(oldPtr->value); // construct and move
                ptr->next->prev = ptr;              
                ptr = ptr->next.get();
                oldPtr = oldPtr->next.get();
            }
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // ans: 后面的 List b = a; 调用拷贝构造而不是拷贝赋值，因而删去不影响

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
        if(head) {
            head->prev = node.get();
            node->next = std::move(head);
        }
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

// 改为传引用，避免不必要的拷贝
void print(List const &lst){
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
    List b = a; // 拷贝构造

    a.at(3)->erase(); 

    print(a);   // [ 1 4 2 5 7 ]   
    print(b);   // [ 1 4 2 8 5 7 ] 

    b = {};
    a = {};

    return 0;
}
