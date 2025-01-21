#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
#pragma leco app

import audio;
import casein;
import sitime;
import vee;
import voo;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
  }

  void run() {
    sitime::stopwatch timer {};
    bool started {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      vee::pipeline_layout pl = vee::create_pipeline_layout({ *dsl }, { vee::vert_frag_push_constant_range<upc>() });

      voo::one_quad_render oqr { "main", &dq, *pl };

      extent_loop(dq.queue(), sw, [&] {
        if (!started) {
          audio::start();
          timer = {};
          started = true;
        }

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          auto scb = sw.cmd_render_pass({
            .command_buffer = *pcb,
            .clear_colours { vee::clear_colour({}) },
          });
          oqr.run(*pcb, sw.extent());
        });
      });
    });
  }
} i;
