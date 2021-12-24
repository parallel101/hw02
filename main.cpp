/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <iostream>

struct Node {
    
    int value;
    std::unique_ptr<Node> next;
    Node* prev;
  
    

    /*
      使用初始化列表，减少可能存在的多次初始化，不过由于存在均为安全类型，所以初始化列       表与直接赋值差别不大，这里并没有给value进行默认初始化一个值，因为不敢确保默认初       始化这个值是否会代表特定的意义
    */
  Node(int val): value(val)
    {}
   
    void insert(int val) {
        auto node = std::make_unique<Node>(val);
        node->next = std::move(next);
        node->prev = prev;
        if (prev)
	  prev->next = std::move(node);
        if (next)
	  next->prev = node.get();
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

    // List() = default;
    List(){
      printf("List 无参数初始化成功 \n");
    }
  
    List(List const &other) {
        printf("List 被拷贝！\n");
        // head = other.head;
	// 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
	if(!other.front()) return;
	head = std::make_unique<Node>(other.front()->value);
	auto tmp = (other.front()->next).get();
	auto tmpNewList = head.get();
	while(tmp){ 
	  tmpNewList->next = std::make_unique<Node>(tmp->value);
	  tmpNewList->next->prev = tmpNewList;
	  tmpNewList = (tmpNewList->next).get();
	  tmp = tmp->next.get();
	}
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    List(List &&) = default;
    List &operator=(List &&) = default;

    Node *front() const {
        return head.get();
    }

    int pop_front() {
        int ret = head->value;
        head =std::move(head->next);
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

void print(List &lst) {  // 有什么值得改进的？
  printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
  /*
    正确的结果应该为List a调用了无参数的默认构造函数，print函数使用的引用因此只会打印    结果，erase函数删除了节点，会调用该节点的析构函数，List b=a,会调用拷贝构造函数，    最后b，a均被置为空，会调用a,b各自节点的析构函数，应该是11个析构函数输出
  */
    //a调用了无参数的默认构造函数
    List a;
    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);
    /*
      print函数是值传递，因此会调用拷贝构造函数形成实参；
      同时脱离函数作用域时会调用析构函数，析构实参
    */
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
