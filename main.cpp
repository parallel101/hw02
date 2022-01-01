/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复,循环引用
    std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    // std::unique_ptr<Node> next;
    // Node * prev;

    int value;

    // 这个构造函数有什么可以改进的？1. 防止隐式转换可以使用explict 2. 使用初始化列表,省去一次构造
    explicit Node(int val):value{val} {
        // value = val;
    }

    void insert(int val) {
        auto node = std::make_shared<Node>(val);
        node->next = next;
        node->prev = prev;
        if (!prev.expired())
            prev.lock()->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        if (!prev.expired())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？1(116 line)+1(122 line)+6(128 line)+5(129 line)=13次。少了的原因有2个：1.浅拷贝 2.都是共享指针，循环引用导致引用计数不可能变为0
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        head=std::make_shared<Node>(other.head->value);
        auto pn=head;
        for(auto p=other.head->next;p;p=p->next){
            auto node=std::make_shared<Node>(p->value);
            node->prev=std::weak_ptr<Node>(pn);
            pn->next=node;
            pn=node;
        }
        // printf("List 被拷贝！\n");
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？尝试先调用拷贝构造函数,然后在调用移动复制函数

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head = head->next;
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_shared<Node>(value);
        node->next = head;
        if (head)
            head->prev = node;
        head = node;
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List &lst) {  // 有什么值得改进的？使用引用传递
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

    a.at(2)->erase(); //析构函数1次

    print(a);   // [ 1 4 2 8 5 7 ]

    List b = a; //临时对象析构

    a.at(3)->erase(); //析构函数1次

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    //深拷贝调用5+6次，浅拷贝调用5次
    b = {}; // 6次析构函数
    a = {}; // 5次析构函数

    return 0;
}
