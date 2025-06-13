#pragma leco add_shader "ofs-pass0.frag"
#pragma leco add_shader "ofs-pass1.frag"
#pragma leco add_shader "ofs.vert"
export module ofs;

import dotz;
import vee;
import voo;

static vee::render_pass create_render_pass() {
  return vee::create_render_pass({
    .attachments {{
      // TODO: use R8 instead of sRGB
      vee::create_colour_attachment(VK_FORMAT_R8G8B8A8_SRGB, vee::image_layout_read_only_optimal),
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

class pass0 {
  vee::descriptor_set m_dset_scriber;
  vee::descriptor_set m_dset_chars;
  vee::pipeline_layout m_pl;
  vee::gr_pipeline m_gp;

public:
  pass0(const voo::one_quad & quad,
        vee::descriptor_set dset_scriber,
        vee::descriptor_set dset_chars,
        vee::render_pass::type rp)
    : m_dset_scriber { dset_scriber }
    , m_dset_chars { dset_chars }
    , m_pl { vee::create_pipeline_layout({
        .descriptor_set_layouts {{
          *vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }),
          *vee::create_descriptor_set_layout({ vee::dsl_fragment_storage() }),
        }}
      }) }
    , m_gp { vee::create_graphics_pipeline({
        .pipeline_layout = *m_pl,
        .render_pass = rp,
        .shaders {
          voo::shader("ofs.vert.spv").pipeline_vert_stage(),
          voo::shader("ofs-pass0.frag.spv").pipeline_frag_stage(),
        },
        .bindings { quad.vertex_input_bind() },
        .attributes { quad.vertex_attribute(0) },
      }) } {}

  void render(vee::command_buffer cb) {
    vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset_scriber);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 1, m_dset_chars);
    vee::cmd_bind_gr_pipeline(cb, *m_gp);
  }
};

class pass1 {
  vee::pipeline_layout m_pl;
  vee::gr_pipeline m_gp;

  struct upc { dotz::vec2 ext; };

public:
  pass1(const voo::one_quad & quad,
        vee::image_view::type in, 
        vee::render_pass::type rp)
    : m_pl { vee::create_pipeline_layout(
      *vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }),
      vee::fragment_push_constant_range<upc>()
    ) }
    , m_gp { vee::create_graphics_pipeline({
        .pipeline_layout = *m_pl,
        .render_pass = rp,
        .shaders {
          voo::shader("ofs.vert.spv").pipeline_vert_stage(),
          voo::shader("ofs-pass1.frag.spv").pipeline_frag_stage(),
        },
        .bindings { quad.vertex_input_bind() },
        .attributes { quad.vertex_attribute(0) },
      }) } {}

  void render(vee::command_buffer cb, vee::descriptor_set dset, vee::extent ext) {
    upc pc { { ext.width, ext.height } };
    vee::cmd_push_fragment_constants(cb, *m_pl, &pc);
    vee::cmd_bind_gr_pipeline(cb, *m_gp);
    vee::cmd_bind_descriptor_set(cb, *m_pl, 0, dset);
  }
};

export class ofs {
  vee::render_pass m_rp;
  vee::extent m_ext;
  voo::one_quad m_quad;
  vee::sampler m_smp;

  vee::descriptor_set_layout m_dsl;
  vee::descriptor_pool m_pool;
  vee::descriptor_set m_dsets[2];

  voo::offscreen::colour_buffer m_c0;
  voo::offscreen::colour_buffer m_c1;

  vee::framebuffer m_fb0;
  vee::framebuffer m_fb1;

  pass0 m_p0;
  pass1 m_p1;

  auto render(vee::command_buffer cb, vee::framebuffer::type fb, auto && fn) {
    auto rp = voo::cmd_render_pass { vee::render_pass_begin {
      .command_buffer = cb,
      .render_pass = *m_rp,
      .framebuffer = fb,
      .extent = m_ext,
      .clear_colours { vee::clear_colour({}) },
    }};
    vee::cmd_set_scissor(cb, m_ext);
    vee::cmd_set_viewport(cb, m_ext);
    fn();
    m_quad.run(cb, 0, 1);
  }

public:
  ofs(const voo::device_and_queue & dq,
      vee::descriptor_set dset_scriber,
      vee::descriptor_set dset_chars,
      vee::extent ext)
    : m_rp { create_render_pass() }
    , m_ext { ext }
    , m_quad { dq.physical_device() }
    , m_smp { vee::create_sampler(vee::linear_sampler) }

    , m_dsl { vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() }) }
    , m_pool { vee::create_descriptor_pool(2, { vee::combined_image_sampler(2) }) }
    , m_dsets {
      vee::allocate_descriptor_set(*m_pool, *m_dsl),
      vee::allocate_descriptor_set(*m_pool, *m_dsl),
    }

    , m_c0 { dq.physical_device(), ext, VK_FORMAT_R8G8B8A8_SRGB, vee::image_usage_sampled }
    , m_c1 { dq.physical_device(), ext, VK_FORMAT_R8G8B8A8_SRGB, vee::image_usage_sampled }
    , m_fb0 { vee::create_framebuffer({
      .render_pass = *m_rp,
      .attachments = {{ m_c0.image_view() }},
      .extent = ext,
    }) }
    , m_fb1 { vee::create_framebuffer({
      .render_pass = *m_rp,
      .attachments = {{ m_c1.image_view() }},
      .extent = ext,
    }) }
    , m_p0 { m_quad, dset_scriber, dset_chars, *m_rp }
    , m_p1 { m_quad, m_c0.image_view(), *m_rp } {
    vee::update_descriptor_set(m_dsets[0], 0, m_c0.image_view(), *m_smp);
    vee::update_descriptor_set(m_dsets[1], 0, m_c1.image_view(), *m_smp);
  }

  void render(vee::command_buffer cb) {
    render(cb, *m_fb0, [this, cb] { m_p0.render(cb); });
    vee::cmd_pipeline_barrier(cb, m_c0.image(), vee::from_fragment_to_fragment);

    for (auto i = 0; i < 64; i++) {
      render(cb, *m_fb1, [this, cb] { m_p1.render(cb, m_dsets[0], m_ext); });
      vee::cmd_pipeline_barrier(cb, m_c1.image(), vee::from_fragment_to_fragment);

      render(cb, *m_fb0, [this, cb] { m_p1.render(cb, m_dsets[1], m_ext); });
      vee::cmd_pipeline_barrier(cb, m_c0.image(), vee::from_fragment_to_fragment);
    }
  }

  [[nodiscard]] auto image_view() const { return m_c0.image_view(); }
};
