#include "translator.h"
#include <gtest.h>


namespace {
    class InspectableTranslator: public Translator {
        public:
        explicit InspectableTranslator(const std::string& expr) : Translator(expr) {}

        size_t term_count() const {return terms.size();}
        size_t polish_notation_count() const {return polish_notation.size();}
        Term *term_at(size_t index) const {return terms[index];}
        Term *polish_notation_at(size_t index) const {return polish_notation[index];}
        void change_expression(const std::string& expr) {expression = expr;}

        void use_terms_as_polish_notation() {polish_notation = terms;}

        std::string polish_notation_to_text() const {
            std::ostringstream out;

            for (size_t i = 0; i < polish_notation.size(); ++i) {
                if (i != 0)
                    out << ' ';

                if (polish_notation[i]->get_type() == types::number)
                    out << static_cast<const Number*>(polish_notation[i])->get_value();
                else if (polish_notation[i]->get_type() == types::operation)
                    out << static_cast<const Operation*>(polish_notation[i])->get_operation();
                else
                    out << "BRACKET";
            }

            return out.str();
        }
    };
};

double evaluate(const std::string& exprt) {
    Translator translator(exprt);
    return translator.calculate();
}

TEST(Translator, stores_expression_without_parsing) {
    InspectableTranslator t("2+3");
    EXPECT_EQ(0, t.term_count());
    EXPECT_EQ(0, t.polish_notation_count());
}

TEST(Translator, parses_expression_into_correct_terms) {
    InspectableTranslator t("12.5+(2*3)");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_EQ(7, t.term_count());
    EXPECT_EQ(types::number, t.term_at(0)->get_type());
    EXPECT_EQ(12.5, static_cast<Number*>(t.term_at(0))->get_value());
    ASSERT_EQ(types::operation, t.term_at(1)->get_type());
    EXPECT_EQ('+', static_cast<Operation*>(t.term_at(1))->get_operation());
    EXPECT_EQ(types::open_bracket, t.term_at(2)->get_type());
    ASSERT_EQ(types::number, t.term_at(3)->get_type());
    EXPECT_DOUBLE_EQ(2, static_cast<Number*>(t.term_at(3))->get_value());
    EXPECT_EQ(types::operation, t.term_at(4)->get_type());
    EXPECT_EQ(types::number, t.term_at(5)->get_type());
    EXPECT_EQ(types::close_bracket, t.term_at(6)->get_type());
}

TEST(Translator, adds_number_at_end_of_expression) {
    InspectableTranslator t("2+123");
    t.expression_to_terms();
    EXPECT_EQ(3, t.term_count());
    EXPECT_EQ(types::number, t.term_at(2)->get_type());
    EXPECT_DOUBLE_EQ(123, static_cast<Number*>(t.term_at(2))->get_value());
}

TEST(Translator, calculate_can_be_called_immediately_after_construction) {
    Translator t("2+3");
    EXPECT_DOUBLE_EQ(5, t.calculate());
}

TEST(Translator, accepts_dot_and_comma_decimal_separators) {
    EXPECT_DOUBLE_EQ(1.75, evaluate("0,5+1.25"));
    EXPECT_DOUBLE_EQ(0.125, evaluate("0,125"));
}

TEST(Translator, ignores_whitespace_between_tokens) {
    EXPECT_DOUBLE_EQ(14, evaluate(" \t2 +\n3 * 4\r "));
}

TEST(Translator, whitespace_does_not_join_separate_numbers) {
    InspectableTranslator t("12 34");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_EQ(2, t.term_count());
    EXPECT_DOUBLE_EQ(12, static_cast<Number*>(t.term_at(0))->get_value());
    EXPECT_DOUBLE_EQ(34, static_cast<Number*>(t.term_at(1))->get_value());
    EXPECT_THROW(t.syntax_check(), std::invalid_argument);
}

TEST(Translator, rejects_unknown_characters) {
    const char* expressions[] = {"a+2", "2^3", "2&3", "1e3", "2;3"};
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_THROW(t.expression_to_terms(), std::invalid_argument);
    }
}

TEST(Translator, rejects_numbers_with_invalid_decimal_separators) {
    const char* expressions[] =
        {".5", ",5", ".", ",", "1.", "1,", "1. ", "1, ",
        "1.+2", "(1.)", "1..2", "1,,2", "1.2.3", "1,2,3", "1.2,3", "1,2.3"
    };
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_THROW(t.expression_to_terms(), std::invalid_argument);
    }
}

