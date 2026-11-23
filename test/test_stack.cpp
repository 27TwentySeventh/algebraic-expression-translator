#include "translator.h"
#include <gtest.h>

TEST(Stack, default_stack_is_empty) {
    Stack<int> s;
    EXPECT_TRUE(s.is_empty());
    EXPECT_EQ(0, s.size());
}

TEST(Stack, push_changes_size_and_top) {
    Stack<int> s;
    s.push(10);
    s.push(20);
    EXPECT_FALSE(s.is_empty());
    EXPECT_EQ(2, s.size());
    EXPECT_EQ(20, s.top());
}

TEST(Stack, elements_are_removed_in_lifo_order) {
    Stack<int> s;
    for (int i = 0; i < 1000; ++i)
        s.push(i);
    for (int i = 999; i >= 0; --i) {
        ASSERT_EQ(i, s.top());
        s.pop();
    }
    EXPECT_TRUE(s.is_empty());
}

TEST(Stack, top_does_not_remove_element) {
    Stack<int> s;
    s.push(42);
    EXPECT_EQ(42, s.top());
    EXPECT_EQ(42, s.top());
    EXPECT_EQ(1, s.size());
}

TEST(Stack, supports_const_top) {
    Stack<int> s;
    s.push(42);
    const Stack<int>& cs = s;
    EXPECT_EQ(42, cs.top());
}

TEST(Stack, empty_top_throws) {
    Stack<int> s;
    EXPECT_THROW(s.top(), std::out_of_range);
}

TEST(Stack, empty_pop_throws) {
    Stack<int> s;
    EXPECT_THROW(s.pop(), std::out_of_range);
}

TEST(Stack, can_be_reused_after_becoming_empty) {
    Stack<int> s;
    s.push(1);
    EXPECT_EQ(1, s.size());
    s.pop();
    EXPECT_TRUE(s.is_empty());
    s.push(2);
    EXPECT_EQ(2, s.top());
    EXPECT_EQ(1, s.size());
}

TEST(Stack, copied_stack_is_independent) {
    Stack<std::string> s;
    s.push("first");
    s.push("second");
    Stack<std::string> copy(s);
    copy.pop();
    copy.push("changed");
    EXPECT_EQ("second", s.top());
    EXPECT_EQ("changed", copy.top());
}

TEST(Stack, assignment_copies_contents_independently) {
    Stack<int> s, copy;
    s.push(10);
    s.push(20);
    copy.push(99);
    copy = s;
    copy.pop();
    EXPECT_EQ(20, s.top());
    EXPECT_EQ(10, copy.top());
    EXPECT_EQ(1, copy.size());
}

TEST(Stack, push_expands_full_storage) {
    Stack<int> s;
    s.push(10);
    while (!s.is_full())
        s.push(20);
    const size_t old_size = s.size();
    ASSERT_NO_THROW(s.push(30));
    EXPECT_EQ(old_size + 1, s.size());
    EXPECT_EQ(30, s.top());
}