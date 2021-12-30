#pragma once

// https://en.cppreference.com/w/cpp/container/list

/* 基于智能指针实现双向链表 */
#include <algorithm>
#include <cstdio>
#include <concepts>
#include <initializer_list>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace parallel101 {

template <class T, class = void>
struct is_iterator : std::false_type { };

template <class T>
struct is_iterator<T, std::void_t<typename std::iterator_traits<T>::iterator_category>> : std::true_type { };

template <class T>
constexpr bool is_iterator_v = is_iterator<T>::value;

template<class E, int32_t log_level>
struct Node {
    using value_type = E;
    using node_type = Node;
    using node_pointer = Node*;

    // 这两个指针会造成什么问题？请修复
    // 使用两个shared_ptr, prev-next可能会引起循环引用，造成资源无法释放
    // A.next = B; B.prev = A;
    // A = nullptr;
    // 因为B还持有A的，导致A不能正确释放
    std::unique_ptr<node_type> next;
    node_pointer prev = nullptr;
    // 如果能改成 unique_ptr 就更好了!
    // 把next改成unique_ptr，prev就类似于基于raw pointer的observer
    // 缺点是在释放A之后，B.prev会失效，访问会引发错误

    // make_unique限制了value_type必须要有默认构造函数
    // make_unique需要默认的或者带参数的
    value_type value;

    // 为了iterator，如果要用make_unique就需要默认构造函数
    // Node() = default;

    // 这个构造函数有什么可以改进的？
    // 1 使用explicit避免隐式构造类型；
    // 2 把value赋值改到 initializer list
    constexpr explicit Node(value_type const& val) : value(val) { }

    constexpr ~Node() noexcept {
        if constexpr (log_level)
            printf("~Node()\n");   // 应输出多少次？为什么少了？
        // 从测试案例来看，最后一次应该连续输出5+6=11次，因为我们不是深度拷贝所以会少一些.
        // 把迭代器和unique_ptr实现后，会追加head和tail两个Node，所以每个list会再多输出两次, 总共是15
    }
};

// ConstIterator中使用了Iterator，所以gcc要求Iterator声明或者定义要在ConstIterator前面，但msvc都可以
template<class L>
class Iterator
{
public:
    template<class, int32_t> friend class List;
    template<class> friend class ConstIterator;

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename L::value_type;
    // 因为List的iterator不是基于数组，iterator与数字运算的范围在List::size() 的size_type内。
    using difference_type = typename L::size_type; // difference_type
    using pointer = typename L::pointer;
    using reference = typename L::reference;
    using node_pointer = typename L::node_pointer;

private:
    node_pointer ptr = nullptr;

public:
    constexpr explicit Iterator(node_pointer ptr) noexcept : ptr(ptr) { }

    [[nodiscard]] constexpr reference operator*() {
        return ptr->value;
    }

    [[nodiscard]] constexpr pointer operator->() noexcept {
        return std::pointer_traits<pointer>::pointer_to(**this);
    }

    constexpr Iterator& operator++() {
        ptr = ptr->next.get();
        return *this;
    }

    constexpr Iterator operator++(int) {
        Iterator ret = *this;
        ptr = ptr->next.get();
        return ret;
    }

    constexpr Iterator& operator--() {
        ptr = ptr->prev;
        return *this;
    }

    constexpr Iterator operator--(int) {
        Iterator ret = *this;
        ptr = ptr->prev;
        return ret;
    }

    constexpr Iterator operator+(difference_type n) {
        Iterator lt = *this;
        while (n--)
            ++lt;
        return lt;
    }

    constexpr Iterator operator-(difference_type n) {
        Iterator lt = *this;
        while (n--)
            --lt;
        return lt;
    }

    friend constexpr bool operator==(Iterator const& x, Iterator const& y) {
        return x.ptr == y.ptr;
    }

    friend constexpr bool operator!=(Iterator const& x, Iterator const& y) {
        return !(x == y);
    }
};

template<class L>
class ConstIterator
{
public:
    template<class, int32_t> friend class List;
    template<class> friend class Iterator;

    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename L::value_type;
    using difference_type = typename L::size_type;
    using pointer = typename L::const_pointer;
    using reference = typename L::const_reference;
    using node_pointer = typename L::node_pointer;

private:
    node_pointer ptr = nullptr;

public:
    constexpr explicit ConstIterator(node_pointer ptr) noexcept : ptr(ptr) { }

