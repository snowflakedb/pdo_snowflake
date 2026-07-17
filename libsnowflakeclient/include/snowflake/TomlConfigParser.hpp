#ifndef SNOWFLAKE_TOMLCONFIGPARSER_HPP
#define SNOWFLAKE_TOMLCONFIGPARSER_HPP

#include <map>
#include <string>
#include <boost/variant.hpp>

struct StringVisitor : public boost::static_visitor<std::string>
{
    std::string operator()(std::string val) const { return val; }
    std::string operator()(int val) const { return std::to_string(val); }
    std::string operator()(bool val) const { return val ? "true" : "false"; }
    std::string operator()(double val) const { return std::to_string(val); }
};

  /**
    * Load toml configuration file.
    * 
    * @return A map of key value pairs parsed from toml file
    */
  std::map<std::string, boost::variant<std::string, int, bool, double>> load_toml_config();

  /*
  *  Load TOML configuration and parse it as a DSN string.
  * 
  * @return DSN string if success, empty string otherwise.
  */
  std::string load_toml_config_as_dsn();

#endif //SNOWFLAKE_TOMLCONFIGPARSER_HPP
