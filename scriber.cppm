export module scriber;
import dotz;
import jute;
import vee;
import voo;
import vtw;
import what_the_font;

struct chr {
  dotz::vec2 pos;
  dotz::vec2 size;
  dotz::vec4 uv;
};

export class scriber {
  static constexpr const auto max_chars = 16;

  voo::host_buffer m_chrs;
  vtw::scriber m_scr;

  void setup_copy(vee::command_buffer cb) { m_scr.setup_copy(cb); }
  void shape(int scr_w, jute::view txt);

public:
  scriber(const voo::device_and_queue & dq, dotz::vec2 ext)
    : m_chrs { dq, sizeof(chr) * max_chars }
    , m_scr { dq.physical_device() } {
    m_scr.bounds(ext);
  }

  [[nodiscard]] auto cbuf() const { return m_chrs.buffer(); }
  [[nodiscard]] auto dsl() const { return m_scr.descriptor_set_layout(); }
  [[nodiscard]] auto dset() const { return m_scr.descriptor_set(); }

  void shape(vee::command_buffer cb, int scr_w, jute::view txt) {
    shape(scr_w, txt);
    setup_copy(cb);
  }
};

module : private;

void scriber::shape(int scr_w, jute::view txt) {
  static constexpr const auto font_h = 128;
  static wtf::library g_library{};
  static wtf::face g_face = g_library.new_face("out/font.ttf");
  g_face.set_char_size(font_h);

  voo::mapmem mem { m_chrs.memory() };
  auto vs = static_cast<chr *>(*mem);

  auto s = g_face.shape_en(txt);
  m_scr.pen((-s.bounding_box() + scr_w) / 2);
  m_scr.draw(s, [&](auto pen, const auto & glyph) {
    *vs++ = chr {
      .pos  = pen + glyph.d,
      .size = glyph.size,
      .uv   = glyph.uv
    };
  });
}
