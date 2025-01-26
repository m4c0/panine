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
    : m_cbuf { dq.physical_device(), ext, vee::image_format_r8 }
    , m_rp { vee::create_render_pass({}) }
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
};
