#include "catch2/catch_all.hpp"
#include "Database.h"
#include "Parser.h"

std::vector<Column> getTestColumns() {
    std::vector<Column> cols;
    cols.emplace_back("ID", DataType::DOUBLE, true, true); // Indexed, Unique
    cols.back().autoIncrement = true;
    cols.emplace_back("Name", DataType::STRING);
    cols.emplace_back("JoinDate", DataType::DATE);
    return cols;
}

TEST_CASE("Value Comparisons and Date Logic", "[data]") {
    SECTION("Double comparisons with epsilon") {
        Value v1(10.000001);
        Value v2(10.000002);
        CHECK(v1 == v2);
    }

    SECTION("Date chronological comparison (ISO 8601)") {
        Value d1("2024-01-01", DataType::DATE);
        Value d2("2024-05-20", DataType::DATE);
        Value d3("2023-12-31", DataType::DATE);

        CHECK(d1 < d2);
        CHECK(d3 < d1);
        CHECK(d2 > d3);
    }

    SECTION("Cross-type comparison (String vs Date)") {
        Value s1("2024-01-01", DataType::STRING);
        Value d1("2024-01-01", DataType::DATE);
        CHECK(d1 == s1);
    }
}

TEST_CASE("Index and Constraints", "[index]") {
    Index idx(true); // Unique index

    SECTION("Unique constraint violation") {
        idx.insert(Value(1.0), 0);
        CHECK_THROWS_AS(idx.insert(Value(1.0), 1), std::logic_error);
    }

    SECTION("Find in non-unique index") {
        Index nonUnique(false);
        nonUnique.insert(Value("Sofia"), 10);
        nonUnique.insert(Value("Sofia"), 20);

        auto results = nonUnique.find(Value("Sofia"));
        REQUIRE(results.size() == 2);
    }
}

TEST_CASE("Table Row Management", "[table]") {
    Table table("TestTable", getTestColumns());

    SECTION("AutoIncrement logic") {
        Row r1; r1.values = { Value(0.0), Value("User1"), Value("2024-01-01") };
        Row r2; r2.values = { Value(0.0), Value("User2"), Value("2024-01-02") };

        table.insertRow(r1);
        table.insertRow(r2);

        REQUIRE(table.getRows().size() == 2);
        CHECK(table.getRows()[0].values[0].numValue == 1.0);
        CHECK(table.getRows()[1].values[0].numValue == 2.0);
    }

    SECTION("Remove row updates indices correctly") {
        Row r1; r1.values = { Value(10.0), Value("Target"), Value("2024-01-01") };
        table.insertRow(r1);

        table.removeRow(0);
        CHECK(table.getRows().empty());
    }
}

TEST_CASE("Parser Precedence and Expressions", "[parser]") {
    Table table("LogicTest", getTestColumns());

    SECTION("AND vs OR Precedence") {
        auto tokens = Parser::tokenize("ID = 1 OR Name = \"Maria\" AND JoinDate = \"2024-01-01\"");
        size_t pos = 0;
        auto expr = Parser::parseWhereExpression(tokens, pos, table);

        Row r;
        r.values = { Value(1.0), Value("Ivan"), Value("1900-01-01") };
        CHECK(expr->evaluate(r, table) == true);

        r.values = { Value(99.0), Value("Maria"), Value("2024-01-01") };
        CHECK(expr->evaluate(r, table) == true);

        r.values = { Value(99.0), Value("Maria"), Value("2025-01-01") };
        CHECK(expr->evaluate(r, table) == false);
    }
}

TEST_CASE("Database Integrity and Checksum", "[database]") {
    const std::string testDb = "test_integrity.db";

    std::remove(testDb.c_str());

    SECTION("Save and Load with Checksum") {
        {
            Database db(testDb);
            db.createTable("Products", getTestColumns());
        }

        CHECK_NOTHROW(Database(testDb));
    }

    SECTION("Detect Corruption") {
        {
            Database db(testDb);
            db.createTable("Secure", getTestColumns());
        }

        std::fstream file(testDb, std::ios::binary | std::ios::in | std::ios::out);
        file.seekp(15, std::ios::beg);
        file.put(0xFF);
        file.close();

        CHECK_THROWS_WITH(Database(testDb), Catch::Matchers::ContainsSubstring("corrupted or invalid"));
    }
}