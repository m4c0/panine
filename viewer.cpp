#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
#pragma leco app

import audio;
import casein;
import sitime;
import vee;
import voo;
import vtw;
import what_the_font;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

static constexpr const auto font_h = 64;

static wtf::library g_library{};
static wtf::face g_face = g_library.new_face("out/font.ttf");

struct upc {};

struct init : public voo::casein_thread {
  init() {
    casein::window_size = { window_width, window_height };
    g_face.set_char_size(font_h);
  }

  void run() {
    sitime::stopwatch timer {};
    bool started {};

    main_loop("panine", [&](auto & dq, auto & sw) {
      auto dsl = vee::create_descriptor_set_layout({ vee::dsl_fragment_sampler() });
      auto dpool = vee::create_descriptor_pool(1, { vee::combined_image_sampler() });
      vtw::scriber scr { dq.physical_device(), vee::allocate_descriptor_set(*dpool, *dsl) };

      auto s = g_face.shape_en("Hello");
      scr.pen({ 0, font_h });
      scr.draw(s, [&](auto pen, const auto & glyph) {
      });

      vee::pipeline_layout pl = vee::create_pipeline_layout(
          { *dsl },
          { vee::vert_frag_push_constant_range<upc>() });

      voo::one_quad_render oqr { "main", &dq, *pl };

      extent_loop(dq.queue(), sw, [&] {
        if (!started) {
          audio::start();
          timer = {};
          started = true;
        }

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          scr.setup_copy(*pcb);

          auto scb = sw.cmd_render_pass({
            .command_buffer = *pcb,
            .clear_colours { vee::clear_colour({}) },
          });
          oqr.run(*pcb, sw.extent(), [&] {
            vee::cmd_bind_descriptor_set(*pcb, *pl, 0, scr.descriptor_set());
          });
        });
      });
    });
  }
} i;