TEST(Translator, rejects_number_that_does_not_fit_double) {
    Translator t(std::string(400, '9'));
    EXPECT_ANY_THROW(t.expression_to_terms());
}

TEST(Translator, parsing_error_clears_partial_result) {
    InspectableTranslator t("12+3+a");
    EXPECT_THROW(t.expression_to_terms(), std::invalid_argument);
    EXPECT_EQ(0, t.term_count());
    EXPECT_EQ(0, t.polish_notation_count());
}

TEST(Translator, two_operations_in_a_row_calls_throw) {
    Translator t("2++3");
    ASSERT_NO_THROW(t.expression_to_terms());
    EXPECT_THROW(t.syntax_check(), std::invalid_argument);
}

TEST(Translator, recognizes_valid_bracket_sequences) {
    const char* expressions[] = {"2+3", "()", "[]", "{}", "((2))", "{2+[3*(4-1)]}"};
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_EQ(0, t.is_bracket_correct());
    }
}

TEST(Translator, rejects_mismatched_missing_and_crossing_brackets) {
    const char* expressions[] = {")", "]", "}", "(", "[", "{", "(2+3]", "[2+3}", "([2)]", "{[2)}", "2+3)", "((2)", ")("};
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_EQ(1, t.is_bracket_correct());
    }
}

TEST(Translator, accepts_valid_syntax) {
    const char* expressions[] = {"42", "2+3", "((2+3))", "{2+[3*4]}", "(0-5)*2", "0.5/2"};
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_NO_THROW(t.expression_to_terms());
        EXPECT_NO_THROW(t.syntax_check());
    }
}

TEST(Translator, rejects_empty_expression_and_empty_brackets) {
    const char* expressions[] = {"", " \t\n", "()", "[]", "{}", "( )", "2+()", "(())"};
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_NO_THROW(t.expression_to_terms());
        EXPECT_THROW(t.syntax_check(), std::invalid_argument);
    }
}

TEST(Translator, rejects_missing_operators_or_operands) {
    const char* expressions[] = {
        "2 3", "2(3)", "(2)3", "(2)(3)", "2++3", "2**3", "2//3", "2+-3", "2+", "2*",
        "+2", "-2", "*2", "2*(-3)", "(2+)", "(+2)"
    };
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        Translator t(expressions[i]);
        EXPECT_NO_THROW(t.expression_to_terms());
        EXPECT_THROW(t.syntax_check(), std::invalid_argument);
    }
}

TEST(Translator, syntax_check_also_checks_original_bracket_types) {
    Translator t("(2+3]");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_THROW(t.syntax_check(), std::invalid_argument);
}

TEST(Translator, creates_expected_polish_notation) {
    const char* expressions[] = {"2+3*4", "(2+3)*4", "16/4/2", "10-5-2", "2+3*4/2", "{2+[3*4]}", "42"};
    const char* expected[] = {"2 3 4 * +", "2 3 + 4 *", "16 4 / 2 /", "10 5 - 2 -", "2 3 4 * 2 / +", "2 3 4 * +", "42"};
    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); ++i) {
        SCOPED_TRACE(expressions[i]);
        InspectableTranslator t(expressions[i]);
        EXPECT_NO_THROW(t.expression_to_terms());
        EXPECT_NO_THROW(t.to_polish_notation());
        EXPECT_EQ(expected[i], t.polish_notation_to_text());
    }
}

TEST(Translator, postfix_contains_existing_term_pointers_without_brackets) {
    InspectableTranslator t("(2+3)*4");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_NO_THROW(t.to_polish_notation());
    ASSERT_EQ(5, t.polish_notation_count());
    for (size_t i = 0; i < t.polish_notation_count(); ++i) {
        Term* p = t.polish_notation_at(i);
        EXPECT_NE(types::open_bracket, p->get_type());
        EXPECT_NE(types::close_bracket, p->get_type());
        bool found = false;
        for (size_t j = 0; j < t.term_count(); ++j)
            if (p == t.term_at(j))
                found = true;
        EXPECT_TRUE(found);
    }
}

TEST(Translator, conversion_runs_syntax_check) {
    Translator t("2++3");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_THROW(t.to_polish_notation(), std::invalid_argument);
}

