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

public:
  pipeline(
    voo::device_and_queue * dq,
    scriber & s,
    mov & m
  ) : m_oqr { "main", dq, *m_pl } {
    vee::update_descriptor_set(m_dset, 0, s.image_view(), *m_smp);
    vee::update_descriptor_set(m_dset, 1, m.image_view(), *m_smp);
  }

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
      scriber s { dq, { 1024, 1024 } };
      macspeech ms {};

      mov m { dq.physical_device(), dq.queue() };
      m.run_once();
      sith::run_guard mg {};

      pipeline ppl { &dq, s, m };

      jute::view cur_text {};
      ms.synth(script);

      extent_loop(dq.queue(), sw, [&] {
        if (!ms.playing()) throw 0;

        auto text = jute::view::unsafe(ms.current());
        if (cur_text != text) {
          silog::log(silog::info, "changing word to: [%s]", text.cstr().begin());
          if (!mg) mg = sith::run_guard { &m };
        }

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (text != cur_text) s.shape(*pcb, 1024, font_h, text);

          {
            auto scb = sw.cmd_render_pass({
              .command_buffer = *pcb,
              .clear_colours { vee::clear_colour({}) },
            });
            ppl.run(*scb, sw.extent());
          }

          if (text != cur_text) {
            s.clear_glyphs(*pcb);
            cur_text = text;
          }
        });
      });
    });
  }
} i;
