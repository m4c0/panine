#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
#pragma leco app

import casein;
import jojo;
import jute;
import macspeech;
import mov;
import scriber;
import scripter;
import silog;
import sith;
import vapp;
import vee;
import voo;
import vtw;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

static constexpr const auto font_h = 128;

static auto script = jojo::read_cstr("out/script.txt");

struct upc {
  float aspect;
};

class pipeline {
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

public:
  pipeline(voo::device_and_queue * dq)
    : m_oqr { "main", dq, *m_pl }
    , m_scr { *dq, { 1024, 1024 } }
    , m_mov { dq->physical_device(), dq->queue() }
  {
    m_mov.run_once();
    vee::update_descriptor_set(m_dset, 0, m_scr.image_view(), *m_smp);
    vee::update_descriptor_set(m_dset, 1, m_mov.image_view(), *m_smp);
  }

  void shape(vee::command_buffer cb, jute::view text) {
    m_scr.shape(cb, 1024, font_h, text);
  }

  void clear_glyphs(vee::command_buffer cb) { m_scr.clear_glyphs(cb); }

  auto play_movie() { return sith::run_guard { &m_mov }; }
  void next_frame() { m_mov.run_once(); }

  void run(vee::command_buffer cb, vee::extent ext) {
    float w = ext.width;
    float h = ext.height;
    upc pc { .aspect = w / h };
    m_oqr.run(cb, ext, [&] {
      vee::cmd_push_fragment_constants(cb, *m_pl, &pc);
      vee::cmd_bind_descriptor_set(cb, *m_pl, 0, m_dset);
    });
  }
};

struct init : public vapp {
  init() {
    casein::window_size = { window_width, window_height };
  }

  void run() {
    main_loop("panine", [&](auto & dq, auto & sw) {
      macspeech ms {};

      pipeline ppl { &dq };

      sith::run_guard mg {};

      jute::view cur_text { "---------------" };
      ms.synth(script);

      extent_loop(dq.queue(), sw, [&] {
        if (!ms.playing()) throw 0;

        auto text = jute::view::unsafe(ms.current());
        if (cur_text != text) {
          silog::log(silog::info, "changing word to: [%s]", text.cstr().begin());
          if (!mg) mg = ppl.play_movie();
        }

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          auto rpb = sw.render_pass_begin();

          if (text != cur_text) ppl.shape(*pcb, text);

          {
            rpb.command_buffer = *pcb;
            rpb.clear_colours = { vee::clear_colour({}) };

            voo::cmd_render_pass rp { rpb };
            ppl.run(*pcb, sw.extent());
          }

          if (text != cur_text) {
            ppl.clear_glyphs(*pcb);
            cur_text = text;
          }
        });
      });
    });
  }
} i;
