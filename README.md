# tiger2
a tony compiler for tiger 

module list
-----------
* const.h 

实现常量池，目前实现了整数常量池，字符串常量池未实现，对于在池中每个常量都会有一个id标识。常量池使用hash链表实现，标识常量的id高位标识在hash链表的哪条链上，剩下位标识该常量的hash位置。

* temp.h temp.c

实现临时变量池，每次返回一个临时变量，该临时变量同样有一个id标识，类似常量池的定义方式。临时变量前缀可以指定，每次返回名字唯一的一个临时变量。在解释执行中间代码前为临时变量可以绑定一个存储空间或者对象，可以很方便的解释执行。

* label.h label.c

实现标号池，类似临时变量池的实现，每次返回唯一名字的标号，标号的目的是指定一个中间代码的位置，方便代码跳转。如if语句解析需要两个标号，标识控制流向。

* token.h token.c

实现单词解析，如果有新增单词，则在该文件增加。

* scanner.h scanner.c

实现词法解析，每次返回一个单词，支持回退一个单词，新增单词同样需要修改该文件。

* tiger_type.h

基本类型定义。

* parser.h parser.c

实现自上而下语法解析，构造语法解析树，应该算AST，实际没必要构造出抽象语法树，可以在语法解析中直接生成中间代码表示，后面基于ir做优化，生成最终的代码。

* main.c

实现驱动模块，各模块的单元测试。
