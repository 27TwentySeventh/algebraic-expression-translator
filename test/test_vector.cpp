#include "translator.h"
#include <gtest.h>

TEST(Vector, can_create_empty_vector) {
    Vector<int> v;
    EXPECT_EQ(0, v.size());
    EXPECT_TRUE(v.is_empty());
}

TEST(Vector, can_create_vector_with_zero_size) {
    Vector<int> v(0);
    EXPECT_EQ(0, v.size());
    EXPECT_TRUE(v.is_empty());
}

TEST(Vector, can_create_vector_with_positive_size) {
    Vector<int> v(5);
    EXPECT_EQ(5, v.size());
    EXPECT_GE(v.get_capacity(), v.size());
    EXPECT_FALSE(v.is_empty());
}

TEST(Vector, initializes_numeric_elements_with_zero) {
    Vector<int> v(5);
    for (size_t i = 0; i < v.size(); ++i)
        EXPECT_EQ(0, v[i]);
}

TEST(Vector, rejects_too_large_size) {
    EXPECT_THROW(Vector<int> v(MAX_VECTOR_SIZE + 1), std::out_of_range);
}

TEST(Vector, rejects_negative_size) {
    EXPECT_THROW(Vector<int> v(static_cast<size_t>(-1)), std::out_of_range);
}

TEST(Vector, can_read_and_change_elements) {
    Vector<int> v(3);
    v[0] = 42;
    v[2] = -7;
    EXPECT_EQ(42, v[0]);
    EXPECT_EQ(-7, v[2]);
}

TEST(Vector, checks_indices_including_const_access) {
    Vector<int> v(3);
    const Vector<int>& cv = v;
    EXPECT_THROW(v[3], std::out_of_range);
    EXPECT_THROW(v[static_cast<size_t>(-1)], std::out_of_range);
    EXPECT_THROW(cv[3], std::out_of_range);
    EXPECT_EQ(0, cv[0]);
}

TEST(Vector, appends_many_elements_and_saves_order) {
    Vector<int> v;
    for (int i = 0; i < 1000; ++i)
        v.push_back(i);
    ASSERT_EQ(1000, v.size());
    for (int i = 0; i < 1000; ++i)
        EXPECT_EQ(i, v[i]);
}

TEST(Vector, extends_when_current_storage_is_full) {
    Vector<int> v(2);
    v[0] = 10;
    v[1] = 20;
    const size_t old_capacity = v.get_capacity();
    while (v.size() < old_capacity)
        v.push_back(30);
    v.push_back(40);
    EXPECT_GT(v.get_capacity(), old_capacity);
    EXPECT_EQ(10, v[0]);
    EXPECT_EQ(20, v[1]);
    EXPECT_EQ(40, v.top());
}

TEST(Vector, can_append_own_element_during_reallocation) {
    Vector<int> v;
    v.push_back(1);
    while (v.size() < v.get_capacity())
        v.push_back(2);
    v.push_back(v[0]);
    EXPECT_EQ(1, v.top());
}

TEST(Vector, top_returns_last_element_without_removing_it) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);
    const Vector<int>& cv = v;
    EXPECT_EQ(20, cv.top());
    EXPECT_EQ(2, cv.size());
    EXPECT_EQ(20, v.top());
    EXPECT_EQ(2, v.size());
}

TEST(Vector, top_of_empty_vector_throws) {
    Vector<int> v;
    EXPECT_THROW(v.top(), std::out_of_range);
}

TEST(Vector, pop_removes_last_element) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);
    EXPECT_EQ(2, v.size());
    EXPECT_EQ(20, v.top());
    v.pop_back();
    EXPECT_EQ(1, v.size());
    EXPECT_EQ(10, v.top());
}

TEST(Vector, pop_of_empty_vector_throws) {
    Vector<int> v;
    EXPECT_TRUE(v.is_empty());
    EXPECT_THROW(v.pop_back(), std::out_of_range);
    EXPECT_TRUE(v.is_empty());
}

TEST(Vector, can_reuse_vector_after_removing_all_elements) {
    Vector<int> v(3);
    while (!v.is_empty())
        v.pop_back();
    v.push_back(99);
    EXPECT_EQ(1, v.size());
    EXPECT_EQ(99, v.top());
}

TEST(Vector, copied_vector_has_independent_storage) {
    Vector<std::string> original(2);
    original[0] = "original";
    Vector<std::string> copy(original);
    copy[0] = "copy";
    copy.pop_back();
    EXPECT_EQ("original", original[0]);
    EXPECT_EQ(2, original.size());
    EXPECT_EQ(1, copy.size());
}

TEST(Vector, copy_survives_after_source_destruction) {
    Vector<int> copy;
    {
        Vector<int> original(2);
        original[1] = 1;
        copy = original;
    }
    ASSERT_EQ(2, copy.size());
    EXPECT_EQ(1, copy[1]);
}

TEST(Vector, assignment_replaces_size_and_copies_elements) {
    Vector<int> original(3), copy(1);
    original[2] = 42;
    copy = original;
    ASSERT_EQ(3, copy.size());
    EXPECT_EQ(42, copy[2]);
    copy[2] = 99;
    EXPECT_EQ(42, original[2]);
}

TEST(Vector, can_assign_empty_vector_and_then_reuse_it) {
    Vector<int> v(3), empty;
    v = empty;
    EXPECT_TRUE(v.is_empty());
    v.push_back(7);
    EXPECT_EQ(7, v.top());
}

TEST(Vector, self_assignment_saves_contents) {
    Vector<int> v(2);
    v[1] = 17;
    Vector<int>& same = v;
    v = same;
    EXPECT_EQ(2, v.size());
    EXPECT_EQ(17, v[1]);
}

TEST(Vector, pop_and_destruction_release_owned_elements) {
    std::shared_ptr<int> p = std::make_shared<int>(42);
    {
        Vector<std::shared_ptr<int>> v;
        v.push_back(p);
        v.push_back(p);
        EXPECT_EQ(3, p.use_count());
        v.pop_back();
        EXPECT_EQ(2, p.use_count());
    }
    EXPECT_EQ(1, p.use_count());
}