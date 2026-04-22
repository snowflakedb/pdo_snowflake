#ifndef SNOWFLAKE_PLATFORMDETECTION_HPP
#define SNOWFLAKE_PLATFORMDETECTION_HPP

#include <string>
#include <vector>
#include <future>

namespace Snowflake::Client::PlatformDetection
{
  static const long timeoutInMs = 200;
  /**
    * fill the picojson object with platforms detected.
    */
  void getDetectedPlatforms(std::vector<std::string>& detectedPlatforms, long timeoutMs = timeoutInMs);

}
#endif //SNOWFLAKE_PLATFORMDETECTION_HPP