TEST(Translator, repeated_conversion_does_not_append_old_polish_notation) {
    InspectableTranslator t("2+3*4");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_NO_THROW(t.to_polish_notation());
    EXPECT_NO_THROW(t.to_polish_notation());
    EXPECT_EQ(5, t.polish_notation_count());
    EXPECT_EQ("2 3 4 * +", t.polish_notation_to_text());
    EXPECT_DOUBLE_EQ(14, t.calculate());
}

TEST(Translator, repeated_parsing_clears_old_polish_notation) {
    InspectableTranslator t("2+3*4");
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_NO_THROW(t.to_polish_notation());
    EXPECT_NO_THROW(t.expression_to_terms());
    EXPECT_EQ(5, t.term_count());
    EXPECT_EQ(0, t.polish_notation_count());
    EXPECT_NO_THROW(t.to_polish_notation());
    EXPECT_DOUBLE_EQ(14, t.calculate());
}

TEST(Translator, calculates_all_four_operations) {
    EXPECT_DOUBLE_EQ(10, evaluate("8+2"));
    EXPECT_DOUBLE_EQ(6, evaluate("8-2"));
    EXPECT_DOUBLE_EQ(16, evaluate("8*2"));
    EXPECT_DOUBLE_EQ(4, evaluate("8/2"));
}

TEST(Translator, subtraction_and_division_preserve_operand_order) {
    EXPECT_DOUBLE_EQ(-6, evaluate("2-8"));
    EXPECT_DOUBLE_EQ(0.25, evaluate("2/8"));
}

TEST(Translator, respects_priority_and_left_associativity) {
    EXPECT_DOUBLE_EQ(14, evaluate("2+3*4"));
    EXPECT_DOUBLE_EQ(20, evaluate("(2+3)*4"));
    EXPECT_DOUBLE_EQ(2, evaluate("16/4/2"));
    EXPECT_DOUBLE_EQ(3, evaluate("10-5-2"));
    EXPECT_DOUBLE_EQ(4, evaluate("8/4*2"));
    EXPECT_DOUBLE_EQ(8, evaluate("2+3*4/2"));
}

TEST(Translator, supports_nested_brackets_and_negative_results) {
    EXPECT_DOUBLE_EQ(11, evaluate("{2+[3*(4-1)]}"));
    EXPECT_DOUBLE_EQ(5, evaluate("(((2+3)))"));
    EXPECT_DOUBLE_EQ(-10, evaluate("(0-5)*2"));
    EXPECT_DOUBLE_EQ(6, evaluate("8-(3-1)"));
}

TEST(Translator, calculates_single_number_zero_and_fraction) {
    EXPECT_DOUBLE_EQ(42, evaluate("42"));
    EXPECT_DOUBLE_EQ(0, evaluate("0"));
    EXPECT_DOUBLE_EQ(0.125, evaluate("1/8"));
    EXPECT_DOUBLE_EQ(0.3, evaluate("0.1+0.2"));
}

TEST(Translator, rejects_division_by_zero_including_computed_zero) {
    EXPECT_ANY_THROW(evaluate("1/0"));
    EXPECT_ANY_THROW(evaluate("1/(2-2)"));
}

TEST(Translator, allows_small_nonzero_divison_within_the_margin_of_error) {
    EXPECT_DOUBLE_EQ(1, evaluate("0.000001/0.000001"));
    EXPECT_DOUBLE_EQ(1, evaluate("0.000001/(0.000002-0.000001)"));
}

TEST(Translator, rejects_arithmetic_overflow) {
    const std::string large(200, '9');
    EXPECT_THROW(evaluate(large + "*" + large), std::overflow_error);
}

TEST(Translator, can_recover_after_parsing_and_arithmetic_errors) {
    // например чтобы не останавливать работу программы калькулятора, при неверно введенном выражении, а ошибку выводить текстом
    InspectableTranslator t("2+a");
    EXPECT_THROW(t.expression_to_terms(), std::invalid_argument);
    t.change_expression("1/0");
    t.expression_to_terms();
    t.to_polish_notation();
    EXPECT_ANY_THROW(t.calculate());
    t.change_expression("2+3");
    t.expression_to_terms();
    t.to_polish_notation();
    EXPECT_DOUBLE_EQ(5, t.calculate());
}

