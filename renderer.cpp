#pragma leco app

import jojo;
import jute;
import pipeline;
import speak;
import silog;
import vee;
import vo;
import voo;

static voo::device_and_queue dq { "panine-render" };
static vo v {};
static int vframes {};

static void run_speech(jute::view bg, jute::view script) {
  constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;
  voo::offscreen::buffers fb { dq.physical_device(), vo::extent, format };
  pipeline ppl { bg, &dq, fb.render_pass(), false };
 
  voo::single_cb cb { dq.queue_family() };
  vee::render_pass_begin rpb = fb.render_pass_begin({});
 
  auto spk = spk::run(script);
  v.write_audio(spk.buffer.begin(), spk.buffer.size());
 
  int frame = 0;
  for (auto & w : spk.words) {
    silog::trace("generate", w.text);
    auto count = w.offset * 30 / 22050;
    for (; frame < count; frame++) {
      {
        voo::cmd_buf_one_time_submit ots { cb.cb() };
        ppl.run(cb.cb(), rpb, *w.text);
        fb.cmd_copy_to_host(cb.cb());
      }
      dq.queue()->queue_submit({
        .command_buffer = cb.cb(),
      });
      
      vee::device_wait_idle();

      {
        auto mm = fb.map_host();
        auto in = static_cast<vo::pix *>(*mm);
        auto out = v.lock();
        for (auto i = 0; i < vo::extent.width * vo::extent.height; i++) {
          *out = {{ in->p[3], in->p[0], in->p[1], in->p[2] }};
          out++;
          in++;
        }
        v.unlock(vframes++);
      }

      ppl.next_frame();
      vee::device_wait_idle();
    }
  }
}

int main() {
  //run_speech(jojo::read_cstr("out/script.txt"));
  run_speech("out/IMG_2450.MOV", "Five reasons to test this.");
  run_speech("out/IMG_2451.MOV", "First, I love it.");
  run_speech("out/IMG_2450.MOV", "Second, I really love it.");
  run_speech("out/IMG_2451.MOV", "Third, who would not love it?");
 
  float time = vframes / 30.0;
  silog::log(silog::info, "Total frames in output: %d (%3.2fs)", vframes, time);

  v.done();
  while (!v.wait());
}
