export module scripter;

import jojo;
import jute;

namespace scripter {
  export jute::view next();
}

module :private;

static bool is_spc(char c) {
  if (c == 0) return true;
  if (c == ' ') return true;
  if (c == '\n') return true;
  if (c == '\r') return true;
  if (c == '\t') return true;
  return false;
}

jute::view scripter::next() {
  static auto txt = jojo::read_cstr("out/script.txt");
  static auto * c = txt.begin();
  while (*c) {
    while (is_spc(*c)) c++;

    auto * b = c;
    while (!is_spc(*c)) c++;

    unsigned len = c - b;
    return jute::view { b, len };
  }
  return {};
}
