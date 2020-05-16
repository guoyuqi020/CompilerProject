#ifndef BOOST_IRTERMFINDER_H
#define BOOST_IRTERMFINDER_H

#include <string>
#include <sstream>

#include "IR.h"
#include "IRVisitor.h"

namespace Boost
{

    namespace Internal
    {

        class term_group
        {
        public:
            Ref<IRNode> term_ptr;
            BinaryOpType left_op;
            std::shared_ptr<LoopNest> for_stmt_group = nullptr;
            std::shared_ptr<IfThenElse> if_stmt_group = nullptr;
            std::shared_ptr<Move> move_stmt = nullptr;
            std::shared_ptr<Var> lhs, lhs_temp;

            template <typename U, typename std::enable_if<std::is_base_of<IRNode, U>::value>::type * = nullptr>
            term_group(std::shared_ptr<const U> _ptr, BinaryOpType _op, std::shared_ptr<Var> _lhs, std::shared_ptr<Var> _lhs_temp)
            {
                term_ptr = Ref<IRNode>(std::const_pointer_cast<U>(_ptr));
                left_op = _op;
                lhs = _lhs;
                lhs_temp = _lhs_temp;
            }
        };

        class IRTermFinder : public IRVisitor
        {
        public:
            //std::vector< std::shared_ptr<IRNode> > term_list;
            std::vector<std::shared_ptr<term_group>> term_group_list;
            std::shared_ptr<Var> _lhs, _lhs_temp;
            std::shared_ptr<Move> pre_move_stmt, post_move_stmt;
            std::shared_ptr<LoopNest> outer_loop1, outer_loop2;

            void visit(Ref<const Binary>) override;
            void visit(Ref<const Move>) override;

            void getForStmtGroup();
            void getIfStmtGroup();
            void getMoveStmt();
            void getOuterLoop();
        };

        class IRIfFinder : public IRVisitor
        {
        public:
            std::vector<std::shared_ptr<IfThenElse>> _if_stmt_group;

            void visit(Ref<const Var>) override;
        };

    } // namespace Internal

} // namespace Boost

#endif // BOOST_IRTERMFINDER_H