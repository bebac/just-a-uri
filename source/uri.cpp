#include "uri.h"

#include <string_view>
#include <algorithm>
#include <vector>
#include <cctype>

namespace uri
{
  namespace
  {
    struct authority_view
    {
      std::string_view userinfo;
      std::string_view host;
      std::string_view port;
    };

    authority_view parse_authority(std::string_view authority_string)
    {
      authority_view result;

      const auto userinfo_end = authority_string.find('@');
      if (userinfo_end != std::string_view::npos)
      {
        result.userinfo = authority_string.substr(0, userinfo_end);
        authority_string.remove_prefix(userinfo_end + 1);
      }

      if (!authority_string.empty() && authority_string[0] == '[')
      {
        const auto host_end = authority_string.find(']');
        if (host_end != std::string_view::npos)
        {
          result.host = authority_string.substr(0, host_end + 1);
          authority_string.remove_prefix(host_end + 1);
          if (!authority_string.empty() && authority_string[0] == ':')
          {
            result.port = authority_string.substr(1);
          }
        }
      }
      else
      {
        const auto port_start = authority_string.find(':');
        if (port_start != std::string_view::npos)
        {
          result.host = authority_string.substr(0, port_start);
          result.port = authority_string.substr(port_start + 1);
        }
        else
        {
          result.host = authority_string;
        }
      }

      return result;
    }

    // Returns the value of a hexadecimal digit, or -1 if c is not a
    // hex digit. Case-insensitive per RFC 3986 HEXDIGIT.
    int hex_value(char c)
    {
      if (c >= '0' && c <= '9')
        return c - '0';
      if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
      if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
      return -1;
    }
  }

  std::vector<query_param_view> parse_query(std::string_view query_string)
  {
    std::vector<query_param_view> result;
    while (!query_string.empty())
    {
      const auto pair_end = query_string.find('&');
      const auto pair = query_string.substr(0, pair_end);

      const auto eq_pos = pair.find('=');
      if (eq_pos != std::string_view::npos)
      {
        result.push_back({pair.substr(0, eq_pos), pair.substr(eq_pos + 1)});
      }
      else
      {
        result.push_back({pair, ""});
      }

      if (pair_end == std::string_view::npos)
        break;
      query_string.remove_prefix(pair_end + 1);
    }
    return result;
  }

  result<std::string> percent_decode(std::string_view input)
  {
    std::string decoded;
    decoded.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i)
    {
      if (input[i] != '%')
      {
        decoded.push_back(input[i]);
        continue;
      }

      if (i + 2 >= input.size())
      {
        return parse_error{error_code::invalid_percent_encoding, "Incomplete percent-encoding"};
      }

      const int high = hex_value(input[i + 1]);
      const int low = hex_value(input[i + 2]);
      if (high < 0 || low < 0)
      {
        return parse_error{error_code::invalid_percent_encoding, "Invalid hex digit in percent-encoding"};
      }

      decoded.push_back(static_cast<char>(high * 16 + low));
      i += 2;
    }

    return decoded;
  }

  result<uri_view> parse(std::string_view uri_string)
  {
    uri_view result_uri;

    // Scheme
    const auto scheme_end = uri_string.find(':');
    if (scheme_end != std::string_view::npos)
    {
      const auto first_slash = uri_string.find('/');
      if (first_slash == std::string_view::npos || scheme_end < first_slash)
      {
        std::string_view potential_scheme = uri_string.substr(0, scheme_end);

        auto is_valid_scheme = [&]()
        {
          if (potential_scheme.empty() || !std::isalpha(static_cast<unsigned char>(potential_scheme[0])))
            return false;
          for (size_t i = 1; i < potential_scheme.size(); ++i)
          {
            char c = potential_scheme[i];
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' && c != '-' && c != '.')
              return false;
          }
          return true;
        };

        if (!is_valid_scheme())
        {
          return parse_error{error_code::invalid_scheme, "Invalid scheme syntax"};
        }

        result_uri.scheme(potential_scheme);
        uri_string.remove_prefix(scheme_end + 1);
      }
    }

    // Authority
    if (uri_string.rfind("//", 0) == 0)
    {
      uri_string.remove_prefix(2);
      const auto authority_end = uri_string.find_first_of("/?#");
      std::string_view potential_authority = uri_string.substr(0, authority_end);

      if (potential_authority.find(' ') != std::string_view::npos)
      {
        return parse_error{error_code::invalid_authority, "Authority contains invalid characters"};
      }

      auto auth = parse_authority(potential_authority);
      result_uri.userinfo(auth.userinfo);
      result_uri.host(auth.host);
      result_uri.port(auth.port);

      if (authority_end != std::string_view::npos)
        uri_string.remove_prefix(authority_end);
      else
        uri_string.remove_prefix(uri_string.size());
    }

    // Path
    const auto path_end = uri_string.find_first_of("?#");
    result_uri.path(uri_string.substr(0, path_end));
    if (path_end != std::string_view::npos)
      uri_string.remove_prefix(path_end);
    else
      uri_string.remove_prefix(uri_string.size());

    // Query
    if (!uri_string.empty() && uri_string[0] == '?')
    {
      uri_string.remove_prefix(1);
      const auto query_end = uri_string.find('#');
      result_uri.query(uri_string.substr(0, query_end));

      if (query_end != std::string_view::npos)
        uri_string.remove_prefix(query_end);
      else
        uri_string.remove_prefix(uri_string.size());
    }

    // Fragment
    if (!uri_string.empty() && uri_string[0] == '#')
    {
      uri_string.remove_prefix(1);
      result_uri.fragment(uri_string);
    }

    return result_uri;
  }
}
