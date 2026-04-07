#pragma once
#include "filter_policy.hpp"
#include "db_format.hpp"


// Filter policy wrapper that converts from internal keys to user keys
class InternalFilterPolicy : public FilterPolicy
{
private:
    const FilterPolicy *const user_policy_;

public:
    explicit InternalFilterPolicy(const FilterPolicy *p) : user_policy_(p) {}
    const char *Name() const override;
    void CreateFilter(const Slice *keys, int n, std::string *dst) const override;
    bool KeyMayMatch(const Slice &key, const Slice &filter) const override;
};