/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>
#include <cstring>

template <typename T>
struct Node {
    // 这两个指针会造成什么问题？请修复
//    std::shared_ptr<Node> next;
//    std::shared_ptr<Node> prev;
    // 如果能改成 shared_ptr 就更好了!
    std::unique_ptr<Node> next;
    Node* prev;

//    int value;
    T value;
    // 这个构造函数有什么可以改进的？拷贝构造,采用列表赋值
//    Node(int const& val):value(val) {
//    }
    Node(T const& val):value(val) {}

    void insert(T val) {
//        auto node = std::make_shared<Node>(val);
//        node->next = next;
//        node->prev = prev;
//        if (prev)
//            prev->next = node;
//        if (next)
//            next->prev = node;
          auto node = std::make_unique<Node>(val);
          node->next = std::move(prev->next);
          prev->next = std::move(node);
          prev->next->prev = prev;
          prev = prev->next.get();
    }

    void erase() {
//        if (prev)
//            prev->next = next;
//        if (next)
//            next->prev = prev;
        if(prev){
            next->prev = prev;
        }
        if(next){
            prev->next = std::move(next);
        }
    }

    ~Node() {
        printf("~Node()\n");   // 应输出多少次？为什么少了？ 在退出main函数应该有10次,但是出现0次.因为shared_ptr存在相互引用.
    }                                //打印了11次
};

template<typename T>
struct List {
//    std::shared_ptr<Node> head;
    std::unique_ptr<Node<T>> head{nullptr};
    List() = default;

    List(List const &other){   // solution2: head(other.head)
        printf("List 被拷贝！\n");
//      head = other.head;  // 这是浅拷贝！
// 请实现拷贝构造函数为 **深拷贝**
//        // woring solution1:
//        std::shared_ptr<Node> *headptr = (std::shared_ptr<Node> *) malloc(size_t(1)*sizeof(std::shared_ptr<Node>));
//        std::memcpy(headptr,&other.head,sizeof(std::shared_ptr<Node>));
//        head = *headptr;
         Node<T>* ptr1 = other.head.get();
         Node<T>* ptr2;
         if(ptr1 != nullptr){
             head = std::make_unique<Node<T>>(ptr1->value);
             ptr2 = head.get();
             while(ptr1->next.get()){
                 ptr1 = ptr1->next.get();
                 ptr2->next = std::make_unique<Node<T>>(ptr1->value);
                 ptr2 = ptr2->next.get();
             }
         }else{
             head = nullptr;
         }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？因为原代码中,List b = a;是拷贝构造函数,不会调用拷贝赋值.

    List(List &&) = default;
    List &operator=(List &&) = default;

    Node<T> *front() const {
        return head.get();
    }

    T pop_front() {
//        int ret = head->value;
//        head = head->next;
//        return ret;
        T ret = head->value;
        head = std::move(head->next);
        head->prev = nullptr;
        return ret;
    }

    void push_front(T value) {
//        auto node = std::make_shared<Node>(value);
//        node->next = head;
//        if (head)
//            head->prev = node;
//        head = node;
        auto node = std::make_unique<Node<T>>(value);
        if(head){
            head->prev = node.get();
        }
        node->next = std::move(head);
        head = std::move(node);
    }

    Node<T> *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template<typename T>
void print(List<T> const &lst) {              // 有什么值得改进的？改成常引用,因为打印函数没有修改内容.值传递会调用拷贝函数,性能较低.
    printf("[");
    for (auto curr = lst.front(); curr; curr = curr->next.get()) {
        printf(" %d", curr->value);
    }
    printf(" ]\n");
}

int main() {
    List<int> a;  //List 被拷贝！

    a.push_front(7);
    a.push_front(5);
    a.push_front(8);
    a.push_front(2);
    a.push_front(9);
    a.push_front(4);
    a.push_front(1);

    print(a);   // [ 1 4 9 2 8 5 7 ]

    a.at(2)->erase();  // ~Node()

    print(a);   // List 被拷贝！ [ 1 4 2 8 5 7 ]

    List<int> b = a;  //List 被拷贝！

//    List c  //有点奇怪,这里如果采用拷贝赋值,为什么还是会报错?没有像课上讲的那样,会采用:拷贝构造函数+移动赋值 b = List(a);难道是编译器的问题吗?
//    c = a;  // error beacause the explicit delete

    a.at(3)->erase();  //~Node()

    print(a);   //                   List 被拷贝！ [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]   List 被拷贝！ [ 1 4 2 5 7 ]

    b = {};
    a = {};

    return 0;
}
