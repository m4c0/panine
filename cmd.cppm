#pragma leco add_impl cmd_image
export module cmd;
import jute;
import ots;

namespace cmd {
  constexpr const auto audio_rate = 22050; // defined by Apple's TTS

  export void image(jute::view line);
  export void zoom_out(ots & ots, jute::view line);
}
