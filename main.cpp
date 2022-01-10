/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) : value(val) {}

    void insert(int val) {  // 在此节点之后插入
        auto node = std::make_unique<Node>(val);
        next->prev = node.get();
        node->next = std::move(next);
        next = std::move(node);
        next->prev = this;
    }

    void erase() {
        if (prev){
            prev->next = std::move(next);
        }
        if (next){
            next->prev = std::move(prev);
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
        Node* ptr1 = other.front(); 
        if(ptr1 == nullptr){
            return;
        }
        head = std::make_unique<Node>(ptr1->value);
        Node* ptr2 = head.get();
        ptr1 = (ptr1->next).get();
        while(ptr1 != nullptr){
            ptr2->next = std::make_unique<Node>(ptr1->value);
            ptr2->next->prev = ptr2;
            ptr1 = ptr1->next.get();
            ptr2 = ptr2->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？答：因为已经实现了移动复制函数，编译器进行就地构造

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        if(head.get() == nullptr){
            return -1;
        }
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        if (head){
            head->prev = node.get();
        }
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

void print(const List& lst) {  // 有什么值得改进的？答：改为常引用，避免拷贝
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
