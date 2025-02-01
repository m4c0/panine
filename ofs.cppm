export module ofs;

import vee;
import voo;

export class ofs {
  voo::offscreen::colour_buffer m_cbuf;
  vee::render_pass m_rp;
  vee::framebuffer m_fb;
  vee::extent m_ext;

  static vee::render_pass create_render_pass() {
    return vee::create_render_pass({
      .attachments {{
        vee::create_colour_attachment(vee::image_format_srgba, vee::image_layout_read_only_optimal),
      }},
      .subpasses {{
        vee::create_subpass({
          .colours {{
            create_attachment_ref(0, vee::image_layout_color_attachment_optimal),
          }},
        })
      }},
      .dependencies {{ vee::create_colour_dependency() }},
    });;
  }

public:
  ofs(const voo::device_and_queue & dq, vee::extent ext)
    : m_cbuf { dq.physical_device(), ext, vee::image_format_srgba, vee::image_usage_sampled }
    , m_rp { create_render_pass() }
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
