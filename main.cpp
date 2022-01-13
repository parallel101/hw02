/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>

struct Node {
    // 这两个指针会造成什么问题？请修复
    /* edit by Liu: 问题 容易引起内存泄漏 如直接将head=nullptr	*/
    /* edit by Liu: 后面有指着他的节点 故链表不会被删除		*/
    /* edit by Liu: 要彻底删除链表要写循环 很麻烦	 	*/
    /* edit by Liu: 未修改前少输出的"~Node()"就是这个原因 	*/
    std::unique_ptr<Node> next;
    Node * prev;
    // 如果能改成 unique_ptr 就更好了!
    /* edit by Liu: 因为头节点 head和其后节点的prev都要指向它	*/
    /* edit by Liu: 所以head用unique prev一定不能用unique	*/
    /* edit by Liu: 所以next用了unique 而prev用了老指针		*/

    int value;

    // 这个构造函数有什么可以改进的？
    /* edit by Liu: 改变了初始化的方式 禁止使用隐式类型转换 	*/
    explicit Node(int val) : value(val), prev(NULL), next(nullptr) {};
    
    /* edit by Liu: 这里的insert是什么意思没看懂 		*/
    /* edit by Liu: 为什么是把自己替换了 不是插一个节点吗 	*/
    void insert(int val) {
        auto node = std::make_unique<Node>(val);
	node->next = std::move(next);
	node->prev = prev;
        if (next)
            next->prev = node.get();
        if (prev)
            prev->next = std::move(next);
    }

    void erase() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = std::move(next);
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    /* edit by Liu: 应该是13次 此代码输出为13次 	*/
    /* edit by Liu: 但是未修改前因为share_ptr造成的问题	*/
    /* edit by Liu: 导致内存泄露没有释放链表占的内存	*/
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;
    //List() = delete;
    //explicit List() = default;
    List(int val) : head(std::make_unique<Node>(val)) {};

    List(List const &other) {
        printf("List 被拷贝！\n");
        // 请实现拷贝构造函数为 **深拷贝**
    /* edit by Liu: 因为unique不能赋值 还是用的老指针处理数据 	*/
	head = std::make_unique<Node>(other.head->value);
	for (auto ptr_m=front(),ptr_o=other.front()->next.get(); ptr_o; ptr_m=ptr_m->next.get(),ptr_o=ptr_o->next.get()) {
	    ptr_m->next = std::make_unique<Node>(ptr_o->value);
	    ptr_m->next->prev = ptr_m;
	    ptr_m->next->value = ptr_o->value;
	}
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    /* edit by Liu: 因为编译器认为a={}调用了移动赋值函数 	*/
    /* edit by Liu: 将{}处理为空的默认构造函数List()	 	*/
    /* edit by Liu: 如果将空的默认构造函数删除 编译就不过 	*/
    /* edit by Liu: 事实默认构造函数不允许隐式类型转换也编译不过*/

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
/* edit by Liu: 传引用不会调用复制构造函数，print函数不会改变值，所以用常引用 */
void print(const List & lst) {  // 有什么值得改进的？
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List a;
    //List a(11);

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
