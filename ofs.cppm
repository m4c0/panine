export module ofs;

import vee;
import voo;

export class ofs {
  voo::offscreen::colour_buffer m_cbuf;
  vee::render_pass m_rp;
  vee::framebuffer m_fb;
  vee::extent m_ext;

public:
  ofs(const voo::device_and_queue & dq, vee::extent ext)
    : m_cbuf { dq.physical_device(), ext, vee::image_format_srgba, vee::image_usage_sampled }
    , m_rp { vee::create_depthless_render_pass({{
        vee::create_colour_attachment(vee::image_format_srgba, vee::image_layout_shader_read_only_optimal)
      }}) }
    , m_fb { vee::create_framebuffer({
      .physical_device = dq.physical_device(),
      .render_pass = *m_rp,
      .attachments = {{ m_cbuf.image_view() }},
      .extent = ext,
    }) }
    , m_ext { ext } {}

  [[nodiscard]] auto cmd_render_pass(vee::command_buffer cb) {
    return voo::cmd_render_pass { vee::render_pass_begin {
      .command_buffer = cb,
      .render_pass = *m_rp,
      .framebuffer = *m_fb,
      .extent = m_ext,
      .clear_colours { vee::clear_colour({}) },
    }};
  }

  [[nodiscard]] auto image_view() const { return m_cbuf.image_view(); }
  [[nodiscard]] auto render_pass() const { return *m_rp; }
};
