#pragma leco add_shader "main.vert"
#pragma leco add_shader "main.frag"
#pragma leco app

import casein;
import jute;
import scriber;
import scripter;
import silog;
import tts;
import vapp;
import vee;
import voo;
import vtw;

static constexpr const auto window_height = 800;
static constexpr const auto window_width = window_height * 9 / 16;

static constexpr const auto font_h = 128;

struct upc {
  float aspect;
};

struct init : public vapp {
  init() {
    casein::window_size = { window_width, window_height };
  }

  void run() {
    main_loop("panine", [&](auto & dq, auto & sw) {
      scriber s { dq, { 1024, 1024 } };

      voo::single_dset dset {
        vee::dsl_fragment_sampler(), vee::combined_image_sampler()
      };
      vee::sampler smp = vee::create_sampler(vee::linear_sampler);
      vee::update_descriptor_set(dset.descriptor_set(), 0, s.image_view(), *smp);

      auto pl = vee::create_pipeline_layout(
          { dset.descriptor_set_layout() },
          { vee::fragment_push_constant_range<upc>() });
      voo::one_quad_render oqr { "main", &dq, *pl };

      jute::view text = "";
      jute::view cur_text {};

      extent_loop(dq.queue(), sw, [&] {
        if (!tts::playing()) {
          text = scripter::next();
          silog::log(silog::info, "changing word to: [%s]", text.cstr().begin());
          tts::word(text);
        }

        upc pc { .aspect = sw.aspect() };

        sw.queue_one_time_submit(dq.queue(), [&](auto pcb) {
          if (text != cur_text) s.shape(*pcb, 1024, font_h, text);

          {
            auto scb = sw.cmd_render_pass({
              .command_buffer = *pcb,
              .clear_colours { vee::clear_colour({}) },
            });
            oqr.run(*scb, sw.extent(), [&] {
              vee::cmd_push_fragment_constants(*pcb, *pl, &pc);
              vee::cmd_bind_descriptor_set(*pcb, *pl, 0, dset.descriptor_set());
            });
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
