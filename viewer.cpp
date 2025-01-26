#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
#pragma leco app

import audio;
import casein;
import scriber;
import sdf_texture;
import sitime;
import vapp;
import vee;
import voo;
import vtw;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

static constexpr const auto font_h = 128;

struct init : public vapp {
  init() {
    casein::window_size = { window_width, window_height };
  }

  void run() {
    sitime::stopwatch timer {};
    bool started {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      scriber s { dq, { 1024, 1024 } };

      sdf_texture sdf { dq, { 1024, 1024 } };

      auto pl = vee::create_pipeline_layout();
      voo::one_quad_render oqr { "main", &dq, *pl };

      bool copied = false;

      extent_loop(dq.queue(), sw, [&] {
        if (!started) {
          audio::start();
          timer = {};
          started = true;
        }

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (!copied) {
            s.shape(*pcb, 1024, font_h, "Engineering");
            copied = true;
          }

          auto scb = sw.cmd_render_pass({
            .command_buffer = *pcb,
            .clear_colours { vee::clear_colour({}) },
          });
          oqr.run(*scb, sw.extent());
        });
      });
    });
  }
} i;
