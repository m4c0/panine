#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
export module pipeline;
import jute;
import mov;
import sith;
import scriber;
import scripter;
import vee;
import voo;

static constexpr const auto font_h = 128;

struct upc {
  float aspect;
};

export class pipeline {
  vee::descriptor_set_layout m_dsl = vee::create_descriptor_set_layout({
    vee::dsl_fragment_sampler(),
    vee::dsl_fragment_sampler(),
  });
  vee::descriptor_pool m_dpool = vee::create_descriptor_pool(1, {
    vee::combined_image_sampler(2)
  });
  vee::descriptor_set m_dset = vee::allocate_descriptor_set(*m_dpool, *m_dsl);
  vee::sampler m_smp = vee::create_sampler(vee::linear_sampler);

  vee::pipeline_layout m_pl = vee::create_pipeline_layout(
    { *m_dsl },
    { vee::fragment_push_constant_range<upc>() }
  );
  voo::one_quad_render m_oqr;

  scriber m_scr;
  mov m_mov;

  jute::view m_cur = "------------";

  void render_pass(vee::command_buffer cb, vee::render_pass_begin rpb) {
    rpb.command_buffer = cb;
    rpb.clear_colours = { vee::clear_colour({}) };
    voo::cmd_render_pass rp { rpb };

    float w = rpb.extent.width;
    float h = rpb.extent.height;
    upc pc { .aspect = w / h };
    m_oqr.run(cb, rpb.extent, [&] {
      vee::cmd_push_fragment_constants(cb, *m_pl, &pc);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset);
    });
  }

public:
  pipeline(voo::device_and_queue * dq, vee::render_pass::type rp, bool rt)
    : m_oqr { "main", dq->physical_device(), rp, *m_pl }
    , m_scr { *dq, { 1024, 1024 } }
    , m_mov { dq->physical_device(), dq->queue(), rt }
  {
    m_mov.run_once();
    vee::update_descriptor_set(m_dset, 0, m_scr.image_view(), *m_smp);
    vee::update_descriptor_set(m_dset, 1, m_mov.image_view(), *m_smp);
  }

  auto play_movie() { return sith::run_guard { &m_mov }; }
  void next_frame() { m_mov.run_once(); }

  void run(vee::command_buffer cb, vee::render_pass_begin rpb, jute::view text) {
    if (m_cur != text) m_scr.shape(cb, 1024, font_h, text);

    render_pass(cb, rpb);

    if (m_cur != text) {
      m_scr.clear_glyphs(cb);
      m_cur = text;
    }
  }
};

