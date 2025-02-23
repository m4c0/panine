#pragma leco app
#pragma leco add_impl video_out

import macspeech;
import pipeline;
import silog;
import vee;
import voo;

struct pix { char p[4]; };

static constexpr const vee::extent extent { 1080, 1920 };
static constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;

extern "C" {
  void * vo_new(int w, int h);
  void vo_delete(void *);
  void * vo_audio(void *);
  void vo_done(void *);
  pix * vo_lock(void * p);
  void vo_unlock(void * p, unsigned frame);
  bool vo_wait(void * p);
}

static auto vo = vo_new(extent.width, extent.height);

static voo::device_and_queue dq { "panine-render" };

static macspeech ms {};

static voo::offscreen::buffers fb { dq.physical_device(), extent, format };
static pipeline ppl { &dq, fb.render_pass(), false };

static voo::single_cb cb { dq.queue_family() };
static vee::render_pass_begin rpb = fb.render_pass_begin({});

static int i = 0;

int main() {
  ms.write(vo_audio(vo), [](float seconds) {
    {
      voo::cmd_buf_one_time_submit ots { cb.cb() };
      ppl.run(cb.cb(), rpb, ms.current());
      fb.cmd_copy_to_host(cb.cb());
    }
    dq.queue()->queue_submit({
      .command_buffer = cb.cb(),
    });
    
    vee::device_wait_idle();

    {
      auto mm = fb.map_host();
      pix * in = static_cast<pix *>(*mm);
      pix * out = vo_lock(vo);
      for (auto i = 0; i < extent.width * extent.height; i++) {
        *out = {{ in->p[3], in->p[0], in->p[1], in->p[2] }};
        out++;
        in++;
      }
      vo_unlock(vo, i++);
    }

    ppl.next_frame();
    vee::device_wait_idle();
  });
  while (ms.playing()) vo_wait(vo);

  vo_done(vo);
  while (!vo_wait(vo));

  vo_delete(vo);
}
