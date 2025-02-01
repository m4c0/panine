#pragma leco add_shader "ofs-pass0.frag"
#pragma leco add_shader "ofs-pass1.frag"
#pragma leco add_shader "ofs.vert"
export module ofs;

import vee;
import voo;

class pass0 {
  voo::offscreen::colour_buffer m_cbuf;

  voo::one_quad m_quad;

  vee::descriptor_set m_dset_scriber;
  vee::descriptor_set m_dset_chars;
  vee::pipeline_layout m_pl;
  vee::render_pass m_rp;
  vee::gr_pipeline m_gp;
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
    });
  }

public:
  pass0(const voo::device_and_queue & dq,
      vee::descriptor_set dset_scriber,
      vee::descriptor_set dset_chars,
      vee::extent ext)
    : m_cbuf { dq.physical_device(), ext, vee::image_format_srgba, vee::image_usage_sampled }
    , m_quad { dq.physical_device() }
    , m_dset_scriber { dset_scriber }
    , m_dset_chars { dset_chars }
    , m_pl { vee::create_pipeline_layout({
        *vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }),
        *vee::create_descriptor_set_layout({ vee::dsl_fragment_storage() }),
      }) }
    , m_rp { create_render_pass() }
    , m_gp { vee::create_graphics_pipeline({
        .pipeline_layout = *m_pl,
        .render_pass = *m_rp,
        .shaders {
          voo::shader("ofs.vert.spv").pipeline_vert_stage(),
          voo::shader("ofs-pass0.frag.spv").pipeline_frag_stage(),
        },
        .bindings { m_quad.vertex_input_bind() },
        .attributes { m_quad.vertex_attribute(0) },
      }) }
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
    vee::cmd_set_scissor(cb, m_ext);
    vee::cmd_set_viewport(cb, m_ext);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset_scriber);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 1, m_dset_chars);
    vee::cmd_bind_gr_pipeline(cb, *m_gp);
    m_quad.run(cb, 0, 1);
  }

  [[nodiscard]] auto image() const { return m_cbuf.image(); }
  [[nodiscard]] auto image_view() const { return m_cbuf.image_view(); }
};

class pass1 {
  voo::offscreen::colour_buffer m_cbuf;

  voo::one_quad m_quad;

  voo::single_dset m_dset;
  vee::pipeline_layout m_pl;
  vee::render_pass m_rp;
  vee::gr_pipeline m_gp;
  vee::framebuffer m_fb;
  vee::sampler m_smp;
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
    });
  }

public:
  pass1(const voo::device_and_queue & dq, vee::image_view::type p0, vee::extent ext)
    : m_cbuf { dq.physical_device(), ext, vee::image_format_srgba, vee::image_usage_sampled }
    , m_quad { dq.physical_device() }
    , m_dset { vee::dsl_fragment_sampler(), vee::combined_image_sampler() }
    , m_pl { vee::create_pipeline_layout({
        *vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }),
      }) }
    , m_rp { create_render_pass() }
    , m_gp { vee::create_graphics_pipeline({
        .pipeline_layout = *m_pl,
        .render_pass = *m_rp,
        .shaders {
          voo::shader("ofs.vert.spv").pipeline_vert_stage(),
          voo::shader("ofs-pass1.frag.spv").pipeline_frag_stage(),
        },
        .bindings { m_quad.vertex_input_bind() },
        .attributes { m_quad.vertex_attribute(0) },
      }) }
    , m_fb { vee::create_framebuffer({
      .physical_device = dq.physical_device(),
      .render_pass = *m_rp,
      .attachments = {{ m_cbuf.image_view() }},
      .extent = ext,
    }) }
    , m_smp { vee::create_sampler(vee::linear_sampler) }
    , m_ext { ext } {
    vee::update_descriptor_set(m_dset.descriptor_set(), 0, p0, *m_smp);
  }

  void render(vee::command_buffer cb) {
    voo::cmd_render_pass rp { vee::render_pass_begin {
      .command_buffer = cb,
      .render_pass = *m_rp,
      .framebuffer = *m_fb,
      .extent = m_ext,
      .clear_colours { vee::clear_colour({}) },
    }};
    vee::cmd_set_scissor(cb, m_ext);
    vee::cmd_set_viewport(cb, m_ext);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset.descriptor_set() );
    vee::cmd_bind_gr_pipeline(cb, *m_gp);
    m_quad.run(cb, 0, 1);
  }

  [[nodiscard]] auto image_view() const { return m_cbuf.image_view(); }
};

export class ofs {
  pass0 m_p0;
  pass1 m_p1;

public:
  ofs(const voo::device_and_queue & dq,
      vee::descriptor_set dset_scriber,
      vee::descriptor_set dset_chars,
      vee::extent ext)
    : m_p0 { dq, dset_scriber, dset_chars, ext }
    , m_p1 { dq, m_p0.image_view(), ext } {}

  void render(vee::command_buffer cb) {
    m_p0.render(cb);
    vee::cmd_pipeline_barrier(cb, m_p0.image(), vee::from_fragment_to_fragment);
    m_p1.render(cb);
  }

  [[nodiscard]] auto image_view() const { return m_p1.image_view(); }
};
