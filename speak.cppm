#pragma leco add_impl speak_objc
export module speak;
import hai;
import jojo;
import jute;

extern "C" void speak(void * self, const char * txt, unsigned n);

namespace spk {
  export struct word {
    jute::heap text;
    unsigned offset;
  };
  
  struct buffers {
    hai::varray<float> buffer { 4 * 1024 * 1024 };
    hai::varray<word> words { 16 * 1024 };
  };

  export [[nodiscard]] auto run() {
    buffers b {};

    auto script = jojo::read_cstr("out/script.txt");
    b.words.push_back(word { jute::view { "" }, 0 });
    speak(&b, script.begin(), script.size());
    (b.words.end() - 1)->offset = b.buffer.size();

    return b;
  }
}

extern "C" void speak_callback(spk::buffers * b, float * data, unsigned samples) {
  for (auto i = 0; i < samples; i++) b->buffer.push_back(data[i]);
}
extern "C" void speak_marker_callback(spk::buffers * b, const char * str, unsigned offset) {
  if (str == nullptr) str = "";
  offset /= 4; // 4 bytes per sample
  (b->words.end() - 1)->offset = offset;
  b->words.push_back(spk::word {
    .text { jute::view::unsafe(str) },
    .offset = offset,
  });
}

