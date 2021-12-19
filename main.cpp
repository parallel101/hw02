/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>

struct Node {
    // 这两个指针会造成什么问题？请修复   会造成循环引用，内存泄漏
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    Node(int value) : value(value) {}  // 有什么可以改进的？

    void insert(int value) {
        std::unique_ptr<Node> node=std::make_unique<Node>(value);
        node->prev=this;

        if(next){
            node->next=std::move(next);
            node->next->prev=node.get();
        }        
        next=std::move(node);
    }

    void erase() {
        if (next){
            next->prev = prev;
        }
        if (prev){
            prev->next=std::move(next);
        }
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？ 有多少个节点就输出多少次，少了是因为shared_ptr互相指，会造成内存泄漏。修改程序之后就不会少了。
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        
        // 这里为了实现深拷贝，不能够简单通过other里的节点的get方法提取出raw pointer，需要通过raw pointer指向的值构造智能指针
        
        head.reset();
        if(!other.front()){
            return;
        }
        std::unique_ptr<Node> new_head=std::make_unique<Node>(other.front()->value);
        // std::cout<< new_head->value << std::endl;
        Node* work=new_head.get();
        Node* other_raw=other.front();

        while(other_raw -> next){
            std::unique_ptr<Node> new_ptr=std::make_unique<Node>(other_raw->next.get()->value);
            new_ptr->prev=work;
            work->next=std::move(new_ptr);

            other_raw=other_raw->next.get();
            work=work->next.get();
            // std::cout<< work->value << std::endl;
        }
        head=std::move(new_head);
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？编译器会使用拷贝初始化（上面那个函数）

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head=std::move(head->next);
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

void print(List const & lst) {  // 有什么值得改进的？ 传引用
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