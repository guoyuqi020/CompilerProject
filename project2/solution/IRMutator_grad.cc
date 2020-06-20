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

extern std::map<std::string, std::pair<int, int>> global_map;

//<矩阵名字，shape>
extern std::map<std::string, std::vector<size_t>> global_shape_map;

//dest_name 存储了需要求导对象的名字，比如对A矩阵求导就会存储["A"]
std::vector<std::string> global_dest_name;

/*
 * dest_index 指示当次访问需要对dest_name这个列表中的哪一个名字求导
 * 例如dest_name=["A","B"]，那么dest_index=0表示本次对A求导
 * */
size_t global_dest_index;

/* 样例中生成函数签名的时候，出现了非常tricky的情况
 * 大概的意思就是，使用了哪几个矩阵，就在签名中出现哪几个矩阵，没有用到的不出现
 * 例如C=A*B(case1),对A求导，结果是C'=A'*B+A*B'，但是B'=0，所以可以被优化成C'=A'*B
 * 这样就只用到了B矩阵，所以函数签名里是没有A矩阵的。
 * 如果不做上面这个优化，函数体中就会出现同时使用A和B矩阵的现象，导致出错
 * 所以生成导数计算式时要注意优化零项
 * 同时global_used用来存储所有用到过的矩阵的名字，用到过的名字就插入这个set
 * 这样在生成签名时会方便一些 
 * */
std::set<std::string> global_used;

//求导目标矩阵下标
std::vector<std::string> global_grad_index;

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
    std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
    ret_ptr->is_zero = true;
    return std::const_pointer_cast<const IntImm>(ret_ptr);
}

Expr IRMutator_grad::visit(Ref<const UIntImm> op)
{
    std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
    ret_ptr->is_zero = true;
    return std::const_pointer_cast<const IntImm>(ret_ptr);
}

Expr IRMutator_grad::visit(Ref<const FloatImm> op)
{
    std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
    ret_ptr->is_zero = true;
    return std::const_pointer_cast<const IntImm>(ret_ptr);
}

Expr IRMutator_grad::visit(Ref<const StringImm> op)
{
    std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
    ret_ptr->is_zero = true;
    return std::const_pointer_cast<const IntImm>(ret_ptr);
}

Expr IRMutator_grad::visit(Ref<const Unary> op)
{
    Expr a_grad = mutate(op->a);
    std::shared_ptr<Unary> ret_ptr = std::make_shared<Unary>(op->type(), op->op_type, a_grad);
    if (!a_grad->is_zero)
        ret_ptr->var_used.insert(a_grad->var_used.begin(), a_grad->var_used.end());
    return std::const_pointer_cast<const Unary>(ret_ptr);
}

