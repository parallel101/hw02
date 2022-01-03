/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复--share_ptr的循环引用
    std::shared_ptr<Node> next;
    // std::shared_ptr<Node> prev;
    std::weak_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？-- 这是先初始化再赋值，多了一步操作，
    Node(int val):value(val) {
        // value = val;
    }

    void insert(int val) {
        auto node = std::make_shared<Node>(val);
        node->next = next; 
        node->prev = prev;
        // if (prev)
        //     prev->next = node;
        if (!prev.expired())//lock()产生share_ptr
            prev.lock()->next = node;
        if (next)
            next->prev = node;
    }

    void erase() {
        // if (prev)
        //     prev->next = next;
        if (!prev.expired())
            prev.lock()->next = next;
        if (next)
            next->prev = prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？-- 13次，循环引用不会析构
    }
};

struct List {
    std::shared_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！---复制出来的指针还是指向同一块内存--有double free的问题
        // 请实现拷贝构造函数为 **深拷贝**
        head = std::make_shared<Node>(other.head->value);//深拷贝利用make_share申请内存，一个一个节点复制过去
        auto tmp_cur = head;
        auto tmp = other.head;
        while(tmp->next)
        {
            auto tmp_next = std::make_shared<Node>(tmp->next->value);
            tmp_cur->next = tmp_next;
            tmp_next->prev = tmp_cur;
            tmp = tmp->next;
            tmp_cur = tmp_next;

        }        


    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？--因为定义了移动赋值函数，赋值时会自动用无参构造和移动赋值

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

void print(List const& lst) {  // 有什么值得改进的？--传常引用避免拷贝
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