TEST(Translator, can_calculate_long_expression) {
    std::string expression = "1";
    for (int i = 0; i < 2000; ++i)
        expression += "+1";
    EXPECT_DOUBLE_EQ(2001, evaluate(expression));
}

TEST(Translator, handles_lots_of_brackets) {
    EXPECT_DOUBLE_EQ(5, evaluate(std::string(300, '(') + "2+3" + std::string(300, ')')));
}

TEST(Translator, generated_expressions_match_independent_cpp_calculations) {
    for (int a = 1; a <= 10; ++a) {
        for (int b = 1; b <= 7; ++b) {
            for (int c = 1; c <= 4; ++c) {
                const std::string x = std::to_string(a);
                const std::string y = std::to_string(b);
                const std::string z = std::to_string(c);
                SCOPED_TRACE(x + ", " + y + ", " + z);
                EXPECT_DOUBLE_EQ(a + b * c, evaluate(x + "+" + y + "*" + z));
                EXPECT_DOUBLE_EQ(static_cast<double>(a - b) / c, evaluate("(" + x + "-" + y + ")/" + z));
                EXPECT_DOUBLE_EQ(static_cast<double>(a) / b * c, evaluate(x + "/" + y + "*" + z));
            }
        }
    }
}

TEST(Translator, calculate_runs_all_syntax_and_bracket_checks) {
    const char* invalid[] = {
        "", " ", "()", "2++3", "2 3", "2(3)", "(2)(3)",
        "2+", "-2", "(2+3]", "([2)]", ")2(", "(2+3"
    };
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        SCOPED_TRACE(invalid[i]);
        Translator t(invalid[i]);
        EXPECT_THROW(t.calculate(), std::invalid_argument);
    }
}

TEST(Translator, calculate_runs_lexical_check) {
    const char* invalid[] = {"a+2", "2^3", ".5", "1.", "1.2.3", "1,2,3"};
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        SCOPED_TRACE(invalid[i]);
        Translator t(invalid[i]);
        EXPECT_THROW(t.calculate(), std::invalid_argument);
    }
}

TEST(Translator, calculate_builds_terms_and_polish_notation) {
    InspectableTranslator t("(2+3)*4");
    EXPECT_EQ(0, t.term_count());
    EXPECT_EQ(0, t.polish_notation_count());
    EXPECT_DOUBLE_EQ(20, t.calculate());
    EXPECT_EQ(7, t.term_count());
    EXPECT_EQ(5, t.polish_notation_count());
    EXPECT_EQ("2 3 + 4 *", t.polish_notation_to_text());
}

TEST(Translator, calculate_uses_current_expression_on_every_call) {
    InspectableTranslator t("2+3");
    EXPECT_DOUBLE_EQ(5, t.calculate());
    t.change_expression("{8-[3*2]}/2");
    EXPECT_DOUBLE_EQ(1, t.calculate());
    EXPECT_EQ("8 3 2 * - 2 /", t.polish_notation_to_text());
}

TEST(Translator, calculate_replaces_existing_incorrect_postfix) {
    InspectableTranslator t("2+3");
    t.expression_to_terms();
    t.use_terms_as_polish_notation(); // Получилось 2 + 3.
    EXPECT_DOUBLE_EQ(5, t.calculate());
    EXPECT_EQ("2 3 +", t.polish_notation_to_text());
}

TEST(Translator, repeated_calculate_does_not_accumulate_terms_or_postfix) {
    InspectableTranslator t("(2+3)*4");
    for (int i = 0; i < 100; ++i) {
        ASSERT_DOUBLE_EQ(20, t.calculate());
        ASSERT_EQ(7, t.term_count());
        ASSERT_EQ(5, t.polish_notation_count());
    }
}

TEST(Translator, repeated_arithmetic_error_does_not_prevent_reuse) {
    InspectableTranslator t("1/(2-2)");
    EXPECT_ANY_THROW(t.calculate());
    EXPECT_ANY_THROW(t.calculate());
    t.change_expression("6/2");
    EXPECT_DOUBLE_EQ(3, t.calculate());
}

TEST(Translator, full_calculation_can_recover_from_syntax_error) {
    InspectableTranslator t("2++3");
    EXPECT_THROW(t.calculate(), std::invalid_argument);
    t.change_expression("7*(4-2)");
    EXPECT_DOUBLE_EQ(14, t.calculate());
}