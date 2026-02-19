#ifndef INCLUDE_URI_URI_H
#define INCLUDE_URI_URI_H

#include <string_view>
#include <vector>
#include <variant>
#include <string>
#include <ostream>
#include <format>

namespace uri
{
  template <typename StringT>
  class uri_base
  {
  public:
    // Getters.
    std::string_view scheme()   const noexcept { return scheme_; }
    std::string_view userinfo() const noexcept { return userinfo_; }
    std::string_view host()     const noexcept { return host_; }
    std::string_view port()     const noexcept { return port_; }
    std::string_view path()     const noexcept { return path_; }
    std::string_view query()    const noexcept { return query_; }
    std::string_view fragment() const noexcept { return fragment_; }

    // Setters
    void scheme(StringT v)   { scheme_ = std::move(v); }
    void userinfo(StringT v) { userinfo_ = std::move(v); }
    void host(StringT v)     { host_ = std::move(v); }
    void port(StringT v)     { port_ = std::move(v); }
    void path(StringT v)     { path_ = std::move(v); }
    void query(StringT v)    { query_ = std::move(v); }
    void fragment(StringT v) { fragment_ = std::move(v); }

    friend std::ostream& operator<<(std::ostream& os, const uri_base& u)
    {
      return os << std::format("{}", u);
    }

  private:
    StringT scheme_;
    StringT userinfo_;
    StringT host_;
    StringT port_;
    StringT path_;
    StringT query_;
    StringT fragment_;
  };

  using uri = uri_base<std::string>;
  using uri_view = uri_base<std::string_view>;

  template <typename StringT = std::string>
  class uri_builder
  {
  public:
    uri_builder& scheme(StringT v)   { uri_.scheme(std::move(v)); return *this; }
    uri_builder& userinfo(StringT v) { uri_.userinfo(std::move(v)); return *this; }
    uri_builder& host(StringT v)     { uri_.host(std::move(v)); return *this; }
    uri_builder& port(StringT v)     { uri_.port(std::move(v)); return *this; }
    uri_builder& path(StringT v)     { uri_.path(std::move(v)); return *this; }
    uri_builder& query(StringT v)    { uri_.query(std::move(v)); return *this; }
    uri_builder& fragment(StringT v) { uri_.fragment(std::move(v)); return *this; }

    uri_base<StringT> build() const { return uri_; }

  private:
    uri_base<StringT> uri_;
  };

  struct query_param_view
  {
    std::string_view key;
    std::string_view value;
  };

  enum class error_code
  {
    invalid_scheme,
    invalid_authority,
    invalid_path,
    invalid_query,
    invalid_fragment
  };

  struct parse_error
  {
    error_code code;
    std::string_view message;
  };

  template <typename T>
  using result = std::variant<T, parse_error>;

  /**
   * @brief Parses a URI string into its primary components.
   *
   * Returns a result containing either the parsed uri_base or a parse_error.
   */
  result<uri_view> parse(std::string_view uri_string);

  /**
   * @brief Deep parses a query string into a collection of key-value pairs.
   */
  std::vector<query_param_view> parse_query(std::string_view query_string);
}

namespace std
{
  template <typename StringT>
  struct formatter<uri::uri_base<StringT>>
  {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    auto format(const uri::uri_base<StringT>& u, format_context& ctx) const
    {
      auto out = ctx.out();
      if (!u.scheme().empty()) {
        out = std::format_to(out, "{}:", u.scheme());
      }
      if (!u.host().empty() || !u.userinfo().empty() || !u.port().empty())
      {
        out = std::format_to(out, "//");
        if (!u.userinfo().empty()) {
          out = std::format_to(out, "{}@", u.userinfo());
        }
        out = std::format_to(out, "{}", u.host());
        if (!u.port().empty()) {
          out = std::format_to(out, ":{}", u.port());
        }
      }
      out = std::format_to(out, "{}", u.path());
      if (!u.query().empty()) {
        out = std::format_to(out, "?{}", u.query());
      }
      if (!u.fragment().empty()) {
        out = std::format_to(out, "#{}", u.fragment());
      }
      return out;
    }
  };
}

#endif // INCLUDE_URI_URI_H