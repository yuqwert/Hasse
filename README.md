# 简易的偏序关系Hasse图可视化程序

输入任意的二元关系（通过输入所有有序对的方式输入），检查其是否为偏序关系，并显示Hasse图。

## 例子

画![demo](https://raw.githubusercontent.com/hhusjr/Hasse/master/doc/example_formula.svg)。

输入元素个数和关系对数，空格隔开：

```
8 27
```

输入集合的每个元素：

```
{}
{c}
{b}
{bc}
{a}
{ac}
{ab}
{abc}
```

输入组成二元关系的所有有序对

```
{} {}
{} {c}
{} {b}
{} {bc}
{} {a}
{} {ac}
{} {ab}
{} {abc}
{c} {c}
{c} {bc}
{c} {ac}
{c} {abc}
{b} {b}
{b} {bc}
{b} {ab}
{b} {abc}
{bc} {bc}
{bc} {abc}
{a} {a}
{a} {ac}
{a} {ab}
{a} {abc}
{ac} {ac}
{ac} {abc}
{ab} {ab}
{ab} {abc}
{abc} {abc}
```

效果图

![demo](https://raw.githubusercontent.com/hhusjr/Hasse/master/doc/demo.png)

