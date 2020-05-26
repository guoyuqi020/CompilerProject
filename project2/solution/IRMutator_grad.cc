/*
 * MIT License
 * 
 * Copyright (c) 2020 Size Zheng

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "../../include/IRMutator_grad.h"

//<矩阵名字，shape>
extern std::map<std::string, std::vector<size_t>> global_shape_map;

//dest_name 存储了需要求导对象的名字，比如对A矩阵求导就会存储["A"]
std::vector<std::string> global_dest_name;
//dest_index 指示当次访问需要对dest_name这个列表中的哪一个名字求导
//例如dest_name=["A","B"]，那么dest_index=0表示本次对A求导
size_t global_dest_index;

//样例中生成函数签名的时候，出现了非常tricky的情况
//大概的意思就是，使用了哪几个矩阵，就在签名中出现哪几个矩阵，没有用到的不出现
//例如C=A*B(case1),对A求导，结果是C'=A'*B+A*B'，但是B'=0，所以可以被优化成C'=A'*B
//这样就只用到了B矩阵，所以函数签名里是没有A矩阵的。
//如果不做上面这个优化，函数体中就会出现同时使用A和B矩阵的现象，导致出错
//所以生成导数计算式时要注意优化零项
//同时global_used用来存储所有用到过的矩阵的名字，用到过的名字就插入这个set
//这样在生成签名时会方便一些
std::set<std::string> global_used;

using namespace Boost::Internal;

Expr IRMutator_grad::mutate(const Expr &expr)
{
    return expr.mutate_expr(this);
}

Stmt IRMutator_grad::mutate(const Stmt &stmt)
{
    return stmt.mutate_stmt(this);
}

Group IRMutator_grad::mutate(const Group &group)
{
    return group.mutate_group(this);
}

Expr IRMutator_grad::visit(Ref<const IntImm> op)
{
    return op;
}

Expr IRMutator_grad::visit(Ref<const UIntImm> op)
{
    return op;
}

Expr IRMutator_grad::visit(Ref<const FloatImm> op)
{
    return op;
}

Expr IRMutator_grad::visit(Ref<const StringImm> op)
{
    return op;
}

Expr IRMutator_grad::visit(Ref<const Unary> op)
{
    Expr new_a = mutate(op->a);
    return Unary::make(op->type(), op->op_type, new_a);
}

Expr IRMutator_grad::visit(Ref<const Binary> op)
{
    Expr new_a = mutate(op->a);
    Expr new_b = mutate(op->b);
    return Binary::make(op->type(), op->op_type, new_a, new_b);
}

Expr IRMutator_grad::visit(Ref<const Compare> op)
{
    Expr new_a = mutate(op->a);
    Expr new_b = mutate(op->b);
    return Compare::make(op->type(), op->op_type, new_a, new_b);
}

Expr IRMutator_grad::visit(Ref<const Select> op)
{
    Expr new_cond = mutate(op->cond);
    Expr new_true_value = mutate(op->true_value);
    Expr new_false_value = mutate(op->false_value);
    return Select::make(op->type(), new_cond, new_true_value, new_false_value);
}

Expr IRMutator_grad::visit(Ref<const Call> op)
{
    std::vector<Expr> new_args;
    for (auto arg : op->args)
    {
        new_args.push_back(mutate(arg));
    }
    return Call::make(op->type(), new_args, op->func_name, op->call_type);
}

Expr IRMutator_grad::visit(Ref<const Cast> op)
{
    Expr new_val = mutate(op->val);
    return Cast::make(op->type(), op->new_type, new_val);
}

Expr IRMutator_grad::visit(Ref<const Ramp> op)
{
    Expr new_base = mutate(op->base);
    return Ramp::make(op->type(), new_base, op->stride, op->lanes);
}

Expr IRMutator_grad::visit(Ref<const Var> op)
{
    std::vector<Expr> new_args;
    for (auto arg : op->args)
    {
        new_args.push_back(mutate(arg));
    }
    return Var::make(op->type(), op->name, new_args, op->shape);
}

Expr IRMutator_grad::visit(Ref<const Dom> op)
{
    Expr new_begin = mutate(op->begin);
    Expr new_extent = mutate(op->extent);
    return Dom::make(op->type(), new_begin, new_extent);
}

Expr IRMutator_grad::visit(Ref<const Index> op)
{
    Expr new_dom = mutate(op->dom);
    return Index::make(op->type(), op->name, new_dom, op->index_type);
}

Stmt IRMutator_grad::visit(Ref<const LoopNest> op)
{
    std::vector<Expr> new_index_list;
    std::vector<Stmt> new_body_list;
    for (auto index : op->index_list)
    {
        new_index_list.push_back(mutate(index));
    }
    for (auto body : op->body_list)
    {
        new_body_list.push_back(mutate(body));
    }
    return LoopNest::make(new_index_list, new_body_list);
}

Stmt IRMutator_grad::visit(Ref<const IfThenElse> op)
{
    Expr new_cond = mutate(op->cond);
    Stmt new_true_case = mutate(op->true_case);
    Stmt new_false_case = mutate(op->false_case);
    return IfThenElse::make(new_cond, new_true_case, new_false_case);
}

Stmt IRMutator_grad::visit(Ref<const Move> op)
{
    Expr new_dst = mutate(op->dst);
    Expr new_src = mutate(op->src);
    return Move::make(new_dst, new_src, op->move_type);
}

Group IRMutator_grad::visit(Ref<const Kernel> op)
{
    std::vector<Expr> new_inputs;
    for (auto expr : op->inputs)
    {
        new_inputs.push_back(mutate(expr));
    }
    std::vector<Expr> new_outputs;
    for (auto expr : op->outputs)
    {
        new_outputs.push_back(mutate(expr));
    }
    std::vector<Stmt> new_stmt_list;
    for (auto stmt : op->stmt_list)
    {
        new_stmt_list.push_back(mutate(stmt));
    }
    return Kernel::make(op->name, new_inputs, new_outputs, new_stmt_list, op->kernel_type);
}
