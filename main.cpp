/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <utility>
#include "stdexcept"
struct Node {
    // 这两个指针会造成什么问题？请修复
    /*
    这两个指针的原本设计会导致在执行main()中如下代码时发生循环引用：
        List a;
        a.push_front(7);
        a.push_front(5);
    在a.push_front(5);执行完毕后，Node 5的引用计数为2而Node 7的引用计数为1，
    因此即使a.head最后置空，a原本管理的Node也不会触发析构，导致内存泄露。
    */
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    /*
    当前的构造函数使用了函数体内的赋值，这实际上会做这些事情：
    1. value会被默认初始化成一个未定义值（可能是任意值）
    2. 进入构造函数体，执行value = val，进行赋值操作
    并且当前的构造函数没有初始化next和prev指针

    建议修改成初始化列表，他可以直接使用val参数来初始化value成员
    （对于 int这样的基本类型，性能差异不明显，主要是代码规范问题。
    但，比如说对于std::string这样的类型，初始化列表会直接调用合适的构造函数（如拷贝构造），
    避免先默认构造再赋值的开销，对于很长的字符串，性能优势就很明显了）
    */
    Node(int val) : value(val), next(nullptr), prev(nullptr) {}

    void insert_after_this_node(int val) {
        auto node = std::make_unique<Node>(val);
        
        // 先保存原来的 next 指针
        Node* old_next = this->next.get();  // 保存 B 的指针
        
        node->next = std::move(this->next);  // X.next 指向 B
        node->prev = this;                   // X.prev 指向 A
        
        this->next = std::move(node);        // A.next 指向 X
        
        // 更新后继节点的prev指针
        if (old_next) {                      // 如果原来有 B
            old_next->prev = this->next.get();  // B.prev = X
        }
    }
    void erase() {
        if (next) {
            next->prev = prev;
        }
        if (prev) {
            prev->next = std::move(next);
        }
        /*
        本来this节点被他的prev->next所持有
        现在prev->next指向this.next了
        所以this节点已经没有 unique_ptr 指向他了，触发unique_ptr的自动析构
        */
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
        /*
            本来应该输出13次，但是实际输出了37次
            因为 void print(List lst) 使用值传递作为参数
            这就导致每次调用void print(List lst)时都会调用拷贝构造函数生成一个临时的List对象lst
            这需要复制整个链表（7个节点）
            然后，一旦void print(List lst)执行完毕，这个临时对象lst会被析构，于是触发所有节点的析构函数
            导致输出很多次 "~Node()"，一共是37次
            将 void print(List lst) 改成引用传递即可
        */
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(const List &other) {
        printf("List 被拷贝！\n");
        // head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        if (!other.head) {
            head = nullptr;
            return;
        }
        head = std::make_unique<Node>(other.head->value);
        Node * other_current = other.head->next.get();
        Node * current = head.get();
        Node * prev_node = nullptr;
        while (other_current != nullptr) {
            // 另找一块内存创建一个新节点
            current->next = std::make_unique<Node>(other_current->value);
            current->prev = prev_node;

            prev_node = current;
            current = current->next.get();
            other_current = other_current->next.get();
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    /*
        因为main()函数中没有使用拷贝赋值操作
            List b = a;    // 这是拷贝构造，不是拷贝赋值
            b = {};        // 这是移动赋值，不是拷贝赋值
            a = {};        // 这也是移动赋值
        
        如果main()中写这样的代码就会出错：
           List c, d;
           c.push_front(1);
           d = c;  // 编译错误！这里调用拷贝赋值函数，但该函数被删除了
    */

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        if (head == nullptr) {
            throw std::out_of_range("Cannot pop from empty list");
        }
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }

    void push_front(int value) {
        auto node = std::make_unique<Node>(value);
        Node* old_head = head.get();
        node->next = std::move(head);
        if (old_head) {
            old_head->prev = node.get();
        }
        head = std::move(node);
    }

    Node *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index && curr; i++) {
            curr = curr->next.get();
        }
        return curr;  // 如果超出范围，返回 nullptr
    }
};

void print(const List& lst) {  // 有什么值得改进的？
    /*
        原本采用值传递，改成const引用传递可以显著减少不必要的拷贝和析构，并且表达该函数不会修改原链表的语义
    */
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