#ifndef SNOWFLAKE_TOMLCONFIGPARSER_HPP
#define SNOWFLAKE_TOMLCONFIGPARSER_HPP

#include <map>
#include <string>
#include <boost/variant.hpp>

  /**
    * Load toml configuration file.
    * 
    * @return A map of key value pairs parsed from toml file
    */
  std::map<std::string, boost::variant<std::string, int, bool, double>> load_toml_config();

#endif //SNOWFLAKE_TOMLCONFIGPARSER_HPP
