/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    //std::shared_ptr<Node> next;
    //std::shared_ptr<Node> prev;
    // 如果能改成 unique_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;
    int value;

    // 这个构造函数有什么可以改进的？
    Node(int val) : value{val} {
    }
    void insert(int val) { //本来那个insert好像有问题?
        auto node = std::make_unique<Node>(val);
        node->next = std::move(prev->next);
        prev->next = std::move(node);
        prev->next->prev = prev;
        prev = prev->next.get();  // 应该是在当前node前面插入了值为val的node
    }

    void erase() {  //删除当前的Node?
        if(next){
            next->prev = prev;
        }
        if(prev){
            prev->next = std::move(next);
        }
        //prev = nullptr; //是否需要做这一步? ??为什么做了会报错??? 因为此时已经没有东西指向当前node,根本访问不到prev;
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head {nullptr};
    List() = default;
/*
    List(List const &other) {
        printf("List 被拷贝！\n");
        head = other.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
    }*/
    List(List const &other){
        //创建一个普通指针指向other
        Node* ptr1 = other.head.get();
        Node* ptr2;
        //如果要复制的list不为空
        if(ptr1!=nullptr){
            head =std::move(std::make_unique<Node>(ptr1->value)); //移动构造函数?
            ptr2 = head.get();
            while(ptr1->next.get()){
                ptr1 = ptr1->next.get();
                ptr2->next =std::move(std::make_unique<Node>(ptr1->value)); 
                ptr2 = ptr2->next.get();
            }
        }
        else{
            head = nullptr; //其实不写也可以,默认值是nullptr;
        }


    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        if(head!=nullptr){
            int ret = head->value;
            head = std::move(head->next);
            head->prev = nullptr;  //将当前头部的prev指向空. 而原来的头部node没有uniqe_ptr指着应该会被释放.
            return ret;
        }
        //其实最好如果是空链表也返回一个值
    }

    void push_front(int value) { 
        auto node = std::make_unique<Node>(value);
        if(head){
            head->prev = node.get();
        }
        node->next =std::move(head);
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

void print(const List &lst) {  // 有什么值得改进的？ //改为常引用可以省掉每次的复制构造和数据修改
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
