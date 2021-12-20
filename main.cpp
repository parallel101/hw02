/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    //会造成循环引用
    std::unique_ptr<Node> next;
    Node * prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？　
    //　改成下面的形式
    explicit Node(int val): next(nullptr), prev(nullptr), value(val) {
    }

    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->next = std::move(this->next);

        if(node->next != nullptr)
        {
            node->next->prev = node.get();
            node->prev = node->next->prev;
        }
        this->next = std::move(node);
    }

    void erase() {

        if(next)
        {
            next->prev = this->prev;
        }

        if(this->prev)
        {
            this->prev->next = std::move(next);
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

        //使用链表的形式　进行操作，　首先要排除没有初始化的链表
        if(other.head == nullptr)
            this->head.reset();
        else
        {
            this->head = std::make_unique<Node>(other.head->value);
            auto *next = other.head->next.get();
            auto * current = this->head.get();
            while(next != nullptr)
            {
                current->insert(next->value);
                current = current->next.get();
                next = next->next.get();
            }
        }


    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 后面没有调用赋值

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
         node->next = std::move(this->head);
         if(node->next)
         {
             node->next->prev = node.get();
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

void print(const List& lst) {  // 有什么值得改进的？　改成传引用，而且没有要修改的地方，　所以用ｃｏｎｓｔ
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

    List b = a;　// 调用的是拷贝构造函数

    a.at(3)->erase();

    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]

    b = {};
    a = {};

    return 0;
}
