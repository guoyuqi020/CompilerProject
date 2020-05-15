#ifndef BOOST_IRPRINTER_GENCCODE_H
#define BOOST_IRPRINTER_GENCCODE_H

#include "IRPrinter.h"

namespace Boost
{

    namespace Internal
    {

        class IRPrinter_genCcode : public IRPrinter
        {
        public:
            IRPrinter_genCcode() : IRPrinter() {}
            void visit(Ref<const Kernel> op) override;
            void visit(Ref<const IntImm>) override;
            void visit(Ref<const UIntImm>) override;
            void visit(Ref<const FloatImm>) override;
            void visit(Ref<const StringImm>) override;
            void visit(Ref<const Unary>) override;
            void visit(Ref<const Binary>) override;
            void visit(Ref<const Var>) override;
            void visit(Ref<const IfThenElse>) override;
            void visit(Ref<const Move>) override;
            void visit(Ref<const LoopNest> op) override;
            void visit(Ref<const Dom>) override;
            void visit(Ref<const Index>) override;
            std::string print(const Group &);

        private:
            int debug_count = 0;
            std::string data_type;
            std::string for_begin, for_end;
        };

    } // namespace Internal

} // namespace Boost

#endif // BOOST_IRPRINTER_GENCCODE_H