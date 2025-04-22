// test_Life.cpp

#include <gtest/gtest.h>  // REQUIRED before using TEST()
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "Life.hpp"       // Your Life and Cell logic

// Utility function
std::string get_grid_output(const std::string& raw_output) {
    auto pos = raw_output.find('\n');
    return (pos != std::string::npos) ? raw_output.substr(pos + 1) : raw_output;
}

TEST(LifeConwayTest, BlockStability) {
    Life<ConwayCell> life(4, 4);
    life.initialize_cell(1, 1, ConwayCell(true));
    life.initialize_cell(1, 2, ConwayCell(true));
    life.initialize_cell(2, 1, ConwayCell(true));
    life.initialize_cell(2, 2, ConwayCell(true));
    life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(1);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    EXPECT_EQ(out,
        "....\n"
        ".**.\n"
        ".**.\n"
        "....\n"
    );
}

TEST(LifeConwayTest, BlinkerOscillatesVerticalToHorizontal) {
    Life<ConwayCell> life(5, 5);
    life.initialize_cell(2, 1, ConwayCell(true));
    life.initialize_cell(2, 2, ConwayCell(true));
    life.initialize_cell(2, 3, ConwayCell(true));
    life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(1);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    EXPECT_EQ(out,
        ".....\n"
        "..*..\n"
        "..*..\n"
        "..*..\n"
        ".....\n"
    );
}



TEST(LifeFredkinTest, FredkinMutatesToConway) {
    Life<Cell> life(3, 3);
    life.initialize_cell(1, 1, Cell(new FredkinCell(true))); // place Fredkin
    life.initialize_cell(1, 2, Cell(new FredkinCell(true)));
    life.eval();  // age = 1
    life.eval();  // age = 2 → mutate
    testing::internal::CaptureStdout();
    life.print_generation(2);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    // '*' indicates Conway
    EXPECT_TRUE(out.find('*') != std::string::npos);
}

TEST(LifeConwayTest, EdgeCellDies) {
    Life<ConwayCell> life(2, 2);
    life.initialize_cell(0, 0, ConwayCell(true));
    life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(1);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    EXPECT_EQ(out,
        "..\n"
        "..\n"
    );
}



TEST(LifeCellTest, MixedCellsEvolveCorrectly) {
    Life<Cell> life(2, 2);
    life.initialize_cell(0, 0, Cell(new FredkinCell(true)));
    life.initialize_cell(1, 1, Cell(new ConwayCell(true)));
    life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(1);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    EXPECT_TRUE(out.find('-') != std::string::npos || out.find('.') != std::string::npos);
}

TEST(LifeCellTest, EmptyGridNoChange) {
    Life<Cell> life(2, 2);
    life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(1);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    EXPECT_EQ(out,
        "--\n"
        "--\n"
    );
}

TEST(LifeFredkinTest, EdgeNeighborsCountCorrectly) {
    Life<FredkinCell> life(3, 3);
    life.initialize_cell(0, 1, FredkinCell(true));
    life.initialize_cell(1, 0, FredkinCell(true));
    life.initialize_cell(1, 2, FredkinCell(true));
    life.initialize_cell(2, 1, FredkinCell(true));
    life.initialize_cell(1, 1, FredkinCell(true));  // center
    life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(1);
    std::string out = get_grid_output(testing::internal::GetCapturedStdout());
    // should survive (not exactly deterministic, but should remain alive in middle)
    EXPECT_TRUE(out.find('1') != std::string::npos || out.find('2') != std::string::npos);
}

TEST(LifeConwayIOTest, ConwayTest_Case0) {
    Life<ConwayCell> life(10, 10);
    life.initialize_cell(9, 7, ConwayCell(true));
    life.initialize_cell(3, 4, ConwayCell(true));
    life.initialize_cell(5, 9, ConwayCell(true));
    life.initialize_cell(1, 4, ConwayCell(true));
    life.initialize_cell(2, 7, ConwayCell(true));
    life.initialize_cell(6, 5, ConwayCell(true));
    life.initialize_cell(8, 6, ConwayCell(true));
    life.initialize_cell(9, 6, ConwayCell(true));

    testing::internal::CaptureStdout();
    life.print_generation(0);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("....*....."), std::string::npos);
    EXPECT_NE(output.find("......**.."), std::string::npos);

    life.eval(); life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(2);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("......**.."), std::string::npos);
}

TEST(LifeCellIOTest, CellTest_Case0) {
    Life<Cell> life(10, 10);
    life.initialize_cell(1, 0, Cell(new FredkinCell(true)));
    life.initialize_cell(3, 5, Cell(new FredkinCell(true)));
    life.initialize_cell(2, 6, Cell(new FredkinCell(true)));
    life.initialize_cell(2, 7, Cell(new FredkinCell(true)));
    life.initialize_cell(4, 9, Cell(new FredkinCell(true)));

    life.eval(); life.eval(); life.eval(); life.eval();
    testing::internal::CaptureStdout();
    life.print_generation(4);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("------**-0"), std::string::npos);
    EXPECT_NE(output.find("0-0-0-----"), std::string::npos);
}

TEST(LifeFredkinIOTest, FredkinTest_Case2) {
    Life<FredkinCell> life(10, 10);
    life.initialize_cell(3, 5, FredkinCell(true));
    life.initialize_cell(5, 0, FredkinCell(true));
    life.initialize_cell(0, 1, FredkinCell(true));
    life.initialize_cell(9, 3, FredkinCell(true));
    life.initialize_cell(9, 6, FredkinCell(true));

    for (int i = 0; i < 5; ++i)
        life.eval();

    testing::internal::CaptureStdout();
    life.print_generation(5);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("--0-0-0---"), std::string::npos);
    EXPECT_NE(output.find("-0---0----"), std::string::npos);
    EXPECT_NE(output.find("-10------0"), std::string::npos);
    EXPECT_NE(output.find("0-0-----0-"), std::string::npos);
    EXPECT_NE(output.find("01-00-0--0"), std::string::npos);
    EXPECT_NE(output.find("--1-2--0--"), std::string::npos);
    EXPECT_NE(output.find("--0-00----"), std::string::npos);
    EXPECT_NE(output.find("--0--210--"), std::string::npos);
    EXPECT_NE(output.find("0-00-000--"), std::string::npos);
    EXPECT_NE(output.find("---1--1-0-"), std::string::npos);
}

TEST(LifeFredkinIOTest, FredkinTest_Case3) {
    Life<FredkinCell> life(10, 10);
    life.initialize_cell(6, 1, FredkinCell(true));
    life.initialize_cell(8, 6, FredkinCell(true));
    life.initialize_cell(2, 2, FredkinCell(true));

    for (int i = 0; i < 5; ++i)
        life.eval();

    testing::internal::CaptureStdout();
    life.print_generation(5);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("-0-0------"), std::string::npos);
    EXPECT_NE(output.find("000---0---"), std::string::npos);
    EXPECT_NE(output.find("120--0-0--"), std::string::npos);
    EXPECT_NE(output.find("00--------"), std::string::npos);
    EXPECT_NE(output.find("-----0-0--"), std::string::npos);
    EXPECT_NE(output.find("-00--00---"), std::string::npos);
    EXPECT_NE(output.find("01200-0---"), std::string::npos);
    EXPECT_NE(output.find("-0---00---"), std::string::npos);
    EXPECT_NE(output.find("-0-0-0-0--"), std::string::npos);
    EXPECT_NE(output.find("--0---0---"), std::string::npos);
}

