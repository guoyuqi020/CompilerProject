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
    Expr a_grad = mutate(op->a);
    return Unary::make(op->type(), op->op_type, a_grad);
}

Expr IRMutator_grad::visit(Ref<const Binary> op)
{
    Expr a_grad = mutate(op->a);
    Expr b_grad = mutate(op->b);
    if (op->op_type == BinaryOpType::Add || op->op_type == BinaryOpType::Sub)
    {
        if (a_grad->is_zero && b_grad->is_zero)
        {
            std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
            ret_ptr->is_zero = true;
            return std::const_pointer_cast<const IntImm>(ret_ptr);
        }
        else
        {
            return Binary::make(op->type(), op->op_type, a_grad, b_grad);
        }
    }
    else if (op->op_type == BinaryOpType::Mul)
    {
        if (a_grad->is_zero && b_grad->is_zero)
        {
            std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
            ret_ptr->is_zero = true;
            return std::const_pointer_cast<const IntImm>(ret_ptr);
        }
        else if (a_grad->is_zero)
        {
            return Binary::make(op->type(), BinaryOpType::Mul, op->a, b_grad);
        }
        else if (b_grad->is_zero)
        {
            return Binary::make(op->type(), BinaryOpType::Mul, a_grad, op->b);
        }
        else
        {
            std::shared_ptr<const Binary> term1_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->a, b_grad);
            std::shared_ptr<const Binary> term2_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, a_grad, op->b);
            std::shared_ptr<const Binary> sum_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Add, term1_ptr, term2_ptr);
            return Unary::make(op->type(), UnaryOpType::Bracket, sum_ptr);
        }
    }
    else if (op->op_type == BinaryOpType::Div || op->op_type == BinaryOpType::ExactlyDiv)
    {
        if (a_grad->is_zero && b_grad->is_zero)
        {
            std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
            ret_ptr->is_zero = true;
            return std::const_pointer_cast<const IntImm>(ret_ptr);
        }
        else if (a_grad->is_zero)
        {
            std::shared_ptr<const Binary> dividend_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->a, b_grad);
            std::shared_ptr<const Binary> divisor_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->b, op->b);
            std::shared_ptr<const Binary> quotient_ptr = std::make_shared<const Binary>(op->type(), op->op_type, dividend_ptr, divisor_ptr);
            return Unary::make(op->type(), UnaryOpType::Neg, quotient_ptr);
        }
        else if (b_grad->is_zero)
        {
            return Binary::make(op->type(), op->op_type, a_grad, op->b);
        }
        else
        {
            std::shared_ptr<const Binary> term1_ptr = std::make_shared<const Binary>(op->type(), op->op_type, a_grad, op->b);
            std::shared_ptr<const Binary> dividend_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->a, b_grad);
            std::shared_ptr<const Binary> divisor_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->b, op->b);
            std::shared_ptr<const Binary> term2_ptr = std::make_shared<const Binary>(op->type(), op->op_type, dividend_ptr, divisor_ptr);
            std::shared_ptr<const Binary> dif_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Add, term1_ptr, term2_ptr);
            return Unary::make(op->type(), UnaryOpType::Bracket, dif_ptr);
        }
    }
    return Binary::make(op->type(), op->op_type, a_grad, b_grad);
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
    global_used.clear();
    std::vector<Stmt> new_stmt_list;
    int temp_count = 1;
    int index_count = 1;
    for (global_dest_index = 0; global_dest_index < global_dest_name.size(); global_dest_index++)
    {
        for (auto stmt : op->stmt_list)
        {
            if (stmt.node_type() == IRNodeType::Move)
            {
                //rename and reshape temp
                index_count = 1;
                std::shared_ptr<Move> ptr = std::const_pointer_cast<Move>(stmt.as<Move>());
                std::shared_ptr<Var> dst_var_ptr = std::const_pointer_cast<Var>(ptr->dst.as<Var>());
                std::shared_ptr<Var> src_var_ptr = std::const_pointer_cast<Var>(ptr->src.as<Var>());
                dst_var_ptr->name = "temp" + std::to_string(temp_count);
                src_var_ptr->name = "temp" + std::to_string(temp_count);
                temp_count += 1;
                while (dst_var_ptr->shape.size() > dst_var_ptr->src_dim)
                {
                    dst_var_ptr->shape.pop_back();
                    dst_var_ptr->args.pop_back();
                    src_var_ptr->shape.pop_back();
                    src_var_ptr->args.pop_back();
                }
                std::vector<size_t> grad_shape = global_shape_map[global_dest_name[global_dest_index]];
                for (auto s : grad_shape)
                {
                    dst_var_ptr->shape.push_back(s);
                    src_var_ptr->shape.push_back(s);
                    dst_var_ptr->args.push_back(int(s));
                    src_var_ptr->args.push_back(StringImm::make(Type::int_scalar(32), "index" + std::to_string(index_count)));
                    index_count += 1;
                }
            }
            new_stmt_list.push_back(stmt);
        }
    }
    std::vector<Expr> new_inputs;
    for (auto expr : op->inputs)
    {
        new_inputs.push_back(expr);
    }
    std::vector<Expr> new_outputs;
    for (auto expr : op->outputs)
    {
        new_outputs.push_back(expr);
    }
    std::vector<Expr> new_grads;
    for (auto expr : op->grads)
    {
        new_grads.push_back(expr);
    }
    std::shared_ptr<Kernel> output_ptr = std::const_pointer_cast<Kernel>(std::make_shared<const Kernel>(op->name, new_inputs, new_outputs, new_stmt_list, op->kernel_type));
    output_ptr->grads = new_grads;
    return std::const_pointer_cast<const Kernel>(output_ptr);
}
