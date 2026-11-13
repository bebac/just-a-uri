#include "doctest.h"
#include "uri.h"

#include <span>
#include <string>
#include <variant>
#include <vector>
#include <sstream>
#include <format>

TEST_CASE("URI data structure can store different string types")
{
  SUBCASE("std::string_view")
  {
    uri::uri_view u;
    u.scheme("https");
    CHECK(u.scheme() == "https");
  }

  SUBCASE("std::string")
  {
    uri::uri u;
    u.scheme("https");
    CHECK(u.scheme() == "https");
  }
}

TEST_CASE("Primary URI parsing")
{
  SUBCASE("Full URI")
  {
    auto res = uri::parse("https://user:pass@www.example.com:8080/path?a=1&b=2#frag");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.scheme() == "https");
    CHECK(u.userinfo() == "user:pass");
    CHECK(u.host() == "www.example.com");
    CHECK(u.port() == "8080");
    CHECK(u.path() == "/path");
    CHECK(u.query() == "a=1&b=2");
    CHECK(u.fragment() == "frag");
  }

  SUBCASE("No authority")
  {
    auto res = uri::parse("mailto:user@example.com");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.scheme() == "mailto");
    CHECK(u.host() == "");
    CHECK(u.path() == "user@example.com");
  }

  SUBCASE("No query or fragment")
  {
    auto res = uri::parse("https://www.example.com/path");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.scheme() == "https");
    CHECK(u.host() == "www.example.com");
    CHECK(u.path() == "/path");
    CHECK(u.query() == "");
    CHECK(u.fragment() == "");
  }

  SUBCASE("Empty path")
  {
    auto res = uri::parse("https://www.example.com");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.host() == "www.example.com");
    CHECK(u.path() == "");
  }
}

TEST_CASE("Invalid URI syntax and error reporting")
{
  SUBCASE("Invalid characters in scheme")
  {
    auto res = uri::parse("sh@eme://example.com");
    REQUIRE(std::holds_alternative<uri::parse_error>(res));
    auto &err = std::get<uri::parse_error>(res);
    CHECK(err.code == uri::error_code::invalid_scheme);
    CHECK(err.message == "Invalid scheme syntax");
  }

  SUBCASE("Invalid authority (space)")
  {
    auto res = uri::parse("https://www.ex ample.com/path");
    REQUIRE(std::holds_alternative<uri::parse_error>(res));
    auto &err = std::get<uri::parse_error>(res);
    CHECK(err.code == uri::error_code::invalid_authority);
    CHECK(err.message == "Authority contains invalid characters");
  }
}

TEST_CASE("Query deep parsing")
{
  SUBCASE("Multiple parameters")
  {
    auto params = uri::parse_query("a=1&b=2&c");
    CHECK(params.size() == 3);
    CHECK(params[0].key == "a");
    CHECK(params[0].value == "1");
    CHECK(params[1].key == "b");
    CHECK(params[1].value == "2");
    CHECK(params[2].key == "c");
    CHECK(params[2].value == "");
  }

  SUBCASE("Empty query string")
  {
    auto params = uri::parse_query("");
    CHECK(params.empty());
  }
}

TEST_CASE("Fragment parsing")
{
  SUBCASE("Simple")
  {
    auto res = uri::parse("https://example.com#section1");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.fragment() == "section1");
  }

  SUBCASE("Empty fragment")
  {
    auto res = uri::parse("https://example.com#");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.fragment() == "");
  }
}

TEST_CASE("Relative URI references and empty strings")
{
  SUBCASE("Empty string")
  {
    auto res = uri::parse("");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.scheme() == "");
    CHECK(u.host() == "");
    CHECK(u.path() == "");
  }

  SUBCASE("Relative path only")
  {
    auto res = uri::parse("path/to/resource");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.path() == "path/to/resource");
  }

  SUBCASE("Rootless path with query")
  {
    auto res = uri::parse("path?query");
    REQUIRE(std::holds_alternative<uri::uri_view>(res));
    auto &u = std::get<uri::uri_view>(res);
    CHECK(u.path() == "path");
    CHECK(u.query() == "query");
  }
}

TEST_CASE("uri_builder")
{
  auto u = uri::uri_builder<std::string>{}
    .scheme("https")
    .userinfo("user:pass")
    .host("example.com")
    .port("8080")
    .path("/path")
    .query("a=1")
    .fragment("frag")
    .build();

  CHECK(u.scheme() == "https");
  CHECK(u.userinfo() == "user:pass");
  CHECK(u.host() == "example.com");
  CHECK(u.port() == "8080");
  CHECK(u.path() == "/path");
  CHECK(u.query() == "a=1");
  CHECK(u.fragment() == "frag");
}

TEST_CASE("Formatting and Output")
{
  auto u = uri::uri_builder{}
    .scheme("https")
    .host("example.com")
    .path("/test")
    .build();

  SUBCASE("std::ostream")
  {
    std::stringstream ss;
    ss << u;
    CHECK(ss.str() == "https://example.com/test");
  }

  SUBCASE("std::format")
  {
    std::string s = std::format("{}", u);
    CHECK(s == "https://example.com/test");
  }

  SUBCASE("Full URI formatting")
  {
    auto full = uri::uri_builder{}
      .scheme("http")
      .userinfo("admin")
      .host("localhost")
      .port("3000")
      .path("/api")
      .query("v=1")
      .fragment("top")
      .build();

    CHECK(std::format("{}", full) == "http://admin@localhost:3000/api?v=1#top");
  }

  SUBCASE("uri_view formatting")
  {
    std::string s = "https://example.com";
    auto res = uri::parse(s);

    auto& u = std::get<uri::uri_view>(res);
    CHECK(std::format("{}", u) == "https://example.com");

    std::stringstream ss;
    ss << u;
    CHECK(ss.str() == "https://example.com");
  }
}

