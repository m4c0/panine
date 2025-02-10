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
  if (a == "AA") return "Q   80";
  if (a == "AE") return "{   80";
  if (a == "AH") return "V   80";
  if (a == "AO") return "O: 120";
  if (a == "AW") return "aU  80";
  if (a == "AX") return "@   80";
  if (a == "AY") return "aI 120";
  if (a == "B")  return "b   80";
  if (a == "CH") return "tS 120";
  if (a == "D")  return "d   80";
  if (a == "DH") return "D   80";
  if (a == "DX") return "4   80";
  if (a == "EH") return "E   80";
  if (a == "ER") return "3: 120";
  if (a == "EY") return "eI 120";
  if (a == "F")  return "f   80";
  if (a == "G")  return "g   80";
  if (a == "HH") return "h   80";
  if (a == "IH") return "I   80";
  if (a == "IY") return "i: 120";
  if (a == "JH") return "dZ  80";
  if (a == "K")  return "k   80";
  if (a == "L")  return "l   80";
  if (a == "M")  return "m   80";
  if (a == "N")  return "n   80";
  if (a == "NG") return "N   80";
  if (a == "OW") return "@U 120";
  if (a == "OY") return "OI 120";
  if (a == "P")  return "p   80";
  if (a == "R")  return "r   80";
  if (a == "S")  return "s   80";
  if (a == "SH") return "S   80";
  if (a == "T")  return "t   80";
  if (a == "TH") return "T   80";
  if (a == "UH") return "U   80";
  if (a == "UW") return "u: 120";
  if (a == "V")  return "v   80";
  if (a == "W")  return "w   80";
  if (a == "Y")  return "j   80";
  if (a == "Z")  return "z   80";
  if (a == "ZW") return "Z   80";
  
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
      m_emb.write_pho("\n");
    }

    if (longer_pause) {
      m_emb.write_pho("_ 160 \n");
    } else {
      m_emb.write_pho("_ 40 \n");
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

