export module tts;
import carnage;
import embrolho;
import jute;
import mtx;
import siaudio;
import silog;

export namespace tts {
  void word(jute::view w);
  bool playing();
}

module :private;

static const char * arpa2sampa(jute::view a) {
  if (a == "AA") return "Q";
  if (a == "AE") return "{";
  if (a == "AH") return "V";
  if (a == "AO") return "O:";
  if (a == "AW") return "aU";
  if (a == "AX") return "@";
  if (a == "AY") return "aI";
  if (a == "B")  return "b";
  if (a == "CH") return "tS";
  if (a == "D")  return "d";
  if (a == "DH") return "D";
  if (a == "DX") return "4";
  if (a == "EH") return "E";
  if (a == "ER") return "3:";
  if (a == "EY") return "eI";
  if (a == "F")  return "f";
  if (a == "G")  return "g";
  if (a == "HH") return "h";
  if (a == "IH") return "I";
  if (a == "IY") return "i:";
  if (a == "JH") return "dZ";
  if (a == "K")  return "k";
  if (a == "L")  return "l";
  if (a == "M")  return "m";
  if (a == "N")  return "n";
  if (a == "NG") return "N";
  if (a == "OW") return "@U";
  if (a == "OY") return "OI";
  if (a == "P")  return "p";
  if (a == "R")  return "r";
  if (a == "S")  return "s";
  if (a == "SH") return "S";
  if (a == "T")  return "t";
  if (a == "TH") return "T";
  if (a == "UH") return "U";
  if (a == "UW") return "u:";
  if (a == "V")  return "v";
  if (a == "W")  return "w";
  if (a == "Y")  return "j";
  if (a == "Z")  return "z";
  if (a == "ZW") return "Z";
  
  silog::log(silog::error, "unknown arpa phoneme: [%.*s]",
             static_cast<int>(a.size()), a.begin());
  throw 0;
}

// Magic rate from MBROLA
static constexpr const auto audio_rate = 16000;
static void audio_filler(float * data, unsigned samples);

class globals {
  carnage::map m_cmdict {};
  embrolho::t m_emb {};

  mtx::mutex m_mutex {};
  jute::view m_next {};
  bool m_playing {};

public:
  globals() {
    m_emb.write_pho("_ 80 \n");
    siaudio::filler(audio_filler);
    siaudio::rate(audio_rate);
  }
  ~globals() {
    siaudio::rate(0);
  }

  void word(jute::view wrd) {
    mtx::lock m { &m_mutex };

    bool longer_pause = false;

    auto w = wrd.cstr();
    for (auto & c : w) {
      if (c >= 'A' && c <= 'Z') c |= 0x20;
      if (c == '.' || c == ',') {
        longer_pause = true;
        c = 0;
      }
    }

    auto arpa = m_cmdict[jute::view::unsafe(w.begin())];
    silog::log(silog::info, "ARPAbet: [%s]", arpa.cstr().begin());
    while (arpa.size()) {
      auto [l, rest] = arpa.split(' ');
      arpa = rest;

      auto e = l.data()[l.size() - 1];
      if (e >= '0' && e <= '9') l = l.subview(l.size() - 1).before;

      auto sampa = arpa2sampa(l);
      silog::log(silog::info, "/%s/", sampa);
      m_emb.write_pho(sampa);
      m_emb.write_pho(sampa[1] ? " 120 " : " 80 ");
      switch (e) {
        // case '0': m_emb.write_pho("100 120");
        // case '1': m_emb.write_pho("100 120");
        // case '2': m_emb.write_pho("100 120");
        default: break;
      }
      m_emb.write_pho("\n");
    }

    if (longer_pause) {
      m_emb.write_pho("_ 160 \n");
    }
    m_emb.write_pho("#\n");
    m_playing = true;
  }

  unsigned read(short * data, unsigned n) {
    mtx::lock m { &m_mutex };

    if (!m_playing) return 0;
    auto res = m_emb.read(data, n);
    if (res == 0) m_playing = false;
    return res;
  }

  bool playing() { return m_playing; }
};
static auto & t() {
  static globals g {};
  return g;
}

static void audio_filler(float * data, unsigned samples) {
  for (auto i = 0; i < samples; i++) data[i] = 0;

  short buf[16000];
  auto i = t().read(buf, samples);
  for (auto x = 0; x < i; x++) {
    float s = buf[x];
    data[x] = s / ((2 << 16) - 1);
  }
}

bool tts::playing() { return t().playing(); }
void tts::word(jute::view w) { t().word(w); }

