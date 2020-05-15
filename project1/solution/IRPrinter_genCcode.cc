

#include "../../include/IRPrinter_genCcode.h"
using namespace Boost::Internal;

void IRPrinter_genCcode::visit(Ref<const Kernel> op)
{
    print_indent();
    oss << "void " << op->name << "(";
    print_arg = true;
    bool has_input = false;
    data_type = op->printer_data_type;
    for (size_t i = 0; i < op->inputs.size(); ++i)
    {
        oss << data_type << " ";
        op->inputs[i].visit_expr(this);
        if (i < op->inputs.size() - 1)
        {
            oss << ", ";
        }
        has_input = true;
    }
    for (size_t i = 0; i < op->outputs.size(); ++i)
    {
        if (has_input || i != 0)
            oss << ", ";
        oss << op->printer_data_type << " ";
        op->outputs[i].visit_expr(this);
    }
    print_arg = false;
    oss << ") {\n";
    enter();
    for (auto stmt : op->stmt_list)
    {
        //(stmt.real_ptr()->visit_node(this));
        stmt.visit_stmt(this);
    }
    exit();
    oss << "}\n";
}
void IRPrinter_genCcode::visit(Ref<const IntImm> op)
{
    oss << op->value();
}

void IRPrinter_genCcode::visit(Ref<const UIntImm> op)
{
    oss << op->value();
}

void IRPrinter_genCcode::visit(Ref<const FloatImm> op)
{
    oss << op->value();
}

void IRPrinter_genCcode::visit(Ref<const StringImm> op)
{
    oss << op->value();
}

void IRPrinter_genCcode::visit(Ref<const Unary> op)
{
    if (op->op_type == UnaryOpType::Neg)
    {
        oss << "-";
        (op->a).visit_expr(this);
    }
    else if (op->op_type == UnaryOpType::Not)
    {
        oss << "!";
        (op->a).visit_expr(this);
    }
    else if (op->op_type == UnaryOpType::Bracket)
    {
        oss << "(";
        (op->a).visit_expr(this);
        oss << ")";
    }
}

void IRPrinter_genCcode::visit(Ref<const Binary> op)
{
    (op->a).visit_expr(this);
    if (op->op_type == BinaryOpType::Add)
    {
        oss << " + ";
    }
    else if (op->op_type == BinaryOpType::Sub)
    {
        oss << " - ";
    }
    else if (op->op_type == BinaryOpType::Mul)
    {
        oss << " * ";
    }
    else if (op->op_type == BinaryOpType::Div)
    {
        oss << " / ";
    }
    else if (op->op_type == BinaryOpType::Mod)
    {
        oss << " % ";
    }
    else if (op->op_type == BinaryOpType::And)
    {
        oss << " && ";
    }
    else if (op->op_type == BinaryOpType::Or)
    {
        oss << " || ";
    }
    else if (op->op_type == BinaryOpType::ExactlyDiv)
    {
        oss << " / ";
    }
    (op->b).visit_expr(this);
}

void IRPrinter_genCcode::visit(Ref<const Var> op)
{
    if (op->shape.size() != 1 || op->shape[0] != 1)
    { // Tensor

        oss << op->name;
        oss << "[";
        for (size_t i = 0; i < op->args.size(); ++i)
        {
            (op->args[i]).visit_expr(this);
            if (i < op->args.size() - 1)
            {
                oss << "][";
            }
        }
        oss << "]";
    }
    else
    { // Scalar
        oss << op->name;
    }
}

void IRPrinter_genCcode::visit(Ref<const IfThenElse> op)
{
    print_indent();
    oss << "if (";
    (op->cond).visit_expr(this);
    oss << ") {\n";
    enter();
    (op->true_case).visit_stmt(this);
    exit();
    print_indent();
    oss << "}\n";
    /*
    oss << "} else {\n";
    enter();
    (op->false_case).visit_stmt(this);
    exit();
    print_indent();
    oss << "}\n";
    */
}

void IRPrinter_genCcode::visit(Ref<const Move> op)
{
    print_indent();
    if (op->move_op == MoveOp::Declare)
    { // Declare temp
        oss << data_type << " ";
        op->dst.visit_expr(this);
    }
    else
    { // Move
        (op->dst).visit_expr(this);
        if (op->move_op == MoveOp::Equal)
        {
            oss << " = ";
        }
        else if (op->move_op == MoveOp::Plus)
        {
            oss << " += ";
        }
        else if (op->move_op == MoveOp::Minus)
        {
            oss << " -= ";
        }
        else if (op->move_op == MoveOp::Zero)
        {
            oss << " = ";
        }
        (op->src).visit_expr(this);
    }
    oss << ";\n";
}

std::string IRPrinter_genCcode::print(const Group &group)
{
    oss.clear();
    oss << "#include \"../run.h\"\n\n";
    group.visit_group(this);
    return oss.str();
}
void IRPrinter_genCcode::visit(Ref<const LoopNest> op)
{
    debug_count += 1;
    for (auto index : op->index_list)
    {
        print_indent();
        oss << "for ";
        index.visit_expr(this);
        oss << "{\n";
        enter();
    }

    for (auto body : op->body_list)
    {
        body.visit_stmt(this);
    }

    for (auto index : op->index_list)
    {
        exit();
        print_indent();
        oss << "}\n";
    }
}
void IRPrinter_genCcode::visit(Ref<const Dom> op)
{
    for_begin = std::to_string((op->begin.as<IntImm>())->value());
    for_end = std::to_string((op->extent.as<IntImm>())->value());
}
void IRPrinter_genCcode::visit(Ref<const Index> op)
{
    op->dom.visit_expr(this);
    oss << "(int " + op->name + " = " << for_begin << ";";
    oss << op->name + " < " << for_end << ";";
    oss << "++" + op->name + ")";
}