    constexpr ConstIterator(Iterator<L> iter) noexcept : ptr(iter.ptr) { }

    [[nodiscard]] constexpr reference operator*() const noexcept {
        return ptr->value;
    }

    [[nodiscard]] constexpr pointer operator->() const noexcept {
        return std::pointer_traits<pointer>::pointer_to(**this);
    }

    constexpr ConstIterator& operator++() {
        ptr = ptr->next.get();
        return *this;
    }

    constexpr ConstIterator operator++(int) {
        ConstIterator ret = *this;
        ptr = ptr->next.get();
        return ret;
    }

    constexpr ConstIterator& operator--() {
        ptr = ptr->prev;
        return *this;
    }

    constexpr ConstIterator operator--(int) {
        ConstIterator ret = *this;
        ptr = ptr->prev;
        return ret;
    }

    constexpr ConstIterator operator+(difference_type n) {
        ConstIterator lt = *this;
        while (n--)
            ++lt;
        return lt;
    }

    constexpr ConstIterator operator-(difference_type n) {
        ConstIterator lt = *this;
        while (n--)
            --lt;
        return lt;
    }

    friend constexpr bool operator==(ConstIterator const& x, ConstIterator const& y) {
        return x.ptr == y.ptr;
    }

    friend constexpr bool operator!=(ConstIterator const& x, ConstIterator const& y) {
        return !(x == y);
    }
};

template<class E, int32_t log_level = 0>
class List {
public:
    using value_type = E;
    using reference = E&;
    using const_reference = E const&;
    using pointer = E*;
    using const_pointer = const E*;
    using difference_type = typename std::pointer_traits<pointer>::difference_type; // std::ptrdiff_t
    using node_type = Node<E, log_level>;
    using node_pointer = Node<E, log_level>*;
    using const_node_pointer = const Node<E, log_level>*;
    using size_type = size_t;
    using iterator = Iterator<List>;
    using const_iterator = ConstIterator<List>;

private:
    // m_head和m_stage不持有value，所以不能使用make_unique初始化
    // m_head->next --> first
    // first->prev -->m_head
    std::unique_ptr<node_type> m_head;
    // last->next --> m_tail
    // m_tail->prev --> last
    // 多一个tail节点：1是为了end()返回迭代器时不用返回nullptr，方便迭代器操作
    // 2因为next要持有节点，所以不能做成闭环的链表，这样m_head就是失去头节点作用，资源也无法释放
    // 3为了支持end()-1这样的操作
    std::unique_ptr<node_type>* m_tail = nullptr;
    // m_stage是一个tail节点的暂存区
    std::unique_ptr<node_type> m_stage;

    size_type m_size = 0;

public:
    ///////// Constructor & Destructor
    // constructs the list
    constexpr List() {
        _init();
    }

    // constructs the list
    constexpr List(List const& lst) :List() {
        if constexpr (log_level)
            printf("List 被拷贝！\n");
        // head = lst.head;  // 这是浅拷贝！
        // 请实现拷贝构造函数为 **深拷贝**
        assign(lst.begin(), lst.end());
    }

    // constructs the list
    constexpr List(size_type n, value_type const& x) :List() {
        while (n--)
            push_back(x);
    }

    // constructs the list
    constexpr List(std::initializer_list<value_type> il) :List() {
        assign(il.begin(), il.end());
    }

    // constructs the list, move 'lst' element.
    constexpr List(List&& lst) noexcept :List() {
        swap(lst);
    }

    // destructs the list
    constexpr ~List() noexcept {
        _clear();
    }

    ///////// Assign
    // List& operator=(List const&) = delete;  // 为什么删除拷贝赋值函数也不出错？
    // 因为编译器会调用这个List& operator=(List&&)函数来代替

    // assigns values to the container
    constexpr List& operator=(List const& lst) {
        if (this != &lst)
            assign(lst.begin(), lst.end());
        return *this;
    }