Expr IRMutator_grad::visit(Ref<const Binary> op)
{
    Expr a_grad = mutate(op->a);
    Expr b_grad = mutate(op->b);

    std::set<std::string> a_used = op->a->var_used;
    std::set<std::string> b_used = op->b->var_used;
    if (op->a->node_type() == IRNodeType::Var)
        a_used.insert((op->a.as<Var>())->name);
    if (op->b->node_type() == IRNodeType::Var)
        b_used.insert((op->b.as<Var>())->name);

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
            std::shared_ptr<Binary> ret_ptr = std::make_shared<Binary>(op->type(), op->op_type, a_grad, b_grad);
            if (!a_grad->is_zero)
                ret_ptr->var_used.insert(a_grad->var_used.begin(), a_grad->var_used.end());
            if (!b_grad->is_zero)
                ret_ptr->var_used.insert(b_grad->var_used.begin(), b_grad->var_used.end());
            return std::const_pointer_cast<const Binary>(ret_ptr);
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
            std::shared_ptr<Binary> ret_ptr = std::make_shared<Binary>(op->type(), BinaryOpType::Mul, op->a, b_grad);
            ret_ptr->var_used.insert(a_used.begin(), a_used.end());
            ret_ptr->var_used.insert(b_grad->var_used.begin(), b_grad->var_used.end());
            return std::const_pointer_cast<const Binary>(ret_ptr);
        }
        else if (b_grad->is_zero)
        {
            std::shared_ptr<Binary> ret_ptr = std::make_shared<Binary>(op->type(), BinaryOpType::Mul, a_grad, op->b);
            ret_ptr->var_used.insert(a_grad->var_used.begin(), a_grad->var_used.end());
            ret_ptr->var_used.insert(b_used.begin(), b_used.end());
            return std::const_pointer_cast<const Binary>(ret_ptr);
        }
        else
        {
            std::shared_ptr<const Binary> term1_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->a, b_grad);
            std::shared_ptr<const Binary> term2_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, a_grad, op->b);
            std::shared_ptr<const Binary> sum_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Add, term1_ptr, term2_ptr);
            std::shared_ptr<Unary> ret_ptr = std::make_shared<Unary>(op->type(), UnaryOpType::Bracket, sum_ptr);
            ret_ptr->var_used.insert(a_used.begin(), a_used.end());
            ret_ptr->var_used.insert(b_grad->var_used.begin(), b_grad->var_used.end());
            ret_ptr->var_used.insert(a_grad->var_used.begin(), a_grad->var_used.end());
            ret_ptr->var_used.insert(b_used.begin(), b_used.end());
            return std::const_pointer_cast<const Unary>(ret_ptr);
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
            std::shared_ptr<Unary> ret_ptr = std::make_shared<Unary>(op->type(), UnaryOpType::Neg, quotient_ptr);
            ret_ptr->var_used.insert(a_used.begin(), a_used.end());
            ret_ptr->var_used.insert(b_grad->var_used.begin(), b_grad->var_used.end());
            ret_ptr->var_used.insert(b_used.begin(), b_used.end());
            return std::const_pointer_cast<const Unary>(ret_ptr);
        }
        else if (b_grad->is_zero)
        {
            std::shared_ptr<Binary> ret_ptr = std::make_shared<Binary>(op->type(), op->op_type, a_grad, op->b);
            ret_ptr->var_used.insert(a_grad->var_used.begin(), a_grad->var_used.end());
            ret_ptr->var_used.insert(b_used.begin(), b_used.end());
            return std::const_pointer_cast<const Binary>(ret_ptr);
        }
        else
        {
            std::shared_ptr<const Binary> term1_ptr = std::make_shared<const Binary>(op->type(), op->op_type, a_grad, op->b);
            std::shared_ptr<const Binary> dividend_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->a, b_grad);
            std::shared_ptr<const Binary> divisor_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Mul, op->b, op->b);
            std::shared_ptr<const Binary> term2_ptr = std::make_shared<const Binary>(op->type(), op->op_type, dividend_ptr, divisor_ptr);
            std::shared_ptr<const Binary> dif_ptr = std::make_shared<const Binary>(op->type(), BinaryOpType::Sub, term1_ptr, term2_ptr);
            std::shared_ptr<Unary> ret_ptr = std::make_shared<Unary>(op->type(), UnaryOpType::Bracket, dif_ptr);
            ret_ptr->var_used.insert(a_used.begin(), a_used.end());
            ret_ptr->var_used.insert(b_grad->var_used.begin(), b_grad->var_used.end());
            ret_ptr->var_used.insert(a_grad->var_used.begin(), a_grad->var_used.end());
            ret_ptr->var_used.insert(b_used.begin(), b_used.end());
            return std::const_pointer_cast<const Unary>(ret_ptr);
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
    if (op->name != global_dest_name[global_dest_index])
    {
        std::shared_ptr<IntImm> ret_ptr = std::make_shared<IntImm>(Type::int_scalar(32), 0);
        ret_ptr->is_zero = true;
        return std::const_pointer_cast<const IntImm>(ret_ptr);
    }
    //TO DO:index match
    //return Select

    std::vector<Expr> conds;
    for (std::vector<Expr>::size_type i = 0; i < global_grad_index.size(); i++)
    {
        conds.push_back(Compare::make(Type::int_scalar(32), CompareOpType::EQ, StringImm::make(Type::int_scalar(32), global_grad_index[i]), op->args[i]));
    }
    Expr cond = conds[0];
    for (std::vector<Expr>::size_type i = 1; i < conds.size(); i++)
    {
        cond = Binary::make(Type::int_scalar(32), BinaryOpType::And, cond, conds[i]);
    }
    std::shared_ptr<Select> ret_ptr = std::make_shared<Select>(Type::int_scalar(32), cond, Expr(1), Expr(0));
    return std::const_pointer_cast<const Select>(ret_ptr);
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
    std::vector<Stmt> new_body_list;
    for (auto body : op->body_list)
    {
        new_body_list.push_back(mutate(body));
    }
    return LoopNest::make(op->index_list, new_body_list);
}

Stmt IRMutator_grad::visit(Ref<const IfThenElse> op)
{
    Stmt new_true_case = mutate(op->true_case);
    return IfThenElse::make(op->cond, new_true_case, op->false_case);
}

