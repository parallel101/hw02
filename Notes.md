## 避免函数参数不必要的拷贝

`List const&` 只拷贝了一个很小的指针，避免了拷贝 `List` 带来的性能底下。

## 修复智能指针造成的问题

```cpp
std::shared_ptr<Node> next;
std::shared_ptr<Node> prev;
```
这会导致 *循环引用*，造成内存泄漏。
把 `prev` 改成 `std::weak_ptr<Node>` 就好了。
这时，数据光 `next` 一变量独有，所以能改为 `std::unique_ptr<Node>`。
因为 `next` 成了 `std::unique_ptr<Node>`，所以 `prev` 就成了 `Node*`。
然后 `List::head` 也要变。
各种和 `next`、`prev`、`head` 有关的操作都要改动一下。

## 改用 `unique_ptr<Node>`

上边 [修复智能指针造成的问题](#修复智能指针造成的问题) 已经说了。

## 实现拷贝构造函数为深拷贝

把原 `List` 里的元素复制一遍。

## 说明为什么可以删除拷贝赋值函数

因为有 *移动赋值函数*。调用拷贝赋值函数的地方都会转换一下，然后去调用移动赋值函数。

## 改进 `Node` 的构造函数

可以改成 `Node(int val) : value(val) {}` 的形式，这样就是用 `val` 初始化 `value`，而不是已经初始换完成了再去赋值。

## 其他

### run.bat

我的系统是 Windows 10，所以重写了一个（用 CMake 和 MSBuild 会出现谜之问题，所以直接用了 `MSVC`）。

### 模板

把所有 `int` 改成 `T`、`Node` 改成 `Node<T>`、`List` 改成 `List<T>`。

### 变参 `push_front`

这样就能把 `main` 里的一长串 `push_front(xxx)` 改成一个 `push_front(xxx, yyy, zzz)`。

### 迭代器

写了迭代器之后 `print` 函数就可以变得很简单了。
为了简单，`end()` 直接返回 `nullptr`。