TEST_CASE("Percent decoding")
{
  SUBCASE("Decodes pct-encoded triplets, hex digits case-insensitive")
  {
    auto res = uri::percent_decode("%2f%2F");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "//");
  }

  SUBCASE("Leaves unreserved characters unchanged")
  {
    auto res = uri::percent_decode("abc-._~");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "abc-._~");
  }

  SUBCASE("Decodes pct-encoded unreserved characters to the bare character")
  {
    auto res = uri::percent_decode("~%7E%7E");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "~~~");
  }

  SUBCASE("Leaves selected percent-encoded characters intact")
  {
    auto res = uri::percent_decode("%2F%3f%23%20", "/?");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "%2F%3f# ");
  }

  SUBCASE("Leaves plus as a literal plus (not form-encoding)")
  {
    auto res = uri::percent_decode("a+b%20c");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "a+b c");
  }

  SUBCASE("Decodes reserved and multi-byte utf-8 sequences byte-wise")
  {
    auto res = uri::percent_decode("%2F%3F%23%20%C3%A9");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "/?# \xc3\xa9");
  }

  SUBCASE("Empty string decodes to empty string")
  {
    auto res = uri::percent_decode("");
    REQUIRE(std::holds_alternative<std::string>(res));
    CHECK(std::get<std::string>(res) == "");
  }

  SUBCASE("Trailing percent is an error")
  {
    auto res = uri::percent_decode("100%");
    REQUIRE(std::holds_alternative<uri::parse_error>(res));
    auto &err = std::get<uri::parse_error>(res);
    CHECK(err.code == uri::error_code::invalid_percent_encoding);
  }

  SUBCASE("Truncated percent-encoding is an error")
  {
    auto res = uri::percent_decode("%2");
    REQUIRE(std::holds_alternative<uri::parse_error>(res));
    auto &err = std::get<uri::parse_error>(res);
    CHECK(err.code == uri::error_code::invalid_percent_encoding);
    CHECK(err.message == "Incomplete percent-encoding");
  }

  SUBCASE("Invalid hex digits are an error")
  {
    auto res = uri::percent_decode("%ZZ");
    REQUIRE(std::holds_alternative<uri::parse_error>(res));
    auto &err = std::get<uri::parse_error>(res);
    CHECK(err.code == uri::error_code::invalid_percent_encoding);
    CHECK(err.message == "Invalid hex digit in percent-encoding");
  }
}

TEST_CASE("Percent decoding into caller-provided buffer")
{
  char buf[16];

  SUBCASE("Decodes into the buffer and returns the written prefix")
  {
    auto res = uri::percent_decode_into("a%20b%2Fc", buf);
    REQUIRE(uri::value_if(res) != nullptr);
    CHECK(*uri::value_if(res) == "a b/c");
  }

  SUBCASE("A buffer of input size always suffices")
  {
    std::string in = "%41%42%43";
    std::vector<char> exact(in.size());
    auto res = uri::percent_decode_into(in, exact);
    REQUIRE(uri::value_if(res) != nullptr);
    CHECK(*uri::value_if(res) == "ABC");
  }

  SUBCASE("Empty input writes nothing")
  {
    auto res = uri::percent_decode_into("", buf);
    REQUIRE(uri::value_if(res) != nullptr);
    CHECK(uri::value_if(res)->empty());
  }

  SUBCASE("Malformed escape is invalid_percent_encoding")
  {
    auto res = uri::percent_decode_into("%zz", buf);
    REQUIRE(uri::error_if(res) != nullptr);
    CHECK(uri::error_if(res)->code == uri::error_code::invalid_percent_encoding);
  }

  SUBCASE("Truncated escape is invalid_percent_encoding")
  {
    auto res = uri::percent_decode_into("%2", buf);
    REQUIRE(uri::error_if(res) != nullptr);
    CHECK(uri::error_if(res)->code == uri::error_code::invalid_percent_encoding);
  }

  SUBCASE("A too-small buffer is buffer_overflow")
  {
    auto res = uri::percent_decode_into("abcd", std::span<char>(buf, 2));
    REQUIRE(uri::error_if(res) != nullptr);
    CHECK(uri::error_if(res)->code == uri::error_code::buffer_overflow);
  }
}

TEST_CASE("Percent-encoding validity scan")
{
  SUBCASE("Valid inputs")
  {
    CHECK(uri::is_valid_percent_encoding(""));
    CHECK(uri::is_valid_percent_encoding("/entity/7"));
    CHECK(uri::is_valid_percent_encoding("a%20b%2F%2fc"));
    CHECK(uri::is_valid_percent_encoding("%C3%A9"));
  }

  SUBCASE("Malformed inputs")
  {
    CHECK_FALSE(uri::is_valid_percent_encoding("100%"));
    CHECK_FALSE(uri::is_valid_percent_encoding("%2"));
    CHECK_FALSE(uri::is_valid_percent_encoding("%zz"));
    CHECK_FALSE(uri::is_valid_percent_encoding("a%2"));
  }
}

TEST_CASE("Result accessors")
{
  auto ok = uri::percent_decode("abc");
  CHECK(uri::is_ok(ok));
  REQUIRE(uri::value_if(ok) != nullptr);
  CHECK(*uri::value_if(ok) == "abc");
  CHECK(uri::error_if(ok) == nullptr);

  auto bad = uri::percent_decode("%zz");
  CHECK_FALSE(uri::is_ok(bad));
  CHECK(uri::value_if(bad) == nullptr);
  REQUIRE(uri::error_if(bad) != nullptr);
  CHECK(uri::error_if(bad)->code == uri::error_code::invalid_percent_encoding);
}