Stmt IRMutator_grad::visit(Ref<const Move> op)
{
    std::shared_ptr<Var> dst_var_ptr = std::const_pointer_cast<Var>(op->dst.as<Var>());
    std::shared_ptr<Var> src_var_ptr = std::const_pointer_cast<Var>(op->src.as<Var>());

    if (op->move_op == MoveOp::Chain_rule)
    {
        std::shared_ptr<Var> new_dst_var_ptr = std::make_shared<Var>(Type::int_scalar(32), "d" + global_dest_name[global_dest_index],
                                                                     std::vector<Expr>(), std::vector<size_t>());
        std::vector<size_t> grad_shape = global_shape_map[global_dest_name[global_dest_index]];
        int index_count = 1;
        for (auto s : grad_shape)
        {
            new_dst_var_ptr->shape.push_back(s);
            new_dst_var_ptr->args.push_back(StringImm::make(Type::int_scalar(32), "index" + std::to_string(index_count)));
            index_count += 1;
        }
        if (dst_var_ptr->name.substr(0, 1).compare("d") != 0)
            dst_var_ptr->name = "d" + dst_var_ptr->name;
        Expr RHS_1_ptr = Expr(std::dynamic_pointer_cast<const Var>(dst_var_ptr));
        Expr RHS_2_ptr = Var::make(src_var_ptr->type(), src_var_ptr->name, src_var_ptr->args, src_var_ptr->shape);
        std::shared_ptr<Binary> RHS_ptr = std::make_shared<Binary>(Type::float_scalar(32), BinaryOpType::Mul, RHS_1_ptr, RHS_2_ptr);
        Expr lhs_ptr = Expr(std::dynamic_pointer_cast<const Var>(new_dst_var_ptr));
        Expr rhs_ptr = Expr(std::dynamic_pointer_cast<const Binary>(RHS_ptr));
        std::shared_ptr<Move> s_ptr = std::make_shared<Move>(lhs_ptr, rhs_ptr, MoveType::MemToMem);
        s_ptr->move_op = MoveOp::Plus;
        return std::const_pointer_cast<const Move>(s_ptr);
    }
    else
    {
        Expr new_src = mutate(op->src);
        for (auto item : new_src->var_used)
        {
            global_used.insert(item);
        }
        std::shared_ptr<const Var> ptr = (op->dst).as<Var>();

        std::shared_ptr<Move> ret_ptr = std::make_shared<Move>(Var::make(ptr->type(), ptr->name, ptr->args, ptr->shape), new_src, op->move_type);
        ret_ptr->move_op = op->move_op;
        return std::const_pointer_cast<const Move>(ret_ptr);
    }
}

Group IRMutator_grad::visit(Ref<const Kernel> op)
{
    global_used.clear();
    std::vector<Stmt> new_stmt_list;
    int temp_count = 1;
    int index_count = 1;
    for (global_dest_index = 0; global_dest_index < global_dest_name.size(); global_dest_index++)
    {
        std::vector<Expr> new_index_list;
        std::vector<Stmt> new_body_list;
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
                global_grad_index.clear();
                for (auto s : grad_shape)
                {
                    /*
                    dst_var_ptr->shape.push_back(s);
                    src_var_ptr->shape.push_back(s);
                    dst_var_ptr->args.push_back(int(s));
                    src_var_ptr->args.push_back(StringImm::make(Type::int_scalar(32), "index" + std::to_string(index_count)));
                    */
                    global_grad_index.push_back("index" + std::to_string(index_count));
                    Expr dom = Dom::make(Type::int_scalar(32), 0, int(s));
                    new_index_list.push_back(Index::make(Type::int_scalar(32), "index" + std::to_string(index_count), dom, IndexType::Block));
                    index_count += 1;
                }
                std::shared_ptr<Move> ret_ptr = std::make_shared<Move>(Var::make(dst_var_ptr->type(), dst_var_ptr->name, dst_var_ptr->args, dst_var_ptr->shape), Var::make(src_var_ptr->type(), src_var_ptr->name, src_var_ptr->args, src_var_ptr->shape), ptr->move_type);
                (ret_ptr->move_op) = ptr->move_op;
                new_body_list.push_back(std::const_pointer_cast<const Move>(ret_ptr));

                //set output array to zero
                std::shared_ptr<Var> output_dst = std::make_shared<Var>(Type::int_scalar(32), "d" + global_dest_name[global_dest_index], std::vector<Expr>(), std::vector<size_t>());
                for (auto idx_name : global_grad_index)
                {
                    output_dst->args.push_back(StringImm::make(Type::int_scalar(32), idx_name));
                }
                std::shared_ptr<Move> init_assign = std::make_shared<Move>(std::const_pointer_cast<const Var>(output_dst), 0.0, MoveType::MemToMem);
                init_assign->move_op = MoveOp::Zero;
                new_body_list.push_back(std::const_pointer_cast<const Move>(init_assign));
            }
            else
            {
                new_body_list.push_back(mutate(stmt));
            }
        }
        std::shared_ptr<LoopNest> outer_loop = std::make_shared<LoopNest>(new_index_list, new_body_list);
        new_stmt_list.push_back(Stmt(std::dynamic_pointer_cast<const LoopNest>(outer_loop)));
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
    output_ptr->printer_data_type = op->printer_data_type;
    return std::const_pointer_cast<const Kernel>(output_ptr);
}
