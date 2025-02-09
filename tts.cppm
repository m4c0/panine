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
  if (a == "AA") return "Q   80 \n";
  if (a == "AE") return "{   80 \n";
  if (a == "AH") return "V   80 \n";
  if (a == "AO") return "O: 120 \n";
  if (a == "AW") return "aU  80 \n";
  if (a == "AX") return "@   80 \n";
  if (a == "AY") return "aI 120 \n";
  if (a == "B")  return "b   80 \n";
  if (a == "CH") return "tS  80 \n";
  if (a == "D")  return "d   80 \n";
  if (a == "DH") return "D   80 \n";
  if (a == "DX") return "4   80 \n";
  if (a == "EH") return "E   80 \n";
  if (a == "ER") return "3: 120 \n";
  if (a == "EY") return "eI 120 \n";
  if (a == "F")  return "f   80 \n";
  if (a == "G")  return "g   80 \n";
  if (a == "HH") return "h   80 \n";
  if (a == "IH") return "I   80 \n";
  if (a == "IY") return "i: 120 \n";
  if (a == "JH") return "dZ  80 \n";
  if (a == "K")  return "k   80 \n";
  if (a == "L")  return "l   80 \n";
  if (a == "M")  return "m   80 \n";
  if (a == "N")  return "n   80 \n";
  if (a == "NG") return "N   80 \n";
  if (a == "OW") return "@U  80 \n";
  if (a == "OY") return "OI 120 \n";
  if (a == "P")  return "p   80 \n";
  if (a == "R")  return "r   80 \n";
  if (a == "S")  return "s   80 \n";
  if (a == "SH") return "S   80 \n";
  if (a == "T")  return "t   80 \n";
  if (a == "TH") return "T   80 \n";
  if (a == "UH") return "U   80 \n";
  if (a == "UW") return "u:  80 \n";
  if (a == "V")  return "v   80 \n";
  if (a == "W")  return "w   80 \n";
  if (a == "Y")  return "j   80 \n";
  if (a == "Z")  return "z   80 \n";
  if (a == "ZW") return "Z   80 \n";
  
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

    auto w = wrd.cstr();
    for (auto & c : w) {
      if (c >= 'A' && c <= 'Z') c |= 0x20;
      if (c < 'a' || c > 'z') c = 0;
    }

    auto arpa = m_cmdict[jute::view::unsafe(w.begin())];
    while (arpa.size()) {
      auto [l, rest] = arpa.split(' ');
      arpa = rest;

      auto e = l.data()[l.size() - 1];
      if (e >= '0' && e <= '9') l = l.subview(l.size() - 1).before;

      m_emb.write_pho(arpa2sampa(l));
    }

    m_emb.write_pho("_ 80 \n");
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

