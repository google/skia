//
// Created by Herb Derby on 4/11/18.
//

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "Test.h"

DEF_TEST(Bizaro, reporter) {

    class B {
    public:
        B(std::unique_ptr<std::string>&& s_) : s{std::move(s_)} {}


    private:
        std::unique_ptr<std::string> s;
    };

    struct SEQ {
        bool operator () (const std::string* a, const std::string* b) const {
            return *a == *b;
        }
    };

    struct HASH {
        uint64_t operator () (const std::string* a) const {
            return a->size();
        }
    };
    std::unordered_map<const std::string*, B, HASH, SEQ> os(static_cast<size_t>(16), HASH(), SEQ());
    std::unique_ptr<std::string> qqq{new std::string("hello")};
    auto pqqq = qqq.get();
    os.emplace(pqqq, std::move(qqq) );
}
