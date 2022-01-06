/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
struct Node {
    // 这两个指针会造成什么问题？请修复
    //子节点只属于父节点用 unique_ptr
    std::unique_ptr<Node> next;
    //父对象不属于自己，直接用指针
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    //避免初始值问题
    int value{0};

    // 这个构造函数有什么可以改进的？
    //避免隐式转换，并且用初始化列表
    explicit Node(int val) :value(val),prev(nullptr),next(nullptr){
    }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        //在当前节点后面插入
        node->next = std::move(next);//next is gone
        if(node->next){
            node->prev = node->next->prev;
            node->next->prev = node.get();
        }
        next = std::move(node);
    }

    void erase() {//擦掉当前的节点
        if (next) {
            printf("erase next\n");
            next->prev = prev;
        }
        if (prev) {
            printf("erase prev\n");
            prev->next = std::move(next);
        }
            
    }

    ~Node() {
        static int i = 0;
        i++;
        printf("~Node() %d\n", i);   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // // 请实现拷贝构造函数为 **深拷贝**
        head.reset();
        if(!other.front()) return;
        Node* p2 = other.front();
        head = std::make_unique<Node>(p2->value);//要创建新的
        Node* p1 = head.get();
        while (p2->next)
        {
            p1->insert(p2->next->value);
            p1 = p1->next.get();
            p2 = p2->next.get();
        }
        
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？根本就没调用啊

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
        node->next = std::move(head);//head is gone!!!!! 被坑了
        if (node->next)
            node->next->prev = node.get();
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

void print(List const &lst) {  // 有什么值得改进的？只读，引用传参
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
