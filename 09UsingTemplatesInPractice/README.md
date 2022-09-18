<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [第九章：在实战中使用模板](#%E7%AC%AC%E4%B9%9D%E7%AB%A0%E5%9C%A8%E5%AE%9E%E6%88%98%E4%B8%AD%E4%BD%BF%E7%94%A8%E6%A8%A1%E6%9D%BF)
  - [包含模型](#%E5%8C%85%E5%90%AB%E6%A8%A1%E5%9E%8B)
  - [模板和inline](#%E6%A8%A1%E6%9D%BF%E5%92%8Cinline)
  - [预编译头文件](#%E9%A2%84%E7%BC%96%E8%AF%91%E5%A4%B4%E6%96%87%E4%BB%B6)
  - [破译大篇幅错误信息](#%E7%A0%B4%E8%AF%91%E5%A4%A7%E7%AF%87%E5%B9%85%E9%94%99%E8%AF%AF%E4%BF%A1%E6%81%AF)
  - [本章后记](#%E6%9C%AC%E7%AB%A0%E5%90%8E%E8%AE%B0)
  - [总结](#%E6%80%BB%E7%BB%93)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# 第九章：在实战中使用模板

本章讨论一些实践和编码相关的东西，比如模板代码组织，解析天书一样的模板编译报错，怎样提升编译效率之类的问题。

## 包含模型

有几个组织模板源码的方式，最流行的方法是：包含模型。

链接错误：
- 通常来说普通C++程序的组织方式大多是：
    - 类和其他定义放在头文件中，通常是一个`.hpp .h .H .hh .hxx`文件。
    - 非内联的函数和非内联的全局变量仅有一个声明在头文件中，定义都在源文件中，通常是一个`.cpp .cc .C .c .cxx`文件。
    - 这种方式在普通程序工作良好：接口实现得到分离，要看到声明只需要包含头文件即可，链接时不会有任何名称重定义。
- 但这种方式在模板中就行不通了，如果将模板（函数模板为例）定义放在一个单独的编译单元。而头文件中只有声明，那么使用了函数模板时，应该会得到一个链接错误：符号未定义。
- 产生原因是：使用到模板的地方只看到了声明而没有定义，导致没有实例化而假定这是一个外部符号，定义在其他地方，而定义的源文件中没有使用到模板编译器不知道该以何种参数实例化所以也没有实例化，最终所有编译单元都未实例化。

头文件中的模板：
- 为了规避链接错误，保证使用模板的地方能够正确实例化，就必须将模板的定义写在头文件中。
- 这和我们处理内联函数以及宏定义的方式相同。
- 这种组织模板的方式称之为**包含模型**（inclusion model）。
- 这种处理方式显著地增加了包含一个头文件的成本，C++标准库中有非常多的模板，模板的定义都是写在其中的，每一个头文件包含都会增加编译时间。
- 解决编译时间问题有一些手段，比如预编译头文件、显式模板实例化。
- 当前的建议都是使用包含模型来组织模板代码，因为这基本是唯一可行的方式了。C++20已经引入module，可以显著减少编译时间。
- 还有一个问题是每个编译单元都会实例化，最终可能会有多个编译单元有同样的函数，不过我们不必担心重定义问题，链接器会合并多个重复的定义（就是说其实链接器为模板开了洞做特殊处理）。
- 类模板、类模板的成员函数和静态成员都是同样道理处理。

## 模板和inline

将函数定义为`inline`可以提示编译器将函数定义为内联，从而在调用处展开以提升性能。但现实中的编译器大多都按照自己的规则来决定是否内联，最终内联与否与是否定义为`inine`关系倒不是那么大了。
- `inline`的唯一保证只剩下了能够出现多次定义，所以通常我们都会将`inline`函数定义写在头文件中。
- 模板使用和`inline`类似的机制，但这并不意味着函数模板会默认`inline`。是否`inline`总是取决于编译器的，对程序员几乎透明。
- 要实现一定会内联的效果的话，可能只能使用编译器特定的不可移植的非标准特性才能做到。
- 要注意的是函数模板的全特化和普通函数一样，放在头文件中就必须声明为`inline`。

## 预编译头文件

C++使用继承自C语言的头文件和源文件组织方式就导致了编译每个编译单元都需要解析其包含的所有头文件，做了一部分重复工作。在大型项目中，即使没有使用模板，编译时间也可能会非常长。在用上模板之后，模板实现也被写到头文件中，多个编译单元会重复实例化同一模板，做了更多重复工作，编译时间大幅延长。实践中有一些方法可以一定程度缓解这个问题：
- 使用分布式编译，将编译任务分配到多台机器，这需要构建系统的配合以及更多硬件资源。
- 使用预编译头文件（Precompiled headers），对一部分头文件比如标准库，进行预编译，在编译时则不用再次编译。
- 使用C++20的Module重新组织程序，减少重复编译，对于老项目代价很大能够大幅缩短编译时间。
- 配合使用多种手段效果更好。
- 还有一种现实中很常用的比较终极的手段，将整个项目合并为一个源文件然后编译甚至能够得到非常好的编译性能（比如[xmake unity build](https://xmake.io/#/features/unity_build)）。

预编译头文件：
- 预编译头是编译器厂商提供了用来提升编译速度的机制，并非C++标准内容。
- 编译每一个源文件时，总是从头解析到尾的，不同源文件中包含的相同头文件必然就会被重复解析。要消除这种重复解析过程，就可以将这些头文件的解析结果存下来下一次继续使用，但是不同的包含过程中可能会有不同的宏开关，文件包含顺序不同甚至可能造成状态的不同。所以单独为每一个头文件生成预编译头是不太可行的，通常来说需要将公共的文件移到一个公共头文件中，为这个文件编译出预编译头文件，然后源文件再包含这个头文件以做到防止重复解析。（VS中的`stdfx.h`就是这种做法）。
- 在理论上来说，只要文件开头有公共的代码，都可以提取出来处理成预编译文件，但是通常我们只会处理包含的头文件。
- 预编译可以用在所有代码很少更改的地方，比如可以将所有标准库头文件放到一个单独头文件`std.hpp`中进行预编译，所有要包含标准库头文件的地方都替换为包含`std.hpp`，编译这个预编译头文件会占用一些内存，第一次编译会占用一定时间。但完成编译后，所有编译单元使用标准库都省掉了解析标准库头文件的时间。
- 对使用到的库和框架通常也可以使用。
- 此类标准库或者框架的预编译头文件通常会成为项目配置中依赖的一部分，通过构建工具或者库的更新得到升级。
- 不是经常改动的头文件都可以尝试加入到预编译头文件中，但是尚在开发中的头文件就不适合了。频繁改动会导致生成预编译头文件的时间都大过使用它节省的时间。
- 预编译头文件可以广泛用于创建一个编译中间层，用以在尚不稳定的项目中使用稳定层的头文件。

## 破译大篇幅错误信息

没有模板的C++程序中，如果出现编译错误，报错信息通常简洁和直观，很快便能定位到原因。但是当涉及到模板时，事情就不是这个样子了，看例子。

简单的类型不匹配：
```C++
#include <iostream>
#include <map>
#include <algorithm>
#include <string>

int main(int argc, char const *argv[])
{
    std::map<std::string, double> coll;
    auto f = [](const std::string& str) { return str != ""; };
    auto pos = find_if(coll.begin(), coll.end(), f);
    return 0;
}
```
- 这是一个简单的类型不匹配问题，lambda的参数应该map元素类型也就是`std::pair<const std::string, double>`而非`std::string`。
- 看一下g++12.1的报错信息：
```C++
In file included from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/stl_algobase.h:71,
                 from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/string:50,
                 from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/locale_classes.h:40,
                 from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/ios_base.h:41,
                 from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/ios:42,
                 from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/ostream:38,
                 from C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/iostream:39,
                 from P143.DecodingErrorInfos.cpp:1:
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/predefined_ops.h: In instantiation of 'bool __gnu_cxx::__ops::_Iter_pred<_Predicate>::operator()(_Iterator) [with _Iterator = std::_Rb_tree_iterator<std::pair<const std::__cxx11::basic_string<char>, double> >; _Predicate = main(int, const char**)::<lambda(const std::string&)>]':
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/stl_algobase.h:2050:42:   required from '_InputIterator std::__find_if(_InputIterator, _InputIterator, _Predicate, input_iterator_tag) [with _InputIterator = _Rb_tree_iterator<pair<const __cxx11::basic_string<char>, double> >; _Predicate = __gnu_cxx::__ops::_Iter_pred<main(int, const char**)::<lambda(const string&)> >]'
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/stl_algobase.h:2112:23:   required from '_Iterator std::__find_if(_Iterator, _Iterator, _Predicate) [with _Iterator = _Rb_tree_iterator<pair<const __cxx11::basic_string<char>, double> >; _Predicate = __gnu_cxx::__ops::_Iter_pred<main(int, const char**)::<lambda(const string&)> >]'
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/stl_algo.h:3877:28:   required from '_IIter std::find_if(_IIter, _IIter, _Predicate) [with _IIter = _Rb_tree_iterator<pair<const __cxx11::basic_string<char>, double> >; _Predicate = main(int, const char**)::<lambda(const string&)>]'
P143.DecodingErrorInfos.cpp:10:23:   required from here
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/predefined_ops.h:318:30: error: no match for call to '(main(int, const char**)::<lambda(const std::string&)>) (std::pair<const std::__cxx11::basic_string<char>, double>&)'
  318 |         { return bool(_M_pred(*__it)); }
      |                       ~~~~~~~^~~~~~~
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/predefined_ops.h:318:30: note: candidate: 'bool (*)(const std::string&)' {aka 'bool (*)(const std::__cxx11::basic_string<char>&)'} (conversion)
C:/CppToolChain/mingw-w64/x86_64-12.1.0-release-posix-seh-rt_v10-rev3/mingw64/lib/gcc/x86_64-w64-mingw32/12.1.0/include/c++/bits/predefined_ops.h:318:30: note:   candidate expects 2 arguments, 2 provided
P143.DecodingErrorInfos.cpp:9:14: note: candidate: 'main(int, const char**)::<lambda(const std::string&)>'
    9 |     auto f = [](const std::string& str) { return str != ""; };
      |              ^
P143.DecodingErrorInfos.cpp:9:14: note:   no known conversion for argument 1 from 'std::pair<const std::__cxx11::basic_string<char>, double>' to 'const std::string&' {aka 'const std::__cxx11::basic_string<char>&'}
```
- 最开头的`In instantiation of`表明错误发生在一个标准库函数模板的实例化过程中（给出了实例化的实参），`required from`表明了其中有多个函数调用层次，`no match for call to`说明是函数调用不匹配，`note: candidate:`给出了候选的调用，最后说明了`no known conversion for argument 1 from`是传入lambda的实参和形参不匹配且不能转换的问题。
- 报错是很清晰的，但并不是一两句话就能说清楚所有问题。
- 这里都算是极其简单的例子了，在现实的模板编程中，模板中的一个小错误最后输出几千行上万行报错信息的情况是很常见的，输出完后最前面的信息已经超出控制台缓冲被丢弃掉了的情况都很常见。
    - 很多时候错误发生在实例化的模板参数类型，而一个模板名称使用实参替换后可以有达到数行长度，模板与模板层层嵌套，理清层次就不轻松，排查错误在某些时候极费心力。
    - 很多时候一个问题可能会造成多个错误，每个错误都及其冗长，要分清每个错误的开头结尾在哪儿都是一件很繁琐的事情。
- 模板的错误大多都是实例化时报的，读错误信息的步骤一般都是先找到实例化哪个模板，用以实例化的模板实参，在什么位置，找到最终实例化错误的位置之后还要反推到调用层，判断是调用层不满足模板要求的问题还是哪一层的模板实现有问题。
- 不同编译器报的信息量可能会大有差别，可能一个很轻松指出问题所在，一个报了巨量篇幅信息，需要慢慢分析。在实践中，测试时有多个编译器可用通常是更好的，不仅为了可移植性，也为了报错时对比分析。

## 本章后记

- C++的头文件与源文件的项目组织风格是由**一个定义原则**（One Definition Rule，ODR）派生出来的。[附录A](../AppendixA/)介绍了一个定义原则。
- 包含模型是现有C++编译器实现下的现实解答。
- C++98标准曾经提供了一个模板定义和实现分离的模型（通过`export`导出模板），最后证实实现起来比预想的复杂很多，没有被任何现实中的编译器所实现。最终C++11移除了模板导出机制，`export`关键字被废弃并于C++20被重新赋予含义之后用于模块中。
- 一个典型的想法是扩展预编译头文件的概念，以同时加载多个预编译头文件。一个现实的障碍是预处理器，宏在预处理阶段就处理完成了，一个宏开关就可能会导致预处理后的代码大相径庭。C++20引入的模块在尝试解决某些C++模块组织、包管理和编译时间问题。

## 总结

- 模板的包含模型是组织模板的最常见手段，14章中讲替代手段——显式实例化。
- 函数模板的全特化同普通函数一样定义在头文件中时需要`inline`。
- 预编译头文件可以有效减低编译时间，源文件中需要使用相同顺序的`#include`指令（一般来说应提取到一个头文件中）。
- 调试模板代码可能会很棘手。
