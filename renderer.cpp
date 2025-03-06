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

static constexpr const auto format = VK_FORMAT_R8G8B8A8_SRGB;
static voo::offscreen::buffers fb { dq.physical_device(), vo::extent, format };
static voo::single_cb cb { dq.queue_family() };
static vee::render_pass_begin rpb = fb.render_pass_begin({});

static void ots(auto fn) {
  {
    voo::cmd_buf_one_time_submit ots { cb.cb() };
    fn();
    fb.cmd_copy_to_host(cb.cb());
  }
  dq.queue()->queue_submit({
    .command_buffer = cb.cb(),
  });
  vee::device_wait_idle();

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

static void run_speech(jute::view bg, jute::view script) {
  pipeline ppl { bg, &dq, fb.render_pass(), false };
 
  auto spk = spk::run(script);
  v.write_audio(spk.buffer.begin(), spk.buffer.size());
 
  int frame = 0;
  for (auto & w : spk.words) {
    silog::trace("generate", w.text);
    auto count = w.offset * 30 / 22050;
    for (; frame < count; frame++) {
      ots([&] {
        ppl.run(cb.cb(), rpb, *w.text);
      });
      
      ppl.next_frame();
      vee::device_wait_idle();
    }
  }
}

void show_image() {
  // pipeline ppl { bg, &dq, fb.render_pass(), false };
  for (int frame = 0; frame < 60; frame++) {
    ots([&] {
    });
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
