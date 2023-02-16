/* 基于智能指针实现双向链表 */
// #include <cstdio>
#include <iostream>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复: 循环引用
    // std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;

    int value;

    // 这个构造函数有什么可以改进的？
    // 1. 加入explicit修饰：避免发生隐式转换
    // 2. 使用初始化列表来初始化成员变量：更规范，效率更高
    explicit Node(int val): value(val) {
    }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        if (next)
            next->prev = node.get();   // 原始指针
        node->next = std::move(next);  // next节点操作完成，转移控制权
        node->prev = prev;
        if (prev)
            prev->next = std::move(node); // node节点操作完成，转移控制权
    }

    void erase() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        std::cout<<"~Node()\n";   // 应输出多少次？为什么少了？应输出 x 次，x 为 Node 总数。循环引用导致 Node 无法正常解析
    }
};

template <typename T>
struct List {
    // std::shared_ptr<Node> head;
    // 链表的第一个节点，RAII
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        if (other.head == nullptr)
            head = nullptr;
        else {
            head = std::make_unique<Node>(other.head->value);
            auto curr = head.get();
            auto other_next = other.head->next.get();
            while (other_next != nullptr) {
                auto new_node = std::make_unique<Node>(other_next->value);
                new_node->prev = curr;
                curr->next = std::move(new_node);
                curr = curr->next.get();
                other_next = other_next->next.get();
            }
        }
        
    }

    // 为什么删除拷贝赋值函数也不出错？
    // 当用户调用v2 = v1时，因为拷贝赋值函数被删除，编译器会尝试：v2 = List(v1)
    // 从而先调用拷贝构造函数，然后因为 List(v1) 相当于就地构造的对象(右值)，
    // 从而变成了移动语义，从而进一步调用移动赋值函数
    List &operator=(List const &) = delete;  

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

// 有什么值得改进的？
// 1.参数改为常量引用，避免不必要的拷贝
// 2.printf需要指定数据类型，不通用，改为cout
template <typename T>
void print(const List<T>& lst) {  
    // printf("[");
    std::cout << "[";
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        // printf(" %d", curr->value);
        std::cout << ' ' << curr->value; 
    }
    // printf(" ]\n");
    std::cout << "]\n";
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

    auto b = a;

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    std::cout << "empty list b:" << std::endl;
    b = {};
    std::cout << "empty list a:" << std::endl;
    a = {};

    return 0;
}
