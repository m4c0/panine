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
  vee::descriptor_set_layout m_dsl;
  vee::descriptor_pool m_dpool;
  vtw::scriber m_scr;

  void setup_copy(vee::command_buffer cb) { m_scr.setup_copy(cb); }
  void shape(int scr_w, jute::view txt);

public:
  scriber(const voo::device_and_queue & dq, dotz::vec2 ext);

  [[nodiscard]] auto cbuf() const { return m_chrs.buffer(); }
  [[nodiscard]] auto dsl() const { return *m_dsl; }
  [[nodiscard]] auto dset() const { return m_scr.descriptor_set(); }

  void shape(vee::command_buffer cb, int scr_w, jute::view txt) {
    shape(scr_w, txt);
    setup_copy(cb);
  }
};

module : private;

scriber::scriber(const voo::device_and_queue & dq, dotz::vec2 ext)
  : m_chrs { dq, sizeof(chr) * max_chars }
  , m_dsl { vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }) }
  , m_dpool { vee::create_descriptor_pool(1, { vee::combined_image_sampler() }) }
  , m_scr { dq.physical_device(), vee::allocate_descriptor_set(*m_dpool, *m_dsl) } {
  m_scr.bounds(ext);
}

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
