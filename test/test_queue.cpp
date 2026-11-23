#include "translator.h"
#include <gtest.h>

TEST(Queue, default_queue_is_empty) {
    Queue<int> q;
    EXPECT_TRUE(q.is_empty());
    EXPECT_FALSE(q.is_full());
    EXPECT_EQ(0, q.size());
    EXPECT_EQ(100, q.get_capacity());
}

TEST(Queue, stores_requested_capacity) {
    Queue<int> q(3);
    EXPECT_EQ(3, q.get_capacity());
}

TEST(Queue, rejects_invalid_capacity) {
    EXPECT_THROW(Queue<int> q(0), std::invalid_argument);
    EXPECT_THROW(Queue<int> q(MAX_VECTOR_SIZE + 1), std::invalid_argument);
    EXPECT_THROW(Queue<int> q(static_cast<size_t>(-1)), std::invalid_argument);
}

TEST(Queue, elements_are_removed_in_fifo_order) {
    Queue<int> q(1000);
    for (int i = 0; i < 1000; ++i)
        q.push(i);
    for (int i = 0; i < 1000; ++i) {
        ASSERT_EQ(i, q.front());
        q.pop();
    }
    EXPECT_TRUE(q.is_empty());
}

TEST(Queue, front_does_not_remove_element) {
    Queue<int> q;
    q.push(42);
    EXPECT_EQ(42, q.front());
    EXPECT_EQ(42, q.front());
    EXPECT_EQ(1, q.size());
}

TEST(Queue, front_on_empty_queue_throws) {
    Queue<int> q;
    EXPECT_THROW(q.front(), std::out_of_range);
}

TEST(Queue, pop_on_empty_queue_throws) {
    Queue<int> q;
    EXPECT_THROW(q.pop(), std::out_of_range);
}

TEST(Queue, full_queue_rejects_push_without_losing_elements) {
    Queue<int> q(2);
    q.push(10);
    q.push(20);
    EXPECT_TRUE(q.is_full());
    EXPECT_THROW(q.push(30), std::overflow_error);
    EXPECT_EQ(2, q.size());
    EXPECT_EQ(10, q.front());
    q.pop();
    EXPECT_EQ(20, q.front());
}

TEST(Queue, removed_elements_frees_space_for_new_elements) {
    Queue<int> q(3);
    q.push(10);
    q.push(20);
    q.push(30);
    q.pop();
    EXPECT_FALSE(q.is_full());
    EXPECT_NO_THROW(q.push(40));
    EXPECT_TRUE(q.is_full());
    for (int expected = 20; expected <= 40; expected += 10) {
        ASSERT_EQ(expected, q.front());
        q.pop();
    }
}

TEST(Queue, new_input_does_not_jump_ahead_of_existing_output) {
    Queue<int> q(5);
    q.push(1);
    q.push(2);
    ASSERT_EQ(1, q.front());
    q.push(3);
    q.pop();
    q.push(4);
    for (int expected = 2; expected <= 4; ++expected) {
        ASSERT_EQ(expected, q.front());
        q.pop();
    }
}

TEST(Queue, can_pop_without_calling_front_first) {
    Queue<int> q;
    q.push(1);
    q.push(2);
    q.pop();
    EXPECT_EQ(2, q.front());
}

TEST(Queue, clear_removes_elements_from_both_stacks) {
    Queue<int> q(3);
    q.push(1);
    q.push(2);
    q.front();
    q.push(3);
    q.clear();
    EXPECT_EQ(0, q.size());
    EXPECT_TRUE(q.is_empty());
    EXPECT_FALSE(q.is_full());
    EXPECT_EQ(3, q.get_capacity());
    q.push(4);
    EXPECT_EQ(4, q.front());
}

TEST(Queue, clear_on_empty_queue_is_allowed) {
    Queue<int> q;
    EXPECT_NO_THROW(q.clear());
}

TEST(Queue, capacity_one_supports_many_reuse_cycles) {
    Queue<int> q(1);
    for (int i = 0; i < 1000; ++i) {
        EXPECT_NO_THROW(q.push(i));
        ASSERT_TRUE(q.is_full());
        ASSERT_EQ(i, q.front());
        q.pop();
        ASSERT_TRUE(q.is_empty());
    }
}

TEST(Queue, copy_saves_both_stacks_and_is_independent) {
    Queue<int> q(5);
    q.push(1);
    q.push(2);
    q.front();
    q.push(3);
    Queue<int> copy(q);
    q.pop();
    for (int expected = 1; expected <= 3; ++expected) {
        ASSERT_EQ(expected, copy.front());
        copy.pop();
    }
    EXPECT_EQ(2, q.front());
}

TEST(Queue, assignment_replaces_contents_and_capacity) {
    Queue<int> source(3), destination(7);
    source.push(10);
    source.push(20);
    source.front();
    source.push(30);
    destination.push(99);
    destination = source;
    EXPECT_EQ(3, destination.get_capacity());
    for (int expected = 10; expected <= 30; expected += 10) {
        ASSERT_EQ(expected, destination.front());
        destination.pop();
    }
    EXPECT_EQ(3, source.size());
}

TEST(Queue, supports_strings) {
    Queue<std::string> q(2);
    q.push("first");
    q.push("second");
    EXPECT_EQ("first", q.front());
    q.pop();
    EXPECT_EQ("second", q.front());
}