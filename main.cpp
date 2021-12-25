/* 基于智能指针实现双向链表 */
#include <cstdio>
#include <memory>

struct Node {
    // 这两个指针会造成什么问题？请修复
    /* Answer : 会导致循环引用，当A和B为两个相邻的Node，A.next指向B，
     *          B.prev指向A，那么会导致内存泄露；解决方案是：
     *          人为约定某一个方向，如从前到后，即前面的Node具有对后面
     *          Node的所属权，即next指针用shared_ptr，prev指针则用weak_ptr；
     *
     *          使用unique_ptr的方案：next指针用unique_ptr，prev指针用原始指针。
     *          同上，约定前面的Node对后面的Node具有所属权，前面的Node像是”父窗口“,
     *          后面的Node像是”子窗口“，如下图，=>代表unique_ptr，<-为原始指针，
     *          在析构A时，A的析构函数会调用unique_ptr的析构函数，即先析构B，
     *          B析构时又会先析构C，因此实际析构完成的顺序是C -> B -> A，因此符合
     *          课件中提到的”当该对象不属于我，但他释放前我必然被释放”的条件。
     *
     *          =>| |=>| |=>| |=>nullptr
     *            |A|  |B|  |C|
     *   nullptr<-| |<-| |<-| |
     */
    std::unique_ptr<Node> next;
    Node* prev;
    // 如果能改成 unique_ptr 就更好了!

    int value;

    // 这个构造函数有什么可以改进的？
    /* Answer : 使用explicit修饰，拒绝从int -> Node的
     *          隐式类型转换。
     */
    explicit Node(int val): value(val), next(nullptr), prev(nullptr) {}

    void insert(int val) {
        // 在当前node后面插入值为val的node
        auto node = std::make_unique<Node>(val);
        node->prev = this;
        if (next) {
            next->prev = node.get();
            node->next = std::move(next);
        }
        next = std::move(node);
    }

    void erase() {
        if (next) {
            value = next->value;
            if (next->next) {
                next->next->prev = this;
            }
            next = std::move(next->next);
        } else {
            if (prev) {
                prev->next = std::move(nullptr);
            }
        }
    }

    ~Node() {
        printf("~Node(), value = %d\n", value);   // 应输出多少次？为什么少了？
    }
};

struct List {
    std::unique_ptr<Node> head;

    List() = default;

    List(List const &other) {
        printf("List 被拷贝！\n");
        // 请实现拷贝构造函数为 **深拷贝**
        if (other.head) {
            head = std::move(std::make_unique<Node>(other.head->value));
            Node* cur_other_node = other.head->next.get();
            Node* prev_this_node = head.get();
            while (cur_other_node) {
                auto node = std::make_unique<Node>(cur_other_node->value);
                // NOTE: set raw pointer first, then std::move(node),
                //       because after std::move(node), node is empty and
                //       assign value to node->prev is invalid!
                node->prev = prev_this_node;
                prev_this_node->next = std::move(node);
                prev_this_node = prev_this_node->next.get();
                cur_other_node = cur_other_node->next.get();
            }
        }
    }

    List &operator=(List const &) = delete;  // 为什么删除拷贝赋值函数也不出错？
    /* Answer : 因为定义了移动赋值运算符，所以调用=时，
     *          会在=右侧原地构造一个List，然后通过移动语义调用移动赋值运算符
     *          移动给=左侧的变量。
     */

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

void print(const List& lst) {  // 有什么值得改进的？
    /* Answer : 传参数时使用const reference，避免没有必要的拷贝构造函数调用 */
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

    b.at(5)->erase();  // test corner case : erase last element;

    print(b);   // [ 1 4 2 8 5 ]

    b.at(0)->erase();  // test corner case : erase first element;

    print(b);   // [ 4 2 8 5 ]

    b.at(1)->insert(100);

    print(b);   // [ 4 2 100 8 5 ]

    b.at(0)->insert(-1);  // test corner case : insert after first element;

    print(b);   // [ 4 -1 2 100 8 5 ]

    b.at(5)->insert(-5);  // test corner case : insert after last elment;

    print(b);   // [ 4 -1 2 100 8 5 -5 ]

    b = {};
    a = {};

    return 0;
}
