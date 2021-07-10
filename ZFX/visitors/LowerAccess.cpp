#include "IRVisitor.h"
#include "Stmts.h"
#include <map>

struct LoaderCallbackBase {
    virtual void operator()(int regid) const = 0;
};

template <class F>
struct LoaderCallbackImpl : LoaderCallbackBase {
    F f;
    LoaderCallbackImpl(F const &f) : f(f) {}
    virtual void operator()(int regid) const override {
        f(regid);
    }
};

struct LoaderCallback {
    std::unique_ptr<LoaderCallbackBase> p;

    LoaderCallback() = default;

    template <class F>
    LoaderCallback(F const &f)
        : p(std::make_unique<LoaderCallbackImpl<F>>(f))
    {}

    void operator()(int regid) const {
        (*p)(regid);
    }
};

struct LowerAccess : Visitor<LowerAccess> {
    using visit_stmt_types = std::tuple
        < AssignStmt
        , BinaryOpStmt
        , LiterialStmt
        , SymbolStmt
        , Statement
        >;

    std::unique_ptr<IR> ir = std::make_unique<IR>();

    void visit(Statement *stmt) {
        ir->push_clone_back(stmt);
    }

    struct RegInfo {
        int last_used = -1;
        int curr_stmtid = -1;
    };

    int now() const {
        return ir->size();
    }

    std::vector<RegInfo> regs{32};
    std::vector<int> memories;
    std::map<int, int> memories_lut;
    std::map<int, LoaderCallback> loaders;

    int temp_save(int regid, int stmtid) {
        for (int i = 0; i < memories.size(); i++) {
            if (memories[i] == -1) {
                memories_lut[stmtid] = i;
                memories[i] = stmtid;
                return i;
            }
        }
        int memid = memories.size();
        memories_lut[stmtid] = memid;
        memories.push_back(stmtid);
        return memid;
    }

    int alloc_register() {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].curr_stmtid == -1)
                return i;
        }
        int regid = 0;
        for (int i = 1; i < regs.size(); i++) {
            if (regs[i].last_used < regs[regid].last_used)
                regid = i;
        }
        if (regs[regid].curr_stmtid != -1) {
            int memid = temp_save(regid, regs[regid].curr_stmtid);
            ir->emplace_back<AsmMemoryStoreStmt>(memid, regid);
        }
        return regid;
    }

    int lookup(int stmtid) {
        for (int i = 0; i < regs.size(); i++) {
            if (regs[i].curr_stmtid == stmtid) {
                regs[i].last_used = now();
                return i;
            }
        }
        auto regid = alloc_register();
        regs[regid].curr_stmtid = stmtid;

        if (auto it = memories_lut.find(stmtid); it != memories_lut.end()) {
            int memid = it->second;
            memories[memid] = -1;
            ir->emplace_back<AsmMemoryLoadStmt>(memid, regid);
        }

        if (auto it = loaders.find(stmtid); it != loaders.end()) {
            it->second(regid);
        } else {
            cout << format("missing load from $%d to r%d", stmtid, regid) << endl;
        }
        return regid;
    }

    void visit(SymbolStmt *stmt) {
        loaders[stmt->id] = [this, stmt](int regid) {
            ir->emplace_back<AsmLoadSymbolStmt>
                ( regid
                , stmt->name
                );
        };
    }

    void visit(LiterialStmt *stmt) {
        loaders[stmt->id] = [this, stmt](int regid) {
            ir->emplace_back<AsmLoadConstStmt>
                ( regid
                , stmt->name
                );
        };
    }

    void visit(BinaryOpStmt *stmt) {
        ir->emplace_back<AsmBinaryOpStmt>
            ( stmt->op
            , lookup(stmt->id)
            , lookup(stmt->lhs->id)
            , lookup(stmt->rhs->id)
            );
    }

    void visit(AssignStmt *stmt) {
        ir->emplace_back<AsmAssignStmt>
            ( lookup(stmt->dst->id)
            , lookup(stmt->src->id)
            );
    }
};

std::unique_ptr<IR> apply_lower_access(IR *ir) {
    LowerAccess visitor;
    visitor.apply(ir);
    return std::move(visitor.ir);
}
