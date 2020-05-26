5.26 by 郭宇祺

目前打算采用的方法是用IRMutator_grad继承IRMutator，然后改写IRMutator_grad

IRMutator_grad.cc里添加了以下全局变量，具体说明可看文件内注释
'''
extern std::map<std::string, std::vector<size_t>> global_shape_map;
std::vector<std::string> global_dest_name;
size_t global_dest_index;
std::set<std::string> global_used;
'''

现在我们的程序生成的文件名字是kernel_case*.cc，给出的样例代码的文件名字为grad_case*.cc。
检查发现case1的样例代码函数头有误，第三个参数名字应该是dA而不是A。

目前的mutator和我们构造的树似乎不兼容，总之从根节点开始mutate会报错。所以目前输出的c代码没有进行mutate操作。
如果想打印一下自己写的表达式，似乎只能通过全局变量传递节点，然后打印这个表达式。(然而还是要先保证外层访问不报错)

在目前的代码结构下，全局cmake是一定会失败的。一种可能的方法是只在project2文件夹内cmake，如果能成功且在kernel下生成了对应代码文件，就应该没有问题。