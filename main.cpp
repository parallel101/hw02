/* 基于智能指针实现双向链表 */
#include <iostream>
#include <memory>
#include <initializer_list>

template <typename Tp> struct Node {
    std::unique_ptr<Node> next;
    Node *prev;
    Tp value;

    Node(Tp val): value(val) {}
    ~Node() { std::clog << "~Node(" << value << ")\n"; }

    void insert(Tp val) { value = val; }
    void erase() {
        if (next) next->prev = prev;
        if (prev) prev->next = std::move(next);
    }
};

template <typename Tp, typename Init = std::initializer_list<Tp>> struct List {
    std::unique_ptr<Node<Tp>> head;

    List() = default;
    List(Init const &range) {
        if (range.begin() == range.end()) return; 
        Node<Tp> *tail;
        auto it = range.begin();
        head = std::make_unique<Node<Tp>>(*it);
        for (++it, tail = head.get(); it != range.end(); ++it) {
          tail->next = std::make_unique<Node<Tp>>(*it);
          tail->next->prev = tail;
          tail = tail->next.get();
        } 
    }
    List &operator=(List const &) = delete; // 为什么删除拷贝赋值函数也不出错？
    // 因为编译器会用“解构函数+拷贝构造”代替拷贝赋值。
    List(List &&) = default;
    List &operator=(List &&) = default;

    struct iterator {
      Node<Tp> *node;
      iterator(Node<Tp> *p): node(p) {}

      Tp operator*() { return node->value; }
      iterator operator++() { return node = node->next.get(); }
      iterator operator--() { return node = node->prev; }
      bool operator==(iterator const &other) { return node == other.node; }
      bool operator!=(iterator const &other) { return node != other.node; }
    };
    iterator begin() const { return iterator(head.get()); }
    iterator end() const { return iterator(nullptr); }

    Node<Tp> *front() const { return head.get(); }
    bool empty() const { return !(head); }
    int pop_front() {
        if (head == nullptr) return 0x114154;
        int ret = head->value;
        head = std::move(head->next);
        return ret;
    }
    void push_front(int value) {
        auto node = new Node<Tp>(value);
        if (head)
            head->prev = node;
        node->next = std::move(head);
        head = std::unique_ptr<Node<Tp>>(node);
    }
    Node<Tp> *at(size_t index) const {
        auto curr = front();
        for (size_t i = 0; i < index; i++) {
            curr = curr->next.get();
        }
        return curr;
    }
};

template <typename Tp, typename Init>
std::ostream &operator<<(std::ostream &os, List<Tp, Init> const &l) {
  for (os << '['; Tp x : l) os << x << ", ";
  os << (l.empty() ? "]" : "\b\b]");
  return os;
}

int main() {
#define print(l) std::cout << l << '\n';
    List<int> a = {1, 4, 9, 2, 8, 5, 7};
    print(a);   // [ 1 4 9 2 8 5 7 ]
    a.at(2)->erase();
    print(a);   // [ 1 4 2 8 5 7 ]
    List<int, List<int>> b = a;
    a.at(3)->erase();
    print(a);   // [ 1 4 2 5 7 ]
    print(b);   // [ 1 4 2 8 5 7 ]
    b = {};
    a = {};
    return 0;
}