    // assigns values to the container, move 'lst' element.
    constexpr List& operator=(List&& lst) noexcept {
        if (this != &lst) {
            clear();
            swap(lst);
        }
        return *this;
    }

    // assigns values to the container
    constexpr List& operator=(std::initializer_list<value_type> il) {
        assign(il.begin(), il.end()); return *this;
    }

    // assigns values to the container
    template<class I>
        requires is_iterator_v<I> //IFNOT: the associated constraints are not satisfied
    constexpr void assign(I first, I const last) {
        if (first == last)
            return;
        auto dstf = begin();
        auto dstl = end();
        while (first != last) {
            if (dstf == dstl)
                break;
            auto next = first;
            ++next;
            // *dstf = *first; // 为什么不用这个？
            // 因为最小依赖实现，一个class可以没有拷贝赋值函数，但一定有析构和拷贝构造函数.
            // 为什么可以原地构造，因为大小够用，同一类型，在第一次构造时编译器就知道了大小信息。
            // 所以新的和旧的是一样的大小
            _destory(&dstf.ptr->value);
            _construct(dstf, *first);
            ++dstf;
            first = next;
        }
        if (first != last)
            insert_before(dstf, first, last);
        else
            erase(dstf, dstl); // 删除多余的
    }

    // assigns values to the container
    constexpr void assign(std::initializer_list<value_type> il) {
        assign(il.begin(), il.end());
    }

    ///////// Element access
    // access the first element
    [[nodiscard]] constexpr reference front() {
        if (0 == size())
            throw std::out_of_range("front on an empty list");
        return m_head->next->value;
    }

    // access the first element
    [[nodiscard]] constexpr const_reference front() const {
        if (0 == size())
            throw std::out_of_range("front on an empty list");
        return m_head->next->value;
    }

    // access the last element
    [[nodiscard]] constexpr reference back() {
        if (0 == size())
            throw std::out_of_range("back on an empty list");
        return (*m_tail)->prev->value;
    }

    // access the last element
    [[nodiscard]] constexpr const_reference back() const {
        if (0 == size())
            throw std::out_of_range("back on an empty list");
        return (*m_tail)->prev->value;
    }

    // access the [index] element
    [[nodiscard]] constexpr reference at(size_type index) {
        if (index >= size())
            throw std::out_of_range("at index out of range");
        auto curr = m_head->next.get();
        for (size_type i = 0; i < index; i++)
            curr = curr->next.get();
        return curr->value;
    }

    // access the [index] element
    [[nodiscard]] constexpr const_reference at(size_type index) const {
        if (index >= size())
            throw std::out_of_range("at index out of range");
        auto curr = m_head->next.get();
        for (size_type i = 0; i < index; i++)
            curr = curr->next.get();
        return curr->value;
    }

    ///////// Iterators
    // returns an iterator to the beginning
    [[nodiscard]] constexpr iterator begin() noexcept {
        return iterator(m_head->next.get());
    }

