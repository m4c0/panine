export module ofs;

import vee;
import voo;

export class ofs {
  voo::offscreen::colour_buffer m_cbuf;

  vee::descriptor_set m_dset_scriber;
  vee::descriptor_set m_dset_chars;
  vee::pipeline_layout m_pl;

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
  ofs(const voo::device_and_queue & dq,
      vee::descriptor_set dset_scriber,
      vee::descriptor_set dset_chars,
      vee::extent ext)
    : m_cbuf { dq.physical_device(), ext, vee::image_format_srgba, vee::image_usage_sampled }
    , m_dset_scriber { dset_scriber }
    , m_dset_chars { dset_chars }
    , m_pl { vee::create_pipeline_layout({
        *vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }),
        *vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }),
      }) }
    , m_rp { create_render_pass() }
    , m_fb { vee::create_framebuffer({
      .physical_device = dq.physical_device(),
      .render_pass = *m_rp,
      .attachments = {{ m_cbuf.image_view() }},
      .extent = ext,
    }) }
    , m_ext { ext } {}

  void render(vee::command_buffer cb) {
    voo::cmd_render_pass rp { vee::render_pass_begin {
      .command_buffer = cb,
      .render_pass = *m_rp,
      .framebuffer = *m_fb,
      .extent = m_ext,
      .clear_colours { vee::clear_colour({}) },
    }};
    vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset_scriber);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 1, m_dset_chars);
  }

  [[nodiscard]] auto image_view() const { return m_cbuf.image_view(); }
};
