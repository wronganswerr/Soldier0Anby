#include <cstddef>
#include <cstdint>
#include <string>

#include "comparator.hpp"
// #include "db.hpp"

#include "filter_policy.hpp"
#include "slice.hpp"
#include "table_builder.hpp"
#include "coding.hpp"
#include "logging.hpp"
#include "internal_key.hpp"

// A comparator for internal keys that uses a specified comparator for
// the user key portion and breaks ties by decreasing sequence number.
class InternalKeyComparator : public Comparator {
 private:
  const Comparator* user_comparator_;

 public:
  explicit InternalKeyComparator(const Comparator* c) : user_comparator_(c) {}
  const char* Name() const override;
  int Compare(const Slice& a, const Slice& b) const override;
  void FindShortestSeparator(std::string* start,
                             const Slice& limit) const override;
  void FindShortSuccessor(std::string* key) const override;

  const Comparator* user_comparator() const { return user_comparator_; }

  int Compare(const InternalKey& a, const InternalKey& b) const;
};