    // returns an iterator to the beginning
    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return const_iterator(m_head->next.get());
    }

    // returns an iterator to the beginning
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    // returns an iterator to the end
    [[nodiscard]] constexpr iterator end() noexcept {
        return iterator(m_tail->get());
    }

    // returns an iterator to the end
    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return const_iterator(m_tail->get());
    }

    // returns an iterator to the end
    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }

    ///////// Capacity
    // checks whether the container is empty
    [[nodiscard]] constexpr bool empty() const noexcept {
        return 0 == size();
    }

    // returns the number of elements
    [[nodiscard]] constexpr size_type size() const noexcept {
        return m_size;
    }

    // returns the number of elements
    [[nodiscard]] constexpr size_type length() const noexcept {
        return size();
    }

    // returns the maximum possible number of elements
    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max();
    }

    ///////// Pop & Push
    // removes the last element
    void pop_front() {
        if (0 == size())
            throw std::out_of_range("pop_front on an empty list");
        auto& next = m_head->next;
        next = std::move(next->next);
        next->prev = m_head.get();
        if (1 == size())
            m_tail = std::addressof(next);
        --m_size;
    }

    // removes the last element
    void pop_back() {
        if (0 == size())
            throw std::out_of_range("pop_back on an empty list");
        auto& tail = *m_tail;
        auto& prev = tail->prev;
        prev = prev->prev;
        prev->next = std::move(tail);
        m_tail = std::addressof(prev->next);
        --m_size;
    }

    // inserts an element to the beginning
    // returns an iterator pointing to the pushed value
    iterator push_front(value_type const& x) {
        return insert_before(begin(), x);
    }

    // inserts an element to the beginning, move x
    // returns an iterator pointing to the pushed value
    iterator push_front(value_type&& x) {
        return insert_before(begin(), std::move(x));
    }

    // adds an element to the end
    // returns an iterator pointing to the pushed value
    iterator push_back(value_type const& x) {
        return insert_before(end(), x);
    }

    // adds an element to the end, move x
    // returns an iterator pointing to the pushed value
    iterator push_back(value_type&& x) {
        return insert_before(end(), std::move(x));
    }

    // inserts elements before pos
    // returns an iterator pointing to the inserted value
    // [pos->prev] [node] [pos]
    iterator insert_before(const_iterator pos, value_type const& x) {
        auto node = std::make_unique<node_type>(x);
        return _insert_before(pos, node);
    }

    // inserts [first, last) elements before pos
    // returns an iterator pointing to the first element inserted, or pos if first==last.
    template<class I>
        requires is_iterator_v<I>
    iterator insert_before(const_iterator pos, I first, I last) {
        iterator ret(pos.ptr);
        bool mark = true;
        while (first != last) {
            auto next = first;
            ++next;
            pos = insert_before(pos, *first);
            if (mark) {
                mark = false;
                ret = iterator(pos.ptr);
            }
            ++pos;
            first = next;
        }
        return ret;
    }

    // inserts initializer_list elements before pos
    // returns an iterator pointing to the first element inserted, or pos if first==last.
    iterator insert_before(const_iterator pos, std::initializer_list<value_type> il) {
        return insert_before(pos, il.begin(), il.end());
    }

    // inserts elements before pos, move x
    // returns an iterator pointing to the inserted value
    // [pos->prev] [node] [pos]
    iterator insert_before(const_iterator pos, value_type&& x) {
        auto node = std::move(_make_node(std::move(x)));
        return _insert_before(pos, node);
    }

    // inserts elements after pos
    // returns an iterator pointing to the inserted value
    // [pos] [node] [pos->next]
    iterator insert_after(const_iterator pos, value_type const& x) {
        // 对于empty list，insert_after(begin) == insert_after(end)
        if (pos.ptr == m_tail->get())
            return insert_before(pos, x);
        return insert_before(pos + 1, x);
    }

    // inserts [first, last) elements after pos
    // returns an iterator pointing to the first element inserted, or pos if first==last.
    template<class I>
        requires is_iterator_v<I>
    iterator insert_after(const_iterator pos, I first, I last) {
        iterator ret(pos.ptr);
        bool mark = true;
        while (first != last) {
            auto next = first;
            ++next;
            pos = insert_after(pos, *first);
            if (mark) {
                mark = false;
                ret = iterator(pos.ptr);
            }
            first = next;
        }
        return ret;
    }

    // inserts initializer_list elements after pos
    // returns an iterator pointing to the first element inserted, or pos if first==last.
    iterator insert_after(const_iterator pos, std::initializer_list<value_type> il) {
        return insert_after(pos, il.begin(), il.end());
    }

    // inserts elements after pos, move x
    // returns an iterator pointing to the inserted value
    // [pos] [node] [pos->next]
    iterator insert_after(const_iterator pos, value_type&& x) {
        // 对于empty list，insert_after(begin) == insert_after(end)
        if (pos.ptr == m_tail->get())
            return insert_before(pos, std::move(x));
        return insert_before(pos + 1, std::move(x));
    }

    ///////// Emplace
    // constructs an element in-place before pos
    // returns an iterator pointing to the constructed value
    template<class... Args>
    iterator emplace_before(const_iterator pos, Args&&... args) {
        auto node = std::move(_make_node(std::forward<Args>(args)...));
        return _insert_before(pos, node);
    }

    // constructs an element in-place after pos
    // returns an iterator pointing to the constructed value
    template<class... Args>
    iterator emplace_after(const_iterator pos, Args&&... args) {
        if (pos.ptr == m_tail->get())
            return emplace_before(pos, std::forward<Args>(args)...);
        return emplace_before(pos + 1, std::forward<Args>(args)...);
    }

    // constructs an element in-place at the end
    // returns an iterator pointing to the constructed value
    template<class... Args>
    iterator emplace_back(Args&&... args) {
        return emplace_before(end(), std::forward<Args>(args)...);
    }

    // constructs an element in-place at the beginning
    // returns an iterator pointing to the constructed value
    template<class... Args>
    iterator emplace_front(Args&&... args) {
        return emplace_before(begin(), std::forward<Args>(args)...);
    }

    ///////// Modifiers
    // swaps the contents
    void swap(List& lst) noexcept {
        std::swap(m_head, lst.m_head);
        std::swap(m_stage, lst.m_stage);
        std::swap(m_tail, lst.m_tail);
        std::swap(m_size, lst.m_size);
    }

    // erases elements at pos
    // returns an iterator following the last removed element.
    iterator erase(const_iterator pos) {
        if (0 == size())
            throw std::out_of_range("erase on an empty list");
        if (pos.ptr == m_tail->get())
            throw std::out_of_range("erase feed end iterator");

        auto next = pos.ptr->next.get();
        auto reset = (next == m_tail->get());
        auto prev = pos.ptr->prev;

        next->prev = prev;
        prev->next = std::move(pos.ptr->next);
        if (reset)
            m_tail = std::addressof(prev->next);
        --m_size;

        return iterator(next);
    }

    // erases [first, last) elements
    // returns an iterator following the last removed element.
    iterator erase(const_iterator first, const_iterator last) {
        iterator pos = iterator(first.ptr);
        while (first != last) {
            auto next = first;
            ++next;
            pos = erase(first);
            first = next;
        }
        return pos;
    }

    // clears the contents, reinit control member.
    constexpr void clear() {
        _clear();
        _init();
    }

