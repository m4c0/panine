#pragma leco add_impl cmd_image
export module cmd;
import jute;
import ots;
import silog;

namespace cmd {
  constexpr const auto audio_rate = 22050; // defined by Apple's TTS

  constexpr int atoi(jute::view v) {
    int res = 0;
    for (auto c : v) {
      if (c < '0' || c > '9') silog::die("invalid integer");
      res = res * 10 + (c - '0');
    }
    return res;
  }
  constexpr float atof(jute::view v) {
    float res = 0;
    int decimals = -1;
    for (auto c : v) {
      if (c == '.' && decimals == -1) {
        decimals = 0;
        continue;
      } else if (c == '.') silog::die("invalid float");
      if (c < '0' || c > '9') silog::die("invalid float");
  
      res = res * 10 + (c - '0');
      
      if (decimals >= 0) decimals++;
    }
    for (auto i = 0; i < decimals; i++) res /= 10;
    return res;
  }

  export void zoom_out(ots & ots, jute::view line);
}
