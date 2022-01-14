/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    // 会造成循环引用, 在删除一个Node后,由于仍有Node指向该对象,因此资源不会被释放
    // std::unique_ptr<Node> next;
    // Node* prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;

    int value;

    // 这个构造函数有什么可以改进的？
    explicit Node(int val) : value(val)
    {}

    // 保证有一个结点前提,向当前结点后插入一个新结点
    void insert(int val) {
        // 此处node为暂时的指针,真正的资源由this.next管理
        auto node = std::make_unique<Node>(val);  
        node->next = std::move(next);   // this.next为空
        // node->next=原始的this.next两个都不为空
        if (node->next) {
            // 利用原始的this.next获取prev=this, 必不为空
            node->prev = node->next->prev; 
            node->next->prev = node.get();  // 需要原始指针
        }
        next = std::move(node);  // 移交所属权给this.next
    }


    void erase() {
        if (prev)
            prev->next = std::move(next);
        if (next)
            next->prev = prev;
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
        // 先创建头指针
        head = std::make_unique<Node>(other.front()->value);
         // 这里要获取原始指针,unique_ptr不支持拷贝赋值,不便迭代
         // 否则将只能在head后面插入, 破坏顺序
        auto n_head = head.get();  

        for (auto curr = other.front()->next.get(); curr;
        curr = curr->next.get(),n_head = n_head->next.get())
            n_head->insert(curr->value);
        
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

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

    // 这里与insert类似,只不过不用考虑node->prev(因为他是新的头指针)
    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        node->next = std::move(head); // node->next指向原来head指向的对象, head清空
        if (node->next)
            node->next->prev = node.get();
        head = std::move(node);  // 移交给head
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

void print(List const &lst) {  // 有什么值得改进的？
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