private:
    ///////// Alloc
    // alloc a new memory block
    template<class T>
    [[nodiscard]] constexpr T* _alloc() {
        return static_cast<T*>(::operator new(sizeof(T)));
    }

    template<class T>
    // call class destructor
    constexpr void _destory(T* p) noexcept {
        p->~T();
    }

    // construct class in-place
    template<class T, class... Args>
    constexpr void _construct(T* ptr, Args&&... args) {
        ::new ((void*)ptr) T(std::forward<Args>(args)...);
    }

    // construct class in-place
    template<class... Args>
    constexpr void _construct(const_iterator pos, Args&&... args) {
        _construct(std::addressof(pos.ptr->value), std::forward<Args>(args)...);
    }

    ///////// Helper
    // make node shortcut
    template<class... Args>
    [[nodiscard]] decltype(auto) _make_node(Args&&... args) {
        static constexpr auto nargs = sizeof...(Args);
        auto node = _alloc<node_type>();
        node->prev = nullptr;
        _construct(std::addressof(node->next), nullptr);
        if constexpr (nargs)
            _construct(std::addressof(node->value), std::forward<Args>(args)...);
        return std::unique_ptr<node_type>(node); //RVO
    }

    // init essential control member
    constexpr void _init() noexcept {
        m_head = std::move(_make_node());
        m_stage = std::move(_make_node());
        m_stage->prev = m_head.get();
        m_head->next = std::move(m_stage);
        m_tail = std::addressof(m_head->next);
    }

    // delete all nodes including control member
    constexpr void _clear() noexcept {
        while (m_head)
            m_head = std::move(m_head->next);
        m_size = 0;
        m_tail = nullptr;
    }

    // insert the node to place that before pos
    // returns an iterator pointing to the inserted value
    constexpr iterator _insert_before(const_iterator pos, std::unique_ptr<node_type>& node) {
        if (size() == max_size())
            throw std::out_of_range("list too long");

        auto nraw = node.get();
        auto tail = m_tail->get();
        auto& prev = pos.ptr->prev;

        node->prev = prev;
        node->next = std::move(prev->next);
        if (pos.ptr == tail)
            m_tail = std::addressof(node->next);
        prev->next = std::move(node);
        prev = nraw;
        ++m_size;

        return iterator(nraw);
    }

}; // class List

} // namespace parallel101
